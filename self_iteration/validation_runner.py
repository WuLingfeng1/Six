#!/usr/bin/env python3
"""Run paired opening/midgame validation serially with frozen inputs and seeds."""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import subprocess

from experiment_board import load_suite
from summarize_match import summarize


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n")
    temporary.replace(path)


def write_summary(out, runs):
    cohorts = {}
    for run in runs:
        kind = "midgame" if run["mode"].endswith("midgames") else "opening"
        cohort = cohorts.setdefault(kind, {"net": 0, "verified_games": 0, "seed_bases": [],
                                          "per_position": {}, "outcome_transitions": {}})
        cohort["net"] += run["net"]
        cohort["verified_games"] += run["verified_games"]
        cohort["seed_bases"].append(run["seed_base"])
        for key, values in run["per_position"].items():
            row = cohort["per_position"].setdefault(key, Counter())
            row.update(values)
        for pair in run["paired_outcomes"]:
            key = pair["candidate_side"] + ":" + pair["control_result"] + "->" + pair["candidate_result"]
            cohort["outcome_transitions"][key] = cohort["outcome_transitions"].get(key, 0) + 1
    summary = {"runs": runs, "cohorts": cohorts,
               "verified_games": sum(r["verified_games"] for r in runs),
               "note": "Cohorts are reported separately; unfinished is not a draw; no significance claim."}
    write_json(out / "summary.json", summary)
    lines = ["# Extended paired validation", "", "| cohort | seed | verified games | net black | net white | net | unfinished |",
             "| --- | ---: | ---: | ---: | ---: | ---: | ---: |"]
    for r in runs:
        unfinished = sum(s.get("unfinished", 0) for s in r["scenarios"].values())
        kind = "midgame" if r["mode"].endswith("midgames") else "opening"
        lines.append("| %s | %d | %d | %+d | %+d | %+d | %d |" %
                     (kind, r["seed_base"], r["verified_games"], r["net_black"], r["net_white"], r["net"], unfinished))
    lines += ["", "Opening and midgame cohorts are separate. Net is a win-count difference against paired controls, not a win rate or statistical significance result.", ""]
    (out / "summary.md").write_text("\n".join(lines))


def check_report_plan(data, task, configs, suite, plies):
    expected_mode = "baseline-vs-candidate" + ("-midgames" if task["kind"] == "midgame" else "")
    if (data["mode"] != expected_mode or data["plies"] != plies or data["randomize"] is not True
            or data["seed_base"] != task["seed"] or data["games"] != task["count"]):
        raise ValueError("report settings differ from frozen plan")
    if task["kind"] == "midgame":
        if data["position_suite"] != suite or data["games_per_position"] != 1:
            raise ValueError("report midgame suite differs from plan")
    elif data["opening_set"] != 8:
        raise ValueError("report opening set differs from plan")
    for name in ("baseline", "candidate"):
        actual = data["workers"][name]
        expected = dict(configs[name]["search_config"], randomize=True, randomSeed=task["seed"])
        if (actual["search_config"] != expected
                or actual["compiled_sources"] != configs[name]["compiled_sources"]
                or actual["tt_effective"] != configs[name]["tt_effective"]):
            raise ValueError("report worker configuration/source differs from plan")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--match-runner", type=Path, required=True)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--suite", type=Path, default=Path(__file__).with_name("midgames") / "suite.json")
    parser.add_argument("--opening-seeds", type=int, nargs="+", default=[1101, 1201])
    parser.add_argument("--midgame-seeds", type=int, nargs="+", default=[2101, 2201])
    parser.add_argument("--plies", type=int, default=30)
    parser.add_argument("--out-root", type=Path, required=True)
    args = parser.parse_args()
    if not 1 <= args.plies <= 30:
        parser.error("--plies must be 1..30")
    suite = load_suite(args.suite)
    count = len(suite["positions"])
    schedule = ([{"kind": "opening", "seed": seed, "count": 8} for seed in args.opening_seeds]
                + [{"kind": "midgame", "seed": seed, "count": count} for seed in args.midgame_seeds])
    used = set()
    for task in schedule:
        seeds = set(range(task["seed"], task["seed"] + task["count"]))
        if task["seed"] < 1 or max(seeds) > 2147483647 or used & seeds:
            parser.error("seed ranges must be positive and nonoverlapping")
        used.update(seeds)
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    inputs = {name: {"path": str(path.resolve()), "sha256": digest(path)} for name, path in
              (("referee", args.match_runner), ("baseline", args.baseline), ("candidate", args.candidate), ("suite", args.suite))}
    configs = {}
    for name, binary in (("baseline", args.baseline), ("candidate", args.candidate)):
        result = subprocess.run([str(binary.resolve()), "--mode", "config"], check=True,
                                capture_output=True, text=True, env=env)
        configs[name] = json.loads(result.stdout)
        if configs[name]["search_config"]["timeLimitMs"] != 2000:
            parser.error("strength validation requires the standard 2000ms budget on both sides")
    plan = {"inputs": inputs, "workers": configs, "schedule": schedule, "plies": args.plies,
            "games_requested": sum(t["count"] * 3 for t in schedule), "randomize": True,
            "selection_uses_results": False, "serial_search": True}
    out = args.out_root.resolve()
    out.mkdir(parents=True, exist_ok=True)
    plan_path = out / "plan.json"
    if plan_path.exists() and json.loads(plan_path.read_text()) != plan:
        parser.error("existing plan differs; choose a fresh output directory")
    write_json(plan_path, plan)
    runs = []
    for task in schedule:
        label = task["kind"] + "-" + str(task["seed"])
        directory = out / label
        directory.mkdir(exist_ok=True)
        report = directory / "match.json"
        write_json(out / "progress.json", {"current": label, "completed_groups": len(runs), "total_groups": len(schedule)})
        for item in inputs.values():
            if digest(Path(item["path"])) != item["sha256"]:
                raise RuntimeError("input changed during validation: " + item["path"])
        if not report.exists():
            command = [str(args.match_runner.resolve()), "--black-worker", str(args.baseline.resolve()),
                       "--white-worker", str(args.candidate.resolve()), "--randomize", "--seed-base", str(task["seed"]),
                       "--plies", str(args.plies), "--out", str(report)]
            if task["kind"] == "opening":
                command += ["--games", "8", "--opening-set", "8"]
            else:
                command += ["--games", "1", "--position-suite", str(args.suite.resolve())]
            print("running", label, flush=True)
            with (directory / "stdout.log").open("w") as stdout, (directory / "stderr.log").open("w") as stderr:
                subprocess.run(command, check=True, env=env, stdout=stdout, stderr=stderr)
        check_report_plan(json.loads(report.read_text()), task, configs, suite, args.plies)
        verified = summarize(report)
        if verified["seed_base"] != task["seed"] or verified["verified_games"] != task["count"] * 3:
            raise RuntimeError("report does not match frozen schedule")
        if task["kind"] == "midgame" and verified["position_suite_sha256"] != inputs["suite"]["sha256"]:
            raise RuntimeError("report suite differs from frozen input")
        runs.append(verified)
        write_summary(out, runs)
        print(label, "verified", verified["verified_games"], "net", verified["net"], flush=True)
    write_json(out / "progress.json", {"completed_groups": len(runs), "total_groups": len(schedule), "complete": True})


if __name__ == "__main__":
    main()
