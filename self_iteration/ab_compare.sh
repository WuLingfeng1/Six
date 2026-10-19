#!/usr/bin/env bash
# A/B 对比：当前工作区版本 vs git 历史某个 commit 的版本
#
# 从 git 历史提取旧版 Game.cpp/Game.h/Move.cpp/Move.h 并构建 worker，
# 与当前源码构建的新版 worker 对打，输出 net（新版相对旧版的净胜局）。
#
# 用法：
#   ./ab_compare.sh --old-commit HEAD~1 --games 8 --plies 30 --seed-base 1
set -u

ROOT="/wlf/Six/程序原版压缩包/six"
SELF_DIR="$ROOT/self_iteration"
QMAKE="/opt/Qt/6.11.1/gcc_64/bin/qmake"
OUT_ROOT="$ROOT/self_iteration/reports/ab-compare"
OLD_COMMIT="HEAD~1"
GAMES=8
PLIES=30
OPENING_SET=4
SEED_BASE=1

while (($#)); do
    case "$1" in
        --old-commit) OLD_COMMIT="$2"; shift 2 ;;
        --games) GAMES="$2"; shift 2 ;;
        --plies) PLIES="$2"; shift 2 ;;
        --opening-set) OPENING_SET="$2"; shift 2 ;;
        --seed-base) SEED_BASE="$2"; shift 2 ;;
        --out-root) OUT_ROOT="$2"; shift 2 ;;
        *) printf 'unknown option: %s\n' "$1" >&2; exit 2 ;;
    esac
done

if [[ ! -x "$QMAKE" ]]; then
    printf 'qmake not executable: %s\n' "$QMAKE" >&2
    exit 2
fi

mkdir -p "$OUT_ROOT"

# 1. 构建 match_runner
MATCH_BUILD="$OUT_ROOT/match-build"
mkdir -p "$MATCH_BUILD"
if ! "$QMAKE" -spec linux-clang "$SELF_DIR/match_runner.pro" -o "$MATCH_BUILD/Makefile" CONFIG+=release CONFIG-=debug >/dev/null || ! make -C "$MATCH_BUILD" -j4 >/dev/null; then
    printf 'match_runner build failed\n' >&2
    exit 2
fi

# 2. 提取旧版源码并构建 worker
OLD_SRC="$OUT_ROOT/old-src"
OLD_BUILD="$OUT_ROOT/old-build"
rm -rf "$OLD_SRC" "$OLD_BUILD"
mkdir -p "$OLD_SRC" "$OLD_BUILD"
for f in Game.cpp Game.h Move.cpp Move.h; do
    if ! git -C "$ROOT" show "$OLD_COMMIT:$f" > "$OLD_SRC/$f" 2>/dev/null; then
        printf 'failed to extract %s from %s\n' "$f" "$OLD_COMMIT" >&2
        exit 2
    fi
done
if ! "$QMAKE" -spec linux-clang "$SELF_DIR/self_iteration.pro" -o "$OLD_BUILD/Makefile" "GAME_DIR=$OLD_SRC" CONFIG+=release CONFIG-=debug >/dev/null || ! make -C "$OLD_BUILD" -j4 >/dev/null; then
    printf 'old worker build failed\n' >&2
    exit 2
fi

# 3. 用当前工作区源码构建新版 worker
NEW_BUILD="$OUT_ROOT/new-build"
rm -rf "$NEW_BUILD"
mkdir -p "$NEW_BUILD"
if ! "$QMAKE" -spec linux-clang "$SELF_DIR/self_iteration.pro" -o "$NEW_BUILD/Makefile" CONFIG+=release CONFIG-=debug >/dev/null || ! make -C "$NEW_BUILD" -j4 >/dev/null; then
    printf 'new worker build failed\n' >&2
    exit 2
fi

# 4. 对打（旧版=black-worker=baseline，新版=white-worker=candidate）
MATCH_DIR="$OUT_ROOT/match"
mkdir -p "$MATCH_DIR"
if ! QT_QPA_PLATFORM=offscreen "$MATCH_BUILD/match_runner" \
    --black-worker "$OLD_BUILD/self_iteration" \
    --white-worker "$NEW_BUILD/self_iteration" \
    --games "$GAMES" --plies "$PLIES" --opening-set "$OPENING_SET" \
    --randomize --seed-base "$SEED_BASE" \
    --out "$MATCH_DIR/match.json" > "$MATCH_DIR/stdout" 2> "$MATCH_DIR/stderr"; then
    printf 'match failed\n' >&2
    exit 2
fi

# 5. 解析结果
python3 - "$MATCH_DIR/match.csv" "$OLD_COMMIT" "$GAMES" << 'PYRANK'
import csv, sys
csv_path, old_commit, games = sys.argv[1], sys.argv[2], int(sys.argv[3])
rows = list(csv.DictReader(open(csv_path)))
def count(sc, w): return sum(1 for r in rows if r["scenario"] == sc and r["winner"] == str(w))
def n(sc): return sum(1 for r in rows if r["scenario"] == sc)
cb, cw = count("control", 1), count("control", 2)
b, w = count("candidate-black", 1), count("candidate-white", 2)
net = (b - cb) + (w - cw)
print("=== A/B 对比（旧版 %s 为 baseline，当前版为候选）===" % old_commit)
print("control（旧版自对）: 黑胜 %d / 白胜 %d / 未完成 %d" % (cb, cw, n("control") - cb - cw))
print("当前版做黑 vs 旧版做白: 黑胜 %d" % b)
print("当前版做白 vs 旧版做黑: 白胜 %d" % w)
print("net = (做黑 %+d) + (做白 %+d) = %+d" % (b - cb, w - cw, net))
print("（正数 = 当前版更强；样本 %d 局/场景）" % games)
PYRANK
