#!/usr/bin/env python3
"""Exercise the real single-position search and reject malformed prefixes."""
import copy
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from experiment_board import load_prefix, prefix_hash

binary = str(Path(sys.argv[1]).resolve())
prefix = ["J10", "I10", "K12"]
valid = {"id": "inspection", "prefix_moves": prefix, "prefix_sha256": prefix_hash(prefix), "current_color": "B"}
with tempfile.TemporaryDirectory(prefix="six-position-") as directory:
    path = Path(directory) / "position.json"
    def run(spec):
        path.write_text(json.dumps(spec))
        return subprocess.run([binary, "--mode", "position", "--position-json", str(path),
                               "--max-depth", "1", "--time-limit", "60000",
                               "--probe-budget", "0", "--probe-discovery", "0", "--incremental-tactics"],
                              env=dict(os.environ, QT_QPA_PLATFORM="offscreen"), capture_output=True, text=True)
    result = run(valid)
    assert result.returncode == 0, result.stderr
    report = json.loads(result.stdout)
    board = load_prefix(prefix)
    for point in report["move"]:
        board.put(point, 1)
    assert len(report["move"]) == 2 and report["illegal"] == 0
    stats = report["search_stats"]
    assert stats["completed_depth"] == 1 and stats["stop_reason"] == "max_depth"
    assert stats["nodes"] > 0 and stats["nodes"] == sum(i["nodes"] for i in stats["iterations"])
    assert stats["tt_hits"] == sum(i["tt_hits"] for i in stats["iterations"])
    assert stats["extensions_used"] == sum(i["extensions_used"] for i in stats["iterations"])
    assert report["prefix_sha256"] == valid["prefix_sha256"]
    assert report["search_config"]["incrementalTactics"] is True
    bad = []
    for key, value in (("prefix_sha256", "0" * 64), ("current_color", "W"),
                       ("prefix_moves", prefix[:-1]), ("prefix_moves", ["J10", "I10", "I10"]),
                       ("prefix_moves", ["J10", "Z10", "K12"])):
        spec = copy.deepcopy(valid)
        spec[key] = value
        bad.append(spec)
    for spec in bad:
        assert run(spec).returncode == 2
print("PASS real position search, independent legality, statistics and five invalid-prefix cases")
