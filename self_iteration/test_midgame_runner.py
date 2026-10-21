#!/usr/bin/env python3
"""Exercise midgame referee input validation and both-color scheduling."""
import argparse
import copy
import json
from pathlib import Path
import subprocess
import tempfile

from summarize_match import summarize

parser = argparse.ArgumentParser()
parser.add_argument("runner", type=Path)
args = parser.parse_args()
suite = json.loads(Path(__file__).with_name("midgames").joinpath("suite.json").read_text())
worker = '''#!/usr/bin/env python3
import json,sys
if sys.argv[sys.argv.index('--mode')+1]=='config':
 print(json.dumps({'search_config':{},'tt_effective':False}));sys.exit(0)
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
    path, out = root / "suite.json", root / "match.json"

    def run(data):
        path.write_text(json.dumps(data))
        return subprocess.run([str(args.runner.resolve()), "--black-worker", str(binary),
            "--white-worker", str(binary), "--position-suite", str(path), "--games", "1",
            "--plies", "1", "--out", str(out)], capture_output=True)

    result = run(suite)
    assert result.returncode == 0, result.stderr
    report = summarize(out)
    assert report["verified_games"] == 24 and report["net"] == 0
    assert len(report["per_position"]) == 8
    print("PASS valid prefixes, both candidate colors, independent replay (24 short games)")
    for name, mutate in (
        ("stale-hash", lambda d: d["positions"][0].update(prefix_sha256="stale")),
        ("wrong-color", lambda d: d["positions"][0].update(current_color="W")),
        ("half-turn", lambda d: d["positions"][0]["prefix_moves"].pop()),
        ("occupied", lambda d: d["positions"][0]["prefix_moves"].__setitem__(1, "J10")),
        ("duplicate", lambda d: d["positions"].append(copy.deepcopy(d["positions"][0])))):
        data = copy.deepcopy(suite)
        mutate(data)
        result = run(data)
        assert result.returncode == 2, (name, result.returncode, result.stderr)
        print("PASS reject", name)
