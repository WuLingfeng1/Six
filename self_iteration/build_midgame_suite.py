#!/usr/bin/env python3
"""Freeze quiet midgames without consulting engine choices or match outcomes."""
import argparse
import hashlib
import json
from pathlib import Path
import re

from experiment_board import Board, load_suite, prefix_hash


def generate(records, count=8):
    sources = {}
    for path in sorted(records.glob("*.txt")):
        raw = path.read_bytes()
        tokens = re.findall(r";\s*([BW])\s*\(([^)]*)\)", raw.decode("utf-8"))
        moves = []
        for color, point in tokens:
            match = re.fullmatch(r"\s*([A-S])\s*,\s*(1[0-9]|[1-9])\s*", point)
            if not match:
                raise ValueError("malformed record move: " + str(path))
            moves.append((color, match[1] + match[2]))
        canonical = hashlib.sha256(json.dumps(moves, separators=(",", ":")).encode()).hexdigest()
        sources.setdefault(canonical, (path, raw, moves))
    positions = []
    used = set()
    for index in range(count):
        wanted = 1 if index % 2 == 0 else 2
        # Earliest eligible complete turn in [25, 49], one prefix per source.
        selected = None
        for canonical, (path, raw, moves) in sorted(sources.items()):
            if canonical in used:
                continue
            board, prefix = Board(), []
            for color, point in moves[:49]:
                try:
                    board.put(point, 1 if color == "B" else 2)
                except ValueError:
                    break
                prefix.append(point)
                if board.winner:
                    break
                if (len(prefix) < 25 or len(prefix) % 2 == 0 or board.next_color != wanted
                        or prefix[0] != "J10"):
                    continue
                if board.completions(1) or board.completions(2):
                    continue
                selected = {"id": "mid%02d" % (index + 1), "source": path.name,
                            "source_sha256": hashlib.sha256(raw).hexdigest(),
                            "record_sha256": canonical, "stones": len(prefix),
                            "current_color": "B" if wanted == 1 else "W", "quiet": True,
                            "prefix_moves": list(prefix), "prefix_sha256": prefix_hash(prefix)}
                used.add(canonical)
                break
            if selected:
                break
        if selected is None:
            raise ValueError("not enough distinct quiet records")
        positions.append(selected)
    return {"schema": "connect6-midgames-v1", "selection": {
        "rule": "canonical SHA256 order; one record each; alternating B/W; earliest quiet odd prefix in 25..49",
        "quiet": "neither color can complete a six-window with one or two stones",
        "outcomes_used": False, "engine_outputs_used": False,
        "unique_sources_available": len(sources)}, "positions": positions}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--records", type=Path, default=Path(__file__).resolve().parent.parent / "棋谱文件")
    parser.add_argument("--count", type=int, default=8)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    if args.count < 2 or args.count % 2:
        parser.error("--count must be positive, even, and at least 2")
    result = generate(args.records, args.count)
    args.out.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
    load_suite(args.out)
    print("froze", len(result["positions"]), "quiet midgames in", args.out)
