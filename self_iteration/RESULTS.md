# Self-Iteration Reliability Results

本轮只修改 `self_iteration/self_iteration.cpp` 和
`self_iteration/match_runner.cpp`，没有修改生产 `Game.cpp`、`tests`、server 或
client。

## 修复

- 独立 `match_runner` referee 现在按 `stones` 计算回合颜色，强制首回合黑一子、之后每色每回合两子，并拒绝终局后的落子。
- `MOVE n` 接受 `n=1` 仅当该首子立即终局；非终局单子和终局后续输出仍为非法。
- worker 在第一子已经六连时截断第二子，避免重复输出把合法终局误判为非法。
- 历史前缀不再硬编码当前必须为黑方；`prefix=31` 的当前方由规则计算。
- 报告把 `compiler` 改为 `not_recorded`，另列 `build_abi`，不再用 ABI 字段冒充编译器。
- opening-set=2 收敛为指定的两个开局：`J10,I10,K12` 与 `J10,L7,H14`。

## Build And Smoke

使用 Qt 6.11.1、`qmake -spec linux-clang`、Release/O2，在 `/tmp` 隔离构建：

- worker build: `/tmp/opencode/six-self-iteration-reliability`
- referee build: `/tmp/opencode/six-self-iteration-reliability-match`
- 编译通过；唯一警告是生产 `Game.cpp` 已有的未使用 `WIN_THRESHOLD`。
- baseline smoke：2 局、每局最多 12 个双子回合、两个指定开局；两局均为
  `unfinished`，`illegal_count=0`，不是胜负结论。
- fixed-vs-buggy smoke：每个开局各运行 control、buggy-black、buggy-white；
  control 是修复版 baseline-vbaseline。所有对局均未终局，非法着法为 0。
- 相邻历史前缀 smoke：`prefix=31` 得到当前 `B`，`prefix=30` 得到当前 `W`，
  两次均合法、非法 0。

## Old Control Boundary

git 中没有找到可用的旧源码基线。因此旧对照只在 `/tmp/opencode/six-buggy-control`
生成：从当前 `Game.cpp` 临时复制，并注入原错误条件
`if (!fatal && myPiece == 1 && threats.size() > 2) fatal = true;`。这是有 bug 的
行为对照，不是可信冠军或历史最佳版本。该副本没有写回仓库。

固定开局 match 的结果：开局 0 的短前缀未分叉；开局 1 的 buggy-black 在第 1
回合分叉，buggy-white 场景因第 3 回合前缀已不同而标记
`unknown/board_diverged`。这些只是行为差异，不证明泛化棋力。

## Captured Reports

完整 JSON（含逐回合 transcript）和 stdout/stderr 保存在以下隔离路径：

- baseline JSON：`/tmp/opencode/six-self-iteration-reliability/report/baseline.json`
- baseline CSV/MD：同目录
- baseline stdout：`/tmp/opencode/six-self-iteration-reliability/stdout.txt`
- baseline stderr：`/tmp/opencode/six-self-iteration-reliability/stderr.txt`
- old match JSON：`/tmp/opencode/six-self-iteration-reliability/old-match/match.json`
- old match stdout/stderr：同目录 `stdout.txt`、`stderr.txt`
- prefix 31 report：`/tmp/opencode/six-prefix31-report/history-selfplay.json`
- prefix 30 report：`/tmp/opencode/six-prefix30-report/history-selfplay.json`

SHA256：

```text
baseline.json  826f86d2a3db0f7aa476604f5d2ab246dc6aa9550383d4eebe61cbad83311ad6
baseline stdout eeae6cf0fccb7dbb7e485407e4c9a44981bce21ac385933294c264ea0ed83d9a
baseline stderr e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
old match.json 3468616c9e102554ceba3f74e5b88e36617508932bf5245504a785b031f7704f
old match stdout e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
old match stderr e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
prefix31 JSON 1743908ce164111311d2cea173615648c3c88a984fe0df0a7cd8fb839ab81588
prefix31 stdout/stderr e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
prefix30 JSON 0d00ac73251b5c87c4aa193ea4320d52d67d4304c03ffebb4c8b9219ba1891c9
prefix30 stdout/stderr e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
self_iteration.cpp c75521b5db9e8d977622cbd927869ee2031f2da8aa2f8da2b7df6a381cc35d21
match_runner.cpp edfee503395755873aa2912255cef0301db4a8c00c690b3a99e4f643c883396a
```

未运行 40 局同棋谱，也未把未完成对局计为胜或平；本次新旧对打不证明泛化。
