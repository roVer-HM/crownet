import xml.etree.ElementTree as ET
import geopandas as gp
import numpy as np
import shapely
import io


def read_bound_and_cooridnate_system(sumo_net_path, grid):
    root = ET.parse(sumo_net_path)
    loc = root.find("location")
    offset = np.array(loc.attrib["netOffset"].split(",")).astype(np.float32) * -1
    bound = (
        np.array(loc.attrib["convBoundary"].split(","))
        .astype(np.float32)
        .reshape((2, 2), order="C")
    )
    proj4 = loc.attrib["projParameter"]
    num_enbs, dist = grid_by_enb_dist(800, bound, override=np.array([5, 4]))
    enb_df, misc_df = create_enb_position(num_enbs, dist, offset, proj4)
    create_enb_config(
        enb_df,
        num_enbs,
        f"/home/vm-sts/repos/crownet/crownet/simulations/multi_enb/enb_{num_enbs[0]}x{num_enbs[1]}.ini",
    )
    create_misc_config(
        misc_df,
        f"/home/vm-sts/repos/crownet/crownet/simulations/multi_enb/misc_{num_enbs[0]}x{num_enbs[1]}.ini",
    )
    single_bonnmotion(
        bound,
        dist,
        f"/home/vm-sts/repos/crownet/crownet/simulations/multi_enb/pnode_{num_enbs[0]}x{num_enbs[1]}.bonnmotion",
    )
    print("hi")


def single_bonnmotion(bound, dist, output_path):
    _to_canvas = np.array([0, bound[1][1]])
    speed = 3.5  # m/s
    start = dist * 1.5
    end = bound[1] - start
    start[1] = _to_canvas[1] - start[1]
    end[1] = _to_canvas[1] - end[1]
    d = np.linalg.norm(end - start)
    time = d / speed  # s
    with open(output_path, "w", encoding="utf-8") as fd:
        fd.writelines([f"0.0 {start[0]} {start[1]} {time} {end[0]} {end[1]}"])


def grid_by_enb_dist(enb_dist, bound, override=None):
    abs_bound = bound[0] + bound[1]
    enb_num = abs_bound / (2 * enb_dist)
    print(f"To cover bound {abs_bound} {enb_num}  enb's needed.")
    if override is None:
        print("round up to integer value ")
        enb_num = np.ceil(enb_num).astype(int)
    else:
        print(f"override number of base stations with {override}")
        enb_num = override

    d = abs_bound / (2 * (enb_num + 1))
    print(f"using {enb_num} number of enbs. ==> distance between enbs is {d}")
    return enb_num, d
    print("hi")


def create_enb_position(num_enbs, dist, offset, crs):
    x_dist, y_dist = dist
    pos_enb = []
    pos_misc = []
    for y in range(num_enbs[1]):
        for x in range(num_enbs[0]):
            pos_enb.append([x * (2 * x_dist) + x_dist, y * (2 * y_dist) + y_dist])

    pos_enb = np.array(pos_enb) + offset
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
    return enb_df, misc_df


def create_misc_config(misc_pos, output_path):
    out = io.StringIO()
    l(out, "[Config stat_misc]")
    l(out, '*.misc[*].middleware.typename = ""')
    l(out, '*.misc[*].mobility.typename = "StationaryMobility"')
    l(out, '*.misc[*].mobility.coordinateSystemModule = "coordConverter"')
    l(out, "*.misc[*].mobility.initFromDisplayString = false")
    l(out, '*.misc[*].typename = "crownet.nodes.ApplicationLayerPedestrian"')
    l(out, f"*.numMisc = {len(misc_pos)}")
    for idx, misc in enumerate(misc_pos):
        l(out, f"*.misc[{idx}].mobility.initialLongitude = {misc.x}deg")
        l(out, f"*.misc[{idx}].mobility.initialLatitude = {misc.y}deg")

    with open(output_path, "w", encoding="utf-8") as fd:
        out.seek(0)
        fd.write(out.getvalue())


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


def create_enb_config(enb_position_list, enb_pos, out_path):
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
        neighbors = neighbor_index(idx, enb_pos)
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


if __name__ == "__main__":
    path = "/home/vm-sts/repos/crownet/crownet/simulations/multi_enb/sumo/munich/munich.net.xml"
    read_bound_and_cooridnate_system(path, grid=np.array([]))
