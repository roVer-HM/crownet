import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys
import os
import re
from typing import List


from oppanalyzer.utils import ScaveTool


@pd.api.extensions.register_dataframe_accessor("opp")
class OppAccessor:
    """
    see https://pandas.pydata.org/pandas-docs/stable/development/extending.html
    opp namespace to access attribute information for scalar, vector and histogram types
    and helpers to access  data by module or name.

    todo: Add helper functions for plot creation and title/axis labels based on result attributes.

    """
    def __init__(self, pandas_obj):
        print("init")
        self._validate(pandas_obj)
        self._obj: pd.DataFrame = pandas_obj

        self._opp_attr_dict = {}

        # subset of attribute information
        run_cnf = self._obj.loc[(self._obj.type == 'runattr') | (self._obj.type == 'param'),
                                ['type', 'attrname', 'attrvalue']]

        self._obj.drop(self._obj.loc[(self._obj.type == 'runattr')].index, inplace=True)
        self._obj.drop(self._obj.loc[(self._obj.type == 'param')].index, inplace=True)

        attr = self._obj.loc[self._obj.type == 'attr', ['module', 'name', 'attrname', 'attrvalue']]
        attr['attr_key'] = attr.module + '.' + attr.name

        # drop rows with attribute information
        self._obj.drop(self._obj.loc[self._obj.type == 'attr'].index, inplace=True)

        # add run_id column
        self._obj['run_id'] = self._obj['run']
        runs = self._obj['run_id'].unique()
        runs.sort()
        self._obj['run_id'] = self._obj['run_id'].apply(lambda x: np.where(runs == x)[0][0])

        for _, row in run_cnf.iterrows():
            item = self._opp_attr_dict.get(row['type'], {})
            item.setdefault(row['attrname'], row['attrvalue'])
            self._opp_attr_dict.setdefault(row['type'], item)

        for _, row in attr.iterrows():
            item = self._opp_attr_dict.get(row['attr_key'], {})
            item.setdefault(row['attrname'], row['attrvalue'])
            item.setdefault('module', row['module'])
            item.setdefault('name', row['name'])
            self._opp_attr_dict.setdefault(row['attr_key'], item)

    @staticmethod
    def _validate(obj: pd.DataFrame):
        cols = ['run', 'type', 'module', 'name', 'attrname', 'attrvalue']
        for c in cols:
            if c not in obj.columns:
                raise AttributeError(f"Must have '{c}' column. ")

    def set_attr(self, key, value):
        self._opp_attr_dict.setdefault(key, value)

    def get_attr(self, lable_idx):
        try:
            attr_key = self._obj.loc[lable_idx]['module'] + '.' + self._obj.loc[lable_idx]['name']
            return self._opp_attr_dict.get(attr_key, {})
        except:
            return {}

    def get_attr_by_key(self, attr_key):
        return self._opp_attr_dict.get(attr_key, {})

    def attr_for_series(self, s: pd.Series):
        return self.get_attr(s.name)

    def module_summary(self):
        ret = self._obj[self._obj.type.isin(['scalar', 'histogram', 'vector'])].module.unique()
        ret.sort()
        return ret

    def name_summary(self):
        ret = self._obj[self._obj.type.isin(['scalar', 'histogram', 'vector'])].name.unique()
        ret.sort()
        return ret

    def by_name(self, name) -> pd.DataFrame:
        return self._obj[self._obj['name'] == name]

    def by_module(self, module) -> pd.DataFrame:
        return self._obj[self._obj['module'] == module]

    def by_name_module(self, name, module) -> pd.DataFrame:
        return self._obj[(self._obj['module'] == module) & (self._obj['name'] == name)]

    def set_labels(self, ax: plt.axes, s: pd.Series, xlabel='time [s]'):
        attr = self.attr_for_series(s)
        ax.set_xlabel(xlabel)
        ax.set_ylabel(f"[{attr.get('unit', '???')}]")
        ax.set_title(f"{attr.get('title', '???')}")


"""
set_xlabel
set_xlimit(left, right)
set_xscale() [linear, log, symlog, logit]
set_xlimit
set_xmargin

set_xticks
set_xticklabels

"""



default_plot_args={
    'linewidth': 0,
    'markersize': 3,
    'marker': '.',
    'color': 'b'
}

plot_marker = ['.', '*', 'o', 'v', '1', '2', '3', '4']
plot_color = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'w']


def marker(idx):
    return plot_marker[idx%len(plot_marker)]


def color(idx):
    return plot_color[idx%len(plot_color)]


def plt_args(idx=0, *args, **kwargs):
    ret = dict(default_plot_args)
    if 'label' not in kwargs.keys():
        ret.setdefault('label', f"{idx}-data")

    ret.update({
        'marker': marker(idx),
        'color': color(idx),
    })
    if kwargs:
        ret.update(kwargs)
    return ret


def create_label(lbl_str: str, remove_filter: List[str] = None):

    if remove_filter is not None:
        for rpl in remove_filter:
            lbl_str = lbl_str.replace(rpl, '')

    return lbl_str


def load_csv(csv_file):
    df = pd.read_csv(csv_file, converters=ScaveTool.converters())
    df.opp.set_attr('scave_filter', '')
    df.opp.set_attr('override_existing_csv', False)
    return df


def load_df(result_file_path, csv_suffix='', override_existing_csv=False, scave_filter=None):
    """
    Create DataFrame
    :param result_file_path:        use *.vec or *.sca file
    :param csv_suffix:              add a suffix to output files.
                                      i.e. (no suffix) foo.vec --> foo_vec.csv
                                      i.e. (suffix='_bar42') foo.vec --> foo_vec_bar42.csv
    :param override_existing_csv:   if False use existing csv file derived from result_file_path and csv_suffix
    :param scave_filter:            only export data from result_file_path which matches with this filter.
                                    See scavetool help filters on how to create filter expresion.
                                    (Shell escapes will be added by this wrapper.)
    :return:                        standard pandas DataFrame with special 'opp' accessor (see OppAccessor)
                                    accessor simplifies access to attribute information for scalar, vector and
                                    histogram types.
    """
    csv_file_path = ScaveTool.create_or_get_csv_file(result_file_path,
                                                     csv_path=csv_suffix,
                                                     override=override_existing_csv,
                                                     scave_filter=scave_filter)
    df = pd.read_csv(csv_file_path, converters=ScaveTool.converters())
    df.opp.set_attr('scave_filter', scave_filter)
    df.opp.set_attr('override_existing_csv', override_existing_csv)
    return df
