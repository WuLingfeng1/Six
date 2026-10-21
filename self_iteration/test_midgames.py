#!/usr/bin/env python3
"""Check frozen midgame selection and rejection of invalid starting positions."""
import copy
import json
from pathlib import Path
import unittest

from build_midgame_suite import generate
from experiment_board import Board, coordinate, load_prefix, load_suite, validate_suite

ROOT = Path(__file__).resolve().parent


class MidgameTests(unittest.TestCase):
    def setUp(self):
        self.data = load_suite(ROOT / "midgames/suite.json")

    def test_frozen_selection_reproduces_without_engine(self):
        self.assertEqual(generate(ROOT.parent / "棋谱文件"), self.data)
        positions = self.data["positions"]
        self.assertEqual(len({p["record_sha256"] for p in positions}), 8)
        self.assertEqual([p["current_color"] for p in positions].count("B"), 4)
        self.assertEqual([p["current_color"] for p in positions].count("W"), 4)

    def test_coordinate_orientation(self):
        self.assertEqual(coordinate("A19"), (0, 0))
        self.assertEqual(coordinate("S1"), (18, 18))
        self.assertEqual(coordinate("K12"), (7, 10))
        for invalid in ("T1", "A0", "A20", "10J", "J010"):
            with self.assertRaises(ValueError):
                coordinate(invalid)

    def test_completion_detector_has_no_top_k(self):
        board = Board()
        board.cells = {(9, c): 1 for c in (0, 1, 3, 5)}
        self.assertIn(((9, 2), (9, 4)), board.completions(1))
        self.assertFalse(board.completions(2))
        board.cells[9, 2] = 2
        self.assertFalse(board.completions(1))

    def test_reject_duplicate_color_and_hash_mismatch(self):
        changes = (lambda d: d["positions"].append(copy.deepcopy(d["positions"][0])),
                   lambda d: d["positions"][0].update(current_color="W"),
                   lambda d: d["positions"][0].update(prefix_sha256="stale"),
                   lambda d: d["positions"][0].update(stones=1))
        for change in changes:
            data = copy.deepcopy(self.data)
            change(data)
            with self.assertRaises(ValueError):
                validate_suite(data)

    def test_reject_occupied_half_turn_and_terminal_prefix(self):
        bad = (["J10", "J10", "K11"], ["J10", "I11"],
               ["J10", "I11", "K11", "G10", "H10", "A1", "B1",
                "I10", "K10", "C2", "D2", "L10", "M10"])
        for moves in bad:
            with self.assertRaises(ValueError):
                load_prefix(moves)


if __name__ == "__main__":
    unittest.main()
