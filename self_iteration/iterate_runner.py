#!/usr/bin/env python3
"""Closed-loop parameter iteration for the six offline engine.

Each round:
  1. Generate `--scan` random candidates (perturbed weights + search params).
  2. Build each candidate (isolated copy of the production sources).
  3. Match each candidate against the CURRENT baseline (independent referee).
  4. Rank by net wins vs baseline; the best candidate with net > 0 becomes
     the baseline for the next round.

Writes reports/iterate/iterate-history.{json,md,csv}.
"""
import argparse
import csv
import json
import os
import random
import re
import shutil
import subprocess
import sys

ROOT = "/wlf/Six/程序原版压缩包/six"
SELF_DIR = os.path.join(ROOT, "self_iteration")
QMAKE = "/opt/Qt/6.11.1/gcc_64/bin/qmake"
ITER_ROOT = os.path.join(SELF_DIR, "reports", "iterate")
BASE_WEIGHTS = "0,1,20,300,2000,6000,100000"
WEIGHT_ANCHOR = "0,1,20,300,2000,6000,100000".encode()


def run(cmd, **kw):
    return subprocess.run(cmd, check=True, **kw)


def build_candidate(dest, weights, params_str):
    """Copy production sources into dest, bake weights+params, build Release."""
    src = os.path.join(dest, "src")
    bld = os.path.join(dest, "build")
    shutil.rmtree(dest, ignore_errors=True)
    os.makedirs(src)
    os.makedirs(bld)
    for f in ("Game.cpp", "Game.h", "Move.cpp", "Move.h"):
        shutil.copy(os.path.join(ROOT, f), os.path.join(src, f))

    gh_path = os.path.join(src, "Game.h")
    with open(gh_path, "rb") as fh:
        gh = fh.read()
    if gh.count(WEIGHT_ANCHOR) != 2:
        raise RuntimeError("weight anchor not found twice in Game.h")
    gh = gh.replace(WEIGHT_ANCHOR, weights.encode())
    for kv in params_str.split():
        if not kv:
            continue
        key, val = kv.split("=", 1)
        pat = re.compile(rb"(\b" + key.encode() + rb"\s*=\s*)\d+")
        gh, n = pat.subn(lambda m: m.group(1) + val.encode(), gh)
        if n != 1:
            raise RuntimeError(f"param {key} found {n} times")
    with open(gh_path, "wb") as fh:
        fh.write(gh)

    run([QMAKE, "-spec", "linux-clang", os.path.join(SELF_DIR, "self_iteration.pro"),
         "-o", os.path.join(bld, "Makefile"), f"GAME_DIR={src}",
         "CONFIG+=release", "CONFIG-=debug"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    run(["make", "-C", bld, "-j4"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return os.path.join(bld, "self_iteration")


def gen_candidates(n, seed):
    random.seed(seed)
    base_w = [0, 1, 20, 300, 2000, 6000, 100000]
    specs = []
    for i in range(1, n + 1):
        w = list(base_w)
        for k in (1, 2, 3, 4, 5):
            w[k] = max(1, int(round(base_w[k] * (2.0 ** random.uniform(-1.0, 1.0)))))
        wstr = ",".join(map(str, w))
        sp = " ".join([
            "maxDepth=%d" % random.choice([5, 6, 7, 8]),
            "qMaxNodes=%d" % random.choice([512, 1024, 2048, 4096]),
            "qMaxPly=%d" % random.choice([1, 2, 3]),
        ])
        specs.append({"id": "cand%d" % i, "weights": wstr, "search_params": sp})
    return specs


def run_match(match_runner, baseline_bin, cand_bin, games, plies, opening_set, seed_base, out):
    env = dict(os.environ)
    env["QT_QPA_PLATFORM"] = "offscreen"
    run([match_runner, "--black-worker", baseline_bin, "--white-worker", cand_bin,
         "--games", str(games), "--plies", str(plies), "--opening-set", str(opening_set),
         "--randomize", "--seed-base", str(seed_base), "--out", out],
        env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def parse_match(csv_path):
    with open(csv_path) as f:
        rows = list(csv.DictReader(f))

    def count(scenario, winner):
        return sum(1 for r in rows if r["scenario"] == scenario and r["winner"] == str(winner))

    cb = count("control", 1)
    cw = count("control", 2)
    b = count("candidate-black", 1)
    w = count("candidate-white", 2)
    net = (b - cb) + (w - cw)
    return {"net": net, "black_wins": b, "white_wins": w, "control_black": cb, "control_white": cw}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--iterate", type=int, default=3, help="rounds (1..20)")
    ap.add_argument("--scan", type=int, default=5, help="random candidates per round (1..40)")
    ap.add_argument("--scan-seed", type=int, default=1)
    ap.add_argument("--match-games", type=int, default=4)
    ap.add_argument("--match-plies", type=int, default=30)
    ap.add_argument("--match-opening-set", type=int, default=4)
    ap.add_argument("--match-seed-base", type=int, default=1)
    ap.add_argument("--baseline-weights", default=BASE_WEIGHTS)
    ap.add_argument("--baseline-params", default="")
    ap.add_argument("--out-root", default=ITER_ROOT)
    a = ap.parse_args()

    if not (1 <= a.iterate <= 20):
        sys.exit("--iterate must be 1..20")
    if not (1 <= a.scan <= 40):
        sys.exit("--scan must be 1..40")

    os.makedirs(a.out_root, exist_ok=True)

    # Build match_runner once.
    match_build = os.path.join(a.out_root, "match-build")
    os.makedirs(match_build, exist_ok=True)
    run([QMAKE, "-spec", "linux-clang", os.path.join(SELF_DIR, "match_runner.pro"),
         "-o", os.path.join(match_build, "Makefile"),
         "CONFIG+=release", "CONFIG-=debug"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    run(["make", "-C", match_build, "-j4"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    match_runner = os.path.join(match_build, "match_runner")

    # Round-0 baseline.
    baseline_dir = os.path.join(a.out_root, "baseline")
    cur_bin = build_candidate(baseline_dir, a.baseline_weights, a.baseline_params)
    cur_weights = a.baseline_weights
    cur_params = a.baseline_params
    cur_origin = "start"

    history = []
    for rnd in range(a.iterate):
        round_dir = os.path.join(a.out_root, "round%d" % rnd)
        specs = gen_candidates(a.scan, a.scan_seed + rnd)

        results = []
        for sp in specs:
            cand_dir = os.path.join(round_dir, sp["id"])
            try:
                cand_bin = build_candidate(cand_dir, sp["weights"], sp["search_params"])
                match_dir = os.path.join(cand_dir, "match")
                os.makedirs(match_dir, exist_ok=True)
                out_json = os.path.join(match_dir, "match.json")
                run_match(match_runner, cur_bin, cand_bin, a.match_games, a.match_plies,
                          a.match_opening_set, a.match_seed_base, out_json)
                stats = parse_match(os.path.join(match_dir, "match.csv"))
                stats.update({"id": sp["id"], "weights": sp["weights"],
                              "search_params": sp["search_params"], "status": "ok"})
            except Exception as e:  # noqa: BLE001
                stats = {"id": sp["id"], "weights": sp["weights"],
                         "search_params": sp["search_params"], "status": "failed",
                         "net": None, "error": str(e)}
            results.append(stats)
            print("  round %d %s: %s net=%s" % (rnd, sp["id"], stats["status"], stats.get("net")))

        ok = [r for r in results if r.get("status") == "ok"]
        winner = None
        if ok:
            winner = max(ok, key=lambda r: r["net"])
        improved = winner is not None and winner["net"] > 0

        round_rec = {
            "round": rnd,
            "baseline_weights": cur_weights,
            "baseline_params": cur_params,
            "baseline_origin": cur_origin,
            "improved": improved,
            "winner": winner["id"] if improved else None,
            "winner_net": winner["net"] if improved else None,
            "results": results,
        }
        history.append(round_rec)

        if improved:
            winner_bin = os.path.join(round_dir, winner["id"], "build", "self_iteration")
            cur_bin = winner_bin
            cur_weights = winner["weights"]
            cur_params = winner["search_params"]
            cur_origin = "round%d:%s" % (rnd, winner["id"])
            print("  round %d baseline -> %s (net %+d, %s / %s)" % (
                rnd, winner["id"], winner["net"], winner["weights"], winner["search_params"]))
        else:
            print("  round %d no improvement (best net %s), baseline unchanged" % (
                rnd, winner["net"] if winner else None))

    write_report(a.out_root, history, a)


def write_report(out_root, history, args):
    with open(os.path.join(out_root, "iterate-history.json"), "w") as f:
        json.dump({"args": vars(args), "history": history}, f, indent=2, ensure_ascii=False)

    with open(os.path.join(out_root, "iterate-history.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["round", "baseline_weights", "baseline_params", "improved",
                    "winner", "winner_net", "winner_weights", "winner_params"])
        for r in history:
            win = next((x for x in r["results"] if x["id"] == r["winner"]), {}) if r["winner"] else {}
            w.writerow([r["round"], r["baseline_weights"], r["baseline_params"], r["improved"],
                        r["winner"], r["winner_net"], win.get("weights", ""), win.get("search_params", "")])

    lines = ["# 迭代历史", ""]
    lines.append("- 起始 baseline：`%s`（%s）" % (args.baseline_weights, args.baseline_params or "默认参数"))
    lines.append("- 每轮 %d 个随机候选，各 %d 场对打（3 场景）" % (args.scan, args.match_games))
    lines.append("")
    lines.append("| round | baseline | 是否提升 | 胜者 | net | 胜者权重 | 胜者参数 |")
    lines.append("| --- | --- | --- | --- | --- | --- | --- |")
    for r in history:
        win = next((x for x in r["results"] if x["id"] == r["winner"]), {}) if r["winner"] else {}
        lines.append("| %d | %s | %s | %s | %s | `%s` | `%s` |" % (
            r["round"],
            "`%s` %s" % (r["baseline_weights"], r["baseline_params"]) if r["baseline_params"] else "`%s`" % r["baseline_weights"],
            "是" if r["improved"] else "否",
            r["winner"] or "-",
            ("%+d" % r["winner_net"]) if r["winner_net"] is not None else "-",
            win.get("weights", ""),
            win.get("search_params", ""),
        ))
    with open(os.path.join(out_root, "iterate-history.md"), "w") as f:
        f.write("\n".join(lines) + "\n")
    print("iteration report written to", out_root)


if __name__ == "__main__":
    main()
