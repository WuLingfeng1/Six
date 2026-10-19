# Independent Offline Tests

This directory tests the standalone target in its parent directory. It does not
compile the client/server, modify production sources, or unpack/change archives.
Only Core, Gui, Widgets and Test are requested by `offline_tests.pro`. There are
no networking headers, connections, protocol helpers, or QtNetwork dependencies
in the test executable. Production implementations are compiled directly, with
no test-only implementation replacements. Private search state is exposed only
in the test translation unit to check restoration.

## Coverage

| Test | Checks |
| --- | --- |
| `blackOpening` | Exactly one central black stone, white to move, step 0, undo enabled signal |
| `turns` | Both colors: legal distinct dual stones, half-turn default call emits one stone, explicit `bestPlay(1)`, color/step switch, last-point preservation, 2000ms budget |
| `tactical` | Both colors: first-stone six stops before unused second stone; two-stone six; live four; jump four; shared multi-threat defense; three uncoverable threats; own win takes precedence over enemy threats |
| `halfTurnTactics` | Both colors: one-stone win, forced defense, two holes cannot be completed in a half turn; `ourplay()` forwards the default request correctly |
| `white15Oracle` | Exact record prefix and independent known losing `E9 G18 + F11 F13` versus coverable `E9 F14 + F11 F13` |
| `white15Search` | Real offline `bestPlay()`, 2000ms budget, legal white pair and actual black/step-0 handoff; reject known losing pairs; all 45,451 legal black pairs independently checked |
| `pairDeltaAndAntisymmetry` | Full six-window recomputation versus pair delta, reversed order, same point, occupied points, four directions, edges, evaluation antisymmetry, no state mutation |
| `timeoutRestoration` | Real depth-6 search times out at 2000ms; board, ordered search list, type, step and last/previous points restored |
| `undoRebuild` | Undo removes remote stone and stale proximity points; expected radius-two square independently constructed; empty board rebuild empty |
| `windowReplacementAndReconnect` | Complete MainWindow shown; two new-game actions delete old Game (QPointer), expose fresh getGame(), reconnect Options `type` and `open_type`, single opening event, undo and replay forwarding, repaint path exercised |
| `productionGuiSmoke` | Actual production executable survives 1000ms offscreen startup; bounded subprocess termination |

The oracle is adapted from the non-network client test referee. It scans complete
six-cell windows in all four directions and never calls the production win,
tactical, candidate or search functions. For the exhaustive black response
check, only windows with at most four initial holes can become threats after two
black stones. Every unordered pair of empty cells is still visited, without
top-K filtering or early termination on a witness. A response fails if black
makes six, or makes threats requiring more than two defensive stones while white
has no immediate counter-win. This is a one-response tactical guarantee for the
tested position, not a proof of a full-game win or arbitrary-depth safety.

Coordinates follow Chequer's record loading: `(19 - row number, column - 'A')`.
The record is read from the user's full absolute path, not a copied/fabricated
fixture. Missing or incorrect inputs fail tests rather than skip them. The first
57 legal stones must end with black15 `G11 H15`, immediately before white15.

`bestPlay()` in this target commits stones and changes `m_type/m_step`; tests do
not reuse client network-mode assertions that leave turn state unchanged.
QTimer/QObject closes result and close-confirmation QMessageBoxes with Discard.
Save signals are checked to remain absent. No mouse input or file-save dialog is
used. The known unowned final Chequer Game is explicitly freed by the window test.

## Build

Toolchain used: `/opt/Qt/6.11.1/gcc_64/bin/qmake`, `-spec linux-clang`, Clang
20.1.8, C++17. Debug uses `-g` with no optimization; Release uses `-O2` and
`QT_NO_DEBUG`. The installed shared Qt libraries themselves are release-built;
"Debug" here means the qmake application/test configuration, not a separately
debug-built Qt library installation.

Always use out-of-source directories. Verify their parents with `ls` before
creating any new directory. The directories below already exist from this run.
Run each command block from its indicated directory, not the source tree.

From `/tmp/opencode/six-offline-tests-debug`:

```bash
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang /wlf/Six/程序原版压缩包/six/tests/offline_tests.pro CONFIG+=debug CONFIG-=release
make -j4
```

From `/tmp/opencode/six-offline-tests-release`:

```bash
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang /wlf/Six/程序原版压缩包/six/tests/offline_tests.pro CONFIG+=release CONFIG-=debug
make -j4
```

From `/tmp/opencode/six-offline-debug` (new separate GUI build directory):

```bash
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang /wlf/Six/程序原版压缩包/six/six.pro CONFIG+=debug CONFIG-=release
make -j4
```

From `/tmp/opencode/six-offline-release` (existing GUI build reused and checked):

```bash
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang /wlf/Six/程序原版压缩包/six/six.pro CONFIG+=release CONFIG-=debug
make -j4
```

Generated uic/moc files, Makefiles, objects, binaries and logs stay outside the
target. None were manually edited.

## Run

From `/wlf/Six/程序原版压缩包/six`, so relative GUI assets can be located:

```bash
export QT_QPA_PLATFORM=offscreen
export SIX_WHITE15_RECORD='/wlf/Six/server/six_server/build/Desktop_Qt_6_11_1-Debug/棋谱文件/C6-serverBvscilentW-先手胜-.txt'
export SIX_GUI_WORKDIR='/wlf/Six/程序原版压缩包/six'
export SIX_GUI_BINARY=/tmp/opencode/six-offline-debug/six
timeout 120s /tmp/opencode/six-offline-tests-debug/offline_tests -o /tmp/opencode/six-offline-tests-debug/results.txt,txt
export SIX_GUI_BINARY=/tmp/opencode/six-offline-release/six
timeout 120s /tmp/opencode/six-offline-tests-release/offline_tests -o /tmp/opencode/six-offline-tests-release/results.txt,txt
```

Run timing-sensitive configurations serially. The 2000ms production budget is
not changed by tests; wall-clock assertions allow 300ms scheduling/dialog
tolerance. The independent oracle runs after the search, outside that budget.
The GUI child gets the offscreen environment and correct resource working
directory, is checked alive, then terminated without attempting to save.

## Limits

Full GUI startup is not real-screen visual acceptance. These checks exercise
widget construction/show/repaint and signal wiring, but do not validate visible
stone rendering, DPI/layout, immediate on-screen paint before search, actual
mouse interaction, file-open/save workflows, or human-visible animation.
Offscreen `propagateSizeHints` warnings are expected.

The standalone source and menus contain no QTcp/socket/network methods or menu
items, and `six.pro` has no explicit `network` entry. However, its existing
`quickwidgets` entry transitively brings QtQuick/Qml/QtNetwork into the full GUI
link command. This is not a network feature in Game, and is not present in the
test project. Eliminating that production GUI dependency would require changing
`six.pro`, explicitly outside this task's allowed scope.

See [RESULTS.md](RESULTS.md) for actual completed runs and SHA-256 fingerprints.
