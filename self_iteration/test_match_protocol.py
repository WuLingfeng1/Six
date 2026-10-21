#!/usr/bin/env python3
"""Exercise rejection of malformed worker moves without running an AI search."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("runner", type=Path)
args = parser.parse_args()
worker = '''#!/usr/bin/env python3
import json,sys
if '--mode' in sys.argv and sys.argv[sys.argv.index('--mode')+1]=='config':
 print(json.dumps({'protocol_test_stub':True}));sys.exit(0)
for line in sys.stdin:
 if not line.startswith('BOARD '):continue
 if BAD:
  print(BAD,flush=True)
 else:
  board=line.split()[2:];legal=[i for i,v in enumerate(board) if v=='0']
  a,b=legal[:2];print('MOVE 2',a//19,a%19,b//19,b%19,flush=True)
'''
with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    baseline = root / "baseline"
    baseline.write_text(worker.replace("BAD", "''"))
    baseline.chmod(0o755)
    for name, bad in (("out-of-range", "MOVE 2 99 0 0 0"),
                      ("half-turn", "MOVE 1 0 0"),
                      ("malformed", "MOVE 2 x 0 0 0")):
        candidate = root / name
        candidate.write_text(worker.replace("BAD", repr(bad)))
        candidate.chmod(0o755)
        out = root / (name + ".json")
        run = subprocess.run([str(args.runner.resolve()), "--black-worker", str(baseline),
                              "--white-worker", str(candidate), "--games", "1", "--plies", "1",
                              "--opening-set", "1", "--out", str(out)], capture_output=True)
        assert run.returncode == 4, (name, run.returncode, run.stderr)
        report = json.loads(out.read_text())
        game = report["scenarios"][1]["games"][0]
        assert game["result"] == "illegal" and game["illegal"] == 1 and game["winner"] == 0
        print("PASS", name)
