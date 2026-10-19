#!/usr/bin/env bash
set -u

ROOT="/wlf/Six/程序原版压缩包/six"
SELF_DIR="$ROOT/self_iteration"
QMAKE="/opt/Qt/6.11.1/gcc_64/bin/qmake"
OUT_ROOT="/tmp/six-candidates"
GAMES=1
PLIES=5
OPENING_SET=0
CRITICAL_REGRESSION=1
MATCH_GAMES=0
MATCH_PLIES=5
MATCH_OPENING_SET=4
MATCH_HISTORY_FIXTURES=""

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
        *) printf 'unknown option: %s\n' "$1" >&2; exit 2 ;;
    esac
done

if [[ ! -x "$QMAKE" ]]; then
    printf 'qmake not executable: %s\n' "$QMAKE" >&2
    exit 2
fi
if [[ "$GAMES" -lt 1 || "$GAMES" -gt 40 || "$OPENING_SET" != 0 && "$OPENING_SET" != 1 && "$OPENING_SET" != 2 && "$OPENING_SET" != 4 ]]; then
    printf 'unsupported games or opening-set\n' >&2
    exit 2
fi
if [[ -n "$MATCH_HISTORY_FIXTURES" && "$MATCH_HISTORY_FIXTURES" != white15 && "$MATCH_HISTORY_FIXTURES" != anhui && "$MATCH_HISTORY_FIXTURES" != all ]]; then
    printf 'unsupported history fixture selection\n' >&2
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
    '0,1,20,300,2000,6000,100000'
    '0,1,25,350,2300,6500,100000'
    '0,1,18,280,2200,6500,100000'
)

for i in "${!IDS[@]}"; do
    id="${IDS[$i]}"
    weights="${WEIGHTS[$i]}"
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

    WEIGHTS="$weights" perl -0pi -e '
        my $r = $ENV{"WEIGHTS"};
        my $my = s/(int scoreOfMyRoad\[7\]\{ )0,1,20,300,2000,6000,100000( \};)/$1$r$2/;
        my $enemy = s/(int scoreOfEnemyRoad\[7\]\{ )0,1,20,300,2000,6000,100000( \};)/$1$r$2/;
        END { exit 1 unless $my == 1 && $enemy == 1; }
    ' "$src/Game.h"
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
    if [[ "$id" != baseline && "$MATCH_GAMES" -gt 0 && -x "$OUT_ROOT/baseline/build/self_iteration" && -x "$build/self_iteration" ]]; then
        match_dir="$dir/match"
        mkdir -p "$match_dir"
        match_fixture_args=()
        if [[ -n "$MATCH_HISTORY_FIXTURES" ]]; then match_fixture_args=(--history-fixtures "$MATCH_HISTORY_FIXTURES"); fi
        if QT_QPA_PLATFORM=offscreen "$MATCH_BUILD/match_runner" --black-worker "$OUT_ROOT/baseline/build/self_iteration" --white-worker "$build/self_iteration" --games "$MATCH_GAMES" --plies "$MATCH_PLIES" --opening-set "$MATCH_OPENING_SET" "${match_fixture_args[@]}" --out "$match_dir/match.json" > "$match_dir/stdout" 2> "$match_dir/stderr"; then
            printf '%s: match ok\n' "$id"
        else
            printf '%s: match failed\n' "$id" >&2
        fi
    fi
    printf '%s: %s/%s\n' "$id" "$build_status" "$run_status"
done
