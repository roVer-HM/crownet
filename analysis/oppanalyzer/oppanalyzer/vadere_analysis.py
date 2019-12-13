#%%

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys
import os


# def parse_if_number(s):
#     try: return float(s)
#     except: return True if s=="true" else False if s=="false" else s if s else None
#
#
# def parse_ndarray(s):
#     return np.fromstring(s, sep=' ') if s else None
#
#
# sca = pd.read_csv('test-data/mf_D2D_1to2_Vadere_00/sca.csv',
#                   # converters={
#                   #     'attrvalues': parse_if_number,
#                   #     'binedges': parse_ndarray,
#                   #     'binvalues': parse_ndarray,
#                   #     'vectime': parse_ndarray,
#                   #     'vecvalue': parse_if_number
#                   # }
#                   )