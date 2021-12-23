
import pandas as pd
from typing import Union
import numpy as np

class QuantityOfInterest:

    RUN_ID = 'run_id'
    ID = 'id'
    STATIC_QOI_KEY = 'static'
    TIME_INDEX_NAME = "time"

    def __init__(self, values : Union[np.array, pd.Series, pd.DataFrame], name="qoi"):
        self.name = name
        self.set_values(values)

    def get_name(self):
        return self.name

    def get_values(self):
        return self.values

    def set_values(self, values):

        #TODO refactor

        if isinstance(values, np.ndarray):
            print(f"Assume there are {len(values)} samples, each of which is assigned a qoi (scaler, time-independent).")
            values = values.reshape((len(values),-1))
            values = pd.DataFrame(values,
                                  index=pd.MultiIndex.from_product([np.arange(len(values)), [0]], names=self.get_multi_index()))

        if isinstance(values, pd.Series):
            values = pd.DataFrame(values)


        if isinstance(values, pd.DataFrame):
            if values.shape[-1] != 1:
                raise ValueError(f"Only one quantity of interest allowed. Got {values.columns}")

            if self.name is not None:
                values.rename(columns={values.columns[0]: self.name}, inplace = True)

            if any(elem in self.get_multi_index() for elem in values.index.names):
                time_index = values.index.names.difference(self.get_multi_index())
                if len(time_index)==0:
                    print(f"Assume the qoi {self.name} is independent from time.")
                    values_ = values[~values.index.duplicated(keep='first')]
                    if values.equals(values_) is False:
                       raise ValueError(f"Duplicate indices found in {values}. Please provide a time index.")
                    else:
                        old_idx = values.index.to_frame()
                        old_idx.insert(0, QuantityOfInterest.TIME_INDEX_NAME, QuantityOfInterest.STATIC_QOI_KEY)
                        values.index = pd.MultiIndex.from_frame(old_idx)

                if len(time_index)==1:
                    # rename time index level if necessary
                    ii = list(values.index.names)
                    ii[ii.index(time_index[0])] = QuantityOfInterest.TIME_INDEX_NAME
                    values.index.set_names(ii, inplace=True) # index.name.replace_name do no use-> depricated

                    if values.index.get_level_values(level="time").drop_duplicates().size == 1:
                        # time index provided, but no time dependency found, time index -> same values
                        values.index =  values.index.set_levels([QuantityOfInterest.STATIC_QOI_KEY], level='time')
                    else:
                        # round time index to 3 digits
                        values.reset_index(QuantityOfInterest.TIME_INDEX_NAME, inplace=True)
                        values[QuantityOfInterest.TIME_INDEX_NAME] = values[QuantityOfInterest.TIME_INDEX_NAME].round(3)
                        values.set_index(QuantityOfInterest.TIME_INDEX_NAME, append=True, inplace=True)

                        # fill with nan values, if there are missing sample values over time
                        # time 1: sample_1_val,  sample_2_val, sample_3_val
                        # time 2: sample_1_val,     nan      , sample_3_val
                        values = values.unstack(QuantityOfInterest.TIME_INDEX_NAME).stack(level=1, dropna=False)
            else:
                raise ValueError(f"Multiindex must contain levels {self.get_multi_index()}. Got: {values.index}")

        values = values.reorder_levels([QuantityOfInterest.TIME_INDEX_NAME, QuantityOfInterest.RUN_ID, QuantityOfInterest.ID])
        values.sort_index(level=QuantityOfInterest.TIME_INDEX_NAME, inplace=True)
        self.values = values

    def get_multi_index(self):
        return [QuantityOfInterest.RUN_ID, QuantityOfInterest.ID]

    def is_static(self):
        return all(self.values.index.get_level_values(0) == QuantityOfInterest.STATIC_QOI_KEY)

    @classmethod
    def from_suqc_output(cls, file_path, name= "QoI", col_name=None ):
        # file_path: file from suqc
        df = pd.read_csv(file_path, index_col=[0, 1, 2])

        nr_cols = len(df.columns)
        if nr_cols !=1:
            if col_name is None:
                raise ValueError(f"Dataframe got more than 1 columns. Specify col_name.")
            else:
                df = df[col_name]
        return QuantityOfInterest(values=df, name=name)
