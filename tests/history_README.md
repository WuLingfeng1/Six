# 历史棋谱批量分析

仅新增 `tests/history*`，直接 Release 编译生产 `Game.cpp`、`Move.cpp`，不修改 Game/UI/client/server 或原棋谱。不链接网络。QApplication offscreen，复用 `dialog_closer.h` 自动 Discard，不保存棋谱。

## 构建

先 `ls /tmp/opencode` 核实父目录，再创建 `/tmp/opencode/history-release`。在该构建目录执行：

```bash
/opt/Qt/6.11.1/gcc_64/bin/qmake -spec linux-clang /wlf/Six/程序原版压缩包/six/tests/history.pro CONFIG+=release CONFIG-=debug
make -j4
```

## 分段执行

```bash
/tmp/opencode/history-release/history_analysis --scan
/tmp/opencode/history-release/history_analysis --verify
timeout 115s /tmp/opencode/history-release/history_analysis --file 0 --from 0 --count 35
/tmp/opencode/history-release/history_analysis --report
```

`--scan` 列出排序后的文件索引、实际结果、重复来源及核验信息，并生成报告。`--file` 选择索引，`--from` 是零起点历史回合索引，`--count` 限制历史回合跨度，不是实际选中样本数。默认跳过已生成 row 文件；`--rerun` 明确重跑。生产搜索不降低 2000ms 预算。默认每个 oracle 截止 1000ms，可以用 `--oracle-ms` 更改独立 oracle 的预算，不能改变生产搜索预算。搜索与 oracle 时间分别记录。

`--verify` 读取现有白15参考谱（可用 `--white15` 指定绝对路径），比较加速 oracle 与未修改 `Referee::forcingPair` 的完整枚举数量和结论，核对 `E9 G18 + F11 F13` 反例及 `E9 F14` 对照，并验证截止返回 unknown。额外核验两色首子六连后生产只发出一子、Referee 拒绝终局后 orphan 第二子、未终局单点不能当完整回合。输出 `history_oracle_verification.json`。这只是 oracle / 终局行为交叉核验，不代表棋局获胜。

`tests/history_reports` 必须存在，报告自动生成至该目录；可用 `--out` 指定已有目录。日志请重定向到 `/tmp/opencode`。中断后已完成 row 保留，`--report` 汇总。不要并行跑搜索，避免时限竞争。

## 方法和边界

- 读取原目录全部 24 txt，记录原文件 SHA256 和标准化着法 SHA256，按标准化完整着法序列去重。
- 独立 Referee 核验每颗坐标、空位、黑一白二黑二轮次、六连和满盘，分析仅到首次 terminal。终局之后的记录单独标注不分析。未终局或半回合短谱不能判胜负，不能根据标签推断。
- 败局双方所有回合都分析；其他重师谱保留全部我方回合；3 个非重师谱两色都分析。颜色只从文件名 `重师B`/`重师W` 识别。重复谱不分析。
- 每个局面新建 Game，独立恢复原历史棋盘、m_type、m_step=0、rebuildSearch。黑方首子执行正常 `whoFirst(1)` 中央单子开局，报告 blackFirst=true、搜索预算 0、深度 -1；后续完整回合调用真实离线 bestPlay()。历史最后半回合只有一子而未终局时，旧着法标 illegal/incomplete，不伪造缺失第二子；新着法仍是该回合的正常完整搜索。
- completedDepth 读取生产 qDebug 的真实值，不以评分或 PASS 代替胜负。棋盘变化与发出信号独立校验；save 信号必须为零。
- 对旧/新着法独立判断合法、即时六连、对手 completion、两子 covers。完整遍历所有合法对手双子组合，无 top-K、不在首次 witness 停止。<=4 holes 的六格窗口与 ordinal incidence 只用于加速；沿用原 Referee::coverable，独立于生产战术函数。
- oracle 反例是对手即时六连，或对手双子形成我方两子不可 cover 的威胁且我方无即时 counterwin。记录具体回复、威胁 holes、枚举数和截止状态。中断是 unknown，已有反例仍单独保留，不得当安全。
- safe_one_reply 只是一层有限战术检查；不宣称更深安全。旧/新动作比较和直接战术比例不是胜率，没有续接替代招到原历史或虚构重赛。

报告：`history_summary.md`（中文逐谱、已运行/未运行）、`history_details.json`（元数据及全部逐回合细节）、`history_details.csv`、`history_key_samples.md`（关键例子及战术转移计数，非胜率）。汇总 JSON 的 position_tactics 区分当前已无法直接防住和某个新动作被反驳。独立 row JSON 支持分段恢复和具体反例审计。
