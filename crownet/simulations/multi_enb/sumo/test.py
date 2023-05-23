from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import random
from collections import defaultdict

# multiprocessing imports
import multiprocessing
import numpy as np


try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

if "SUMO_HOME" in os.environ:
    sys.path.append("/home/vm-sts/repos/sumo/tools/")
import sumolib  # noqa
from sumolib.miscutils import parseTime  # noqa

root = "/home/vm-sts/repos/crownet/crownet/simulations/multi_enb/sumo/munich/"


def xy_from_lane(lane, offsetPos):
    # x,y = sumolib.geomhelper.positionAtShapeOffset(_net.getLane(laneID).getShape(), lanePos)
    x, y = sumolib.geomhelper.positionAtShapeOffset(lane.getShape(), offsetPos)
    return x, y


def xy_from_edge(_net, edgeID, offset):
    """ungenau..."""
    shape = _net.getEdge(edgeID).getShape()
    return sumolib.geomhelper.positionAtShapeOffset(shape, offset)


def main():
    net = sumolib.net.readNet(os.path.join(root, "munich.net.xml"))
    output = sumolib.xml.parse(
        os.path.join(root, "trajectory.xml"), element_names="timestep"
    )
    bonn_motion = {}
    y_bound = net.getBoundary()[-1]
    dropped = set()
    for idx, time_obj in enumerate(output):
        t = time_obj.time
        print(f"{idx}/ time: {t} with {len(time_obj['edge'])} items")
        for e in time_obj["edge"]:
            edge_id = e.id
            person = e["person"][0]
            offset = float(person.pos)
            p_id = e["person"][0].id
            if edge_id.startswith(":"):
                # print(f"    drop: time {t}, edge {edge_id}, person {p_id}")
                dropped.add(p_id)
                continue
            edge = net.getEdge(edge_id)
            lanes = [l for l in edge.getLanes() if l.allows("pedestrianx")]
            if len(lanes) > 0:
                x, y = sumolib.geomhelper.positionAtShapeOffset(
                    lanes[0].getShape(), offset
                )
            else:
                x, y = sumolib.geomhelper.positionAtShapeOffset(edge.getShape(), offset)
            _line = bonn_motion.get(p_id, [])
            _line.append(f"{float(t)} {x} {y_bound - y}")
            bonn_motion[p_id] = _line
    print(f"dropped p_ids: {dropped}")
    with open(os.path.join(root, "munich.bonnmotion"), "w", encoding="utf-8") as fd:
        for key, val in bonn_motion.items():
            if key in dropped:
                continue
            fd.write(" ".join(val))
            fd.write("\n")
    print("done")


if __name__ == "__main__":
    main()
