# 当前实验入口（2026-10-21）

当前命令、8 项有效棋型权重、构建验证与报告说明见 [运行手册](运行手册.md)。

扩展评测入口为 `validation_runner.py`：固定八个中盘前缀，候选分别执黑、执白真实续弈；
默认新增两组开局种子与两组中盘种子，共 96 盘。选取规则、构建命令、独立重放和续跑约束见
[中盘评测说明](midgames/README.md)。

**历史 A/B 证据更正：**旧 `self_iteration.pro` 的 `GAME_DIR = ..` 覆盖了命令行候选目录。已保存的 `reports/threat-20/{old,new}-build/Makefile` 两侧都编译生产 `Game.cpp`。此前通过该工程产生的候选/历史版本 A/B，不能证明候选参数、评估改动或威胁延伸的收益。原始数据保留；后续只使用通过 `verify_worker.py` 核验的构建。新结果见 `reports/reliability-20261021/`。

# 小规模自动迭代基线

## Release 历史失败分析器

`failure_analysis` 是只读生产源码的独立离线分析器，不修改 `Game.cpp`、`Move.cpp`、`six.pro`、client/server 或棋谱。它固定读取以下三个真实谱前缀：

- `C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt`，前 67 子，当前黑方；
- `C6-重师Bvs吉林大学一队W-后手胜-祁门一中.txt`，前 35 子，当前黑方；
- `C6-东北秦皇岛分校Bvs重师W-先手胜-祁门一中.txt`，前 29 子，当前白方。

构建时使用 Release，且输出目录必须已存在：

```bash
mkdir -p /tmp/opencode/six-failure-analysis
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang \
  /wlf/Six/程序原版压缩包/six/self_iteration/failure_analysis.pro \
  -o /tmp/opencode/six-failure-analysis/Makefile CONFIG+=release CONFIG-=debug
make -C /tmp/opencode/six-failure-analysis -j4
mkdir -p /tmp/opencode/six-failure-analysis/report
QT_QPA_PLATFORM=offscreen /tmp/opencode/six-failure-analysis/failure_analysis \
  --out /tmp/opencode/six-failure-analysis/report
```

输出 `failure_analysis.json` 和 `failure_analysis.md`。候选是当前行棋方所有能覆盖对手当前 direct completions 的合法双子，未使用生产候选生成或 top-K 裁剪。每个候选随后进行独立的完整对手双子枚举；`refuted` 保存对手即时六连或“不可覆盖且我方无即时反击胜”的 witness，截止时间内未完成记为 `unknown`。`safe_one_reply` 只是 1 层，不是全局证明。Markdown 明确报告每个局面的 safe 数、生产候选是否唯一以及是否存在真正安全的替代。

这是只读生产实现的离线基线工具。默认直接编译 `../Game.cpp` 和 `../Move.cpp`；也支持通过 qmake 的 `GAME_DIR` 编译临时源副本。不修改生产 `Game.cpp`、`Game.h`、`six.pro`、client/server、历史棋谱或现有 `tests`。

## 构建

```bash
mkdir -p /tmp/opencode/six-self-iteration
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang \
  /wlf/Six/程序原版压缩包/six/self_iteration/self_iteration.pro \
  -o /tmp/opencode/six-self-iteration/Makefile CONFIG+=release CONFIG-=debug
make -C /tmp/opencode/six-self-iteration -j4
```

## 运行

输出目录必须事先存在。默认读取项目内 `棋谱文件/` 中固定列出的 4 个文件和固定前缀（不依赖 `tests/history_reports`）：

```bash
QT_QPA_PLATFORM=offscreen /tmp/opencode/six-self-iteration/self_iteration \
  --mode baseline --games 2 --plies 30 --opening-set 0 --out /tmp/opencode/six-baseline
```

生成 `baseline.json`、`baseline.csv`、`baseline.md`。`--games` 支持 1..40，`--plies` 支持 1..30，`--opening-set` 支持 0、1、2、4。使用 `--opening-set 4 --games 40` 可让四种固定开局各运行10局。默认启用关键局面回归；使用 `--no-critical-regression` 可关闭。

`baseline.json` 的 `critical_regressions` 包含两个完整 oracle 检查：白 15 棋谱前 57 子（确认黑方包含 `G11,H15`），以及安徽大学棋谱前 51 子并强制设置白方。每个结果记录生产 `bestPlay` 新着、`status`、`oracle.checked`、`oracle.witness`；完整双子枚举截止为 2 秒，`unknown` 也会触发 `critical_failed`。白 15 关键门槛必须安全；历史局面若前置局面已存在不可避免的不可防局面则记为 `unavoidable`，不淘汰候选。

`--opening-set 0` 保持原有自对弈流程，不强制开局。设置为 1、2 或 4 时，每局按零基 `game_index % opening_set` 选择固定白方首个双子；黑方仍严格落中心 `J10`。开局 0 为 `I10,K12`（内部 `(9,8),(7,10)`），开局 1 为 `L7,H14`（内部 `(12,11),(5,7)`），开局 2 为 `H11,L9`（内部 `(8,7),(10,11)`），开局 3 为 `G8,M12`（内部 `(11,6),(7,12)`）。独立 `Referee` 严格执行黑中心和白双子，检查坐标、空位、回合颜色、棋子数和未终局；任一固定开局非法则程序直接退出，再从黑方回合调用真实 `Game`。强制开局不计入直接胜/直接漏防样本。每个 `games[]` 条目包含 `opening_id`、棋谱坐标 `opening_moves`、包含开局的完整棋谱坐标 `moves`、`source_sha256_game` 和 `config_sha256`；顶层也报告 `opening_set`。

按当前默认生产源构建并运行两局 smoke：

```bash
mkdir -p /tmp/opencode/six-self-iteration-smoke
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang \
  /wlf/Six/程序原版压缩包/six/self_iteration/self_iteration.pro \
  -o /tmp/opencode/six-self-iteration-smoke/Makefile CONFIG+=release CONFIG-=debug
make -C /tmp/opencode/six-self-iteration-smoke -j4
mkdir -p /tmp/opencode/six-self-iteration-smoke/report
QT_QPA_PLATFORM=offscreen /tmp/opencode/six-self-iteration-smoke/self_iteration \
  --mode baseline --games 2 --plies 5 --opening-set 2 \
  --out /tmp/opencode/six-self-iteration-smoke/report
```

## 判定边界

- 每个固定局面恢复历史前缀后，生产 `Game::bestPlay()` 负责新着；首着走生产 `whoFirst(1)`。
- 独立 `Referee` 逐子检查颜色、空位、六连和双子回合完整性。
- 直接胜、对手直接 completion、以及一层不可覆盖强制反例由工具自己的窗口枚举检查；截止时间为 2 秒时标记 `unknown`，不当作安全。
- 固定局面使用 2 秒一层裁判截止；自对弈使用 10 ms 一层裁判截止以保持小规模基线可重复，截止同样记为 `unknown`。
- 自对弈双方都各自新建生产 `Game`，每局最多 30 个双子回合；超时、非法或未终局只记为未完成，不判胜。
- 基线运行自动关闭生产终局对话框并选择 Discard，终局后继续写出报告，不保存棋谱。
- 工具不修改源码、不尝试参数更新、不把失败候选写入生产。

## 候选参数 smoke 实验

`candidate_runner.sh` 在 `self_iteration/reports/` 下创建候选副本和报告。候选通过 `config_tools.py` 修改 `SearchConfig` 的 8 个实际棋型权重及搜索参数，完整校验通过后才写入副本。默认运行 1 局、5 个双子回合：

```bash
./candidate_runner.sh --games 1 --plies 5 --opening-set 0
```

候选实验也支持 `--opening-set 1`、`--opening-set 2` 或 `--opening-set 4`，该参数会传给每个临时构建的 `self_iteration`。`--games` 支持 1..40。

每个候选目录包含 `report.json`、`baseline.json`、`baseline.csv` 和 `baseline.md`。`report.json` 内嵌完整 `baseline` 报告及其 `critical_regressions`、`critical_failed`；关键回归失败时候选运行状态为 `failed`，但报告仍会保存。候选运行器也接受 `--no-critical-regression`。候选目录不会复制回生产目录。

## 候选对局比较

`candidate_runner.sh` 的现有 `--match-games`、`--match-plies` 和 `--match-opening-set` 参数保持兼容。匹配报告现在对每个 opening 和 game 运行三个完整场景：`control`（baseline-black vs baseline-white）、`candidate-black`（candidate-black vs baseline-white）以及 `candidate-white`（baseline-black vs candidate-white）。每个场景的逐回合结果和 transcript 都保存在 `match/match.json` 的 `scenarios` 数组中。

例如运行 1 局候选 smoke（4 个固定开局集合）：

```bash
./candidate_runner.sh --games 1 --plies 5 --opening-set 4
```

`comparisons` 对每个 game 生成黑、白两条记录，字段为 `opening_id`、`candidate_side` 和 `first_difference`。比较前缀必须逐颗棋子、同色、同回合完全相同；候选方首次不同才报告 `turn`、`color`、`control_move`、`candidate_move`。如果对手回合或 transcript 已无法对齐，则停止比较并标记 `kind: "unknown/board_diverged"`，不把分叉后的着法当作同局面差异。所有场景仍由独立 `Referee` 检查落子合法性、颜色、空位和六连。

历史局面 smoke 可用 `--match-history-fixtures white15|anhui|all`。runner 从真实棋谱恢复白 15 棋谱前 57 子或安徽大学棋谱前 51 子，报告 `fixture`、`source`、`prefix_plies` 和按规则计算的 `current_color`；前缀必须合法且未终局，白 15 还验证包含 `G11`、`H15`。历史棋盘直接发送给 worker，不注入 opening。每个 fixture 运行 `control` 和当前行棋方的 `candidate-white`/`candidate-black`，`first_difference` 只比较相同恢复前缀上的后续 transcript。

例如运行 attack/defense 历史 smoke（脚本默认仍不启用历史 fixture）：

```bash
./candidate_runner.sh --match-games 1 --match-plies 5 --match-history-fixtures all
```

对 baseline、attack、defense 各运行 4 局、10 个双子回合：

```bash
./candidate_runner.sh --match-games 4 --match-plies 10 --match-opening-set 4
```

## 历史谱当前 AI 自对弈

`match_runner` 另有独立的目录模式。`--records-dir DIR` 按文件名顺序扫描目录下全部 `*.txt`，`--prefix N` 恢复每个文件前 N 颗合法历史棋子。本任务使用 `N=3`：黑 `J10`，随后白方两子；恢复后必须未终局且下一方为黑。旧棋谱的后续着法只用于计算 `actual_historical`，不会注入当前自对弈。

目录模式把同一个 worker 可执行文件同时作为黑方和白方，每条记录默认运行一局、最多 30 个双子回合。胜负始终由 `match_runner` 的独立 `Referee` 判定，worker 断线、超时、非法着法或未终局不会被判为对手胜。输出目录中生成 `history-selfplay.json`、`history-selfplay.csv` 和 `history-selfplay.md`，包含文件名、文件名标签、独立重建的历史结果、prefix moves、当前结果、winner、illegal、stones、transcript，以及 `critical` 和 first-direct 统计。

直接使用已构建的 worker 和 runner：

```bash
mkdir -p /tmp/opencode/history-ai-selfplay
QT_QPA_PLATFORM=offscreen /tmp/opencode/history-ai-selfplay-match-build/match_runner \
  --records-dir /wlf/Six/程序原版压缩包/six/build/Desktop_Qt_6_11_1-Release/棋谱文件 \
  --prefix 3 --games 1 --plies 30 \
  --black-worker /tmp/opencode/history-ai-selfplay-build/self_iteration \
  --white-worker /tmp/opencode/history-ai-selfplay-build/self_iteration \
  --out /tmp/opencode/history-ai-selfplay
```

也可以由新增脚本完成当前 `self_iteration` worker 和 `match_runner` 的 Release 构建后运行：

```bash
./history_selfplay.sh
```

脚本默认使用 `/wlf/Six/程序原版压缩包/six/build/Desktop_Qt_6_11_1-Release/棋谱文件`、`--prefix 3`、`--games 1`、`--plies 30`，结果写入 `/tmp/opencode/history-ai-selfplay`。`candidate_runner.sh` 不透传该模式；候选实验与历史自对弈保持分离。

本轮12份 `six-human` 棋谱的实际结果：当前AI从前三子开局重新续弈后，12/12的胜负颜色与历史独立裁判结果一致，非法着法为0。注意这些棋谱不能统一标为AI对AI；按用户说明，除唯一网络转发落子的记录外，其余包含用户实际落子。该模式只是共同开局的当前AI控制自对弈，不是历史人类着法的重赛。
