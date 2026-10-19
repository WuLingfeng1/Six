# Backward Analysis

`backward_analysis` is an offline tool in `self_iteration`. It compiles the production `Game.cpp` and `Move.cpp` read-only and does not modify `Game`, `Move`, `six.pro`, tests, client/server code, or records.

It analyzes the six independently verified Chongshi losses from the history report. For each loss it restores the historical prefix and scans Chongshi turns backwards. The default selection is the last 10 own turns intersecting the five-turn window ending at the report's known refuted turn. `--max-turns` and `--window` change those limits.

Every legal double-stone move that covers every current opponent direct completion is an exact cover. Each exact cover is checked with a complete legal opponent double-stone enumeration, with `unknown` on timeout. The production move is recorded separately. `safe_one_reply` means only that reply layer was completed; it is not a global win and is never treated as an unavoidable-win proof.

## Build and run

```bash
mkdir -p /tmp/opencode/six-backward-analysis
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang \
  /wlf/Six/程序原版压缩包/six/self_iteration/backward_analysis.pro \
  -o /tmp/opencode/six-backward-analysis/Makefile CONFIG+=release CONFIG-=debug
make -C /tmp/opencode/six-backward-analysis -j4
mkdir -p /tmp/opencode/six-backward-analysis/report
QT_QPA_PLATFORM=offscreen /tmp/opencode/six-backward-analysis/backward_analysis \
  --max-turns 10 --window 5 --out /tmp/opencode/six-backward-analysis/report
```

The output is `backward_analysis.json` and `backward_analysis.md`. JSON includes the source hashes, configuration hash, restored prefix, historical follow-up, production timing/depth fields, candidate counts, witnesses, and unknown counts. The default oracle budget is 2000 ms per candidate and can be changed with `--oracle-ms`.
