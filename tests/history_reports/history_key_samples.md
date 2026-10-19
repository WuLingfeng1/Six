# 关键历史局面

以下仅是独立原历史局面的着法替换与一回复战术检查，不是重赛或整局胜率。`当前直接无法防` 指对手现有 completion 无两子 cover，且我方无即时六连着法；新动作 refuted 不自动等价于所有动作都输。

|文件/回合|色/角色|当前直接无法防|旧动作/结论|新动作/结论|具体反例|
|---|---|---|---|---|---|
|C6-aaaBvsxxxW-后手胜-z.txt / 53 (B 27)|B/non_chongshi|no|L8 R8 / refuted|K8 Q8 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["O10","O7"],"threats":[["O12","O11"],["O11","O6"],["O6","O5"],["L10","P6"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["O10","O7"],"threats":[["O12","O11"],["O11","O6"],["O6","O5"],["L10","P6"]]}|
|C6-aaaBvsxxxW-后手胜-z.txt / 65 (B 33)|B/non_chongshi|no|K17 F15 / direct_loss|O13 N15 / safe_one_reply|old={"kind":"opponent_completion","reply":["O13","P12"]}; new=-|
|C6-aaaBvsxxxW-后手胜-z.txt / 66 (W 33)|W/non_chongshi|no|O13 P12 / direct_win|O13 P12 / direct_win|old=-; new=-|
|C6-东北秦皇岛分校Bvs重师W-先手胜-祁门一中.txt / 16 (W 8)|W/own|no|L7 Q7 / refuted|K7 Q7 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["O10","O8"],"threats":[["O12","O11"],["O11","O6"],["O6","O5"],["M8","P8"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["O10","O8"],"threats":[["O12","O11"],["O11","O6"],["O6","O5"],["M8","P8"]]}|
|C6-东北秦皇岛分校Bvs重师W-先手胜-祁门一中.txt / 18 (W 9)|W/own|yes|O8 P11 / direct_loss|A19 B19 / direct_loss|old={"kind":"opponent_completion","reply":["P6","P5"]}; new={"kind":"opponent_completion","reply":["P12","P11"]}|
|C6-东北秦皇岛分校Bvs重师W-先手胜-祁门一中.txt / 19 (B 10)|B/opponent_control|no|P6 P5 / direct_win|P6 P5 / direct_win|old=-; new=-|
|C6-北京信息科技大学Bvs重师W-后手胜-祁门一中.txt / 38 (W 19)|W/own|no|L11 L13 / direct_win|L13 L11 / direct_win|old=-; new=-|
|C6-吉林大学一队Bvs重师W-后手胜-祁门一中.txt / 44 (W 22)|W/own|no|J13 K13 / direct_win|D13 E13 / direct_win|old=-; new=-|
|C6-吉林大学二队Bvs重师W-后手胜-祁门一中.txt / 22 (W 11)|W/own|no|I7 I6 / direct_win|I7 I6 / direct_win|old=-; new=-|
|C6-啊啊啊下Bvs说W-先手胜-暗示.txt / 6 (W 3)|W/non_chongshi|no|M10 / illegal|J9 L11 / safe_one_reply|old=-; new=-|
|C6-大连交通大学Bvs重师W-后手胜-祁门一中.txt / 24 (W 12)|W/own|no|K15 L16 / direct_win|L16 K15 / direct_win|old=-; new=-|
|C6-安庆师范大学Bvs重师W-先手胜-祁门一中.txt / 18 (W 9)|W/own|no|O10 I16 / refuted|H17 N11 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L17","K16"],"threats":[["N19","M18"],["M18","H13"],["L16","L15"],["H13","G12"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L17","K16"],"threats":[["N19","M18"],["M18","H13"],["L16","L15"],["H13","G12"]]}|
|C6-安庆师范大学Bvs重师W-先手胜-祁门一中.txt / 20 (W 10)|W/own|yes|H13 L15 / direct_loss|A19 B19 / direct_loss|old={"kind":"opponent_completion","reply":["N19","M18"]}; new={"kind":"opponent_completion","reply":["N19","M18"]}|
|C6-安庆师范大学Bvs重师W-先手胜-祁门一中.txt / 21 (B 11)|B/opponent_control|no|M18 N19 / direct_win|N19 M18 / direct_win|old=-; new=-|
|C6-安徽大学Bvs重师W-先手胜-f.txt / 26 (W 13)|W/own|no|O13 F14 / refuted|I11 F11 / safe_one_reply|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["I9","K9"],"threats":[["G7","F6"],["G9","H9"],["I11","L8"]]}; new=-|
|C6-安徽大学Bvs重师W-先手胜-f.txt / 48 (W 24)|W/own|no|G18 H19 / direct_win|H19 G18 / direct_win|old=-; new=-|
|C6-安徽大学Bvs重师W-先手胜-fff.txt / 26 (W 13)|W/own|no|M12 M7 / refuted|M12 M7 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["K8","J7"],"threats":[["O12","N11"],["N11","I6"],["I6","H5"],["J6","J5"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["K8","J7"],"threats":[["O12","N11"],["N11","I6"],["I6","H5"],["J6","J5"]]}|
|C6-安徽大学Bvs重师W-先手胜-fff.txt / 28 (W 14)|W/own|yes|N11 I6 / direct_loss|A19 B19 / direct_loss|old={"kind":"opponent_completion","reply":["J6","J5"]}; new={"kind":"opponent_completion","reply":["O12","N11"]}|
|C6-安徽大学Bvs重师W-先手胜-fff.txt / 29 (B 15)|B/opponent_control|no|J6 J5 / direct_win|J6 J5 / direct_win|old=-; new=-|
|C6-沈阳建筑Bvs重师W-先手胜-zzz.txt / 106 (W 53)|W/own|no|Q4 P3 / direct_win|Q4 P3 / direct_win|old=-; new=-|
|C6-重师BvsqW-先手胜-ZXZ.txt / 11 (B 6)|B/own|no|I9 J9 / direct_win|I9 J9 / direct_win|old=-; new=-|
|C6-重师BvsqqW-先手胜-zx.txt / 37 (B 19)|B/own|no|I8 H7 / direct_win|I8 H7 / direct_win|old=-; new=-|
|C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt / 35 (B 18)|B/own|no|E9 L3 / refuted|E9 L3 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}|
|C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt / 37 (B 19)|B/own|no|P8 J2 / refuted|O7 J2 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}|
|C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt / 39 (B 20)|B/own|no|K5 Q5 / refuted|L5 Q5 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["F12","F10"],"threats":[["F11","F9"],["B12","C12"],["C13","G9"]]}|
|C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt / 45 (B 23)|B/own|no|J4 P4 / refuted|J4 P4 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["P6","M3"],"threats":[["R8","Q7"],["Q7","L2"],["L2","K1"],["M2","M1"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["P6","M3"],"threats":[["R8","Q7"],["Q7","L2"],["L2","K1"],["M2","M1"]]}|
|C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt / 47 (B 24)|B/own|yes|N2 M2 / direct_loss|A19 B19 / direct_loss|old={"kind":"opponent_completion","reply":["S8","R7"]}; new={"kind":"opponent_completion","reply":["S8","R7"]}|
|C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt / 48 (W 24)|W/opponent_control|no|O3 B13 / safe_one_reply|S8 R7 / direct_win|old=-; new=-|
|C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt / 49 (B 25)|B/own|yes|D13 P2 / direct_loss|A19 B19 / direct_loss|old={"kind":"opponent_completion","reply":["S8","R7"]}; new={"kind":"opponent_completion","reply":["A13","D13"]}|
|C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt / 50 (W 25)|W/opponent_control|no|R7 S8 / direct_win|S8 R7 / direct_win|old=-; new=-|
|C6-重师Bvs北京信息科技大学W-先手胜-祁门一中.txt / 23 (B 12)|B/own|no|O9 P10 / direct_win|P10 O9 / direct_win|old=-; new=-|
|C6-重师Bvs吉林大学一队W-后手胜-祁门一中.txt / 19 (B 10)|B/own|no|H14 N8 / refuted|H14 N8 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L12","L9"],"threats":[["L14","L13"],["L13","L8"],["L8","L7"],["N9","O9"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L12","L9"],"threats":[["L14","L13"],["L13","L8"],["L8","L7"],["N9","O9"]]}|
|C6-重师Bvs吉林大学一队W-后手胜-祁门一中.txt / 21 (B 11)|B/own|yes|L7 L13 / direct_loss|A19 B19 / direct_loss|old={"kind":"opponent_completion","reply":["N9","O9"]}; new={"kind":"opponent_completion","reply":["L13","L12"]}|
|C6-重师Bvs吉林大学一队W-后手胜-祁门一中.txt / 22 (W 11)|W/opponent_control|no|N9 O9 / direct_win|N9 O9 / direct_win|old=-; new=-|
|C6-重师Bvs吉林大学二队W-先手胜-祁门一中.txt / 25 (B 13)|B/own|no|P13 L9 / direct_win|P13 L9 / direct_win|old=-; new=-|
|C6-重师Bvs大连交通大学W-先手胜-祁门一中.txt / 25 (B 13)|B/own|no|M10 P7 / direct_win|M10 P7 / direct_win|old=-; new=-|
|C6-重师Bvs安庆师范大学W-后手胜-祁门一中.txt / 13 (B 7)|B/own|no|H13 N13 / refuted|H13 N13 / refuted|old={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L16","K15"],"threats":[["N18","M17"],["M17","H12"],["L15","L14"],["H12","G11"]]}; new={"kind":"uncoverable_no_counterwin","own_immediate_counterwin":false,"reply":["L16","K15"],"threats":[["N18","M17"],["M17","H12"],["L15","L14"],["H12","G11"]]}|
|C6-重师Bvs安庆师范大学W-后手胜-祁门一中.txt / 15 (B 8)|B/own|yes|H12 L14 / direct_loss|A19 B19 / direct_loss|old={"kind":"opponent_completion","reply":["N18","M17"]}; new={"kind":"opponent_completion","reply":["N18","M17"]}|
|C6-重师Bvs安庆师范大学W-后手胜-祁门一中.txt / 16 (W 8)|W/opponent_control|no|M17 N18 / direct_win|N18 M17 / direct_win|old=-; new=-|

当前已核验直接无法防局面 7；战术转移计数（绝非胜率）：`{"direct_loss -> direct_loss":7,"direct_loss -> safe_one_reply":1,"direct_win -> direct_win":18,"full_draw -> full_draw":2,"illegal -> safe_one_reply":1,"refuted -> refuted":10,"refuted -> safe_one_reply":1,"safe_one_reply -> direct_win":1,"safe_one_reply -> safe_one_reply":670}`。表格最多列 40 个关键例子；全部回合和全部 witness 见 history_details.json / history_details.csv / history_summary.md。
