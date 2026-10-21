#!/usr/bin/env python3
"""Exercise serial scheduling, frozen-input resume, and stale-report rejection."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("runner", type=Path)
args = parser.parse_args()
worker = '''#!/usr/bin/env python3
import json,sys
if sys.argv[sys.argv.index('--mode')+1]=='config':
 seed=int(sys.argv[sys.argv.index('--seed')+1]) if '--seed' in sys.argv else 0
 print(json.dumps({'search_config':{'timeLimitMs':2000,'randomize':'--randomize' in sys.argv,'randomSeed':seed},
                   'tt_effective':False,'compiled_sources':{'stub':'test'}}));sys.exit(0)
for line in sys.stdin:
 if not line.startswith('BOARD '):continue
 board=line.split()[2:];a,b=[i for i,v in enumerate(board) if v=='0'][:2]
 print('MOVE 2',a//19,a%19,b//19,b%19,flush=True)
'''
with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    binary = root / "worker"
    binary.write_text(worker)
    binary.chmod(0o755)
    out = root / "reports"
    command = [sys.executable, str(Path(__file__).with_name("validation_runner.py")),
               "--match-runner", str(args.runner.resolve()), "--baseline", str(binary),
               "--candidate", str(binary), "--plies", "1", "--out-root", str(out)]
    subprocess.run(command, check=True, capture_output=True)
    report = json.loads((out / "summary.json").read_text())
    assert report["verified_games"] == 96 and len(report["cohorts"]) == 2
    assert all(c["net"] == 0 for c in report["cohorts"].values())
    paths = sorted(out.glob("*/match.json"))
    timestamps = [p.stat().st_mtime_ns for p in paths]
    subprocess.run(command, check=True, capture_output=True)
    assert timestamps == [p.stat().st_mtime_ns for p in paths]
    print("PASS 96 simulated short games, separate cohorts, verified resume without replay")
    changed = subprocess.run(command + ["--plies", "2"], capture_output=True)
    assert changed.returncode == 2 and b"existing plan differs" in changed.stderr
    overlap = subprocess.run(command + ["--midgame-seeds", "1102"], capture_output=True)
    assert overlap.returncode == 2 and b"nonoverlapping" in overlap.stderr
    data = json.loads(paths[0].read_text())
    data["workers"]["candidate"]["search_config"]["randomSeed"] = 0
    paths[0].write_text(json.dumps(data))
    stale = subprocess.run(command, capture_output=True)
    assert stale.returncode != 0 and b"configuration/source differs" in stale.stderr
    print("PASS changed-plan, overlapping-seed and stale-worker-report rejection")
