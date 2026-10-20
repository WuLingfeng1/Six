# Tactics suite

Independent one-reply tactical checks for the standalone engine. This is **not**
a game win-rate and does not prove a forced win; it classifies the engine's
chosen move against an exhaustive opponent-pair enumeration (the same oracle as
`tests/history_analysis.cpp`).

## Files

- `suite.json` — 45 positions extracted from the history analysis
  (`tests/history_reports/history_details.json`) plus the baseline, Anqing,
  white15 and turn17 fixtures. Each entry has `file`, `stones`, `color`,
  `expected`, and a note.
- Expected values:
  - `direct_win`: after the move the engine has made six (sanity control).
  - `safe_one_reply`: the move leaves no opponent direct completion and the
    exhaustive one-reply oracle finds no refutation.
  - `unavoidable`: the position already has an uncoverable opponent direct
    threat; counted as a control, not a pass/fail.

## Run

Build `self_iteration` normally (`self_iteration/self_iteration.pro`), then:

```bash
ROOT=/wlf/Six/程序原版压缩包/six
mkdir -p $ROOT/self_iteration/reports/tactics
cd $ROOT
QT_QPA_PLATFORM=offscreen self_iteration/reports/build/self_iteration \
  --mode tactics --time-limit 2000 --oracle-ms 1000 \
  --out self_iteration/reports/tactics/current
```

The mode writes `tactics.json` and `tactics.csv`. The JSON summary has
`pass`, `fail`, `unknown`, `lost`, `control`, `invalid`. `lost` means the
engine's chosen move scored `-WIN_SCORE`: the root exact-defense probe found all
covers refuted, so it intentionally played a losing move rather than a refuted
safe-looking one. Such positions are reported separately and are not counted as
ordinary failures.

## Comparing engines

The runner is compiled with the engine (`GAME_DIR=...`), so build one binary per
engine source tree, e.g. the pre-change source:

```bash
mkdir -p self_iteration/reports/tactics-old-src
for f in Game.cpp Game.h Move.cpp Move.h; do
  git show <commit>:$f > self_iteration/reports/tactics-old-src/$f
done
mkdir -p self_iteration/reports/build-tactics-old
cd self_iteration/reports/build-tactics-old
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang \
  "$ROOT/self_iteration/self_iteration.pro" \
  "GAME_DIR=$ROOT/self_iteration/reports/tactics-old-src" CONFIG+=release CONFIG-=debug
make -j8
```

Then run both with the same `--suite` / `--time-limit` and diff
`tactics.json` (`positions[].move`, `status`, `result`, `completed_depth`).

## Limits

- A `refuted` move only means the one-reply oracle found a counterexample; it
  does not prove every move loses.
- The history positions marked `unavoidable` are excluded from scoring.
- The suite was insensitive to the 2.3x evaluation speedup (the pre-change and
  post-change engines produced identical moves/statuses on all 45 positions), so
  it is a safety/regression set, not a depth benchmark.
