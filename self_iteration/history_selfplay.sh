#!/usr/bin/env bash
set -u

ROOT="/wlf/Six/程序原版压缩包/six"
SELF_DIR="$ROOT/self_iteration"
QMAKE="/opt/Qt/6.11.1/gcc_64/bin/qmake"
RECORDS_DIR="/wlf/Six/程序原版压缩包/six/build/Desktop_Qt_6_11_1-Release/棋谱文件"
OUT="/tmp/opencode/history-ai-selfplay"
PREFIX=3
GAMES=1
PLIES=30

while (($#)); do
    case "$1" in
        --records-dir) RECORDS_DIR="$2"; shift 2 ;;
        --out) OUT="$2"; shift 2 ;;
        --prefix) PREFIX="$2"; shift 2 ;;
        --games) GAMES="$2"; shift 2 ;;
        --plies) PLIES="$2"; shift 2 ;;
        *) printf 'unknown option: %s\n' "$1" >&2; exit 2 ;;
    esac
done

if [[ ! -x "$QMAKE" || ! -d "$RECORDS_DIR" ]]; then
    printf 'qmake or records directory unavailable\n' >&2
    exit 2
fi

BUILD="/tmp/opencode/history-ai-selfplay-build"
mkdir -p "$BUILD" "$OUT"
if ! "$QMAKE" -spec linux-clang "$SELF_DIR/self_iteration.pro" -o "$BUILD/Makefile" CONFIG+=release CONFIG-=debug >/dev/null || ! make -C "$BUILD" -j4 >/dev/null; then
    printf 'self_iteration Release build failed\n' >&2
    exit 2
fi

MATCH_BUILD="/tmp/opencode/history-ai-selfplay-match-build"
mkdir -p "$MATCH_BUILD"
if ! "$QMAKE" -spec linux-clang "$SELF_DIR/match_runner.pro" -o "$MATCH_BUILD/Makefile" CONFIG+=release CONFIG-=debug >/dev/null || ! make -C "$MATCH_BUILD" -j4 >/dev/null; then
    printf 'match_runner Release build failed\n' >&2
    exit 2
fi

QT_QPA_PLATFORM=offscreen "$MATCH_BUILD/match_runner" \
    --records-dir "$RECORDS_DIR" --prefix "$PREFIX" --games "$GAMES" --plies "$PLIES" \
    --black-worker "$BUILD/self_iteration" --white-worker "$BUILD/self_iteration" --out "$OUT"
