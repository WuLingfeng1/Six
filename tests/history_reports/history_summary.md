# 历史棋谱离线分析

实际结果由独立 Referee 逐颗核验。未终局短谱不判胜负；重复谱不重复计样本。败局双方全部回合、其他重师谱我方全部回合、非重师谱双方全部回合列入待分析集合。每个新着法只替换当前回合，下一样本恢复原历史，绝非伪重赛。

`direct_win` 是即时六连；`direct_loss` 是对手直接 completion；`refuted` 有对手双子造不可 cover 且我无即时 counterwin 的反例；`safe_one_reply` 仅排除这一个回复层的战术反例，不表示深层安全或全局获胜；`unknown` 是 oracle 截止中断，不能当安全。原记录与新动作差异不是胜率。搜索保持生产 2000ms 预算，战术命中可提前返回。黑方首子走正常中央开局，未调用双子搜索。

## 总计

文件 24；重复 1；独立重师败局 6；已证实标签不符 3；计划样本 711；已完成 711；未运行 0。颜色分类按谱中重师颜色：B 333、W 300、非重师 78（实际落子颜色与我方/对手对照见逐行 JSON）。

原动作战术：`{"direct_loss":8,"direct_win":18,"full_draw":2,"illegal":1,"refuted":11,"safe_one_reply":671}`

新动作战术：`{"direct_loss":7,"direct_win":19,"full_draw":2,"refuted":10,"safe_one_reply":673}`

## C6-aaaBvsxxxW-后手胜-z.txt

颜色：both_non_chongshi；文件标签：W；实际结果：W；标签不符：否；合法前缀：是；核验 131/131 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/non_chongshi|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|2|W/non_chongshi|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/non_chongshi|J12 H9|I9 H9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/non_chongshi|I11 I9|K11 L11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/non_chongshi|I12 H11|I13 K11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/non_chongshi|H12 G10|I8 L11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/non_chongshi|F10 G13|K11 I13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/non_chongshi|I7 F12|I8 L11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/non_chongshi|H7 I6|I8 M12|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/non_chongshi|H10 G11|H10 J8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/non_chongshi|J8 E13|E13 K7|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/non_chongshi|G8 G9|G9 G8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/non_chongshi|G12 G6|G7 J6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/non_chongshi|F13 H6|F8 H8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/non_chongshi|F14 F8|I8 J9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/non_chongshi|G15 F5|F4 G5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/non_chongshi|F9 F7|J7 H5|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/non_chongshi|F11 H5|F6 H5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/non_chongshi|E12 E11|J5 J6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/non_chongshi|E8 E14|G5 E5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/non_chongshi|D9 D8|J5 J6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/non_chongshi|G5 J5|G5 E3|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|23|B/non_chongshi|I5 F4|E5 K5|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|24|W/non_chongshi|D7 E5|E5 E6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|25|B/non_chongshi|F6 D5|C5 E9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|26|W/non_chongshi|G4 E6|E6 B9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|27|B/non_chongshi|C8 I2|B9 H3|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|28|W/non_chongshi|D12 E7|E7 L11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|29|B/non_chongshi|E9 E4|E9 E3|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|30|W/non_chongshi|I4 G2|D15 C16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|31|B/non_chongshi|A5 C7|C9 C10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|32|W/non_chongshi|C9 B6|B6 C9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|33|B/non_chongshi|H3 F3|C4 A4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|34|W/non_chongshi|C6 G3|L4 J6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|35|B/non_chongshi|B5 D6|A6 A7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|36|W/non_chongshi|K11 L11|F15 E15|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|37|B/non_chongshi|N11 B7|N11 D10|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|38|W/non_chongshi|K12 M10|N9 M10|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|39|B/non_chongshi|J13 E10|J13 K14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|40|W/non_chongshi|K13 K14|K14 K13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|41|B/non_chongshi|K10 K15|K15 K9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|42|W/non_chongshi|N9 M8|N9 M9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|43|B/non_chongshi|L7 P7|P7 L7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|44|W/non_chongshi|M9 M7|M14 M9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|45|B/non_chongshi|M6 M11|M11 M6|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|46|W/non_chongshi|P9 O9|L9 L13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|47|B/non_chongshi|L9 R9|K9 Q9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|48|W/non_chongshi|L6 N8|L13 K5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|49|B/non_chongshi|K5 P10|Q11 K5|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|50|W/non_chongshi|N7 N5|O8 P8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|51|B/non_chongshi|N6 N10|N6 N12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|52|W/non_chongshi|O8 P8|O8 P8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|53|B/non_chongshi|L8 R8|K8 Q8|yes|1/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["O10","O7"],"threats":[["O12","O11"],["O11","O6"],["O6","O5"],["L10","P6"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["O10","O7"],"threats":[["O12","O11"],["O11","O6"],["O6","O5"],["L10","P6"]]}|
|54|W/non_chongshi|O11 O10|O7 O6|yes|24/1|safe_one_reply/safe_one_reply|old=-; new=-|
|55|B/non_chongshi|O7 O12|O12 O7|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|56|W/non_chongshi|P11 R11|P11 R11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|57|B/non_chongshi|Q10 Q12|Q12 S12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|58|W/non_chongshi|N12 Q11|M14 N14|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|59|B/non_chongshi|J14 J15|R6 P6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|60|W/non_chongshi|M15 J16|J17 N15|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|61|B/non_chongshi|L15 I15|R6 P6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|62|W/non_chongshi|K16 L16|M14 N14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|63|B/non_chongshi|I14 I16|R6 O6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|64|W/non_chongshi|I17 N14|I17 N14|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|65|B/non_chongshi|K17 F15|O13 N15|yes|2000/3|direct_loss/safe_one_reply|old={"kind":"opponent_completion","reply":["O13","P12"]}; new=-|
|66|W/non_chongshi|O13 P12|O13 P12|yes|41/0|direct_win/direct_win|old=-; new=-|

已完成 66/66 个独立历史局面；其余未运行，不作推断。

## C6-东北秦皇岛分校Bvs重师W-先手胜-祁门一中.txt

颜色：W；文件标签：B；实际结果：B；标签不符：否；合法前缀：是；核验 37/37 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/opponent_control|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|2|W/own|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/opponent_control|H9 I9|I9 H9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|H8 J9|K11 I11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/opponent_control|K8 L8|J8 G11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|K9 J8|H11 I11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/opponent_control|L10 N10|L9 L10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|L9 M9|K10 L11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/opponent_control|O9 N7|N9 N8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|M10 M11|K10 M8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/opponent_control|N9 N8|N8 N9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|N6 N11|N11 N6|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/opponent_control|M7 P10|P10 Q11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|L6 Q11|Q11 L6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/opponent_control|O7 P7|O7 L7|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|L7 Q7|K7 Q7|yes|1/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["O10","O8"],"threats":[["O12","O11"],["O11","O6"],["O6","O5"],["M8","P8"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["O10","O8"],"threats":[["O12","O11"],["O11","O6"],["O6","O5"],["M8","P8"]]}|
|17|B/opponent_control|P8 P9|O8 O10|yes|33/1|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|O8 P11|A19 B19|yes|0/0|direct_loss/direct_loss|old={"kind":"opponent_completion","reply":["P6","P5"]}; new={"kind":"opponent_completion","reply":["P12","P11"]}|
|19|B/opponent_control|P6 P5|P6 P5|yes|100/0|direct_win/direct_win|old=-; new=-|

已完成 19/19 个独立历史局面；其余未运行，不作推断。

## C6-北京信息科技大学Bvs重师W-后手胜-祁门一中.txt

颜色：W；文件标签：W；实际结果：W；标签不符：否；合法前缀：是；核验 75/75 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|2|W/own|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|H12 H9|H9 H12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|K12 M7|M7 H11|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|H10 H11|H11 G12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|J9 K8|G12 J9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|I9 G9|M12 L12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|J8 L6|G11 F12|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|I6 C12|C12 I6|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|I14 C8|I14 C8|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/own|F7 F13|F13 F8|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/own|I15 D10|I15 C9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|24|W/own|E12 E7|E13 E7|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|26|W/own|G11 G16|G16 G11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|28|W/own|L10 F16|F16 K11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|30|W/own|K13 F8|K13 D8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|32|W/own|L9 J7|L9 J7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|34|W/own|L8 N8|J4 K5|yes|35/1|safe_one_reply/safe_one_reply|old=-; new=-|
|36|W/own|L12 O9|J4 K5|yes|13/1|safe_one_reply/safe_one_reply|old=-; new=-|
|38|W/own|L11 L13|L13 L11|yes|40/0|direct_win/direct_win|old=-; new=-|

已完成 19/19 个独立历史局面；其余未运行，不作推断。

## C6-吉林大学一队Bvs重师W-后手胜-祁门一中.txt

颜色：W；文件标签：W；实际结果：W；标签不符：否；合法前缀：是；核验 87/87 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|2|W/own|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|H9 K12|K12 H8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|H8 L12|L12 J12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|I11 I13|K13 M11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|G10 F12|H13 I12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|K7 J15|L6 F11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|J7 L14|L14 J7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|H7 H6|F11 D13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|M13 I5|I6 J6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/own|N13 O10|O10 G7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/own|K9 M9|K9 G7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|24|W/own|K8 P14|P14 P11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|26|W/own|L11 Q11|L11 R11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|28|W/own|K6 O16|O16 J9|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|30|W/own|L7 P12|Q12 G7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|32|W/own|G14 N9|G14 N9|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|34|W/own|H13 M10|M6 N5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|36|W/own|N7 F15|F13 G13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|38|W/own|N8 N5|F13 F14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|40|W/own|F14 F13|F13 F14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|42|W/own|G13 E15|G13 E15|yes|28/1|safe_one_reply/safe_one_reply|old=-; new=-|
|44|W/own|J13 K13|D13 E13|yes|38/0|direct_win/direct_win|old=-; new=-|

已完成 22/22 个独立历史局面；其余未运行，不作推断。

## C6-吉林大学二队Bvs重师W-后手胜-祁门一中.txt

颜色：W；文件标签：W；实际结果：W；标签不符：否；合法前缀：是；核验 43/43 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|2|W/own|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|H12 H9|H9 H12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|J12 K12|K12 I12|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|L12 J13|I12 H13|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|H8 N6|N6 I14|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|H10 K6|K6 I14|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|I8 J7|I14 L11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|G11 I5|L11 I14|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|L7 M8|J14 I13|yes|48/1|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/own|I9 N9|N9 I9|yes|26/1|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/own|I7 I6|I7 I6|yes|38/0|direct_win/direct_win|old=-; new=-|

已完成 11/11 个独立历史局面；其余未运行，不作推断。

## C6-哎哎哎Bvs哎哎哎哎哎哎W-先手胜-啊.txt

颜色：both_non_chongshi；文件标签：B；实际结果：unfinished；标签不符：否；合法前缀：是；核验 11/11 子；终局后忽略 0 子；重复来源：。

未终局/短谱：无法依据文件标签判胜负。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/non_chongshi|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|2|W/non_chongshi|K10 L10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/non_chongshi|K9 L8|J9 I9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/non_chongshi|M7 I11|I11 K11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/non_chongshi|J12 J11|J8 K8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/non_chongshi|I9 J9|J9 L11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|

已完成 6/6 个独立历史局面；其余未运行，不作推断。

## C6-啊啊啊下Bvs说W-先手胜-暗示.txt

颜色：both_non_chongshi；文件标签：B；实际结果：unfinished；标签不符：否；合法前缀：是；核验 10/10 子；终局后忽略 0 子；重复来源：。

未终局/短谱：无法依据文件标签判胜负。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/non_chongshi|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|2|W/non_chongshi|K10 L10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/non_chongshi|K9 L8|J9 I9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/non_chongshi|M7 I11|I11 K11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/non_chongshi|J12 J11|J8 K8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/non_chongshi|M10|J9 L11|yes|2000/3|illegal/safe_one_reply|old=-; new=-|

已完成 6/6 个独立历史局面；其余未运行，不作推断。

## C6-大连交通大学Bvs重师W-后手胜-祁门一中.txt

颜色：W；文件标签：W；实际结果：W；标签不符：否；合法前缀：是；核验 47/47 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|2|W/own|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|K11 L11|K11 H11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|N12 H12|O11 J12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|K13 M10|I13 J14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|M14 N6|M16 H13|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|O13 P16|P16 L16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|P15 J15|K15 P15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|N18 P14|N18 P14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|L14 P13|L14 S17|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/own|M17 K14|N11 O12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/own|J14 O19|O14 S17|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|24|W/own|K15 L16|L16 K15|yes|39/0|direct_win/direct_win|old=-; new=-|

已完成 12/12 个独立历史局面；其余未运行，不作推断。

## C6-安庆师范大学Bvs重师W-先手胜-祁门一中.txt

颜色：W；文件标签：B；实际结果：B；标签不符：否；合法前缀：是；核验 41/41 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/opponent_control|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|2|W/own|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/opponent_control|K12 L12|I9 H9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|K11 L11|I12 I9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/opponent_control|I11 M11|I11 H12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|H12 K13|J12 L10|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/opponent_control|M12 N12|N12 L10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|I12 O12|I12 O12|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/opponent_control|O13 P14|K9 P14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|K9 Q15|Q15 L10|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/opponent_control|M13 M14|M13 M15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|M9 M15|M15 M9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/opponent_control|K15 L14|L14 O11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|O11 J16|J16 O11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/opponent_control|K14 I14|N14 N11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|N14 H14|H14 N14|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/opponent_control|J15 L13|L13 J15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|O10 I16|H17 N11|yes|1/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L17","K16"],"threats":[["N19","M18"],["M18","H13"],["L16","L15"],["H13","G12"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L17","K16"],"threats":[["N19","M18"],["M18","H13"],["L16","L15"],["H13","G12"]]}|
|19|B/opponent_control|K16 L17|L17 K16|yes|48/1|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/own|H13 L15|A19 B19|yes|0/0|direct_loss/direct_loss|old={"kind":"opponent_completion","reply":["N19","M18"]}; new={"kind":"opponent_completion","reply":["N19","M18"]}|
|21|B/opponent_control|M18 N19|N19 M18|yes|43/0|direct_win/direct_win|old=-; new=-|

已完成 21/21 个独立历史局面；其余未运行，不作推断。

## C6-安徽大学Bvs重师W-先手胜-f.txt

颜色：W；文件标签：B；实际结果：W；标签不符：是；合法前缀：是；核验 95/95 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|2|W/own|J11 K10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|L10 L12|L9 L12|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|M10 O11|N11 M10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|K12 M8|L9 L8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|I8 M14|N8 L8|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|L13 M9|M6 M9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|L7 K6|L7 O10|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|J6 H6|J6 I5|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|H5 H7|H5 H7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/own|L14 I13|G7 F8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/own|N13 H11|N13 J13|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|24|W/own|H14 H13|G6 F5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|26|W/own|O13 F14|I11 F11|yes|2000/3|refuted/safe_one_reply|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["I9","K9"],"threats":[["G7","F6"],["G9","H9"],["I11","L8"]]}; new=-|
|28|W/own|I11 I9|I11 G7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|30|W/own|I15 O15|J15 O15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|32|W/own|J13 D14|Q13 G14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|34|W/own|G16 G10|G16 G12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|36|W/own|G12 H15|G12 G6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|38|W/own|D15 J16|J16 J15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|40|W/own|I16 I17|I17 J15|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|42|W/own|J15 K14|K14 J15|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|44|W/own|C14 E14|E14 E16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|46|W/own|E16 F17|E16 F17|yes|57/1|safe_one_reply/safe_one_reply|old=-; new=-|
|48|W/own|G18 H19|H19 G18|yes|46/0|direct_win/direct_win|old=-; new=-|

已完成 24/24 个独立历史局面；其余未运行，不作推断。

## C6-安徽大学Bvs重师W-先手胜-fff.txt

颜色：W；文件标签：B；实际结果：B；标签不符：否；合法前缀：是；核验 57/57 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/opponent_control|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|2|W/own|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/opponent_control|K11 K12|I9 H9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|K10 L12|I9 I11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/opponent_control|H12 I11|J13 I14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|H13 I12|L9 I12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/opponent_control|G14 L9|F15 L9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|I13 G13|G13 I13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/opponent_control|F14 K13|K9 K13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|F13 E14|F13 H11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/opponent_control|E13 J12|E13 G12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|H10 G12|G12 H11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/opponent_control|G15 H11|H11 D12|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|I14 F11|F11 E10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/opponent_control|E10 K16|J15 E10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|K15 H16|K15 I17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/opponent_control|K9 L14|K9 C11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|L8 N16|M15 M7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/opponent_control|J9 M9|M9 N9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/own|I9 N9|H9 N9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/opponent_control|D12 L10|J8 J7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/own|N8 B10|C11 N8|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|23|B/opponent_control|J8 M11|J8 I7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|24|W/own|N12 I7|N12 I7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|25|B/opponent_control|M10 M8|M10 M8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|26|W/own|M12 M7|M12 M7|yes|1/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["K8","J7"],"threats":[["O12","N11"],["N11","I6"],["I6","H5"],["J6","J5"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["K8","J7"],"threats":[["O12","N11"],["N11","I6"],["I6","H5"],["J6","J5"]]}|
|27|B/opponent_control|J7 K8|J7 K8|yes|86/1|safe_one_reply/safe_one_reply|old=-; new=-|
|28|W/own|N11 I6|A19 B19|yes|0/0|direct_loss/direct_loss|old={"kind":"opponent_completion","reply":["J6","J5"]}; new={"kind":"opponent_completion","reply":["O12","N11"]}|
|29|B/opponent_control|J6 J5|J6 J5|yes|46/0|direct_win/direct_win|old=-; new=-|

已完成 29/29 个独立历史局面；其余未运行，不作推断。

## C6-宿州学院Bvs重师W-平局-祁门一中.txt

颜色：W；文件标签：draw；实际结果：draw；标签不符：否；合法前缀：是；核验 361/361 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|2|W/own|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|J12 K9|H9 H12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|L12 G7|L12 J14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|J13 K15|K15 F7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|I14 M14|M14 G8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|L11 L13|L13 L9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|E9 F8|H6 F8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|G8 G6|E8 G8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|I8 H7|I8 H7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/own|I6 J5|J5 I6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/own|I7 K7|G5 I5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|24|W/own|D7 F5|G4 G16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|26|W/own|I4 K6|K6 I4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|28|W/own|L6 M6|M6 L6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|30|W/own|M10 M8|M10 M7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|32|W/own|F4 O7|G2 F4|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|34|W/own|N4 Q3|Q3 G2|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|36|W/own|M5 E3|E3 G2|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|38|W/own|P6 Q5|O8 Q6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|40|W/own|N10 P10|M3 J6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|42|W/own|O13 P9|P8 P9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|44|W/own|O8 Q10|N13 Q13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|46|W/own|N16 F12|N15 O16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|48|W/own|G13 I15|G13 N15|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|50|W/own|H11 H17|H15 I16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|52|W/own|D12 J18|E14 F13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|54|W/own|F16 C14|C14 N14|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|56|W/own|G16 E16|G16 E18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|58|W/own|K16 J16|O16 N15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|60|W/own|L15 J17|F3 F6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|62|W/own|Q14 P16|M3 M1|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|64|W/own|E15 D18|D18 C13|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|66|W/own|C16 C17|C16 C17|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|68|W/own|G14 A8|G14 A8|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|70|W/own|D9 F10|F10 A12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|72|W/own|E17 B13|E17 B13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|74|W/own|L3 F3|M3 L4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|76|W/own|B2 L2|A1 A6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|78|W/own|C6 L1|D6 M2|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|80|W/own|H4 O4|K1 J1|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|82|W/own|N12 O14|P12 J1|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|84|W/own|R11 R15|R15 N5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|86|W/own|S14 A2|S14 B8|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|88|W/own|B5 B11|B9 O18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|90|W/own|M1 N1|N1 J1|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|92|W/own|S12 S13|M17 M18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|94|W/own|C9 O6|M16 M17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|96|W/own|D5 F1|D5 G1|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|98|W/own|K18 Q18|M18 Q18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|100|W/own|C7 C8|A12 A13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|102|W/own|G2 B19|A12 A11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|104|W/own|S6 P17|P17 H19|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|106|W/own|F19 E19|A12 A13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|108|W/own|N2 L19|A12 A13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|110|W/own|A14 N19|R5 R3|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|112|W/own|R4 R8|R8 R2|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|114|W/own|H8 K10|M4 R2|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|116|W/own|H9 I12|Q4 S2|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|118|W/own|I13 G11|A3 K1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|120|W/own|M13 E7|B3 I1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|122|W/own|H15 K17|S7 H1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|124|W/own|H16 N13|J6 G1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|126|W/own|L17 H5|A5 E1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|128|W/own|D8 B10|A5 C1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|130|W/own|B12 K5|A5 B1|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|132|W/own|I2 K2|S7 A1|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|134|W/own|G3 B9|P7 S2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|136|W/own|C2 P12|S8 R2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|138|W/own|O9 B4|S8 Q2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|140|W/own|P7 Q7|S9 P2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|142|W/own|M3 P3|A9 J2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|144|W/own|R2 N3|S11 R3|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|146|W/own|O1 Q1|C12 L18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|148|W/own|C1 E1|C13 E18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|150|W/own|S7 Q8|O15 K19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|152|W/own|S4 Q11|S9 P19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|154|W/own|Q15 R9|A7 R19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|156|W/own|L18 E13|A4 A18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|158|W/own|H19 H18|B3 C18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|160|W/own|G19 K19|S2 S18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|162|W/own|C13 B16|B1 B16|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|164|W/own|C18 B18|I1 M19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|166|W/own|R17 S16|S9 A9|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|168|W/own|A17 A18|A5 A15|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|170|W/own|A10 A9|A3 S19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|172|W/own|J2 I1|B3 S19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|174|W/own|A5 A1|S17 A11|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|176|W/own|A4 A11|S19 S9|yes|358/6|safe_one_reply/safe_one_reply|old=-; new=-|
|178|W/own|S17 S9|R19 S18|yes|24/6|safe_one_reply/safe_one_reply|old=-; new=-|
|180|W/own|R19 S18|R19 S19|yes|1/6|safe_one_reply/safe_one_reply|old=-; new=-|

已完成 90/90 个独立历史局面；其余未运行，不作推断。

## C6-沈阳建筑Bvs重师W-先手胜-zzz.txt

颜色：W；文件标签：B；实际结果：W；标签不符：是；合法前缀：是；核验 211/211 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|2|W/own|J11 I10|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/own|H12 I9|K12 H12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/own|F11 J7|E12 G12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/own|H10 G11|G11 H10|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/own|F10 E9|F10 E9|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/own|L8 F8|E11 H11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/own|I12 D7|D10 C11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/own|J9 G8|C11 D10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/own|H13 E11|E11 E10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/own|H14 D11|D11 D12|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/own|G15 C12|C12 L9|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|24|W/own|E14 I16|J17 L9|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|26|W/own|F14 H6|F14 K12|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|28|W/own|L12 K12|K12 L13|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|30|W/own|L10 L9|L9 L10|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|32|W/own|K7 M9|M9 N10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|34|W/own|K13 N10|N10 O9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|36|W/own|N9 O9|P8 N8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|38|W/own|N12 N11|N8 N6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|40|W/own|K15 K16|M12 K16|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|42|W/own|M12 P13|M12 E6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|44|W/own|G16 M14|E6 F5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|46|W/own|K5 O14|K5 K6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|48|W/own|N15 M16|N15 M16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|50|W/own|L15 M15|L15 M15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|52|W/own|J17 N16|L16 J17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|54|W/own|N14 Q14|M19 N14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|56|W/own|O16 N17|O16 P17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|58|W/own|K18 Q18|R16 R13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|60|W/own|Q16 Q15|Q16 P16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|62|W/own|M6 P19|P19 E7|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|64|W/own|J5 I5|E6 F5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|66|W/own|G6 F6|E6 F5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|68|W/own|E15 P8|E15 E16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|70|W/own|D17 Q10|N6 O7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|72|W/own|B13 B16|F17 E17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|74|W/own|N6 O7|N6 O7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|76|W/own|O6 Q6|C17 B18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|78|W/own|O5 P4|C17 B18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|80|W/own|M4 L3|C17 G17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|82|W/own|N5 L2|N5 C17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|84|W/own|K3 O3|K3 R10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|86|W/own|Q5 N2|C17 B18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|88|W/own|G17 F17|C17 G17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|90|W/own|I3 H3|I3 H4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|92|W/own|I4 I2|I4 F3|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|94|W/own|D5 H1|G3 G1|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|96|W/own|F3 E4|G2 G3|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|98|W/own|C7 G3|G3 G2|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|100|W/own|N19 O19|M19 L19|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|102|W/own|R5 N1|B9 B8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|104|W/own|S6 O2|B9 B10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|106|W/own|Q4 P3|Q4 P3|yes|41/0|direct_win/direct_win|old=-; new=-|

已完成 53/53 个独立历史局面；其余未运行，不作推断。

## C6-重师BvsqW-先手胜-ZXZ.txt

颜色：B；文件标签：B；实际结果：B；标签不符：否；合法前缀：是；核验 21/21 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|I11 H9|I9 H9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|H13 G12|J12 H10|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|F9 E9|K9 L8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|G10 G9|G9 H10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|I9 J9|I9 J9|yes|40/0|direct_win/direct_win|old=-; new=-|

已完成 6/6 个独立历史局面；其余未运行，不作推断。

## C6-重师BvsqqW-先手胜-zx.txt

颜色：B；文件标签：B；实际结果：B；标签不符：否；合法前缀：是；核验 73/73 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|J11 J9|J9 J8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|I10 K12|J8 I10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|K8 H11|L13 H11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|I11 G11|J8 I11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|E12 E10|I12 I14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/own|E11 E9|E11 E9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/own|L13 K6|L13 I8|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/own|G7 F8|L8 J8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/own|D12 H6|H6 K9|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/own|K7 K5|H8 G8|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|23|B/own|I4 J3|J8 J3|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|25|B/own|I6 D8|G8 D8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|27|B/own|I5 I3|I8 J8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|29|B/own|H3 G3|I8 J8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|31|B/own|L10 C6|G6 F6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|33|B/own|E6 G6|G6 D6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|35|B/own|F5 E4|F5 E4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|37|B/own|I8 H7|I8 H7|yes|41/0|direct_win/direct_win|old=-; new=-|

已完成 19/19 个独立历史局面；其余未运行，不作推断。

## C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt

颜色：B；文件标签：W；实际结果：W；标签不符：否；合法前缀：是；核验 99/99 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|2|W/opponent_control|H10 K9|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|J9 J8|K11 J12|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/opponent_control|J11 J7|J7 J12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|K8 I8|K8 I8|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/opponent_control|H8 H7|H8 H11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|I10 H11|H11 I10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/opponent_control|L7 G12|G12 M6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|I9 I7|I7 I9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/opponent_control|I6 I12|I12 I6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|H6 M8|K10 M10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/opponent_control|L8 L6|L8 G8|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/own|L9 G8|G5 L5|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/opponent_control|M6 N6|H12 F12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/own|M7 K6|K6 M10|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/opponent_control|G5 N10|H12 F12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/own|N7 O6|K10 P5|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/opponent_control|P5 G14|P5 H13|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/own|H12 H13|M12 K10|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|20|W/opponent_control|M9 O11|M9 K7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/own|K7 P12|P12 K7|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|22|W/opponent_control|H14 F14|G13 G15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|23|B/own|G13 I14|E9 G9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|24|W/opponent_control|F13 E14|L5 K5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|25|B/own|F15 D14|C14 E9|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|26|W/opponent_control|E12 D15|E12 H15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|27|B/own|H15 C16|C16 D11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|28|W/opponent_control|E13 D11|E13 E11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|29|B/own|E15 C10|C10 E10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|30|W/opponent_control|G15 D12|G15 D12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|31|B/own|C11 I17|I17 C11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|32|W/opponent_control|I13 J12|E11 F12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|33|B/own|K11 F16|F16 K11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|34|W/opponent_control|E11 L4|G9 I11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|35|B/own|E9 L3|E9 L3|yes|1/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}|
|36|W/opponent_control|M5 K3|G9 I11|yes|34/1|safe_one_reply/safe_one_reply|old=-; new=-|
|37|B/own|P8 J2|O7 J2|yes|2/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}|
|38|W/opponent_control|N5 O5|G9 I11|yes|28/1|safe_one_reply/safe_one_reply|old=-; new=-|
|39|B/own|K5 Q5|L5 Q5|yes|1/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}|
|40|W/opponent_control|O4 C13|G9 I11|yes|36/1|safe_one_reply/safe_one_reply|old=-; new=-|
|41|B/own|G9 P3|F10 Q2|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|42|W/opponent_control|F10 F9|F9 F10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|43|B/own|F12 B14|B14 F12|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|44|W/opponent_control|M4 N4|N4 M4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|45|B/own|J4 P4|J4 P4|yes|1/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["P6","M3"],"threats":[["R8","Q7"],["Q7","L2"],["L2","K1"],["M2","M1"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["P6","M3"],"threats":[["R8","Q7"],["Q7","L2"],["L2","K1"],["M2","M1"]]}|
|46|W/opponent_control|N3 Q6|M3 P6|yes|36/1|safe_one_reply/safe_one_reply|old=-; new=-|
|47|B/own|N2 M2|A19 B19|yes|0/0|direct_loss/direct_loss|old={"kind":"opponent_completion","reply":["S8","R7"]}; new={"kind":"opponent_completion","reply":["S8","R7"]}|
|48|W/opponent_control|O3 B13|S8 R7|yes|41/0|safe_one_reply/direct_win|old=-; new=-|
|49|B/own|D13 P2|A19 B19|yes|0/0|direct_loss/direct_loss|old={"kind":"opponent_completion","reply":["S8","R7"]}; new={"kind":"opponent_completion","reply":["A13","D13"]}|
|50|W/opponent_control|R7 S8|S8 R7|yes|1/0|direct_win/direct_win|old=-; new=-|

已完成 50/50 个独立历史局面；其余未运行，不作推断。

## C6-重师Bvs北京信息科技大学W-先手胜-祁门一中.txt

颜色：B；文件标签：B；实际结果：B；标签不符：否；合法前缀：是；核验 45/45 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|N6 N4|K10 L10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|M5 P6|M5 L6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|O3 L6|O3 L6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|N7 O8|M7 N8|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|L3 Q8|Q8 L3|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/own|N3 K3|N8 P8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/own|K8 I8|I8 P8|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/own|L7 G7|G7 M7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/own|L8 K5|K5 L8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/own|M7 N8|N8 M7|yes|33/1|safe_one_reply/safe_one_reply|old=-; new=-|
|23|B/own|O9 P10|P10 O9|yes|42/0|direct_win/direct_win|old=-; new=-|

已完成 12/12 个独立历史局面；其余未运行，不作推断。

## C6-重师Bvs吉林大学一队W-后手胜-祁门一中.txt

颜色：B；文件标签：W；实际结果：W；标签不符：否；合法前缀：是；核验 43/43 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|2|W/opponent_control|I10 J9|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|I9 H11|I11 H11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/opponent_control|H9 J11|K8 H8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|H8 G8|K11 K12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/opponent_control|G7 I8|L13 L12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|G10 H7|E8 F8|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/opponent_control|K12 K10|L13 F9|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|I12 M14|L13 I12|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/opponent_control|K11 K9|L11 J13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|K8 K14|K14 K8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/opponent_control|J13 L11|J13 L11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/own|M12 M10|N13 M10|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|14|W/opponent_control|M11 I14|M11 N11|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/own|H15 N11|H15 N11|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|16|W/opponent_control|L10 J8|L10 J8|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/own|H6 N12|N12 I7|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|18|W/opponent_control|J12 M9|M9 J12|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/own|H14 N8|H14 N8|yes|1/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L12","L9"],"threats":[["L14","L13"],["L13","L8"],["L8","L7"],["N9","O9"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L12","L9"],"threats":[["L14","L13"],["L13","L8"],["L8","L7"],["N9","O9"]]}|
|20|W/opponent_control|L9 L8|L9 L8|yes|31/1|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/own|L7 L13|A19 B19|yes|0/0|direct_loss/direct_loss|old={"kind":"opponent_completion","reply":["N9","O9"]}; new={"kind":"opponent_completion","reply":["L13","L12"]}|
|22|W/opponent_control|N9 O9|N9 O9|yes|40/0|direct_win/direct_win|old=-; new=-|

已完成 22/22 个独立历史局面；其余未运行，不作推断。

## C6-重师Bvs吉林大学二队W-先手胜-祁门一中.txt

颜色：B；文件标签：B；实际结果：B；标签不符：否；合法前缀：是；核验 49/49 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|H12 I11|K10 L10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|K9 L10|G13 H14|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|H13 H14|K10 I10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|M11 N12|N12 M11|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|H10 K10|H15 N10|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/own|J13 K8|N10 O9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/own|M14 G8|M14 H9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/own|K14 L14|N11 H17|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/own|N13 O12|K7 H15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/own|N11 N10|N10 N11|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|23|B/own|M10 P11|L15 M10|yes|21/1|safe_one_reply/safe_one_reply|old=-; new=-|
|25|B/own|P13 L9|P13 L9|yes|41/0|direct_win/direct_win|old=-; new=-|

已完成 13/13 个独立历史局面；其余未运行，不作推断。

## C6-重师Bvs大连交通大学W-先手胜-祁门一中.txt

颜色：B；文件标签：B；实际结果：B；标签不符：否；合法前缀：是；核验 49/49 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|K11 I12|K9 L9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|K12 L11|L10 J12|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|I13 J14|I13 M11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|K14 L14|L14 I14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|H12 G11|K15 M13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/own|G9 O13|H10 H8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/own|F11 E10|M11 N12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/own|M11 N11|M11 N12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/own|N12 P14|N12 K9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/own|N10 N9|N10 N9|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|23|B/own|H11 O8|M10 M8|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|25|B/own|M10 P7|M10 P7|yes|45/0|direct_win/direct_win|old=-; new=-|

已完成 13/13 个独立历史局面；其余未运行，不作推断。

## C6-重师Bvs安庆师范大学W-后手胜-祁门一中.txt

颜色：B；文件标签：W；实际结果：W；标签不符：否；合法前缀：是；核验 31/31 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|2|W/opponent_control|K10 J9|K8 L9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|K9 I8|K11 L11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|4|W/opponent_control|I11 J11|L11 L8|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|I12 H11|K11 I9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|6|W/opponent_control|J13 L11|L11 K11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|J12 K12|H12 H10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|8|W/opponent_control|L12 M11|M12 N11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|K11 L10|K11 I13|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|10|W/opponent_control|J14 K13|K13 N10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|N10 H16|I15 N10|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|12|W/opponent_control|I13 L13|I13 L13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/own|H13 N13|H13 N13|yes|1/0|refuted/refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L16","K15"],"threats":[["N18","M17"],["M17","H12"],["L15","L14"],["H12","G11"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L16","K15"],"threats":[["N18","M17"],["M17","H12"],["L15","L14"],["H12","G11"]]}|
|14|W/opponent_control|K15 L16|L16 K15|yes|54/1|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/own|H12 L14|A19 B19|yes|0/0|direct_loss/direct_loss|old={"kind":"opponent_completion","reply":["N18","M17"]}; new={"kind":"opponent_completion","reply":["N18","M17"]}|
|16|W/opponent_control|M17 N18|N18 M17|yes|38/0|direct_win/direct_win|old=-; new=-|

已完成 16/16 个独立历史局面；其余未运行，不作推断。

## C6-重师Bvs宿州大学W-平局-祁门一中.txt

颜色：B；文件标签：draw；实际结果：draw；标签不符：否；合法前缀：是；核验 361/361 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|I11 H9|I9 H9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|I9 K11|I9 H8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|G9 F9|G10 F11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|F10 F8|F10 F11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|H7 E10|H12 I13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/own|K8 M14|J8 J7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/own|N12 J16|J16 E11|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/own|K13 L15|L10 J12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/own|K10 L10|K10 M16|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/own|K14 P9|K14 P9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|23|B/own|O11 M10|M8 M16|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|25|B/own|P13 P15|P15 P13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|27|B/own|L9 M8|L9 J7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|29|B/own|H5 N6|N6 H14|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|31|B/own|M7 P16|H14 I13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|33|B/own|O5 P4|H14 I13|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|35|B/own|O15 N15|H14 G13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|37|B/own|Q12 Q5|Q5 R13|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|39|B/own|L6 N3|H14 I13|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|41|B/own|J5 I5|H14 I13|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|43|B/own|L4 F4|H4 H6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|45|B/own|M6 G3|M6 H14|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|47|B/own|H12 F14|O16 Q18|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|49|B/own|G12 F12|D10 C10|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|51|B/own|C11 E13|H3 H4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|53|B/own|F16 F15|O16 Q18|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|55|B/own|F6 H15|J17 H19|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|57|B/own|D16 C14|J17 H19|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|59|B/own|D11 D5|D11 E16|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|61|B/own|E4 B11|B11 F3|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|63|B/own|E16 C16|E15 J15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|65|B/own|G14 C18|C18 G14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|67|B/own|J17 I16|I16 J17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|69|B/own|J15 E17|J15 H17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|71|B/own|F18 B14|G18 G19|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|73|B/own|Q10 Q9|O16 P17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|75|B/own|N14 Q17|N14 Q17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|77|B/own|N18 N19|N17 N18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|79|B/own|P8 R7|P8 P12|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|81|B/own|H3 H2|R19 Q18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|83|B/own|F2 G2|G2 I2|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|85|B/own|K6 O6|R19 Q18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|87|B/own|L18 M18|J18 K17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|89|B/own|R16 B5|K17 M15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|91|B/own|S10 S4|S5 R6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|93|B/own|J1 E1|E1 J1|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|95|B/own|N16 S16|N1 M1|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|97|B/own|B6 B4|N1 M1|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|99|B/own|D7 C3|N1 O1|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|101|B/own|P17 R12|O17 N1|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|103|B/own|C6 C5|R6 R2|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|105|B/own|O1 R5|O2 N2|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|107|B/own|A10 A16|A16 R6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|109|B/own|H19 A5|N2 O2|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|111|B/own|N2 O2|N2 O2|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|113|B/own|J12 D19|D19 J8|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|115|B/own|J8 L8|Q4 R2|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|117|B/own|J13 H13|R4 P1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|119|B/own|I7 K7|D3 M1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|121|B/own|O13 E7|E3 L1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|123|B/own|C7 D8|I6 K1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|125|B/own|E6 I6|Q6 I1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|127|B/own|I15 I17|E5 D1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|129|B/own|P10 J18|K5 C1|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|131|B/own|O8 I14|S5 B1|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|133|B/own|P7 Q11|A4 B8|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|135|B/own|Q16 R15|G4 S2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|137|B/own|G4 I4|G4 R2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|139|B/own|O17 R14|A4 Q2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|141|B/own|K3 O4|S5 M2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|143|B/own|P3 R4|Q6 K2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|145|B/own|Q2 R3|B8 O19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|147|B/own|Q1 S1|R10 Q18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|149|B/own|S13 S12|S11 B18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|151|B/own|L1 M2|A11 J19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|153|B/own|J2 E5|B2 M19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|155|B/own|D2 I1|B2 P19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|157|B/own|E14 C13|B2 S19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|159|B/own|B13 A11|S2 D18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|161|B/own|H17 G16|B1 Q18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|163|B/own|E18 B15|D1 K1|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|165|B/own|B7 D3|D1 G19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|167|B/own|B2 A17|S9 S18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|169|B/own|A19 E19|A7 O19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|171|B/own|M19 G19|A4 S19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|173|B/own|S9 Q19|A2 Q19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|175|B/own|Q18 S18|A4 A2|yes|1098/6|safe_one_reply/safe_one_reply|old=-; new=-|
|177|B/own|A4 S2|S2 R1|yes|101/6|safe_one_reply/safe_one_reply|old=-; new=-|
|179|B/own|A7 A3|A7 A3|yes|5/6|safe_one_reply/safe_one_reply|old=-; new=-|
|181|B/own|A1 R1|A1 R1|yes|39/6|full_draw/full_draw|old=-; new=-|

已完成 91/91 个独立历史局面；其余未运行，不作推断。

## C6-重师Bvs沈阳建筑W-先手胜-zzz.txt

颜色：B；文件标签：B；实际结果：draw；标签不符：是；合法前缀：是；核验 361/361 子；终局后忽略 0 子；重复来源：。

|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|
|---|---|---|---|---|---|---|---|
|1|B/own|J10|J10|yes|0/-1|safe_one_reply/safe_one_reply|old=-; new=-|
|3|B/own|I9 K11|I9 J9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|5|B/own|H9 G9|K9 K10|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|7|B/own|K10 K8|G11 G12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|9|B/own|L11 M11|L9 M8|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|11|B/own|K12 K14|K12 J13|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|13|B/own|J13 I14|J13 I14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|15|B/own|L14 J14|J14 H14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|17|B/own|H11 L15|L15 I12|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|19|B/own|J15 J16|J15 L17|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|21|B/own|M13 K15|K15 I17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|23|B/own|H17 N15|L17 K16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|25|B/own|L16 M17|L17 K16|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|27|B/own|G14 L17|L17 K16|yes|27/1|safe_one_reply/safe_one_reply|old=-; new=-|
|29|B/own|O12 G15|G10 F11|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|31|B/own|G12 G13|G12 G13|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|33|B/own|E10 M18|E10 M18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|35|B/own|N10 P9|F11 C8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|37|B/own|F10 N9|P8 O9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|39|B/own|E11 D12|O9 P8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|41|B/own|E13 C11|N7 N8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|43|B/own|F16 C14|F13 O9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|45|B/own|O9 P6|P6 D15|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|47|B/own|O8 M5|Q9 Q8|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|49|B/own|N7 N6|N6 L6|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|51|B/own|L6 N4|N4 N5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|53|B/own|O5 Q7|P2 O5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|55|B/own|I7 R11|P2 P4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|57|B/own|P4 Q3|P4 Q3|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|59|B/own|M4 O4|Q4 O4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|61|B/own|I8 I5|I8 I5|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|63|B/own|L5 N3|L5 G5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|65|B/own|G5 J5|Q6 P5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|67|B/own|P5 Q6|Q6 S6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|69|B/own|H6 R5|R5 E7|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|71|B/own|K3 J4|J4 K3|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|73|B/own|S10 N2|F4 H4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|75|B/own|F6 Q12|Q12 J2|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|77|B/own|J2 J3|J2 I3|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|79|B/own|G3 I3|I3 H2|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|81|B/own|G4 G1|G1 G6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|83|B/own|F7 S7|D15 E16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|85|B/own|E8 P12|C8 B9|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|87|B/own|D9 C10|C10 C8|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|89|B/own|G18 D15|D15 G18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|91|B/own|E14 E16|E14 E16|yes|2000/2|safe_one_reply/safe_one_reply|old=-; new=-|
|93|B/own|E17 B12|E18 B16|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|95|B/own|M19 C5|C5 D4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|97|B/own|E5 B18|B18 D4|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|99|B/own|C6 C4|C4 E6|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|101|B/own|C3 D3|C3 G2|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|103|B/own|A3 B3|B3 Q15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|105|B/own|P14 F2|Q15 Q13|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|107|B/own|O16 O17|P16 O15|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|109|B/own|K2 H2|A6 A7|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|111|B/own|A14 M1|A6 A5|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|113|B/own|Q13 Q16|Q14 R14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|115|B/own|O18 S14|S14 R14|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|117|B/own|S3 B9|E19 F18|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|119|B/own|A9 R15|R15 A9|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|121|B/own|A11 A10|A10 E19|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|123|B/own|O1 Q1|B19 B17|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|125|B/own|G19 R1|P1 E19|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|127|B/own|J19 F19|B19 B14|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|129|B/own|H19 S19|I19 S17|yes|2000/3|safe_one_reply/safe_one_reply|old=-; new=-|
|131|B/own|O19 N19|B14 B15|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|133|B/own|B14 B15|B19 B15|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|135|B/own|F1 B17|B19 D1|yes|2000/4|safe_one_reply/safe_one_reply|old=-; new=-|
|137|B/own|S13 S12|S17 S12|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|139|B/own|D7 O13|S8 B2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|141|B/own|O14 P11|Q8 A2|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|143|B/own|H16 J18|S8 R3|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|145|B/own|H18 K17|Q8 D18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|147|B/own|I18 P10|C8 D17|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|149|B/own|G17 K19|Q8 R18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|151|B/own|P18 Q14|S8 A18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|153|B/own|Q8 R9|G2 D18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|155|B/own|B10 B8|P2 Q18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|157|B/own|A13 F18|S2 A17|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|159|B/own|D18 A16|B1 S16|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|161|B/own|R6 R4|D1 S8|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|163|B/own|S9 R12|I1 K1|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|165|B/own|P2 S8|I1 Q19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|167|B/own|K1 R13|R3 A6|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|169|B/own|Q2 R3|S4 Q18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|171|B/own|S4 S6|A4 A18|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|173|B/own|N1 I1|A2 Q19|yes|2000/5|safe_one_reply/safe_one_reply|old=-; new=-|
|175|B/own|D19 C19|S17 A6|yes|1057/6|safe_one_reply/safe_one_reply|old=-; new=-|
|177|B/own|B19 A2|S16 D1|yes|76/6|safe_one_reply/safe_one_reply|old=-; new=-|
|179|B/own|C1 D1|Q19 R19|yes|5/6|safe_one_reply/safe_one_reply|old=-; new=-|
|181|B/own|S17 S16|S17 S16|yes|40/6|full_draw/full_draw|old=-; new=-|

已完成 91/91 个独立历史局面；其余未运行，不作推断。

## C6-重师Bvs沈阳建筑W-平局-aaa.txt

颜色：B；文件标签：draw；实际结果：draw；标签不符：否；合法前缀：是；核验 361/361 子；终局后忽略 0 子；重复来源：C6-重师Bvs沈阳建筑W-先手胜-zzz.txt。

## 审计

新动作非法 0；生产预算错误 0；正常黑方单子开局 16；累计搜索墙钟 1262314 ms；完整/已尝试双子枚举 54299678；单 oracle 最大 18 ms；oracle unknown 0。

双方/颜色战术控制：`{"non_chongshi_B":{"new_refuted":1,"new_safe_one_reply":38,"old_direct_loss":1,"old_refuted":1,"old_safe_one_reply":37,"samples":39},"non_chongshi_W":{"new_direct_win":1,"new_safe_one_reply":38,"old_direct_win":1,"old_illegal":1,"old_safe_one_reply":37,"samples":39},"opponent_control_B":{"new_direct_win":3,"new_safe_one_reply":33,"old_direct_win":3,"old_safe_one_reply":33,"samples":36},"opponent_control_W":{"new_direct_win":4,"new_safe_one_reply":40,"old_direct_win":3,"old_safe_one_reply":41,"samples":44},"own_B":{"new_direct_loss":4,"new_direct_win":5,"new_full_draw":2,"new_refuted":6,"new_safe_one_reply":272,"old_direct_loss":4,"old_direct_win":5,"old_full_draw":2,"old_refuted":6,"old_safe_one_reply":272,"samples":289},"own_W":{"new_direct_loss":3,"new_direct_win":6,"new_refuted":3,"new_safe_one_reply":252,"old_direct_loss":3,"old_direct_win":6,"old_refuted":4,"old_safe_one_reply":251,"samples":264}}`

深度 -1 仅表示正常首子不进行搜索；深度 0 为生产战术快速返回或未完成迭代，不是深层证明。
