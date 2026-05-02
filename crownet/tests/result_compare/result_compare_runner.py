#!/usr/bin/env python3
from __future__ import annotations

import argparse
import datetime as dt
import json
import re
import sqlite3
import subprocess
import sys
from pathlib import Path

import yaml

plt = None
np = None
pd = None

DEFAULT_APP_MODULE_REGEX = r".*\.app(?:\[\d+\])?\.app$"


def ensure_runtime_deps() -> None:
    global plt, np, pd
    if plt is not None and np is not None and pd is not None:
        return
    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as _plt
        import numpy as _np
        import pandas as _pd
    except ImportError as exc:
        raise RuntimeError(
            "Missing runtime dependency for result_compare_runner.py. "
            "Please activate crownet_user environment (make analysis-build && source out/crownet_user/bin/activate)."
        ) from exc

    plt = _plt
    np = _np
    pd = _pd


def repo_root_from_script() -> Path:
    return Path(__file__).resolve().parents[3]


def crownet_module_root() -> Path:
    return repo_root_from_script() / "crownet"


def load_yaml(path: Path) -> dict:
    with open(path, "r", encoding="utf-8") as fd:
        data = yaml.safe_load(fd)
    return data if data is not None else {}


def _selector_matches_config_path(path: Path, selectors: set[str], root_dir: Path) -> bool:
    candidates = {
        path.name,
        str(path),
        str(path.resolve()),
    }
    try:
        candidates.add(str(path.relative_to(root_dir)))
    except ValueError:
        pass
    return any(sel in candidates for sel in selectors)


def load_case_configs_from_directory(
    config_dir: Path,
    selected_config_files: list[str] | None = None,
) -> tuple[dict, dict[str, Path], list[Path]]:
    cases: dict = {}
    case_config_dirs: dict[str, Path] = {}
    loaded_files: list[Path] = []

    selectors = set(selected_config_files or [])
    config_dir = Path(config_dir).resolve()

    yml_files = sorted(config_dir.glob("*.yml"))

    if not yml_files:
        raise FileNotFoundError(f"no .yml config files found in {config_dir}")

    for yml_path in yml_files:
        if selectors and not _selector_matches_config_path(yml_path, selectors, config_dir):
            continue

        cfg = load_yaml(yml_path)
        if not isinstance(cfg, dict):
            raise ValueError(f"config must be a mapping: {yml_path}")

        loaded_files.append(yml_path)
        for case_name, case_cfg in cfg.items():
            if case_name in cases:
                raise ValueError(f"duplicate case '{case_name}' across config files (found in {yml_path})")
            cases[case_name] = case_cfg
            case_config_dirs[case_name] = yml_path.parent

    if selectors and not loaded_files:
        raise ValueError(
            "none of the selected --config-file entries matched .yml files from "
            f"{config_dir}"
        )

    if not loaded_files:
        raise FileNotFoundError(f"no valid .yml config files found in {config_dir}")

    return cases, case_config_dirs, loaded_files


def has_arg(args: list[str], key: str) -> bool:
    return any(a == key or a.startswith(f"{key}=") for a in args)


def arg_value(args: list[str], key: str, default: str | None = None) -> str | None:
    for i, a in enumerate(args):
        if a == key and i + 1 < len(args):
            return args[i + 1]
        if a.startswith(f"{key}="):
            return a.split("=", 1)[1]
    return default


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def sanitize_metric_id(metric_id: str) -> str:
    safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", metric_id).strip("_")
    if not safe:
        raise ValueError(f"invalid metric id for baseline file name: {metric_id!r}")
    return safe


def vector_baseline_file_name(metric_id: str) -> str:
    return f"vector_{sanitize_metric_id(metric_id)}.csv"


def scalar_baseline_file_name(metric_id: str) -> str:
    return f"scalar_{sanitize_metric_id(metric_id)}.csv"


def updated_baseline_variant_file(path: Path) -> Path:
    return path.with_name(f"{path.name}.UPDATED")


def failed_baseline_variant_file(path: Path) -> Path:
    return path.with_name(f"{path.name}.FAILED")


def write_vector_baseline_csv(path: Path, series: list[dict]) -> None:
    ensure_dir(path.parent)
    df = pd.DataFrame(series, columns=["time", "value"])
    if df.empty:
        df = pd.DataFrame(columns=["time", "value"])
    df.to_csv(path, index=False)


def write_scalar_baseline_csv(path: Path, value: float | None) -> None:
    ensure_dir(path.parent)
    if value is None:
        pd.DataFrame(columns=["value"]).to_csv(path, index=False)
        return
    pd.DataFrame([{"value": float(value)}]).to_csv(path, index=False)


def write_baseline_dir_csv(baseline_dir: Path, current: dict, case_cfg: dict) -> list[Path]:
    ensure_dir(baseline_dir)

    written_files: list[Path] = []
    expected_files: set[str] = set()

    vectors_cfg = case_cfg.get("extract", {}).get("vectors", [])
    scalars_cfg = case_cfg.get("extract", {}).get("scalars", [])

    for sel in vectors_cfg:
        sid = sel["id"]
        path = baseline_dir / vector_baseline_file_name(sid)
        write_vector_baseline_csv(path, current.get("vectors", {}).get(sid, []))
        written_files.append(path)
        expected_files.add(path.name)

    for sel in scalars_cfg:
        sid = sel["id"]
        path = baseline_dir / scalar_baseline_file_name(sid)
        write_scalar_baseline_csv(path, current.get("scalars", {}).get(sid))
        written_files.append(path)
        expected_files.add(path.name)

    # Remove stale metric files from previous selector sets.
    for stale in baseline_dir.glob("vector_*.csv"):
        if stale.name not in expected_files:
            stale.unlink()
    for stale in baseline_dir.glob("scalar_*.csv"):
        if stale.name not in expected_files:
            stale.unlink()

    return sorted(written_files)


def clear_case_failure_artifacts(baseline_dir: Path, case_cfg: dict) -> None:
    vectors_cfg = case_cfg.get("extract", {}).get("vectors", [])
    scalars_cfg = case_cfg.get("extract", {}).get("scalars", [])

    baseline_files: list[Path] = []
    for sel in vectors_cfg:
        baseline_files.append(baseline_dir / vector_baseline_file_name(sel["id"]))
    for sel in scalars_cfg:
        baseline_files.append(baseline_dir / scalar_baseline_file_name(sel["id"]))

    for base_path in baseline_files:
        for artifact_path in (updated_baseline_variant_file(base_path), failed_baseline_variant_file(base_path)):
            if artifact_path.exists():
                artifact_path.unlink()


def write_vector_failure_artifacts(
    baseline_file: Path,
    baseline_series: list[dict],
    current_series: list[dict],
    comp_df,
    reason: str,
) -> tuple[Path, Path]:
    updated_file = updated_baseline_variant_file(baseline_file)
    failed_file = failed_baseline_variant_file(baseline_file)

    write_vector_baseline_csv(updated_file, current_series)
    if comp_df.empty:
        pd.DataFrame(
            [
                {
                    "reason": reason,
                    "baseline_points": len(baseline_series),
                    "current_points": len(current_series),
                }
            ]
        ).to_csv(failed_file, index=False)
    else:
        comp_df.to_csv(failed_file, index=False)

    return failed_file, updated_file


def write_scalar_failure_artifacts(
    baseline_file: Path,
    baseline_value: float | None,
    current_value: float | None,
    abs_delta: float | None,
) -> tuple[Path, Path]:
    updated_file = updated_baseline_variant_file(baseline_file)
    failed_file = failed_baseline_variant_file(baseline_file)

    write_scalar_baseline_csv(updated_file, current_value)
    pd.DataFrame(
        [
            {
                "baseline": baseline_value,
                "current": current_value,
                "abs_delta": abs_delta,
            }
        ]
    ).to_csv(failed_file, index=False)

    return failed_file, updated_file


def read_vector_baseline_csv(path: Path) -> tuple[bool, list[dict]]:
    if not path.exists():
        return False, []

    try:
        df = pd.read_csv(path)
    except pd.errors.EmptyDataError:
        return True, []

    if df.empty:
        return True, []

    if "time" not in df.columns or "value" not in df.columns:
        raise ValueError(f"invalid vector baseline format in {path}. Expected columns: time,value")

    data = df[["time", "value"]].apply(pd.to_numeric, errors="coerce").dropna()
    if data.empty:
        return True, []

    data = data.sort_values("time")
    out = []
    for t, v in zip(data["time"], data["value"]):
        out.append({"time": float(t), "value": float(v)})
    return True, out


def read_scalar_baseline_csv(path: Path) -> tuple[bool, float | None]:
    if not path.exists():
        return False, None

    try:
        df = pd.read_csv(path)
    except pd.errors.EmptyDataError:
        return True, None

    if df.empty:
        return True, None

    if "value" in df.columns:
        col = "value"
    elif len(df.columns) == 1:
        col = df.columns[0]
    else:
        raise ValueError(f"invalid scalar baseline format in {path}. Expected one 'value' column")

    vals = pd.to_numeric(df[col], errors="coerce").dropna()
    if vals.empty:
        return True, None

    return True, float(vals.iloc[0])


def load_baseline_payload(case_cfg: dict, config_dir: Path) -> tuple[dict, Path, set[str], set[str]]:
    baseline_rel = case_cfg["baseline"]
    baseline_path = config_dir / baseline_rel

    if not baseline_path.exists():
        raise FileNotFoundError(f"baseline directory not found: {baseline_path}")

    vectors_cfg = case_cfg.get("extract", {}).get("vectors", [])
    scalars_cfg = case_cfg.get("extract", {}).get("scalars", [])

    baseline = {
        "vectors": {},
        "scalars": {},
    }
    missing_vector_files: set[str] = set()
    missing_scalar_files: set[str] = set()

    for sel in vectors_cfg:
        sid = sel["id"]
        path = baseline_path / vector_baseline_file_name(sid)
        exists, series = read_vector_baseline_csv(path)
        baseline["vectors"][sid] = series
        if not exists:
            missing_vector_files.add(sid)

    for sel in scalars_cfg:
        sid = sel["id"]
        path = baseline_path / scalar_baseline_file_name(sid)
        exists, value = read_scalar_baseline_csv(path)
        baseline["scalars"][sid] = value
        if not exists:
            missing_scalar_files.add(sid)

    return baseline, baseline_path, missing_vector_files, missing_scalar_files


def list_result_dbs(base_dir: Path) -> tuple[Path | None, Path | None]:
    def newest_pair(vec_files) -> tuple[Path | None, Path | None]:
        newest_vec = None
        newest_sca = None
        newest_mtime = -1.0

        for vec_file in vec_files:
            sca_file = vec_file.with_suffix(".sca")
            if not sca_file.exists():
                continue

            pair_mtime = max(vec_file.stat().st_mtime, sca_file.stat().st_mtime)
            if pair_mtime > newest_mtime:
                newest_mtime = pair_mtime
                newest_vec = vec_file
                newest_sca = sca_file

        return newest_vec, newest_sca

    seen: set[Path] = set()
    limited_candidates: list[Path] = []
    for pattern in ("*.vec", "*/*.vec", "*/*/*.vec"):
        for vec_file in base_dir.glob(pattern):
            if vec_file in seen:
                continue
            seen.add(vec_file)
            limited_candidates.append(vec_file)

    newest_vec, newest_sca = newest_pair(limited_candidates)
    if newest_vec is not None and newest_sca is not None:
        return newest_vec, newest_sca

    # Fallback for unexpected nested layouts.
    return newest_pair(base_dir.rglob("*.vec"))


def query_vector_series(
    vec_con: sqlite3.Connection,
    vector_name: str,
    module_regex: str = DEFAULT_APP_MODULE_REGEX,
) -> list[dict]:

    info = pd.read_sql_query(
        "select vectorId, moduleName from vector where vectorName = ?",
        vec_con,
        params=(vector_name,),
    )
    if info.empty:
        return []

    rx = re.compile(module_regex)
    info = info[info["moduleName"].apply(lambda x: bool(rx.match(x)))]
    if info.empty:
        return []

    ids = info["vectorId"].astype(int).tolist()
    id_csv = ",".join(str(i) for i in ids)
    values = pd.read_sql_query(
        f"select vectorId, simtimeRaw, value from vectorData where vectorId in ({id_csv})",
        vec_con,
    )

    if values.empty:
        return []

    values["time"] = values["simtimeRaw"].astype(float) / 1e12
    series = values.groupby("time", as_index=True)["value"].sum()

    out = []
    for t, v in series.items():
        out.append({"time": float(t), "value": float(v)})
    return out


def query_scalar_value(
    sca_con: sqlite3.Connection,
    scalar_name: str,
    module_regex: str = DEFAULT_APP_MODULE_REGEX,
) -> float | None:

    data = pd.read_sql_query(
        "select moduleName, scalarValue from scalar where scalarName = ?",
        sca_con,
        params=(scalar_name,),
    )

    if data.empty:
        return None

    rx = re.compile(module_regex)
    data = data[data["moduleName"].apply(lambda x: bool(rx.match(x)))]
    if data.empty:
        return None

    vals = pd.to_numeric(data["scalarValue"], errors="coerce").dropna()
    if vals.empty:
        return None

    return float(vals.sum())


def to_frame(points: list[dict]) -> pd.DataFrame:
    if not points:
        return pd.DataFrame(columns=["time", "value"])
    df = pd.DataFrame(points)
    if df.empty:
        return pd.DataFrame(columns=["time", "value"])
    return df.sort_values("time")


def compare_vector(series_baseline: list[dict], series_current: list[dict]) -> tuple[dict | None, pd.DataFrame]:
    b = to_frame(series_baseline)
    c = to_frame(series_current)

    if b.empty or c.empty:
        return None, pd.DataFrame(columns=["time", "baseline", "current", "delta"])

    comp = (
        b.rename(columns={"value": "baseline"})
        .merge(c.rename(columns={"value": "current"}), on="time", how="inner")
        .sort_values("time")
    )

    if comp.empty:
        return None, pd.DataFrame(columns=["time", "baseline", "current", "delta"])

    comp["delta"] = (comp["current"] - comp["baseline"]).abs()
    delta = comp["delta"].to_numpy()
    if len(delta) == 0:
        return None, comp

    metrics = {
        "max_abs_delta": float(delta.max()),
        "rmse": float(np.sqrt(np.mean(np.square(delta)))),
    }
    return metrics, comp


def plot_vector(comp_df: pd.DataFrame, selector_id: str, out_png: Path) -> None:
    ensure_dir(out_png.parent)
    fig, axs = plt.subplots(2, 1, figsize=(10, 6), sharex=True)

    if comp_df.empty:
        axs[0].text(0.5, 0.5, "No overlapping data", ha="center", va="center")
        axs[1].text(0.5, 0.5, "No delta data", ha="center", va="center")
        axs[0].set_title(selector_id)
    else:
        axs[0].plot(comp_df["time"], comp_df["baseline"], label="baseline", linewidth=1.5)
        axs[0].plot(comp_df["time"], comp_df["current"], label="current", linewidth=1.5)
        axs[0].set_title(selector_id)
        axs[0].set_ylabel("value")
        axs[0].legend(loc="best")

        axs[1].plot(comp_df["time"], comp_df["delta"], color="tab:red", linewidth=1.3)
        axs[1].set_ylabel("|delta|")
        axs[1].set_xlabel("time [s]")

    fig.tight_layout()
    fig.savefig(out_png, dpi=140)
    plt.close(fig)


def extract_current(case_cfg: dict, vec_path: Path, sca_path: Path) -> dict:
    extract_cfg = case_cfg.get("extract", {})
    vectors_cfg = extract_cfg.get("vectors", [])
    scalars_cfg = extract_cfg.get("scalars", [])

    out = {
        "vectors": {},
        "scalars": {},
    }

    with sqlite3.connect(str(vec_path)) as vec_con, sqlite3.connect(str(sca_path)) as sca_con:
        for sel in vectors_cfg:
            out["vectors"][sel["id"]] = query_vector_series(
                vec_con=vec_con,
                vector_name=sel["vector_name"],
                module_regex=sel.get("module_regex", DEFAULT_APP_MODULE_REGEX),
            )

        for sel in scalars_cfg:
            out["scalars"][sel["id"]] = query_scalar_value(
                sca_con=sca_con,
                scalar_name=sel["scalar_name"],
                module_regex=sel.get("module_regex", DEFAULT_APP_MODULE_REGEX),
            )

    return out


def run_case(case_name: str, case_cfg: dict, config_dir: Path, output_dir: Path, update_baseline: bool) -> dict:
    now = dt.datetime.now().strftime("%Y%m%dT%H%M%S")
    wd = crownet_module_root() / case_cfg["wd"]

    run_cfg = case_cfg.get("run", {})
    exec_cmd = run_cfg.get("exec", "python3")
    args = list(run_cfg.get("args", []))

    run_name = f"rc_{case_name}_{now}".replace("/", "_")
    experiment_label = f"result_compare_{now}"

    run_root = output_dir / case_name / now
    ensure_dir(run_root)
    logs_dir = run_root / "logs"
    ensure_dir(logs_dir)

    if not has_arg(args, "--resultdir"):
        args.extend(["--resultdir", str(run_root / "sim")])
    if not has_arg(args, "--experiment-label"):
        args.extend(["--experiment-label", experiment_label])
    if not has_arg(args, "--run-name"):
        args.extend(["--run-name", run_name])

    cmd = [exec_cmd, *args]
    print(f"[{case_name}] running: {' '.join(cmd)}")

    log_path = logs_dir / "command.out"
    with open(log_path, "w", encoding="utf-8") as fd:
        fd.write(f"$ {' '.join(cmd)}\n\n")
        proc = subprocess.run(
            cmd,
            cwd=str(wd),
            stdout=fd,
            stderr=subprocess.STDOUT,
            text=True,
        )

    result = {
        "case": case_name,
        "command": cmd,
        "working_dir": str(wd),
        "exitcode": int(proc.returncode),
        "ok": False,
        "errors": [],
        "updated_candidate_files": [],
        "failure_artifact_files": [],
        "vector_results": {},
        "scalar_results": {},
    }

    if proc.returncode != 0:
        result["errors"].append(f"simulation command failed with exit code {proc.returncode}")
        return result

    sim_root = Path(arg_value(args, "--resultdir", str(run_root / "sim")))
    config_name = arg_value(args, "--opp.-c")
    if config_name:
        expected = sim_root / f"{config_name}_{experiment_label}"
        search_root = expected if expected.exists() else sim_root
    else:
        search_root = sim_root

    vec_path, sca_path = list_result_dbs(search_root)
    if vec_path is None or sca_path is None:
        result["errors"].append(f"could not locate .vec/.sca files under {search_root}")
        return result

    current = extract_current(case_cfg=case_cfg, vec_path=vec_path, sca_path=sca_path)
    current["meta"] = {
        "vec": str(vec_path),
        "sca": str(sca_path),
        "generated_at": now,
        "case": case_name,
    }

    with open(run_root / "current.json", "w", encoding="utf-8") as fd:
        json.dump(current, fd, indent=2)

    baseline_rel = case_cfg["baseline"]
    baseline_path = config_dir / baseline_rel

    if update_baseline:
        written_files = write_baseline_dir_csv(
            baseline_dir=baseline_path,
            current=current,
            case_cfg=case_cfg,
        )
        result["updated_baseline"] = str(baseline_path)
        result["updated_baseline_files"] = [str(p) for p in written_files]
        result["ok"] = True
        return result

    try:
        baseline, baseline_root, missing_vector_files, missing_scalar_files = load_baseline_payload(
            case_cfg=case_cfg,
            config_dir=config_dir,
        )
    except (FileNotFoundError, ValueError, json.JSONDecodeError) as exc:
        result["errors"].append(str(exc))
        return result

    clear_case_failure_artifacts(baseline_root, case_cfg)

    plots_dir = run_root / "plots"

    for sel in case_cfg.get("extract", {}).get("vectors", []):
        sid = sel["id"]
        baseline_file = baseline_root / vector_baseline_file_name(sid)
        b_series = baseline.get("vectors", {}).get(sid, [])
        c_series = current.get("vectors", {}).get(sid, [])

        try:
            max_abs_threshold = float(sel["max_abs_threshold"])
            rmse_threshold = float(sel["rmse_threshold"])
        except (KeyError, TypeError, ValueError) as exc:
            err_msg = (
                f"vector '{sid}' has invalid or missing thresholds; required keys: max_abs_threshold, rmse_threshold ({exc})"
            )
            result["errors"].append(err_msg)
            result["vector_results"][sid] = {
                "max_abs_delta": None,
                "rmse": None,
                "max_abs_threshold": None,
                "rmse_threshold": None,
                "passed": False,
            }

            failed_file, updated_file = write_vector_failure_artifacts(
                baseline_file=baseline_file,
                baseline_series=b_series,
                current_series=c_series,
                comp_df=pd.DataFrame(columns=["time", "baseline", "current", "delta"]),
                reason=err_msg,
            )
            result["failure_artifact_files"].append(str(failed_file))
            result["updated_candidate_files"].append(str(updated_file))
            continue

        if sid in missing_vector_files:
            passed = False
            max_delta = None
            rmse = None
            err_msg = f"baseline vector file missing for '{sid}': {baseline_file}"
            result["errors"].append(err_msg)
            result["vector_results"][sid] = {
                "max_abs_delta": max_delta,
                "rmse": rmse,
                "max_abs_threshold": max_abs_threshold,
                "rmse_threshold": rmse_threshold,
                "passed": passed,
            }

            failed_file, updated_file = write_vector_failure_artifacts(
                baseline_file=baseline_file,
                baseline_series=b_series,
                current_series=c_series,
                comp_df=pd.DataFrame(columns=["time", "baseline", "current", "delta"]),
                reason=err_msg,
            )
            result["failure_artifact_files"].append(str(failed_file))
            result["updated_candidate_files"].append(str(updated_file))
            continue

        vector_metrics, comp_df = compare_vector(
            series_baseline=b_series,
            series_current=c_series,
        )

        plot_vector(comp_df, sid, plots_dir / f"{sid}.png")

        failure_reasons: list[str] = []
        if vector_metrics is None:
            passed = False
            max_delta = None
            rmse = None
            err_msg = f"vector '{sid}' missing or without overlap"
            failure_reasons.append(err_msg)
            result["errors"].append(err_msg)
        else:
            max_delta = float(vector_metrics["max_abs_delta"])
            rmse = float(vector_metrics["rmse"])
            passed_max = max_delta <= max_abs_threshold
            passed_rmse = rmse <= rmse_threshold
            passed = passed_max and passed_rmse
            if not passed_max:
                err_msg = f"vector '{sid}' max abs delta {max_delta:.6g} exceeds threshold {max_abs_threshold:.6g}"
                failure_reasons.append(err_msg)
                result["errors"].append(err_msg)
            if not passed_rmse:
                err_msg = f"vector '{sid}' RMSE {rmse:.6g} exceeds threshold {rmse_threshold:.6g}"
                failure_reasons.append(err_msg)
                result["errors"].append(err_msg)

        result["vector_results"][sid] = {
            "max_abs_delta": max_delta,
            "rmse": rmse,
            "max_abs_threshold": max_abs_threshold,
            "rmse_threshold": rmse_threshold,
            "passed": passed,
        }

        if not passed:
            failed_file, updated_file = write_vector_failure_artifacts(
                baseline_file=baseline_file,
                baseline_series=b_series,
                current_series=c_series,
                comp_df=comp_df,
                reason="; ".join(failure_reasons) if failure_reasons else f"vector '{sid}' comparison failed",
            )
            result["failure_artifact_files"].append(str(failed_file))
            result["updated_candidate_files"].append(str(updated_file))

        if not comp_df.empty:
            comp_df.to_csv(run_root / f"compare_{sid}.csv", index=False)

    for sel in case_cfg.get("extract", {}).get("scalars", []):
        sid = sel["id"]
        baseline_file = baseline_root / scalar_baseline_file_name(sid)
        b_val = baseline.get("scalars", {}).get(sid)
        c_val = current.get("scalars", {}).get(sid)

        try:
            threshold = float(sel["abs_threshold"])
        except (KeyError, TypeError, ValueError) as exc:
            err_msg = f"scalar '{sid}' has invalid or missing threshold; required key: abs_threshold ({exc})"
            result["errors"].append(err_msg)
            result["scalar_results"][sid] = {
                "abs_delta": None,
                "threshold": None,
                "passed": False,
                "baseline": b_val,
                "current": c_val,
            }

            failed_file, updated_file = write_scalar_failure_artifacts(
                baseline_file=baseline_file,
                baseline_value=b_val,
                current_value=c_val,
                abs_delta=None,
            )
            result["failure_artifact_files"].append(str(failed_file))
            result["updated_candidate_files"].append(str(updated_file))
            continue

        if sid in missing_scalar_files:
            passed = False
            abs_delta = None
            err_msg = f"baseline scalar file missing for '{sid}': {baseline_file}"
            result["errors"].append(err_msg)
            result["scalar_results"][sid] = {
                "abs_delta": abs_delta,
                "threshold": threshold,
                "passed": passed,
                "baseline": b_val,
                "current": c_val,
            }

            failed_file, updated_file = write_scalar_failure_artifacts(
                baseline_file=baseline_file,
                baseline_value=b_val,
                current_value=c_val,
                abs_delta=abs_delta,
            )
            result["failure_artifact_files"].append(str(failed_file))
            result["updated_candidate_files"].append(str(updated_file))
            continue

        failure_reason = ""
        if b_val is None or c_val is None:
            passed = False
            abs_delta = None
            failure_reason = f"scalar '{sid}' missing in baseline or current"
            result["errors"].append(failure_reason)
        else:
            abs_delta = abs(float(c_val) - float(b_val))
            passed = abs_delta <= threshold
            if not passed:
                failure_reason = f"scalar '{sid}' abs delta {abs_delta:.6g} exceeds threshold {threshold:.6g}"
                result["errors"].append(failure_reason)

        result["scalar_results"][sid] = {
            "abs_delta": abs_delta,
            "threshold": threshold,
            "passed": passed,
            "baseline": b_val,
            "current": c_val,
        }

        if not passed:
            failed_file, updated_file = write_scalar_failure_artifacts(
                baseline_file=baseline_file,
                baseline_value=b_val,
                current_value=c_val,
                abs_delta=abs_delta,
            )
            result["failure_artifact_files"].append(str(failed_file))
            result["updated_candidate_files"].append(str(updated_file))

    result["ok"] = len(result["errors"]) == 0
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description="Result-based test runner")
    parser.add_argument(
        "--config-file",
        action="append",
        default=[],
        help=("Limit loaded config YAML files (repeatable)"),
    )
    parser.add_argument("--output-dir", required=True, help="Output root for logs/plots/results")
    parser.add_argument("--case", action="append", default=[], help="Run only selected case name (repeatable)")
    parser.add_argument("--update-baseline", action="store_true", help="Update baseline CSV files from current run")
    args = parser.parse_args()

    ensure_runtime_deps()

    start_time = dt.datetime.now()
    config_dir = Path(__file__).resolve().parent
    output_dir = Path(args.output_dir).resolve()
    ensure_dir(output_dir)

    try:
        config, case_config_dirs, loaded_config_files = load_case_configs_from_directory(
            config_dir=config_dir,
            selected_config_files=args.config_file,
        )
    except (FileNotFoundError, ValueError, yaml.YAMLError) as exc:
        print(str(exc), file=sys.stderr)
        return 2

    if not config:
        print("No test cases found in config", file=sys.stderr)
        return 2

    selected = set(args.case)
    all_results = []

    selected_missing = sorted(selected - set(config.keys()))
    if selected_missing:
        print(f"Warning: unknown case(s): {', '.join(selected_missing)}", file=sys.stderr)

    for case_name, case_cfg in config.items():
        if selected and case_name not in selected:
            continue
        res = run_case(
            case_name=case_name,
            case_cfg=case_cfg,
            config_dir=case_config_dirs[case_name],
            output_dir=output_dir,
            update_baseline=args.update_baseline,
        )
        all_results.append(res)
        state = "PASS" if res.get("ok") else "FAIL"
        print(f"[{case_name}] {state}")
        for e in res.get("errors", []):
            print(f"  - {e}")
        for p in res.get("updated_candidate_files", []):
            print(f"  - updated baseline candidate: {p}")
        for p in res.get("failure_artifact_files", []):
            print(f"  - failure details: {p}")
        print()

    total_runs = len(all_results)
    passed_runs = sum(1 for r in all_results if r.get("ok"))
    failed_runs = total_runs - passed_runs
    elapsed_seconds = (dt.datetime.now() - start_time).total_seconds()

    print()
    print("----------------------------------------------------------")
    print("Summary\n")
    print(f"Elapsed time: {elapsed_seconds:.2f}s")
    print(f"Test results: {passed_runs} passed, {failed_runs} failed, {total_runs} total")

    summary = {
        "generated_at": dt.datetime.now().isoformat(),
        "config": str(config_dir),
        "loaded_config_files": [str(p) for p in loaded_config_files],
        "update_baseline": bool(args.update_baseline),
        "elapsed_seconds": elapsed_seconds,
        "total_runs": total_runs,
        "passed_runs": passed_runs,
        "failed_runs": failed_runs,
        "results": all_results,
        "ok": all(r.get("ok") for r in all_results) if all_results else False,
    }

    with open(output_dir / "summary.json", "w", encoding="utf-8") as fd:
        json.dump(summary, fd, indent=2)

    if args.update_baseline:
        return 0

    return 0 if summary["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
