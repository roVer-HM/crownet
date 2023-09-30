from dataclasses import dataclass
import os
import random
from crownetutils.omnetpp.scave import CrownetSql
from crownetutils.utils.plot import enb_with_hex
import folium
from typing import Literal
import xml.etree.ElementTree as ET
import geopandas as gp
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon, Patch
from matplotlib.collections import LineCollection, PatchCollection
from shapely.geometry import Point
from shapely.ops import transform
import io
from crownetutils.utils.misc import Project
from crownetutils.utils.path import PathHelper
from crownetutils.utils.file import open_txt_gz
from crownetutils.sumo import SumoSim, SimDir
from crownetutils.sumo.bonnmotion_reader import bm_df_with_dist_and_speed

EnbType = Literal["grid", "hex"]


def read_bound_and_coordinate_system(
    sim_root: PathHelper,
    sumo_net_path: str,
    enb_x: int = 5,
    enb_y: int = 3,
    dist: float = 1600.0,
    suffix="_",
    enb_type: EnbType = "grid",
):
    with open_txt_gz(sumo_net_path) as fd:
        root = ET.parse(fd)
    loc = root.find("location")
    offset = np.array(loc.attrib["netOffset"].split(",")).astype(np.float32) * -1
    bound = (
        np.array(loc.attrib["convBoundary"].split(","))
        .astype(np.float32)
        .reshape((2, 2), order="C")
    )
    proj4 = loc.attrib["projParameter"]

    num_enbs = [enb_x, enb_y]
    if enb_type == "grid":
        enb_df, misc_df, n_index = create_enb_position(
            num_enbs, [dist / 2, dist / 2], offset, bound, proj4
        )
        suffix = f"_grid{suffix}"
    else:
        enb_df, misc_df, n_index = hex_grid(
            XN=5, YN=3, enb_dist=1300, bound=bound, offset=offset, crs=proj4
        )
        suffix = f"_hex{suffix}"

    with open(
        sim_root.join(f"enb_position{suffix}.csv"), mode="w", encoding="utf-8"
    ) as fd:
        fd.write(f"#{proj4}\n")
        enb_df.to_csv(fd, sep=",")
    with open(
        sim_root.join(f"misc_position{suffix}.csv"), mode="w", encoding="utf-8"
    ) as fd:
        fd.write(f"#{proj4}\n")
        misc_df.to_csv(fd, sep=",")

    create_enb_config(
        enb_df.geometry,
        num_enbs,
        sim_root.join(f"enb{suffix}{num_enbs[0]}x{num_enbs[1]}.ini"),
        n_index=n_index,
    )
    create_misc_config(
        misc_df.geometry,
        sim_root.join(f"misc{suffix}{num_enbs[0]}x{num_enbs[1]}.ini"),
    )
    # single_bonnmotion(
    #     bound,
    #     dist,
    #     sim_root.join("traces", f"pnode{suffix}{num_enbs[0]}x{num_enbs[1]}.bonnmotion"),
    # )


def single_bonnmotion(bound, dist, output_path):
    abs_bound = bound[1] - bound[0]

    lines = []
    speed = 3.5  # m/s

    t = np.array([2, 2, 50, 50, 100, 100, 150, 150, 200, 200])
    for i in range(10):
        # round trip path
        path = [
            [400.0, 400.0],
            [4800.0, 4000.0],
            [4800.0, 2800.0],
            [400.0, 2800.0],
            [400.0, 400.0],
        ]
        path = np.array(path)
        # path += bound[0]
        # change direction for half of all traces
        if i % 2 == 0:
            path = np.flip(path, axis=0)

        # add random noise to coordinates
        rnd = np.array([np.random.randint(-20, 20) for _ in range(path.size)]).reshape(
            (-1, 2)
        )
        path += rnd

        # get cumulative time point for each coordinate + random time offset
        time_offset = t[i] + np.random.randint(0, 10)
        dist = np.linalg.norm(path - np.roll(path, 1, axis=0), axis=1)
        time = np.cumsum(dist / speed) + time_offset

        # write bonnmotionline for i'th trace
        s = io.StringIO()
        for j in range(path.shape[0]):
            s.write(f"{time[j]} {path[j][0]} {path[j][1]} ")
        s.seek(0)
        lines.append(s.getvalue())
        print(f"{i}'th trace: {s.getvalue()}")
        s.close()

    # write out bonnmotion file
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as fd:
        for l in lines:
            fd.write(l)
            fd.write(os.linesep)


def grid_by_enb_dist(enb_dist, bound, override=None):
    abs_bound = bound[1] - bound[0]
    enb_num = abs_bound / (2 * enb_dist)
    print(f"To cover bound {abs_bound} {enb_num}  enb's needed.")
    if override is None:
        print("round up to integer value ")
        enb_num = np.ceil(enb_num).astype(int)
    else:
        print(f"override number of base stations with {override}")
        enb_num = override

    d = abs_bound / (2 * (enb_num))
    print(f"using {enb_num} number of enbs. ==> distance between enbs is {d}")
    return enb_num, d


def create_enb_position(num_enbs, dist, offset, bound, crs):
    x_dist, y_dist = dist
    pos_enb = []
    pos_misc = []
    for y in range(num_enbs[1]):
        for x in range(num_enbs[0]):
            pos_enb.append([x * (2 * x_dist) + x_dist, y * (2 * y_dist) + y_dist])

    pos_enb = np.array(pos_enb) + offset + bound[0]
    pos_misc = pos_enb + 10

    enb_df = gp.GeoDataFrame(
        geometry=gp.points_from_xy(pos_enb[:, 0], pos_enb[:, 1]), crs=crs
    )
    enb_df = enb_df.to_crs(
        "EPSG:4326"
    ).geometry  # WGS84 (same as +proj=longlat +datum=WGS84 +no_defs +type=crs)
    misc_df = gp.GeoDataFrame(
        geometry=gp.points_from_xy(pos_misc[:, 0], pos_misc[:, 1]), crs=crs
    )
    misc_df = misc_df.to_crs(
        "EPSG:4326"
    ).geometry  # WGS84 (same as +proj=longlat +datum=WGS84 +no_defs +type=crs)

    enb_count = enb_df.shape[0]
    n_index = [neighbor_index(i, enb_df.shape) for i in range(enb_count)]

    return enb_df, misc_df, n_index


def create_misc_config(misc_pos, output_path):
    out = io.StringIO()
    l(out, "[Config misc_static_position]")
    l(out, f"*.numMisc = {len(misc_pos)}")
    for idx, misc in enumerate(misc_pos):
        l(out, f"*.misc[{idx}].mobility.initialLongitude = {misc.x}deg")
        l(out, f"*.misc[{idx}].mobility.initialLatitude = {misc.y}deg")

    with open(output_path, "w", encoding="utf-8") as fd:
        out.seek(0)
        fd.write(out.getvalue())


def neighbor_index_hex(i, X, Y):
    _x = i % X
    _y = (i - i % X) / X
    neighbors = []
    even_col = _x % 2 == 0
    ueven_col = _x % 2 != 0

    # top_right
    if even_col and (_x + 1) < X and (_y + 1) < Y:
        neighbors.append(i + X + 1)
    elif ueven_col and (_x + 1):
        neighbors.append(i + 1)

    # top
    if _y + 1 < Y:
        neighbors.append(i + X)

    # top_left
    if even_col and (_x - 1) >= 0 and (_y + 1) < Y:
        neighbors.append(i + X - 1)
    elif ueven_col and (_x - 1) >= 0:
        neighbors.append(i - 1)

    # bottom_left
    if even_col and (_x - 1) >= 0:
        neighbors.append(i - 1)
    elif ueven_col and (_x - 1) >= 0 and (_y - 1) >= 0:
        neighbors.append(i - X - 1)

    # bottom
    if _y - 1 >= 0:
        neighbors.append(i - X)

    # bottom_right
    if even_col and (_x + 1) < X:
        neighbors.append(i + 1)
    elif ueven_col and (_x + 1) < X and (_y - 1) >= 0:
        neighbors.append(i - X + 1)

    return neighbors


def neighbor_index(N, enb_shape):
    max_id = enb_shape[0] * enb_shape[1]
    X = enb_shape[0]
    Y = enb_shape[1]
    _x = N % X
    _y = (N - N % X) / X
    neighbors = []

    # left/right
    if (_x - 1) >= 0:
        neighbors.append(N - 1)
    if (_x + 1) < X:
        neighbors.append(N + 1)
    # up/down
    if (_y - 1) >= 0:
        neighbors.append(N - X)
    if (_y + 1) < Y:
        neighbors.append(N + X)
    # left-up:
    if (_x - 1) >= 0 and (_y + 1) < Y:
        neighbors.append(N + X - 1)
    # left-down:
    if (_x - 1) >= 0 and (_y - 1) >= 0:
        neighbors.append(N - X - 1)
    # right-up:
    if (_x + 1) < X and (_y + 1) < Y:
        neighbors.append(N + X + 1)
    # right-down:
    if (_x + 1) < X and (_y - 1) >= 0:
        neighbors.append(N - X + 1)
    return neighbors


def create_links(all: set, enb, connected_to):
    for i in connected_to:
        if i < enb:
            all.add((i, enb))
        else:
            all.add((enb, i))

    return all


def create_enb_config(enb_position_list, enb_pos, out_path, n_index):
    links = set()
    out = io.StringIO()
    l(out, f"[Config enb_{enb_pos[0]}x{enb_pos[1]}]")
    l(out, f"extends = _enb_default")
    l(out, "")
    l(out, f"*.numEnb = {len(enb_position_list)}")
    l(
        out,
        "*.eNB[*].x2App[*].server.localPort = 5000 + ancestorIndex(1) # Server ports (x2App[0]=5000, x2App[1]=5001, ...)",
    )
    # l(out, f"*.eNB[*].numX2Apps = {len(enb_position_list)-1}")
    for idx, enb in enumerate(enb_position_list):
        neighbors = n_index[idx]
        links = create_links(links, idx, neighbors)
        l(out, f"# eNB {idx}")
        l(out, f"*.eNB[{idx}].numX2Apps = {len(neighbors)}")
        l(out, f"*.eNB[{idx}].mobility.initialLongitude = {enb.x}deg")
        l(out, f"*.eNB[{idx}].mobility.initialLatitude = {enb.y}deg")
        _idx = 0
        # for _enb_idx in range(len(enb_position_list)):
        for _enb_idx in neighbors:
            if idx == _enb_idx:
                continue
            else:
                l(
                    out,
                    f'*.eNB[{idx}].x2App[{_idx}].client.connectAddress = "eNB[{_enb_idx}]%x2ppp0"',
                )
                _idx += 1
        l(out, "")
    print("done")
    out.seek(0)
    with open(out_path, "w", encoding="utf-8") as fd:
        fd.write(out.getvalue())

    link_str = io.StringIO()
    print(f"write to {out_path}")
    links = list(links)
    links.sort()
    for link in list(links):
        l(link_str, f"eNB[{link[0]}].x2++ <--> Eth10G <--> eNB[{link[1]}].x2++;")
    link_str.seek(0)
    print(link_str.getvalue())


def l(iostr, line):
    iostr.write(line)
    iostr.write("\n")


def hex_grid(XN, YN, enb_dist: float, offset, bound, crs):
    def hex_patch(p, ri):
        ru = ri / (np.sqrt(3) / 2)
        xy = []
        for i in range(6):
            xy.append([ru * np.cos(i * np.pi / 3), ru * np.sin(i * np.pi / 3)])
        xy = np.array(xy)
        xy = xy + p
        print(xy)
        p = Polygon(xy, closed=True)
        return p

    ri = enb_dist / 2  # in-circle radius
    ru = ri / (np.sqrt(3) / 2)  # outter-circle radius
    enb_pos = []
    for y in range(YN):
        enb_pos.append([0, y * 2 * ri])
    enb_pos = np.array(enb_pos)
    x_cols = []
    for x in range(XN):
        if x % 2 == 0:
            _y = 0
        else:
            _y = -ri
        a = enb_pos + np.array([x * 3 / 2 * ru, _y])
        x_cols.append(a)

    enb_pos = np.array(x_cols).reshape((-1, 2))
    # first x than y (same order as regular grid), needed for neighbor index creation
    enb_pos = enb_pos[np.arange(15).reshape((5, 3)).T.flatten()]

    neighbor_index = [
        neighbor_index_hex(i, X=XN, Y=YN) for i in range(enb_pos.shape[0])
    ]
    for i, n in enumerate(neighbor_index):
        print(f"{i} -> {n}")

    # translate by ru/2
    enb_pos = enb_pos + np.array([0.75 * ru, ru])

    enb_cart = enb_pos + offset + bound[0]
    misc_offset = np.array([20, 0])
    misc_cart = enb_cart + misc_offset  # misc node 20 meter offset in x
    enb_df = gp.GeoDataFrame(
        geometry=gp.points_from_xy(enb_cart[:, 0], enb_cart[:, 1], crs=crs)
    )
    enb_df = enb_df.to_crs("EPSG:4326")  # lat/lon
    enb_df[["base_x", "base_y"]] = enb_pos
    enb_df[["utm_x", "utm_y"]] = enb_cart
    enb_df["lon"] = enb_df.geometry.x
    enb_df["lat"] = enb_df.geometry.y

    misc_df = gp.GeoDataFrame(
        geometry=gp.points_from_xy(misc_cart[:, 0], misc_cart[:, 1], crs=crs)
    )
    misc_df = misc_df.to_crs("EPSG:4326")  # lat/lon
    misc_df[["base_x", "base_y"]] = enb_pos + misc_offset
    misc_df[["utm_x", "utm_y"]] = misc_cart
    misc_df["lon"] = misc_df.geometry.x
    misc_df["lat"] = misc_df.geometry.y

    # patches = PatchCollection([hex_patch(enb_pos[i], ri) for i in range(enb_pos.shape[0])], edgecolors="black", facecolors="none")

    # plt.scatter(enb_pos[:,0], enb_pos[:,1], marker="o", color="black")
    # plt.gca().add_collection(patches)
    # plt.gca().set_aspect("equal", "box")
    # plt.show()
    # print("hi")
    return enb_df, misc_df, neighbor_index


if __name__ == "__main__":
    root_dir = PathHelper("/home/vm-sts/repos/crownet/crownet/simulations/multi_enb/")
    net_file = root_dir.join("sumo/munich/muc_cleaned/muc.net.xml.gz")
    read_bound_and_coordinate_system(
        sim_root=root_dir, sumo_net_path=net_file, suffix="_muc_clean", enb_type="hex"
    )

    print("hi")
