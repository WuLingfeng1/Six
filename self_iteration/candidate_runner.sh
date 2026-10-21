#!/usr/bin/env bash
set -u

ROOT="/wlf/Six/程序原版压缩包/six"
SELF_DIR="$ROOT/self_iteration"
QMAKE="/opt/Qt/6.11.1/gcc_64/bin/qmake"
OUT_ROOT="$ROOT/self_iteration/reports"
GAMES=1
PLIES=5
OPENING_SET=0
CRITICAL_REGRESSION=1
MATCH_GAMES=0
MATCH_PLIES=5
MATCH_OPENING_SET=4
MATCH_HISTORY_FIXTURES=""
MATCH_RANDOMIZE=0
MATCH_SEED_BASE=1
SCAN=0
SCAN_SEED=1

while (($#)); do
    case "$1" in
        --games) GAMES="$2"; shift 2 ;;
        --plies) PLIES="$2"; shift 2 ;;
        --opening-set) OPENING_SET="$2"; shift 2 ;;
        --out-root) OUT_ROOT="$2"; shift 2 ;;
        --no-critical-regression) CRITICAL_REGRESSION=0; shift ;;
        --match-games) MATCH_GAMES="$2"; shift 2 ;;
        --match-plies) MATCH_PLIES="$2"; shift 2 ;;
        --match-opening-set) MATCH_OPENING_SET="$2"; shift 2 ;;
        --match-history-fixtures) MATCH_HISTORY_FIXTURES="$2"; shift 2 ;;
        --match-randomize) MATCH_RANDOMIZE=1; shift ;;
        --match-seed-base) MATCH_SEED_BASE="$2"; shift 2 ;;
        --scan) SCAN="$2"; shift 2 ;;
        --scan-seed) SCAN_SEED="$2"; shift 2 ;;
        *) printf 'unknown option: %s\n' "$1" >&2; exit 2 ;;
    esac
done

if [[ ! -x "$QMAKE" ]]; then
    printf 'qmake not executable: %s\n' "$QMAKE" >&2
    exit 2
fi
if [[ "$GAMES" -lt 1 || "$GAMES" -gt 40 || "$OPENING_SET" -lt 0 || "$OPENING_SET" -gt 8 ]]; then
    printf 'unsupported games or opening-set\n' >&2
    exit 2
fi
if [[ -n "$MATCH_HISTORY_FIXTURES" && "$MATCH_HISTORY_FIXTURES" != white15 && "$MATCH_HISTORY_FIXTURES" != anhui && "$MATCH_HISTORY_FIXTURES" != all ]]; then
    printf 'unsupported history fixture selection\n' >&2
    exit 2
fi
if [[ "$SCAN" -lt 0 || "$SCAN" -gt 40 ]]; then
    printf 'unsupported scan count\n' >&2
    exit 2
fi
if [[ "$SCAN" -gt 0 && "$MATCH_GAMES" -lt 1 ]]; then
    printf '--scan requires --match-games >= 1\n' >&2
    exit 2
fi
mkdir -p "$OUT_ROOT"
MATCH_BUILD="$OUT_ROOT/match-build"
mkdir -p "$MATCH_BUILD"
if ! "$QMAKE" -spec linux-clang "$SELF_DIR/match_runner.pro" -o "$MATCH_BUILD/Makefile" CONFIG+=release CONFIG-=debug >/dev/null || ! make -C "$MATCH_BUILD" -j4 >/dev/null; then
    printf 'match_runner build failed\n' >&2
    exit 2
fi

declare -a IDS=(baseline attack defense)
declare -a WEIGHTS=(
    '1,20,100,1000,3000,20000,60000,100000'
    '1,25,120,1200,3300,23000,65000,100000'
    '1,18,100,900,3300,22000,65000,100000'
)
declare -a SEARCH_PARAMS=(
    ''
    'maxDepth=8'
    'qMaxNodes=2048 qMaxPly=3'
)

if [[ "$SCAN" -gt 0 ]]; then
    eval "$(PYTHONPATH="$SELF_DIR" python3 - "$SCAN_SEED" "$SCAN" "$OUT_ROOT" << 'PYGEN'
import sys, json
from config_tools import DEFAULT_WEIGHTS, gen_candidates
seed = int(sys.argv[1]); n = int(sys.argv[2]); out_root = sys.argv[3]
specs = [{"id": "baseline", "weights": DEFAULT_WEIGHTS, "search_params": ""}]
specs += gen_candidates(n, seed)
ids = [x["id"] for x in specs]
weights = [x["weights"] for x in specs]
params = [x["search_params"] for x in specs]
with open(out_root + "/scan-specs.json", "w") as f:
    json.dump(specs, f, indent=2)
print("declare -a IDS=(%s)" % " ".join(ids))
print("declare -a WEIGHTS=(%s)" % " ".join("'%s'" % x for x in weights))
print("declare -a SEARCH_PARAMS=(%s)" % " ".join("'%s'" % x for x in params))
PYGEN
)"
fi

for i in "${!IDS[@]}"; do
    id="${IDS[$i]}"
    weights="${WEIGHTS[$i]}"
    search_params="${SEARCH_PARAMS[$i]}"
    dir="$OUT_ROOT/$id"
    src="$dir/src"
    build="$dir/build"
    report="$dir/report.json"
    rm -rf "$dir"
    mkdir -p "$src" "$build" "$dir/report-output"

    cp "$ROOT/Game.cpp" "$src/Game.cpp"
    cp "$ROOT/Game.h" "$src/Game.h"
    cp "$ROOT/Move.cpp" "$src/Move.cpp"
    cp "$ROOT/Move.h" "$src/Move.h"

    python3 "$SELF_DIR/config_tools.py" "$src/Game.h" --weights "$weights" --params "$search_params"
    replace_status=$?

    game_sha="$(sha256sum "$src/Game.cpp" | cut -d' ' -f1)"
    header_sha="$(sha256sum "$src/Game.h" | cut -d' ' -f1)"
    move_sha="$(sha256sum "$src/Move.cpp" | cut -d' ' -f1)"
    if [[ $replace_status -ne 0 ]]; then
        printf '{"id":"%s","weights":[%s],"status":"replacement_failed","source_sha256_game":"%s","source_sha256_header":"%s","source_sha256_move":"%s"}\n' "$id" "$weights" "$game_sha" "$header_sha" "$move_sha" > "$report"
        continue
    fi

    if "$QMAKE" -spec linux-clang "$SELF_DIR/self_iteration.pro" -o "$build/Makefile" "GAME_DIR=$src" CONFIG+=release CONFIG-=debug >/dev/null && make -C "$build" -j4 >/dev/null; then
        build_status=ok
    else
        build_status=failed
    fi
    if [[ "$build_status" == ok ]] && ! python3 "$SELF_DIR/verify_worker.py" "$build/self_iteration" "$src" > "$dir/manifest.json"; then
        build_status=failed
    fi
    if [[ "$build_status" == ok ]]; then
        critical_arg=()
        if [[ "$CRITICAL_REGRESSION" -eq 0 ]]; then critical_arg=(--no-critical-regression); fi
        if QT_QPA_PLATFORM=offscreen "$build/self_iteration" --mode baseline --games "$GAMES" --plies "$PLIES" --opening-set "$OPENING_SET" "${critical_arg[@]}" --out "$dir/report-output" --source-root "$src" --fixture-root "$ROOT" > "$dir/runner.stdout" 2> "$dir/runner.stderr"; then
            run_status=ok
        else
            run_status=failed
        fi
    else
        run_status=not_run
    fi

    if [[ -f "$dir/report-output/baseline.json" ]]; then
        cp "$dir/report-output/baseline.json" "$dir/baseline.json"
        cp "$dir/report-output/baseline.csv" "$dir/baseline.csv"
        cp "$dir/report-output/baseline.md" "$dir/baseline.md"
        printf '{"id":"%s","weights":[%s],"status":"%s","run_status":"%s","source_sha256_game":"%s","source_sha256_header":"%s","source_sha256_move":"%s","baseline_report":"baseline.json","baseline":' "$id" "$weights" "$build_status" "$run_status" "$game_sha" "$header_sha" "$move_sha" > "$report"
        cat "$dir/baseline.json" >> "$report"
        printf '}\n' >> "$report"
    else
        printf '{"id":"%s","weights":[%s],"status":"%s","run_status":"%s","source_sha256_game":"%s","source_sha256_header":"%s","source_sha256_move":"%s"}\n' "$id" "$weights" "$build_status" "$run_status" "$game_sha" "$header_sha" "$move_sha" > "$report"
    fi
    if [[ "$id" == baseline && "$run_status" != ok ]]; then
        printf 'baseline validation failed; stopping candidate comparisons\n' >&2
        exit 4
    fi
    if [[ "$run_status" == ok && "$id" != baseline && "$MATCH_GAMES" -gt 0 && -x "$OUT_ROOT/baseline/build/self_iteration" && -x "$build/self_iteration" ]]; then
        match_dir="$dir/match"
        mkdir -p "$match_dir"
        match_fixture_args=()
        if [[ -n "$MATCH_HISTORY_FIXTURES" ]]; then match_fixture_args=(--history-fixtures "$MATCH_HISTORY_FIXTURES"); fi
        match_randomize_args=()
        if [[ "$MATCH_RANDOMIZE" -eq 1 ]]; then match_randomize_args=(--randomize --seed-base "$MATCH_SEED_BASE"); fi
        if QT_QPA_PLATFORM=offscreen "$MATCH_BUILD/match_runner" --black-worker "$OUT_ROOT/baseline/build/self_iteration" --white-worker "$build/self_iteration" --games "$MATCH_GAMES" --plies "$MATCH_PLIES" --opening-set "$MATCH_OPENING_SET" "${match_fixture_args[@]}" "${match_randomize_args[@]}" --out "$match_dir/match.json" > "$match_dir/stdout" 2> "$match_dir/stderr"; then
            printf '%s: match ok\n' "$id"
        else
            printf '%s: match failed\n' "$id" >&2
        fi
    fi
    printf '%s: %s/%s\n' "$id" "$build_status" "$run_status"
done

if [[ "$SCAN" -gt 0 ]]; then
    python3 - "$OUT_ROOT" << 'PYRANK'
import csv, json, sys, os
out_root = sys.argv[1]
specs = json.load(open(os.path.join(out_root, "scan-specs.json")))
spec = {s["id"]: s for s in specs}
cand_ids = [s["id"] for s in specs if s["id"] != "baseline"]

def read(path):
    with open(path) as f:
        return list(csv.DictReader(f))

def count(rows, scenario, winner):
    return sum(1 for r in rows if r["scenario"] == scenario and r["winner"] == str(winner))

def n(rows, scenario):
    return sum(1 for r in rows if r["scenario"] == scenario)

control_path = None
for cid in cand_ids:
    p = os.path.join(out_root, cid, "match", "match.csv")
    if os.path.isfile(p):
        control_path = p; break
if control_path is None:
    print("# scan ranking: no matches found")
    sys.exit(0)

control = read(control_path)
cb = count(control, "control", 1)
cw = count(control, "control", 2)
cg = n(control, "control")

ranking = []
for cid in cand_ids:
    p = os.path.join(out_root, cid, "match", "match.csv")
    if not os.path.isfile(p):
        continue
    rows = read(p)
    if any(r["result"] not in ("black_win", "white_win", "unfinished", "draw") for r in rows):
        continue
    b = count(rows, "candidate-black", 1)
    w = count(rows, "candidate-white", 2)
    g = n(rows, "candidate-black")
    net = (b - count(rows, "control", 1)) + (w - count(rows, "control", 2))
    ranking.append((net, cid, b, w, g))
ranking.sort(key=lambda t: (-t[0], t[1]))

csv_path = os.path.join(out_root, "scan-ranking.csv")
with open(csv_path, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["rank", "id", "net_vs_baseline", "black_wins", "white_wins", "games", "weights", "search_params"])
    for rank, (net, cid, b, w, g) in enumerate(ranking, 1):
        s = spec[cid]
        writer.writerow([rank, cid, net, b, w, g, s["weights"], s["search_params"].replace(" ", "|")])

md_path = os.path.join(out_root, "scan-ranking.md")
with open(md_path, "w") as f:
    f.write("# 参数扫描排名\n\n")
    f.write("- 每个候选使用自己的 control 对照计算净胜局，详情见各自 match.json。\n")
    f.write("- net = (候选做黑多赢的局) + (候选做白多赢的局)；正数表示本组样本净增胜局，不代表统计显著。\n\n")
    f.write("| rank | id | net | 做黑胜 | 做白胜 | weights | search_params |\n")
    f.write("| --- | --- | --- | --- | --- | --- | --- |\n")
    for rank, (net, cid, b, w, g) in enumerate(ranking, 1):
        s = spec[cid]
        f.write("| %d | %s | %+d | %d | %d | `%s` | `%s` |\n" % (rank, cid, net, b, w, s["weights"], s["search_params"]))
print("scan ranking written to %s and %s" % (csv_path, md_path))
PYRANK
fi
