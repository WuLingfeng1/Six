#!/usr/bin/env python3
"""Check replay against known first-stone and second-stone wins."""
import copy
import unittest

from summarize_match import verify_game


class ReplayTests(unittest.TestCase):
    def black_win(self):
        moves = [("B", ["G10", "H10"]), ("W", ["A1", "B1"]),
                 ("B", ["I10", "K10"]), ("W", ["C2", "D2"]), ("B", ["L10"])]
        return {"opening_moves": ["I11", "K11"], "winner": 1, "result": "black_win",
                "stones": 12, "illegal": 0,
                "transcript": [{"color": c, "move": m} for c, m in moves]}

    def test_first_stone_six_is_legal(self):
        self.assertEqual(verify_game(self.black_win()), "black_win")

    def test_second_stone_white_six(self):
        moves = [("B", ["G10", "H11"]), ("W", ["C1", "D1"]),
                 ("B", ["G12", "H13"]), ("W", ["E1", "F1"])]
        game = {"opening_moves": ["A1", "B1"], "winner": 2, "result": "white_win",
                "stones": 11, "illegal": 0,
                "transcript": [{"color": c, "move": m} for c, m in moves]}
        self.assertEqual(verify_game(game), "white_win")

    def test_reject_wrong_winner_post_terminal_and_half_turn(self):
        original = self.black_win()
        wrong = copy.deepcopy(original)
        wrong["winner"] = 2
        orphan = copy.deepcopy(original)
        orphan["transcript"][-1]["move"].append("M10")
        incomplete = copy.deepcopy(original)
        incomplete["transcript"][0]["move"] = ["G10"]
        for game in (wrong, orphan, incomplete):
            with self.assertRaises(ValueError):
                verify_game(game)

    def test_midgame_prefix_and_first_stone_terminal(self):
        original = self.black_win()
        prefix = ["J10"] + original["opening_moves"]
        for turn in original["transcript"][:-1]:
            prefix.extend(turn["move"])
        game = dict(original, prefix_moves=prefix, current_color="B", initial_stones=11,
                    transcript=original["transcript"][-1:])
        self.assertEqual(verify_game(game), "black_win")
        game["current_color"] = "W"
        with self.assertRaises(ValueError):
            verify_game(game)

    def test_midgame_reject_half_turn_and_stale_hash(self):
        game = dict(self.black_win(), prefix_moves=["J10", "I11"], current_color="W")
        with self.assertRaises(ValueError):
            verify_game(game)
        game.update(prefix_moves=["J10", "I11", "K11"], current_color="B", prefix_sha256="stale")
        with self.assertRaises(ValueError):
            verify_game(game)


if __name__ == "__main__":
    unittest.main()
