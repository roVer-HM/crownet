# To add a new cell, type '# %%'
# To add a new markdown cell, type '# %% [markdown]'
# %%
from re import I
import roveranalyzer as r 
import geopandas as g
import pandas as pd
import contextily  as ctx
from shapely.geometry import Polygon, box, Point
from matplotlib import pyplot as plt
import numpy as np
import math
import random

WGS84_lat_lon = 4326
WGS84_pseudo_m = 3857 

def create_grid(minx, miny, maxx, maxy, x_size=10, y_size=10):
    """
    Create a regular grid with given size in the given bound
    """
    num_x = math.floor((maxx - minx) / x_size)
    num_y = math.floor((maxy - miny) / y_size)
    cells = []
    _id = 0
    for x in range(num_x):
        x_min = minx + x_size * x 
        for y in range(num_y):
            y_min = miny + y_size * y 
            cells.append([_id, 'none', '#99382C',  box(x_min, y_min, x_min + x_size, y_min + y_size)])
            _id += 1
    
    return cells

def to_opp_coord(x, y, bound):
    return x, bound[3] -y

def export_cart_opp(peds: g.GeoDataFrame, bound, node='misc'):
    lines = []
    # *.misc[0].mobility.initialX = 5m
    # *.misc[0].mobility.initialY = 2m
    _id = 0
    for x, y in zip(peds.geometry.x, peds.geometry.y):
        opp_coord = to_opp_coord(x, y , bound)
        lines.append(f"*.{node}[{_id}].mobility.initialX = {round(opp_coord[0], 2)}m")
        lines.append(f"*.{node}[{_id}].mobility.initialY = {round(opp_coord[1], 2)}m")
        _id += 1
    
    return lines

def export_geo_opp(peds: g.GeoDataFrame, bound, node='misc'):
    lines = []
    # *.misc[0].mobility.initialX = 5m
    # *.misc[0].mobility.initialY = 2m
    _peds = peds.to_crs(epsg=WGS84_lat_lon)
    _id = 0
    for x, y in zip(_peds.geometry.x, _peds.geometry.y):
        opp_coord = to_opp_coord(x, y , bound)
        lines.append(f"*.{node}[{_id}].mobility.initialLatitude = {y}deg")
        lines.append(f"*.{node}[{_id}].mobility.initialLongitude = {x}deg")
        _id += 1
    
    return lines

class ClickHandler:
    def __init__(self, offset) -> None:
        self.offset = offset

    def on_click(self, event):
        x = round( event.xdata - self.offset[0], 2)
        y = round( event.ydata - self.offset[1], 2)
        print(f"peds.append([-1, '#187EAA', '#187EAA', Point([{x}, {y}] + offset)])")

    def on_click_geo(self, event):
        s = g.GeoSeries([Point([event.xdata, event.ydata])], crs=f"EPSG:{WGS84_pseudo_m}")
        s = s.to_crs(epsg=WGS84_lat_lon)
        print(f"peds.append([-1, '#187EAA', '#187EAA', Point([{s.loc[0].x}, {s.loc[0].y}])]")

# CRS offset to work with smaller numbers around Müncher Freiheit. 
offset = np.array([1.289e+06, 6.133e+06])
c_muc_f = np.array([780, 845]) + offset # center point of crossing at Müncher Freiheit in EPSG:3857 (cartesian)
# bounding box to work with
bound = [c_muc_f[0] - 50, c_muc_f[1] - 50, c_muc_f[0]+150, c_muc_f[1] + 200]

# create regular grid inside bound
grid_df =  create_grid(*bound)
grid_df = g.GeoDataFrame(grid_df, columns=["name", "facecolor", "edgecolor", "geometry"], crs="EPSG:3857")

# create pedestrians
peds = []
# peds.append([1, '#187EAA', '#187EAA', Point([875, 888] + offset)])
# peds.append([2, '#187EAA', '#187EAA', Point([877, 889] + offset)])
# peds.append([3, '#187EAA', '#187EAA', Point([876, 888] + offset)])
# peds.append([4, '#187EAA', '#187EAA', Point([886, 887] + offset)])
# peds.append([5, '#187EAA', '#187EAA', Point([889, 889] + offset)])
# peds.append([6, '#187EAA', '#187EAA', Point([892, 890] + offset)])
# peds.append([7, '#187EAA', '#187EAA', Point([875, 908] + offset)])
# peds.append([8, '#187EAA', '#187EAA', Point([845, 890] + offset)])
# peds.append([9, '#187EAA', '#187EAA', Point([803.63, 852.28] + offset)])
# peds.append([10, '#187EAA', '#187EAA', Point([807.36, 851.62] + offset)])
# peds.append([11, '#187EAA', '#187EAA', Point([807.14, 848.98] + offset)])
# peds.append([12, '#187EAA', '#187EAA', Point([806.26, 839.53] + offset)])
# peds.append([13, '#187EAA', '#187EAA', Point([802.75, 841.29] + offset)])
# peds.append([14, '#187EAA', '#187EAA', Point([802.31, 831.18] + offset)])
# peds.append([15, '#187EAA', '#187EAA', Point([805.38, 831.18] + offset)])
# peds.append([16, '#187EAA', '#187EAA', Point([814.4, 832.28] + offset)])
# peds.append([17, '#187EAA', '#187EAA', Point([812.42, 828.76] + offset)])
# peds.append([18, '#187EAA', '#187EAA', Point([795.93, 831.4] + offset)])
# peds.append([19, '#187EAA', '#187EAA', Point([797.47, 828.76] + offset)])
# peds.append([20, '#187EAA', '#187EAA', Point([804.73, 870.3] + offset)])
# peds.append([21, '#187EAA', '#187EAA', Point([809.56, 870.52] + offset)])
# peds.append([22, '#187EAA', '#187EAA', Point([805.16, 861.29] + offset)])
# peds.append([23, '#187EAA', '#187EAA', Point([803.19, 858.43] + offset)])
# peds.append([24, '#187EAA', '#187EAA', Point([806.92, 858.87] + offset)])
# peds.append([25, '#187EAA', '#187EAA', Point([804.73, 850.74] + offset)])
# peds.append([26, '#187EAA', '#187EAA', Point([806.04, 847.88] + offset)])
# peds.append([27, '#187EAA', '#187EAA', Point([806.92, 842.83] + offset)])
# peds.append([28, '#187EAA', '#187EAA', Point([807.36, 837.12] + offset)])
# peds.append([29, '#187EAA', '#187EAA', Point([807.36, 832.72] + offset)])
# peds.append([30, '#187EAA', '#187EAA', Point([811.54, 831.84] + offset)])
# peds.append([31, '#187EAA', '#187EAA', Point([815.93, 828.76] + offset)])
# peds.append([32, '#187EAA', '#187EAA', Point([825.38, 879.09] + offset)])
# peds.append([33, '#187EAA', '#187EAA', Point([813.74, 870.08] + offset)])
###
peds.append([1, '#187EAA', '#187EAA', Point([893.0, 1032.59] + offset)])
peds.append([2, '#187EAA', '#187EAA', Point([892.27, 1030.03] + offset)])
peds.append([3, '#187EAA', '#187EAA', Point([896.3, 1030.76] + offset)])
peds.append([4, '#187EAA', '#187EAA', Point([893.74, 1027.83] + offset)])
peds.append([5, '#187EAA', '#187EAA', Point([872.42, 927.88] + offset)])
peds.append([6, '#187EAA', '#187EAA', Point([843.19, 852.28] + offset)])
peds.append([7, '#187EAA', '#187EAA', Point([886.7, 800.63] + offset)])
peds.append([8, '#187EAA', '#187EAA', Point([876.15, 800.63] + offset)])
peds.append([9, '#187EAA', '#187EAA', Point([883.41, 802.39] + offset)])
peds.append([10, '#187EAA', '#187EAA', Point([875.71, 806.79] + offset)])
peds.append([11, '#187EAA', '#187EAA', Point([874.4, 803.93] + offset)])
peds.append([12, '#187EAA', '#187EAA', Point([871.98, 803.49] + offset)])
peds.append([13, '#187EAA', '#187EAA', Point([872.42, 806.79] + offset)])
peds.append([14, '#187EAA', '#187EAA', Point([867.14, 804.59] + offset)])
peds.append([15, '#187EAA', '#187EAA', Point([866.04, 806.79] + offset)])
peds.append([16, '#187EAA', '#187EAA', Point([863.85, 808.1] + offset)])
peds.append([17, '#187EAA', '#187EAA', Point([860.55, 806.79] + offset)])
peds.append([18, '#187EAA', '#187EAA', Point([857.47, 808.98] + offset)])
peds.append([19, '#187EAA', '#187EAA', Point([856.81, 808.1] + offset)])
peds.append([20, '#187EAA', '#187EAA', Point([854.84, 808.54] + offset)])
peds.append([21, '#187EAA', '#187EAA', Point([855.05, 810.3] + offset)])
peds.append([22, '#187EAA', '#187EAA', Point([851.54, 809.64] + offset)])
peds.append([23, '#187EAA', '#187EAA', Point([850.22, 808.1] + offset)])
peds.append([24, '#187EAA', '#187EAA', Point([848.68, 810.08] + offset)])
peds.append([25, '#187EAA', '#187EAA', Point([848.02, 809.86] + offset)])
peds.append([26, '#187EAA', '#187EAA', Point([843.41, 809.2] + offset)])
peds.append([27, '#187EAA', '#187EAA', Point([842.97, 809.86] + offset)])
peds.append([28, '#187EAA', '#187EAA', Point([742.09, 843.49] + offset)])
peds.append([29, '#187EAA', '#187EAA', Point([745.6, 843.05] + offset)])
peds.append([30, '#187EAA', '#187EAA', Point([747.14, 842.17] + offset)])
peds.append([31, '#187EAA', '#187EAA', Point([746.26, 840.63] + offset)])
peds.append([32, '#187EAA', '#187EAA', Point([745.38, 839.75] + offset)])
peds.append([33, '#187EAA', '#187EAA', Point([741.43, 839.97] + offset)])
peds.append([34, '#187EAA', '#187EAA', Point([742.53, 841.29] + offset)])
peds.append([35, '#187EAA', '#187EAA', Point([744.95, 838.21] + offset)])
peds.append([36, '#187EAA', '#187EAA', Point([746.92, 840.41] + offset)])
peds = g.GeoDataFrame(peds, columns = ["name", "facecolor", "edgecolor", "geometry"], crs="EPSG:3857")


lines = export_geo_opp(peds, bound)
[print(l) for l in lines]

ax = grid_df.plot(figsize=(16, 10), alpha=1.0,  edgecolor=grid_df.edgecolor, facecolor=grid_df.facecolor)
ctx.add_basemap(ax, zoom=18)
peds.plot(ax=ax, edgecolor=peds.edgecolor, facecolor=peds.facecolor, markersize=5)
ax.set_xlabel("Easting [meters]")
ax.set_ylabel("Northing [meters]")
ax.set_title("WSG 84 Pseudo-Mercator EPSG:3857")

# add labels 
for x, y, label in zip(peds.geometry.x, peds.geometry.y, peds.name):
    angle = random.random() * 2*math.pi

    ax.annotate(str(label), xy=(x, y), xytext=(3*np.cos(angle), 3*np.sin(angle)), textcoords="offset points")

hdl = ClickHandler(offset)
# ax.get_figure().canvas.mpl_connect('button_press_event', hdl.on_click)
ax.get_figure().canvas.mpl_connect('button_press_event', hdl.on_click_geo)

plt.show()