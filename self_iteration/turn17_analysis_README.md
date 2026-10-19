# Turn 17 Analysis

`turn17_analysis` restores prefix31 of the real record
`C6-sixBvshumanW-先手胜-06.txt`, where the next side is black. It records the
current production `Game::bestPlay()` move, calls production
`generateCandidates(B, 40, 25)` after `rebuildSearch()`, and analyzes every
returned root candidate. The historical next black move is recorded separately
and is included even if it is outside the production top 25.

The independent referee enumerates every legal white double-stone reply for
each candidate. A reply is unsafe if white makes an immediate six, or if the
resulting white direct-completion windows have no exact legal black double-stone
cover. A candidate is reported as
`two_ply_no_immediate_forcing` only when all replies completed before the
deadline pass that test. A deadline produces `unknown`; it is never treated as
safe. This is a two-ply safety result, not a global proof.

The analyzer does not modify production `Game.cpp`, `Move.cpp`, `six.pro`,
tests, client/server code, or棋谱. It writes only `turn17_analysis.json` and
`turn17_analysis.md` to the requested output directory.

## Release Build And Run

```bash
mkdir -p /tmp/turn17-analysis-build /tmp/turn17-analysis
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang \
  /wlf/Six/程序原版压缩包/six/self_iteration/turn17_analysis.pro \
  -o /tmp/turn17-analysis-build/Makefile CONFIG+=release CONFIG-=debug
make -C /tmp/turn17-analysis-build -j4
QT_QPA_PLATFORM=offscreen /tmp/turn17-analysis-build/turn17_analysis \
  --out /tmp/turn17-analysis --candidate-ms 3000 --total-ms 60000
```

The default record path is the requested Release build record. The output
directory must exist before running. `--record` can override the input path.
