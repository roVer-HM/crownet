from __future__ import annotations
from collections import Counter
from dataclasses import dataclass, field
import dataclasses
import glob
import io
import shutil
import xml.etree.ElementTree as ET
from json import load, dump, JSONDecodeError, JSONEncoder, JSONDecoder
from typing import Any, List, Protocol
import numpy as np
import numpy.linalg as lg
import pandas as pd
import matplotlib.pyplot as plt
import os
import sys
import contextlib
import gzip
from crownetutils.utils.parallel import run_args_map
from crownetutils.sumo import SimDir as SumoSimDir

if "SUMO_HOME" in os.environ:
    sys.path.append(os.path.join(os.environ["SUMO_HOME"], "tools"))
import sumolib  # noqa
import traceExporter as tExp


@contextlib.contextmanager
def _open(path: str):
    fd = None
    try:
        if path.endswith(".gz"):
            fd = gzip.open(path, "rt", encoding="utf-8")
        else:
            fd = open(path, "rt", encoding="utf-8")
        yield fd
        fd.close()
        fd = None
    except Exception as e:
        if fd is not None:
            fd.close()
        raise e


class BonnMotionReader:
    def __init__(self, path) -> None:
        self.path = path
        self.handler = None
        self.sumo_ids = None

    @contextlib.contextmanager
    def _open(self, path: str):
        fd = None
        try:
            if path.endswith(".gz"):
                fd = gzip.open(path, "rt", encoding="utf-8")
            else:
                fd = open(path, "rt", encoding="utf-8")
            yield fd
            fd.close()
            fd = None
        except Exception as e:
            if fd is not None:
                fd.close()
            raise e

    def __iter__(self):
        comment_count = 0
        ID_MAP_COMMENT = "# Sumo id map:"
        with self._open(self.path) as fd:
            for c, _row in enumerate(fd):
                if _row.startswith("#"):
                    comment_count += 1
                    if _row.startswith(ID_MAP_COMMENT):
                        _c = _row[len(ID_MAP_COMMENT) :].strip()
                        self.sumo_ids = np.array(_c.split(" ")).astype(int)
                    continue

                row_c = c - comment_count
                dbl_row = np.array(_row.split(" ")).astype(float)
                if len(dbl_row) % 3 != 0:
                    print(
                        f"row {row_c} number of elements in row are not divisible by 3, thus not a valid bonn motion trace. Skip."
                    )
                    continue
                if self.handler is not None:
                    dbl_row = self.handler(row_c, dbl_row)
                yield row_c, dbl_row

    def to_frame(self) -> pd.DataFrame:
        df = []
        for c, _row in self:
            # print(f"process row {c}")
            df.append(_row)
        df = pd.DataFrame(
            np.concatenate(df, axis=0),
            columns=["id", "time", "x", "y", "dist", "speed"],
        )
        df = df.fillna(0.0)
        df["id"] = df["id"].astype(int)
        return df


def row(id, data: np.array):
    data = data.reshape((-1, 3))
    data
    data_shift = np.copy(data)
    data_shift[-1] = data_shift[0]
    data_shift = np.roll(data_shift, shift=1, axis=0)
    data_diff = data - data_shift
    dist = lg.norm(data_diff[:, 1:3], axis=1)
    with np.errstate(divide="ignore", invalid="ignore"):
        speed = dist / data_diff[:, 0]
    data = np.concatenate(
        [
            np.repeat(id, data.shape[0]).reshape(-1, 1),
            data,
            dist.reshape(-1, 1),
            speed.reshape(-1, 1),
        ],
        axis=1,
    )
    return data


def out_of(a, b):
    try:
        r = f"{a}/{b} ({100*a/b:.1f}%)"
    except ZeroDivisionError as e:
        r = f"{a}/{b} (N/A %)"
    return r


@dataclass
class BmTraceInfo:
    traces: pd.DataFrame
    max_speed: float = 2.2
    max_dist: float = 5.0
    num_nodes: int = field(init=False)
    df_speed_violations: pd.DataFrame = field(init=False)
    num_nodes_violated_speed: int = field(init=False)
    df_dist_violations: pd.DataFrame = field(init=False)
    num_nodes_violated_dist: int = field(init=False)

    def __post_init__(self):
        self._find_speed_violations()

    def _find_speed_violations(self):
        self.num_nodes = len(self.traces["id"].unique())
        self.df_speed_violations = self.traces[self.traces["speed"] > self.max_speed]
        self.df_dist_violations = self.traces[self.traces["dist"] > self.max_dist]
        self.num_nodes_violated_speed = len(self.df_speed_violations["id"].unique())
        self.num_nodes_violated_dist = len(self.df_dist_violations["id"].unique())

    def info_str(self, fd=sys.stdout):
        df_most_violating_nodes = self.df_speed_violations.groupby("id")["time"].count()
        _num_most_violating_nodes_gt3 = df_most_violating_nodes[
            df_most_violating_nodes > 3
        ].shape[0]

        print("-" * 80, file=fd)
        print(
            f"Descriptive statistics of all speed values at delta_t 0.4s over all nodes (N={self.num_nodes})",
            file=fd,
        )
        print("-" * 80, file=fd)
        print(self.traces[["speed", "dist"]].describe().T, file=fd)
        print("-" * 80, file=fd)
        # print("-"*80, file=fd)
        # print(f"Number of values with speed over {self.max_speed} m/s", file=fd)
        # print(f"     Total incidents: {out_of(self.df_speed_violations.shape[0],self.traces.shape[0])}", file=fd)
        # print(f"     Number of violating nodes: {out_of(self.num_nodes_violated_speed, self.num_nodes)}", file=fd)
        # print(f"     Number of nodes with more than 3 violations: {out_of(_num_most_violating_nodes_gt3,self.num_nodes_violated_speed)}", file=fd)
        if self.df_dist_violations.shape[0] > 0:
            print(
                f"Number of values with moved distance over: {self.max_dist} m", file=fd
            )
            print(
                f"     Total incidents: {out_of(self.df_dist_violations.shape[0],self.traces.shape[0])}",
                file=fd,
            )
            print(
                f"     Number of violating nodes: {out_of(self.num_nodes_violated_dist, self.num_nodes)}",
                file=fd,
            )
            print("-" * 80, file=fd)


@dataclass
class BmSumoStep:
    bm_id: int = field(default=-1)
    sumo_id: int = field(default=-1)
    time: float = field(default=-1.0)
    dist: float = field(default=0.0)
    speed: float = field(default=0.0)
    edge_1: str = field(default="-")
    edge_2: str = field(default="-")

    @classmethod
    def csv_header(cls, fd, sep=";", end=os.linesep):
        keys = [f.name for f in dataclasses.fields(cls)]
        print(sep.join(keys), file=fd, end=end)

    @staticmethod
    def dict_by_id(
        steps: List[BmSumoStep], sumo_ids=True
    ) -> dict[int, List[BmSumoStep]]:
        """BmSumoStep dict sorted by key (can be sumo or bonnmotion id)

        Args:
            steps (List[BmSumoStep]): List of steps sorted into dict
            sumo_ids (bool, optional): If true use Sumo ID's. Defaults to True.

        Returns:
            dict[int, List[BmSumoStep]]: BmSumoStep's sorted by in and time
        """
        if sumo_ids:
            _id = lambda x: x.sumo_id
        else:
            _id = lambda x: x.bm_id
        ret = {_id(s): [] for s in steps}
        steps.sort(key=lambda x: x.time)
        for s in steps:
            ret[_id(s)].append(s)

        return ret

    def __str__(self) -> str:
        return f"BmSumoStep(bm_id={self.bm_id}, sumo_id={self.sumo_id}, time={self.time}, e1={self.edge_1}, e2={self.edge_2})"

    def csv_line(self, fd, sep=";", *args, end=os.linesep):

        if len(args) == 0:
            keys = [f.name for f in dataclasses.fields(self.__class__)]
        else:
            keys = args

        val = []
        for k in keys:
            if k in self.__dict__:
                val.append(str(self.__dict__[k]))
            else:
                print(f"no value for key '{k}' found. Skip")
        print(sep.join(val), file=fd, end=end)

    def info_str(self):
        out = io.StringIO()

        print(
            f"Node [Bm/Sumo] {self.bm_id}/{self.sumo_id} at time {self.time}", file=out
        )
        print(f"dist {self.dist}m, speed: {self.speed} m/s", file=out)
        print(f"edge: {self.edge_1}->{self.edge_2}", end="", file=out)
        if self.same_edge:
            print(" (Same)", file=out)
        else:
            print(" (DIFF!!!!!)", file=out)

        out.seek(0)
        return out.getvalue()

    @property
    def edges(self):
        return f"{self.edge_1}->{self.edge_2}"

    @property
    def edges_dist(self):
        return f"{self.edge_1}->{self.edge_2}->{self.dist}"

    @property
    def same_edge(self):
        return self.edge_1 == self.edge_2


def describe_fast_steps(fast_steps: List[BmSumoStep], fd=sys.stdout):
    _same = []
    _diff = []
    edge_count = Counter()
    for s in fast_steps:
        if s.same_edge:
            _same.append(s)
            edge_count.update([s.edge_1])
        else:
            _diff.append(s)
            edge_count.update([f"{s.edges}"])

    _without_walking_area = [s for s in fast_steps if "_w" not in s.edges]

    speed_same = pd.Series([_s.speed for _s in _same])
    speed_diff = pd.Series([_s.speed for _s in _diff])

    if len(fast_steps) > 10:
        print("-" * 80, file=fd)
        print(
            f"Number of incidents containing walking areas: {out_of(len(fast_steps)-len(_without_walking_area), len(fast_steps))}",
            file=fd,
        )
        print("-" * 80, file=fd)
        print(f"Same edges {out_of(len(_same), len(fast_steps))}", file=fd)
        print(speed_same.describe().to_frame().T, file=fd)
        print("-" * 80, file=fd)
        print(f"Different edges {out_of(len(_diff), len(fast_steps))}", file=fd)
        print(speed_diff.describe().to_frame().T, file=fd)
        print("-" * 80, file=fd)
        print(f"Number of different edges involved {len(edge_count)}", file=fd)
        # print(f"Most frequent edges {edge_count.most_common(n=5)}", file=fd)
    return None


@dataclass
class TimeIdMapping:
    time: float
    sumo_id: int
    bm_id: int


def extract_steps(fcd, violation_time_steps, sumo_id_list: List[int]):

    with _open(fcd) as fd:
        fcdxml = ET.parse(fd)

    # prepare id paring with time points where bonnmotion traces show irregular behavior
    sumo_id_list = [int(i) for i in sumo_id_list]
    time_id_records = list(
        violation_time_steps[["time", "id"]].sort_values("time").to_records(index=False)
    )
    time_id_records = [
        TimeIdMapping(time=time, bm_id=id, sumo_id=sumo_id_list[id])
        for time, id in time_id_records
    ]

    # output
    fast_steps = []
    traces = {sumo_id_list[s]: list() for s in violation_time_steps["id"].unique()}

    # create trace only for pedestrians with irregular behavior
    for tid_record in time_id_records:
        xpath_ts = f".//timestep/person[@id='{tid_record.sumo_id}']/.."
        xpath_person = f"./person[@id='{tid_record.sumo_id}']"
        xml_trace = fcdxml.findall(xpath_ts)
        if len(xml_trace) == 0:
            print(f"no traces found for {xpath_ts}")
        for time_step in xml_trace:
            trace_point = {"time": float(time_step.attrib["time"])}
            person = time_step.find(xpath_person)
            trace_point.update(**person.attrib)
            traces[tid_record.sumo_id].append(trace_point)

    traces = {k: Trace.from_json_list(v) for k, v in traces.items()}

    # create Steps
    for tid_record in time_id_records:
        _trace = traces[tid_record.sumo_id]
        s = _trace.get_step_at(tid_record.time, tid_record.bm_id, tid_record.sumo_id)
        fast_steps.append(s)

    return fast_steps, traces


class DataClassEncoder(JSONEncoder):
    def default(self, o: Any) -> Any:
        if dataclasses.is_dataclass(o):
            return dataclasses.asdict(o)
        elif isinstance(o, np.int64):
            return int(o)
        else:
            return super().default(o)


def decode_BmSumoStep(obj):
    try:
        if obj == {}:
            return obj
        return BmSumoStep(**obj)
    except Exception as e:
        return obj


def dump_bm_sumo_steps(steps: List[BmSumoStep], path):
    with open(path, "w", encoding="utf-8") as fd:
        dump(steps, fp=fd, cls=DataClassEncoder, indent=2)


def load_bm_sumo_steps(path):
    with open(path, "r", encoding="utf-8") as fd:
        ret = load(fd, object_hook=decode_BmSumoStep)
    return ret


@dataclass
class TracePoint:
    angle: float
    edge: str
    id: int
    pos: float
    slope: float
    speed: float
    x: float
    y: float
    time: float

    def dist(self, other: TracePoint):
        return np.linalg.norm(
            [float(self.x) - float(other.x), float(self.y) - float(other.y)]
        )

    def calc_speed(self, other: TracePoint):
        return float(np.abs(self.dist(other) / (self.time - other.time)))

    def route_stat(self, target: TracePoint, fd=sys.stdout):
        if self.time > target.time:
            return target.route_stat(self)
        time_delta = target.time - self.time
        print(f"Route from point: \n\t{self}\n\t{target}", file=fd)
        print(f"\tTime: {time_delta}", file=fd)
        print(f"\tDistance: {self.dist(target)}", file=fd)


class Dir(Protocol):
    def has_next_in_dir(self, x) -> bool:
        ...

    def go_dir(self, x) -> int:
        ...


class DirNext:
    def __init__(self, l) -> None:
        self.l = l

    def has_next_in_dir(self, x):
        return (x + 1) < self.l

    def go_dir(self, x):
        return x + 1


class DirPrev:
    def has_next_in_dir(self, x):
        return (x - 1) >= 0

    def go_dir(self, x):
        return x - 1


@dataclass
class Trace:
    trace: List[TracePoint]
    _head: int = field(default=0, init=False)

    @classmethod
    def from_json_list(cls, data):
        trace = [TracePoint(**t) for t in data]
        return cls(trace)

    def _get_dir(self, dir):
        if dir == "next":
            return DirNext(len(self.trace))
        elif dir == "prev":
            return DirPrev()
        else:
            raise ValueError(f"Expected direction 'next' or 'prev' got '{dir}'")

    def __post_init__(self):
        self.trace.sort(key=lambda x: x.time)

    def __repr__(self) -> str:
        return f"Trace(trace_len:{len(self.trace)} Head: {self._head})"

    def _peek(self, dir):
        dir_f = self._get_dir(dir)

        _e = self.get_current_edge()
        _new_head = None
        while dir_f.has_next_in_dir(_next):
            _next = dir_f.go_dir(_next)
            if self.trace[_next].edge != _e:
                _new_head = _next
                break

        return _new_head

    def get_edge(self, point_id):
        if point_id is None:
            return None
        else:
            return self.trace[point_id].edge

    def get_trace_point(self, id) -> TracePoint:
        return self.trace[id]

    def get_current_edge(self):
        return self.get_edge(self._head)

    def get_current_trace_point(self):
        return self.trace[self._head]

    def peek_next_edge(self):
        _h = self._peek(dir="next")
        return self.get_edge(_h)

    def has_next(self):
        return self._peek("next") is not None

    def next_edge(self):
        self._head = self._peek("next")
        if self._head is None:
            raise ValueError("No next trace point found")
        return self.get_current_edge()

    def peek_previous_edge(self):
        _h = self._peek(dir="prev")
        return self.get_edge(_h)

    def has_previous(self):
        return self._peek("prev") is not None

    def previous_edge(self):
        self._head = self._peek("prev")
        if self._head is None:
            raise ValueError("No previous trace point found")
        return self.get_current_edge()

    def get_step_at(self, time, bm_id, sumo_id):
        _h = self._head
        self.go_to_time(time)
        e_now = self.get_current_trace_point()
        if self._head == 0:
            e_prev = e_now  # start of trace
        else:
            e_prev = self.get_trace_point(self._head - 1)
        self._head = _h

        return BmSumoStep(
            bm_id=bm_id,
            sumo_id=sumo_id,
            time=time,
            dist=e_now.dist(e_prev),
            speed=e_now.calc_speed(e_prev),
            edge_1=e_prev.edge,
            edge_2=e_now.edge,
        )

    def go_to_time(self, time, exact_match: bool = True):
        for i, point in enumerate(self.trace):
            if exact_match:
                if point.time == time:
                    self._head = i
                    return self.get_current_trace_point()
            else:
                if point.time > time and i > 0:
                    self._head = i - 1
                    return self.get_current_trace_point()

        raise ValueError(f"No TracePoint with time '{time}' found")

    @property
    def first(self):
        return 0

    @property
    def last(self):
        return len(self.trace) - 1

    def _find(self, edges, dir):
        dir_f = self._get_dir(dir)
        x = self._head
        while dir_f.has_next_in_dir(x):
            x = dir_f.go_dir(x)
            if self.get_edge(x) in edges:
                return x

        return None

    def print_route(self, start, end, fd=sys.stdout):
        self.get_trace_point(start).route_stat(self.get_trace_point(end), fd=fd)

    def find_edge_match(
        self, edges: List[str], time, exact_match: bool = True, fd=os.devnull
    ):
        _head = self._head
        self.go_to_time(time, exact_match)
        prev_id = self._find(edges, dir="prev")
        next_id = self._find(edges, dir="next")

        routes = []
        if prev_id is not None:
            e_idx = edges.index(self.get_edge(prev_id))
            routes.append(slice(0, e_idx + 1))
            self.print_route(0, prev_id)

        if next_id is not None:
            e_idx = edges.index(self.get_edge(next_id))
            routes.append(slice(e_idx, len(edges)))
            self.print_route(next_id, self.last)

        self._head = _head
        return routes


def update_route(run_dict: dict, sim_dir: SumoSimDir):

    for run in run_dict:
        id = f"{run['run_id']:03}"
        route_path = sim_dir.route(f"{id}__ped_route.xml")
        route_path_backup = sim_dir.route("backup.d", f"{id}__ped_route.xml")
        if os.path.exists(route_path_backup):
            print(f"skip {route_path}")
            continue
        routes = ET.parse(route_path)

        step_dict = BmSumoStep.dict_by_id(run["steps"])
        traces = {
            int(k): Trace.from_json_list(v["trace"]) for k, v in run["traces"].items()
        }
        if not (set(step_dict.keys()) == set(traces.keys())):
            print("miss match step_dict and traces")
            continue
        for x in [f".//person[@id='{i}']" for i in step_dict.keys()]:
            person = routes.find(x)
            if person is None:
                print(f"person not found with xpath: '{x}'")
                continue
            pid = int(person.attrib["id"])

            walk = person.find("walk")
            if walk is None:
                print(f"skip route with id {pid}. No walk element found.")
                continue
            slices = []
            for step in step_dict[pid]:
                walk_edges = walk.attrib["edges"].split(" ")
                trace = traces[pid]
                print(f"pid({pid}): remove {step} from trace")
                walk_slices = trace.find_edge_match(
                    walk_edges, step.time, exact_match=True, fd=sys.stdout
                )
                slices.extend(walk_slices)
            print("-" * 80)
            print(
                f"found {len(slices)} route slices: {slices}. Choose first one for now."
            )
            print(f"replace walk edges for person {pid} with edges id's {slices[0]}")
            walk.attrib["edges"] = " ".join(walk_edges[slices[0]])
            print("-" * 80)
            print("=" * 80, end="\n\n")

        os.makedirs(os.path.dirname(route_path_backup), exist_ok=True)
        shutil.copyfile(route_path, route_path_backup)
        with open(route_path, "w", encoding="utf-8") as fd:
            routes.write(fd, encoding="unicode")

        print("hi")


def process_area_distribution(net, trace, bin_size=10.0, fd=sys.stdout):

    bins = pd.interval_range(
        start=0.0, end=np.ceil(trace["time"].max()), closed="left", freq=bin_size
    )
    bins = pd.cut(trace["time"], bins=bins)
    trace["time_b"] = bins
    # (N,4) with N bins containing the maximal area occupied by nodes in the time interval given by freq
    bbox = (
        trace.groupby("time_b")[["x", "y"]]
        .agg(["min", "max"])
        .swaplevel(0, -1, axis=1)
        .loc[:, [("min", "x"), ("min", "y"), ("max", "x"), ("max", "y")]]
    ).to_numpy()
    abs_bound = bbox[:, 2:] - bbox[:, :2]
    area = abs_bound[:, 0] * abs_bound[:, 1]
    net_bound = net.getBBoxXY()[1]
    net_area = net_bound[0] * net_bound[1]
    area_ratio = pd.Series(area / net_area)
    max_route_extend = (
        trace.max()[["x", "y"]].to_numpy() - trace.min()[["x", "y"]].to_numpy()
    )
    max_route_extend_area = max_route_extend[0] * max_route_extend[1]
    max_route_extend_ratio = max_route_extend_area / net_area
    print("-" * 80, file=fd)
    print(f"Scenario bound: {net_bound} m with area {net_area}", file=fd)
    print(
        f"Maximal route extend over simulation time: {max_route_extend} m area ratio {max_route_extend_ratio:0.3f}",
        file=fd,
    )
    print("-" * 80, file=fd)
    print(f"Route extend ratio over time with {bin_size} seconds bins.", file=fd)
    print(area_ratio.describe().to_frame().T, file=fd)
    print("-" * 80, file=fd)


def run(net, bm_path, row_f, fcd):
    fd = io.StringIO()
    bmr = BonnMotionReader(bm_path)
    bmr.handler = row_f

    trace_info = BmTraceInfo(bmr.to_frame(), max_speed=2.2, max_dist=5.0)

    print("#" * 80, file=fd)
    print(f"# process [...]{bmr.path[34:]}", file=fd)
    print("#" * 80, file=fd)

    # speed violation info
    trace_info.info_str(fd=fd)

    steps, traces = extract_steps(fcd, trace_info.df_dist_violations, bmr.sumo_ids)
    describe_fast_steps(steps, fd)

    # area info
    # net = sumolib.net.readNet(net)
    # process_area_distribution(net, trace_info.traces, bin_size=10.0, fd=fd)

    fd.seek(0)
    id = int(os.path.basename(bm_path)[:3])
    return fd, bmr, steps, traces, id


def main(sim_dir: SumoSimDir, out_prefix="traj_check"):
    net = sim_dir.root("muc.net.xml.gz")
    bm_files = glob.glob(sim_dir.bm("*__muc.bonnmotion.gz"))
    args = []
    for bm in bm_files:
        # id = int(os.path.basename(bm)[:3])
        # if id != 11:
        #     continue
        fcd = sim_dir.fcd(f"{os.path.basename(bm)[:3]}__fcd_muc.xml.gz")
        args.append((net, bm, row, fcd))

    ret = run_args_map(
        run, args_iter=args, pool_size=5, raise_on_error=False, append_args=False
    )
    # ret = run(*args[0])
    # ret = [(True, ret)]

    for err, out in ret:
        if err:
            fd, bmr, _, _, _ = out
            fd.seek(0)
            print(fd.getvalue())
        else:
            print(out)

    data = [o[1] for o in ret if o[0]]
    data.sort(key=lambda x: x[-1])

    edge_count = Counter()
    step_list = []

    with open(
        sim_dir.out(out_prefix, "steps.csv", ensure_dir_exists=True),
        "wt",
        encoding="utf-8",
    ) as fd:
        BmSumoStep.csv_header(fd, end="")
        print(";run", file=fd)
        for _, _mbr, steps, traces, id in data:
            for s in steps:
                s: BmSumoStep
                s.csv_line(fd, end=None)
                print(f";{id}", file=fd)
                edge_count.update([s.edges_dist])

            step_list.append({"run_id": id, "steps": steps, "traces": traces})

    with open(
        sim_dir.out(out_prefix, "steps.json", ensure_dir_exists=True),
        "wt",
        encoding="utf-8",
    ) as fd:
        dump(step_list, fd, indent=2, cls=DataClassEncoder)

    with open(
        sim_dir.out(out_prefix, "dist_valiation.csv", ensure_dir_exists=True),
        "w",
        encoding="utf-8",
    ) as f:
        f.write(f"count;e1;e2;dist\n")
        for edge, count in edge_count.items():
            e1, e2, dist = edge.split("->")
            f.write(f"{count};{e1};{e2};{dist}\n")

    print("done.")


if __name__ == "__main__":

    root = "/home/vm-sts/repos/crownet/crownet/simulations/multi_enb/sumo/munich/muc_cleaned"

    # sim_dir = SumoSimDir(sim_root=root, output_root="output/run_60_min")
    sim_dir = SumoSimDir(sim_root=root, output_root="output/run_60_min_cleaned")
    sim_dir.create_dirs()

    # main(sim_dir, out_prefix="traj_check")

    steps = load_bm_sumo_steps(sim_dir.out("traj_check", "steps.json"))
    update_route(steps, sim_dir)
    sim_dir.copy_to_clean(out_suffix="_2")
    # print("hi")
