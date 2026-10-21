# Expanded Opening/Midgame Validation (2026-10-21)

本阶段新增可复现中盘续弈评测，不修改生产 Game/Move 搜索实现。
八个局面来自八份去重原谱，黑白待行各四个，完整前缀为 25..31 子；
选取不使用引擎结果或胜负标签，并排除双方即时两子连六。
运行入口与边界见 [midgames/README.md](midgames/README.md)。

构建：Qt 6.11.1 / linux-clang / Release O2；两侧实际 Game.cpp 指纹相同，
SearchConfig 差异仅为 `threatExtension=false/true`，保持原有 TT 联动。
输入、种子、30 回合上限在开赛前冻结，原始证据位于
`reports/validation-20261021/`，正式比赛计划为 `matrix/plan.json`。

正式 **96 盘全部通过独立逐子重放**，没有非法着法、worker 超时或启动失败。
其中 control 32 盘、候选对基线 64 盘，12 盘未完成，保留 unfinished，不计作平局。

| 类型 | seed base | control 黑/白/未完成 | 候选执黑 黑/白/未完成 | 候选执白 黑/白/未完成 | net 黑 / 白 | net |
| --- | ---: | --- | --- | --- | --- | ---: |
| 开局 | 1101 | 5 / 3 / 0 | 4 / 4 / 0 | 5 / 3 / 0 | -1 / 0 | -1 |
| 开局 | 1201 | 4 / 3 / 1 | 2 / 5 / 1 | 4 / 3 / 1 | -2 / 0 | -2 |
| 中盘 | 2101 | 5 / 2 / 1 | 4 / 2 / 2 | 4 / 2 / 2 | -1 / 0 | -1 |
| 中盘 | 2201 | 3 / 4 / 1 | 6 / 1 / 1 | 4 / 2 / 2 | +3 / -2 | +1 |

开局 48 盘 net **-3**，中盘 48 盘 net **0**。net 是相对各自 control 的胜局差，
不是胜率或显著性结论，两类不合并解释。与上一阶段有效开局 +4/+1 相比，
新增样本未确认稳定收益；生产默认保持不变。

补充诊断与限制：

- 关闭随机选棋、2000ms 的八局面诊断：双方均 8 pass，fail/unknown/lost/invalid 均为 0，
  一层反击状态均为 `safe_one_reply`。仅 mid05 的双子不同：基线 J12/P14、候选 J12/M16，
  名义完成深度为 4/5。这不是完整对局获胜证明。
- 中盘 2101 曾中断，17 盘只有完成日志而无完整组棋谱；保留日志后整组重跑，旧 17 盘不计数。
  相同冻结输入的重叠日志中有 10/17 个结果变化（control 5/8），提示需进一步检查重复运行。
  旧日志无法独立复盘；这是事后日志观察，不能据此确定根因或估计噪声分布。
- 已保存 19 个胜局指标变化配对的首次棋盘分叉；它们用于排查，不作为未来调参的独立验证集。

完整逐局面结果与下一阶段计划见 [进展总结.md 第十四节](进展总结.md)。下一阶段先补逐步统计、
逐盘恢复与同引擎重复对照，明确差异来源，再评估延伸/TT 组合和新搜索算法。
原始证据均在 `reports/validation-20261021/`：`matrix/summary.json`、`repeat-noise.json`、
`midgame-probe-summary.json`、`outcome-changes.json`；最终审计为 `validation-summary.json`，
源码指纹为 `final-source-fingerprints.json`。二进制、局面集、生产引擎和原谱最终指纹核验通过。

已完成的工具验证：

- 5 项中盘选取/坐标/窗口/坏前缀检查，5 项独立重放测试通过。
- 新 C++ 裁判通过 24 盘模拟短测的双颜色调度和独立重放，拒绝过期指纹、错行棋方、半回合、重复点和重复局面。
- 原有 3 类非法 worker 协议检查通过。
- 批次调度器通过 96 盘模拟短测，开局/中盘分开统计；续跑不重复比赛，变更计划、重叠种子、过期 worker 报告均被拒绝。
- 新版重放器重新核验上一阶段 48 盘，结果仍为 +4/+1。

这些模拟对局仅验证工具行为，不计入正式棋力样本。生产引擎及离线测试源码
本阶段未改变，因此未重复上一阶段的 40 项 GUI/搜索测试。

下文保留更早的历史工具记录，不作为本阶段结果。

---

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
