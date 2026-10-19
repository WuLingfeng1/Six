# Completed Verification

## Trusted Audit Run (2026-10-18)

Scope was restricted to `tests/`. Production, self-iteration, client, server,
and archive files were not edited. The offline test project now registers the
production `six.qrc`; the generated test binaries therefore contain the board
resources instead of relying on an empty resource set. The in-source QtTest
MOC include was also made an explicit out-of-source generated target, so all
current slots are actually executed.

The independent referee now covers real six-cell windows, independent
coverability, black/white symmetry for a three-window shared defense, own-win
priority, and the historical white15 and Anqing positions. The prefix31 test
checks exactly three generated candidates without requiring a unique coordinate
and independently validates the engine's selected pair with exhaustive
one-reply `exactSafe`; it does not use the production score as an oracle.

Both complete suites were run serially from the production source directory,
with the actual newly built GUI executable passed through `SIX_GUI_BINARY`:

| Configuration | Result | Time | Log |
| --- | --- | ---: | --- |
| Qt 6.11.1 Debug, `linux-clang` | **38 passed, 0 failed, 0 skipped** | 179071 ms | `/tmp/opencode/six-offline-tests-debug/results-trusted-180.txt` |
| Qt 6.11.1 Release, `linux-clang`, `-O2` | **38 passed, 0 failed, 0 skipped** | 48153 ms | `/tmp/opencode/six-offline-tests-release/results-trusted-final.txt` |

The shared-defense audit passed for both colors. Prefix31 reported three
candidates, three safe, zero unknown, and 160884 independent reply pairs
examined; both builds selected `I17 O11` in this run and passed the independent
real-board safety check. White15 independently checked all `45451` legal black
pairs and reported no forced fatal response. Anqing's direct exact-cover count
was `689`, and the selected pair passed `exactSafe`. The actual GUI subprocess
survived 1000 ms offscreen in both configurations.

The earlier sections below are retained for history only. Their statistics are
**not reliable as current audit results**: they predate the shared-defense and
prefix31 audit changes, include a stale QtTest MOC situation in one iteration,
and used a mismatched default prefix31 record for the old coordinate list.

Final source hashes for the current run:

```text
7a7d993d56b1257b277633b455b44d50e0f923cb1ca29756203ebaef4bae5fc0  Game.cpp
d3196277df0b96f5b9172ff374c637a5bdf311fc6a81b67269c71921e7adf36c  Game.h
a437f97e4099ee340be96e16b13ffa70b6b4cdbf0c39329bafd175e76bb89831  Move.cpp
4b416c0803ce4a86250eb2cdde65a9d6f0d03fa34832a211c4c790ab1facb27  tests/referee.h
81d00928fcae3c7492568c6f237094a8e44c949c6bac581af2041cf9a8114194  tests/offline_tests.cpp
989de15fc321db86024f24b5d2680a5569db23d2bc3225a39d5b9c4707c24312  tests/offline_tests.pro
```


## Latest Exact-Defense Revision

The standalone production `Game.cpp` now uses an exact-defense probe for root
positions with direct opponent completions. It keeps all exact covers and, within
the 1100ms propagation budget, independently enumerates legal opponent pairs;
`safe`, `unknown`, and `refuted` remain distinct. This specifically fixes the
historical Anqing prefix-15 position where `K11 I13` was refuted while six exact
covers, including `K11 H12`, passed the complete one-reply oracle.

Fresh Qt 6.11.1 `linux-clang` runs against the current production sources:

| Configuration | Result |
| --- | --- |
| Debug | **36 passed, 0 failed, 0 skipped** |
| Release | **36 passed, 0 failed, 0 skipped** |

The 36 cases include `anqingPrefix15ExactDefense`, the existing white15 oracle
and search, half-turn/undo/state tests, and the production GUI smoke test. The
GUI smoke used the freshly built current binaries under `/tmp/opencode/six-exact-gui-{debug,release}`. The earlier 35-pass report below predates this exact-defense change.

The rebuilt Release self-iteration baseline then ran 8 games across 4 fixed
openings (2 games per opening, up to 30 dual turns). It produced 6 black wins,
2 unfinished games, 0 illegal moves, and `critical_failed: false`. The prior
pre-exact-defense two-game baseline had two white wins. This is a small
regression signal, not a tournament win rate; the game transcripts are stored
in `/tmp/opencode/six-exact-baseline-8x30/baseline.json`.

An intermediate 16-game run (4 games per fixed opening, same 30-turn limit)
produced 12 black wins, 4 unfinished games, 0 white wins, 0 illegal moves and
`critical_failed: false`. The repeated deterministic transcripts were stable;
this remains a fixed-opening self-play signal rather than an external-opponent
win rate. Report: `/tmp/opencode/six-exact-baseline-16/baseline.json`.

The completed 40-game Release baseline (10 games per each of 4 fixed openings,
same 30-turn limit) produced **30 black wins, 10 unfinished games, 0 white
wins, 0 illegal moves**, with `critical_failed: false`. This is a stable
fixed-opening self-play signal after exact-defense, not an external-opponent
tournament win rate. Report: `/tmp/opencode/six-exact-baseline-40/baseline.json`.

After the exact-defense change, the Release historical analyzer rechecked the
selected own-turn windows of all 6 Chongshi loss records: 157 positions
completed, 0 illegal moves, 0 oracle unknowns. The Anqing record's earliest
avoidable turn changed from `K11 I13/refuted` to `K11 H12/safe_one_reply`.
Later historical refuted positions remain, so this is a localized correction,
not a claim that all six historical games are now wins.

The current-source backward scan (last 10 own turns, five-turn window around
known refutations) now reports no additional avoidable position. The repaired
Anqing turn is safe; every later `refuted` position in the scanned windows has
zero safe exact-cover alternatives under the complete one-reply oracle.

Environment: Linux x86_64, Qt/QtTest 6.11.1, qmake 3.1, linux-clang, Clang 20.1.8.
Both final complete suites exited 0 under an external 120s timeout, run serially.
All requested coverage was executed, with no skips or expected failures.

| Configuration | Final QtTest result | Total time | White15 search | White pair | Black oracle |
| --- | --- | --- | --- | --- | --- |
| Debug (`-g`) | 35 passed, 0 failed, 0 skipped, 0 blacklisted | 35,443ms | 2000ms, completed depth 3 | E9 D11 | 45,451 checked, no fatal response |
| Release (`-O2`, `QT_NO_DEBUG`) | 35 passed, 0 failed, 0 skipped, 0 blacklisted | 35,180ms | 2000ms, completed depth 3 | E9 D11 | 45,451 checked, no fatal response |

The 35 results include QtTest init/cleanup; there are 33 actual test cases/data
rows. White15 root precheck excluded 591 concrete counterexamples, final score
-406, cumulative qNodes 1024 in both configurations. The independent oracle
visits all `302 * 301 / 2 = 45,451` black pairs, rather than trusting the finite
production reply list. Both actual timeout-restoration searches elapsed 2000ms.
Ordinary turn searches elapsed 2000-2001ms in Debug and 2000ms in Release.

Both GUI configurations built successfully from the unchanged `six.pro`.
Debug uses `/tmp/opencode/six-offline-debug/six`; Release reuses the verified
up-to-date `/tmp/opencode/six-offline-release/six`. Both actual GUI subprocesses
were alive after 1000ms offscreen and were then deliberately terminated.
Both suites also showed a full MainWindow, executed two new-game replacements,
checked old Game deletion and fresh getGame(), and verified reconnects for
opening, resumed play, undo and restart forwarding without mouse input.
All tested result/close dialogs selected Discard, with no save signal emitted.
These are startup/wiring/repaint smoke results, not real-screen visual acceptance.

## Audit

- Only `tests/` was added. Preexisting modified `Chequer.cpp`, `Game.cpp`,
  `Game.h` and `mainwindow.ui` were preserved byte-for-byte; `six.pro` unchanged.
- No production/client/server/archive code was edited, staged or committed.
- Source/header/UI/project search for QTcp, QUdp, QAbstractSocket, socket,
  network/Network, net-method prefixes, onConnect/onReceive and network menu text
  returned no matches in this standalone target.
- `offline_tests.pro`: only core/gui/widgets/testlib. `readelf -d` on both test
  executables confirms Qt Widgets/Gui/Test/Core only, no QtNetwork/Quick/Qml.
- `six.pro`: no explicit network module. Existing quickwidgets has a transitive
  QtNetwork dependency in the full GUI build, retained under the no-edit rule.
- Compared standalone/client search constants: 2000ms turn budget, 200ms probe
  discovery, 1100ms probe propagation, 28 probe points, 32 replies, quiescence
  depth 2, 1024 nodes, 16 defenses, root 40 points/25 pairs and maximum depth 6
  match. No test changes these production search parameters.
- Builds report existing production warnings: unused WIN_THRESHOLD/event
  parameters, signed size comparison and timer equality-expression result.
  They do not prevent successful builds; no warning-driven production edits.
- `git diff --check` on the preexisting tracked production diff reports existing
  trailing-whitespace/CRLF diagnostics. Those files were not normalized.

The first development Debug run had 28 passes and one test-assertion failure:
undo expected the old incremental `add()` line neighborhood rather than
`rebuildSearch()`'s radius-two square. The test was corrected to independently
construct all 24 nearby empty points, and half-turn/resumed-forwarding coverage
was extended. The final results above are full reruns of the final test sources,
not a selective rerun or weakened production behavior.

## Fingerprints

SHA-256 of production inputs before and after testing (identical):

```text
86beada47e90246cfd1985465775f3d96bf9b2134f45787581a521384831d5b0  Game.cpp
d3196277df0b96f5b9172ff374c637a5bdf311fc6a81b67269c71921e7adf36c  Game.h
01771fab5196f41ccf05ee592e9325b190426530d3e406bc98ec50b8523afa82  Chequer.cpp
ebbd27f3bbda498070fc88a48987d44b44e3b022e4a9f2ae6b1d46abf46c7798  mainwindow.ui
df133eb1426bc5fa148d120a0d3eb743acb874544d379ff0cf8490b995c8d1f2  six.pro
```

Exact external record (also logged by both white15 cases):

```text
985f070ca282e255c0e3ae431fc39b7052b555c14fe92a6f392bd6830fec6a85  /wlf/Six/server/six_server/build/Desktop_Qt_6_11_1-Debug/棋谱文件/C6-serverBvscilentW-先手胜-.txt
```

Final compiled test source inputs:

```text
843bde9645bb354a855dabb44aef8063ec55adcbf66a379f94ef9184f48d28b3  tests/offline_tests.cpp
ed14062f76b3310c1ea846207f2cf7ff739bee5c90c27a1346bab7aae7583f31  tests/offline_tests.pro
4b416c0803dce4a86250eb2cdde65a9d6f0d03fa34832a211c4c790ab1facb27  tests/referee.h
69a35f669325665a6bc042277a8c82781c2ef32279f1d5c33679d7db47af5cc0  tests/dialog_closer.h
```

Test and GUI executables actually run:

```text
a0064c91d5824b450a506caf0683b39003f1331105a8d8065cd29a50569cebf9  /tmp/opencode/six-offline-tests-debug/offline_tests
b995f1946eb4f125b8ebc6890fc1933d536c8e2963c337b915bd2a20ed4cbf07  /tmp/opencode/six-offline-tests-release/offline_tests
12404e80c2638ac7ecdce2cf83a3aa899eb6a0563a513469cb84c7e12f0cdc07  /tmp/opencode/six-offline-debug/six
2b115c67d3e64b43ee0df2ab70d9c54db700f88727b9b8306aee4385078aa9f0  /tmp/opencode/six-offline-release/six
```

Complete QtTest logs retained outside the source target:

```text
7a740083eaabe6fb000a0f433a3f4354305a2ab04990481e411c4ecd9f7eb24f  /tmp/opencode/six-offline-tests-debug/results.txt
4f94941ca75074ed68c4669c374c83c56a837d06a7bdd19f6b5e31689fd27a1c  /tmp/opencode/six-offline-tests-release/results.txt
```

Reproduction commands and limitations are in [README.md](README.md). New
documentation is not included in its own fingerprint list to avoid self-hashing.
