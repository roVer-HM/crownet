import re
from typing import List

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


class OppFilterItem:
    """
    Filter item applicable to OMNeT++ based data frame. #name corresponds to column of df.
    """

    def __init__(self, name, value, regex):
        self.name = name
        self.value = value
        self.regex = regex

    def __str__(self):
        return self.value

    def __repr__(self) -> str:
        return (
            f"FilterItem(name: {self.name}, value: {self.value}, regex: {self.regex})"
        )


class OppFilter:
    """
    Builder pattern for OMNeT based data frames. This allows simplified selection of
    rows of the dataframe. Most columns of the data frame has two methods.
    <column_name> and <column_name>_regex. The former accepts only perfect matches
    whereas the later accepts a regex matter. If allow_number_range=True of the
    regex method is True number ranges such as '.*foo\[3..5\]\.bar\.baz' will be accepted
    and replaced with '.*foo\[\[3|4|5]]\.bar\.baz'.
    Note: All special characters must be escaped if literal brackets or dots should be matched.
    """

    def __init__(self, df: pd.DataFrame = None):
        self._filter_dict = {}
        self._name = None
        self._name_regex = None
        self._type = None
        self._module = None
        self._module_regex = None
        self._number_range_pattern = re.compile(
            "(?P<n_range>(?P<start>\d+)\.\.(?P<end>\d+))"
        )
        self._df = df

    def _apply_number_range(self, val):
        matches = re.findall(self._number_range_pattern, val)
        for match in matches:
            old_range = match[0]
            start = int(match[1])
            end = int(match[2])
            if start < end:
                range_new = (
                    "[" + "|".join([str(i) for i in range(start, end + 1)]) + "]"
                )
                val = val.replace(old_range, range_new)
            else:
                raise ValueError(f"number range in regex {val} invalid")
        return val

    def _add_filter(self, f: OppFilterItem):
        self._filter_dict[f.name] = f

    def name(self, name):
        self._add_filter(OppFilterItem("name", name, False))
        return self

    def name_regex(self, name: str, allow_number_range=True):
        if allow_number_range:
            name = self._apply_number_range(name)

        self._add_filter(OppFilterItem("name", name, True))
        return self

    def module(self, module):
        self._add_filter(OppFilterItem("module", module, False))
        return self

    def module_regex(self, module: str, allow_number_range=True):
        if allow_number_range:
            module = self._apply_number_range(module)

        self._add_filter(OppFilterItem("module", module, True))
        return self

    def scalar(self):
        self._add_filter(OppFilterItem("type", "scalar", False))
        return self

    def vector(self):
        self._add_filter(OppFilterItem("type", "vector", False))
        return self

    def apply(self, df: pd.DataFrame = None):

        if df is not None:
            self._df = df

        if self._df is None:
            raise ValueError("no Dataframe set")

        bool_filter = pd.Series(
            [True for i in range(0, self._df.shape[0])], self._df.index
        )
        for key, filter_item in self._filter_dict.items():
            if filter_item.regex:
                bool_filter = bool_filter & self._df[key].str.match(filter_item.value)
            else:
                bool_filter = bool_filter & (self._df[key] == filter_item.value)
        return self._df[bool_filter]

    def __str__(self) -> str:
        return self._filter_dict.__str__()

    def __repr__(self) -> str:
        ret = "Filter: "
        for key, value in self._filter_dict.items():
            ret += f"\n   {value.__repr__()}"
        return ret


class OppAttributes:
    """
    OMNeT++ Attributes for vectors, scalars and histograms
    """

    def __init__(self, attribute_dict, obj):
        self._opp_attr_dict = attribute_dict
        self._obj = obj

    def set(self, key, value):
        self._opp_attr_dict.setdefault(key, value)

    def run(self, run_id):
        return self._opp_attr_dict.get(run_id, {})

    def get(self, df_row_label):
        """
        :param df_row_label: row label (loc) of data frame.
        :return: attribute dictionary for given row label or empty dictionary if not found.
        """
        try:
            full_data_path = (
                self._obj.loc[df_row_label]["module"]
                + "."
                + self._obj.loc[df_row_label]["name"]
            )
            return self.run(self._obj.loc[df_row_label]["run"]).get(full_data_path, {})
        except:
            return {}

    def attr_for_series(self, s: pd.Series):
        return self.get(s.name)

    def unit(self, df_row_label):
        return self.get(df_row_label).get("unit", "???")

    def title(self, df_row_label):
        return self.get(df_row_label).get("title", "???")


class OppPlot:
    """
    Plot helper functions.

    set_xlabel
    set_xlimit(left, right)
    set_xscale() [linear, log, symlog, logit]
    set_xlimit
    set_xmargin

    set_xticks
    set_xticklabels

    """

    default_plot_args = {"linewidth": 0, "markersize": 3, "marker": ".", "color": "b"}

    plot_marker = [".", "*", "o", "v", "1", "2", "3", "4"]
    plot_color = ["b", "g", "r", "c", "m", "y", "k", "w"]

    @classmethod
    def marker(cls, idx):
        return cls.plot_marker[idx % len(cls.plot_marker)]

    @classmethod
    def color(cls, idx):
        return cls.plot_color[idx % len(cls.plot_color)]

    @classmethod
    def plt_args(cls, idx=0, *args, **kwargs):
        ret = dict(cls.default_plot_args)
        if "label" not in kwargs.keys():
            ret.setdefault("label", f"{idx}-data")

        ret.update(
            {"marker": cls.marker(idx), "color": cls.color(idx),}
        )
        if kwargs:
            ret.update(kwargs)
        return ret

    @classmethod
    def create_label(cls, lbl_str: str, remove_filter: List[str] = None):

        if remove_filter is not None:
            for rpl in remove_filter:
                lbl_str = lbl_str.replace(rpl, "")

        return lbl_str

    def __init__(self, accessor):
        self._opp = accessor

    def _set_labels(self, ax: plt.axes, s: pd.Series, xlabel="time [s]"):
        ax.set_xlabel(xlabel)
        ax.set_ylabel(f"[{self._opp.attr.unit(s.name)}]")
        ax.set_title(self._opp.attr.title(s.name))

    def create_time_series(self, ax: plt.axes, s: pd.Series, *args, **kwargs):
        self._set_labels(ax, s)
        if "label" not in kwargs:
            kwargs.setdefault("label", self.create_label(s.module, []))
        ax.plot(s.vectime, s.vecvalue, **self.plt_args(idx=0, **kwargs))

    def create_histogram(self, ax: plt.axes, s: pd.Series, bins=40):
        ax.hist(
            s.vecvalue, bins, density=True,
        )
        ax.set_xlabel(f"[{self._opp.attr.unit(s.name)}]")


@pd.api.extensions.register_dataframe_accessor("opp")
class OppAccessor:
    """
    see https://pandas.pydata.org/pandas-docs/stable/development/extending.html
    opp namespace to access attribute information for scalar, vector and
    histogram types, helpers to access data via filters and helpers to create
    simple plots like time series and histogram plots.

    See classes #OppAttributes, #OppFilter, OppFilterItem and #OppPlot for more detail.
    """

    def __init__(self, pandas_obj):
        self._validate(pandas_obj)
        self._obj: pd.DataFrame = pandas_obj
        self.plot = OppPlot(self)

        # subset of attribute information
        run_cnf = self._obj.loc[
            (self._obj.type == "runattr")
            | (self._obj.type == "param")
            | (self._obj.type == "itervar"),
            ["run", "type", "attrname", "attrvalue"],
        ]

        self._obj.drop(self._obj.loc[(self._obj.type == "runattr")].index, inplace=True)
        self._obj.drop(self._obj.loc[(self._obj.type == "param")].index, inplace=True)
        self._obj.drop(self._obj.loc[(self._obj.type == "itervar")].index, inplace=True)

        attr = self._obj.loc[
            self._obj.type == "attr", ["run", "module", "name", "attrname", "attrvalue"]
        ]
        attr["full_data_path"] = attr.module + "." + attr.name

        # drop rows with attribute information
        self._obj.drop(self._obj.loc[self._obj.type == "attr"].index, inplace=True)

        # add run_id column
        self._obj["run_id"] = self._obj["run"]
        runs = self._obj["run_id"].unique()
        runs.sort()
        self._obj["run_id"] = self._obj["run_id"].apply(
            lambda x: np.where(runs == x)[0][0]
        )

        _opp_attr_dict = {}
        for _, row in run_cnf.iterrows():
            # get dictionary for run or create new one
            run_dict = _opp_attr_dict.get(row["run"], {})

            # write item
            item = run_dict.get(row["type"], {})
            item.setdefault(row["attrname"], row["attrvalue"])

            # write back
            run_dict.setdefault(row["type"], item)
            _opp_attr_dict.setdefault(row["run"], run_dict)

        for _, row in attr.iterrows():
            # get dictionary for run or create new one
            run_dict = _opp_attr_dict.get(row["run"], {})

            # write item
            item = run_dict.get(row["full_data_path"], {})
            item.setdefault(row["attrname"], row["attrvalue"])
            item.setdefault("module", row["module"])
            item.setdefault("name", row["name"])

            # write back
            run_dict.setdefault(row["full_data_path"], item)
            _opp_attr_dict.setdefault(row["run"], run_dict)

        # run->runattr->{all run attributes for run}
        # run->full_data_path->{all attributes for full_module_path in run}
        # full_data_path is the combination of "<module>.<name>" columns were name is the name of the data point.
        self.attr: OppAttributes = OppAttributes(_opp_attr_dict, self._obj)
        print("OppAccessor initialized")

    @staticmethod
    def _validate(obj: pd.DataFrame):
        cols = ["run", "type", "module", "name", "attrname", "attrvalue"]
        for c in cols:
            if c not in obj.columns:
                raise AttributeError(f"Must have '{c}' column. ")

    def module_summary(self):
        ret = self._obj[
            self._obj.type.isin(["scalar", "histogram", "vector"])
        ].module.unique()
        ret.sort()
        return ret

    def name_summary(self):
        ret = self._obj[
            self._obj.type.isin(["scalar", "histogram", "vector"])
        ].name.unique()
        ret.sort()
        return ret

    def filter(self, f: OppFilter = None):
        if f is not None:
            return f.apply(self._obj)
        else:
            return OppFilter(self._obj)
