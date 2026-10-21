# 中盘续弈评测

`suite.json` 固定八个中盘局面，来自八份不同的历史棋谱，黑白待行各四个。
每个条目包含完整落子前缀、原谱 SHA256、标准化原谱 SHA256 和前缀 SHA256。
裁判直接读取嵌入的前缀，续弈不依赖原谱的文件路径，也不使用文件名中的胜负标签。

## 选取方法

`build_midgame_suite.py` 从独立版 `棋谱文件/` 读取棋谱，按包含颜色的标准化完整着法去重，
再按其 SHA256 排序。依次交替选黑方、白方待行，每份原谱最多一个局面；选择 25..49 子范围内
最早符合条件的完整回合前缀。前缀须合法、非终局，且双方都没有可由一子或两子补成六连的窗口。
这个条件只排除即时战术，不能证明更深层安全。

选取不读取搜索结果、历史分析评分或胜负标签。这些原谱本身曾用于历史分析，因此本集合是
固定的中盘评测集，不声称是从未接触过的外部测试资料。修改选取规则或局面后应建立新版本，
不要根据某个候选的胜负筛选样本。

```bash
python3 self_iteration/build_midgame_suite.py --out /tmp/six-midgames-regenerated.json
cmp self_iteration/midgames/suite.json /tmp/six-midgames-regenerated.json
```

前缀哈希按空格连接坐标（无末尾空格、无换行）后做 SHA256，例：`J10 I10 K12`。
坐标使用 `(19 - 行号, 列字母 - 'A')`。

## 运行

从独立版源码目录构建两侧 worker 和裁判：

```bash
bash self_iteration/ab_compare.sh --old-params 'threatExtension=false' --new-params 'threatExtension=true' --build-only --out-root "$PWD/self_iteration/reports/extended-builds"
python3 self_iteration/validation_runner.py --match-runner "$PWD/self_iteration/reports/extended-builds/match-build/match_runner" --baseline "$PWD/self_iteration/reports/extended-builds/old-build/self_iteration" --candidate "$PWD/self_iteration/reports/extended-builds/new-build/self_iteration" --out-root "$PWD/self_iteration/reports/extended-validation"
```

默认新增开局种子 1101/1201、中盘种子 2101/2201，共 96 盘，固定 2000ms、最多 30 回合，
搜索串行执行。每个种子范围包含八个不同种子，四组互不重叠。两侧都保留 `ttEnabled=true`，
所以威胁延伸开关比较包含其禁用 TT 的现有联动。

每组包含 control（基线双方）、candidate-black、candidate-white 三场景。
对每个相同前缀，候选都分别执黑和执白，即使该颜色不是前缀之后先行的一方。
新着法会真正续接到后续局面，区别于只把当前引擎插入历史局面选一手的历史批量分析。

单独使用裁判：

```bash
mkdir -p self_iteration/reports/midgame-only
self_iteration/reports/extended-builds/match-build/match_runner --black-worker "$PWD/self_iteration/reports/extended-builds/old-build/self_iteration" --white-worker "$PWD/self_iteration/reports/extended-builds/new-build/self_iteration" --position-suite self_iteration/midgames/suite.json --games 1 --plies 30 --randomize --seed-base 2101 --out self_iteration/reports/midgame-only/match.json
python3 self_iteration/summarize_match.py self_iteration/reports/midgame-only/match.json
```

中盘模式 `--games N` 表示每个局面重复 N 次，八个局面在三场景下共 `24*N` 盘。
种子为 `seed-base + repetition_index * position_count + position_index`，三场景一致。
前缀必须结束在完整回合（奇数子数），不接受半回合、终局、重复局面、错误颜色或过期哈希。

## 报告与续跑

- `plan.json` 在开赛前冻结二进制指纹、实际参数、局面集哈希和种子安排。
- `progress.json` 标记当前组；组目录内同名文件标记场景、局面及完成盘数。
- 每组 `match.json` 保存完整前缀、全部续弈着法、两侧配置及源码指纹。
- `summary.json` / `summary.md` 由独立 Python 裁判重放后生成。开局与中盘分开统计，
  JSON 另有每局面净胜局和配对结果转换；未完成局不算平局或胜局。
- 相同命令可续跑。已完成组先核对计划与实际报告参数，再重新验证棋谱并跳过比赛；
  输入改变则拒绝复用目录。中断中的未完成组会整组重跑。

`net` 是候选相对各自 control 的胜局差，不是胜率或显著性检验。有限回合、少量固定局面和
限时搜索噪声都限制结论；不能仅凭总 net 推断所有局面均有收益。

固定种子用于复用随机选棋的输入；搜索仍受实际耗时限制，相同二进制、局面和种子
不保证完成相同搜索深度或走出相同棋谱。判断小幅收益前，应另做相同引擎的重复对照，
记录每步深度、停止原因和首次棋盘分叉。中断日志若缺少完整棋谱，只能作为日志层面的
差异线索，不能计入通过独立复盘的正式样本。

## 工具测试

```bash
python3 self_iteration/test_midgames.py
python3 self_iteration/test_match_summary.py
python3 self_iteration/test_midgame_runner.py self_iteration/reports/extended-builds/match-build/match_runner
python3 self_iteration/test_validation_runner.py self_iteration/reports/extended-builds/match-build/match_runner
```

后两条使用模拟 worker 检查调度、输入拒绝、独立重放、计划冻结和续跑，不是棋力结果。
