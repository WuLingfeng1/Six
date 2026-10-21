#!/usr/bin/env python3
"""Independently replay A/B transcripts before reporting their net wins."""
import argparse
from collections import Counter
import json
from pathlib import Path

from experiment_board import Board, load_prefix, prefix_hash, validate_suite


def verify_game(game):
    if "prefix_moves" in game:
        moves = game["prefix_moves"]
        board = load_prefix(moves)
        if game.get("initial_stones", board.count) != board.count or game.get("prefix_plies", board.count) != board.count:
            raise ValueError("initial stone count mismatch")
        if game.get("prefix_sha256") and game["prefix_sha256"] != prefix_hash(moves):
            raise ValueError("initial prefix hash mismatch")
        if game.get("current_color") != ("B" if board.next_color == 1 else "W"):
            raise ValueError("initial current-color mismatch")
    else:
        board = Board()
        board.put("J10", 1)
        if len(game["opening_moves"]) != 2:
            raise ValueError("opening requires white pair")
        for point in game["opening_moves"]:
            board.put(point, 2)
    for index, turn in enumerate(game["transcript"], 1):
        if turn.get("turn", index) != index:
            raise ValueError("turn index mismatch")
        color = {"B": 1, "W": 2}[turn["color"]]
        if not 1 <= len(turn["move"]) <= 2:
            raise ValueError("invalid turn length")
        for point in turn["move"]:
            board.put(point, color)
        if len(turn["move"]) == 1 and not board.winner:
            raise ValueError("incomplete nonterminal turn")
    result = "black_win" if board.winner == 1 else "white_win" if board.winner == 2 else "unfinished"
    if game.get("illegal") or game["winner"] != board.winner or game["result"] != result or game["stones"] != board.count:
        raise ValueError("reported outcome does not match independent board")
    return result


def summarize(path):
    data = json.loads(Path(path).read_text())
    is_midgame = data["mode"] == "baseline-vs-candidate-midgames"
    if data["mode"] != "baseline-vs-candidate" and not is_midgame:
        raise ValueError("expected a fixed-opening or midgame A/B report")
    positions = validate_suite(data["position_suite"])["positions"] if is_midgame else []
    if is_midgame and data["games"] != len(positions) * data["games_per_position"]:
        raise ValueError("wrong midgame sample count")
    if data["games"] < 1:
        raise ValueError("empty match")
    stats, by_scenario = {}, {}
    for scenario in data["scenarios"]:
        games = scenario["games"]
        name = scenario["scenario"]
        if name in stats or len(games) != data["games"]:
            raise ValueError("duplicate or incomplete scenario")
        totals = Counter()
        for i, game in enumerate(games):
            if game["seed"] != data["seed_base"] + i or game["game"] != i + 1:
                raise ValueError("seed/game mismatch")
            if is_midgame:
                expected = positions[i % len(positions)]
                if (game["fixture"] != expected["id"] or game["prefix_moves"] != expected["prefix_moves"]
                        or game["prefix_sha256"] != expected["prefix_sha256"]
                        or game["source_sha256"] != expected["source_sha256"]
                        or game["repetition"] != i // len(positions) + 1):
                    raise ValueError("midgame input mismatch")
            elif game["opening_id"] != i % data["opening_set"]:
                raise ValueError("opening mismatch")
            if len(game["transcript"]) > data["plies"]:
                raise ValueError("too many turns")
            totals[verify_game(game)] += 1
        stats[name] = dict(totals)
        by_scenario[name] = games
    if set(stats) != {"control", "candidate-black", "candidate-white"}:
        raise ValueError("missing scenario")
    pairs = []
    per_position = {}
    for i, control in enumerate(by_scenario["control"]):
        key = control["fixture"] if is_midgame else str(control["opening_id"])
        row = per_position.setdefault(key, {"net": 0, "candidate_black_delta": 0,
                                           "candidate_white_delta": 0, "comparisons": 0})
        for side, color in (("black", 1), ("white", 2)):
            candidate_game = by_scenario["candidate-" + side][i]
            input_key = "prefix_moves" if is_midgame else "opening_moves"
            if control[input_key] != candidate_game[input_key]:
                raise ValueError("unpaired starting positions")
            delta = int(candidate_game["winner"] == color) - int(control["winner"] == color)
            row["net"] += delta
            row["candidate_" + side + "_delta"] += delta
            row["comparisons"] += 1
            pairs.append({"position": key, "seed": control["seed"], "candidate_side": side,
                          "control_result": control["result"], "candidate_result": candidate_game["result"],
                          "win_delta": delta})

    net_black = stats["candidate-black"].get("black_win", 0) - stats["control"].get("black_win", 0)
    net_white = stats["candidate-white"].get("white_win", 0) - stats["control"].get("white_win", 0)
    baseline = data["workers"]["baseline"]
    candidate = data["workers"]["candidate"]
    differences = {key: [baseline["search_config"].get(key), value]
                   for key, value in candidate["search_config"].items()
                   if baseline["search_config"].get(key) != value}
    return {"report": str(path), "seed_base": data["seed_base"], "games_per_scenario": data["games"],
            "mode": data["mode"], "opening_set": data.get("opening_set"), "scenarios": stats,
            "position_suite_sha256": data.get("position_suite_sha256"),
            "per_position": per_position, "paired_outcomes": pairs, "net_black": net_black,
            "net_white": net_white, "net": net_black + net_white,
            "verified_games": sum(len(s["games"]) for s in data["scenarios"]),
            "config_differences": differences,
            "tt_effective": [baseline["tt_effective"], candidate["tt_effective"]],
            "first_differences": sum(c["first_difference"] is not None for c in data["comparisons"])}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reports", nargs="+", type=Path)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    result = {"runs": [summarize(path) for path in args.reports]}
    result["net"] = sum(r["net"] for r in result["runs"])
    result["verified_games"] = sum(r["verified_games"] for r in result["runs"])
    output = json.dumps(result, ensure_ascii=False, indent=2) + "\n"
    if args.out:
        args.out.write_text(output)
    print(output)
