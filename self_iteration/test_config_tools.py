#!/usr/bin/env python3
"""Regression checks for effective parameter patching and experiment inputs."""
from pathlib import Path
import tempfile
import unittest

from config_tools import DEFAULT_WEIGHTS, gen_candidates, patch_header

HEADER = Path(__file__).resolve().parent.parent / "Game.h"


class ConfigTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name) / "Game.h"
        self.path.write_bytes(HEADER.read_bytes())

    def test_effective_weights_and_crlf(self):
        patch_header(self.path, "2,30,150,1500,4500,30000,90000,100000")
        data = self.path.read_bytes()
        self.assertIn(b"shapeOpenFour = 30000;", data)
        self.assertIn(b"scoreOfMyRoad[7]{ 0,1,20,300,2000,6000,100000 }", data)
        self.assertNotIn(b"\n", data.replace(b"\r\n", b""))

    def test_bool_decimal_and_scoped_patch(self):
        patch_header(self.path, params="threatExtension=false randomTemp=1250.5 maxDepth=8")
        data = self.path.read_text()
        self.assertIn("bool threatExtension = false;", data)
        self.assertIn("double randomTemp = 1250.5;", data)
        self.assertIn("int maxDepth = 8;", data)

    def test_obsolete_weights_fail_without_writing(self):
        before = self.path.read_bytes()
        with self.assertRaises(ValueError):
            patch_header(self.path, "0,1,20,300,2000,6000,100000")
        self.assertEqual(before, self.path.read_bytes())

    def test_invalid_patch_is_atomic(self):
        before = self.path.read_bytes()
        for params in ("maxDepth=8 missing=1", "maxDepth=8 maxDepth=9",
                       "randomTemp=nan", "rootMaxPoints=1", "ttEnabled=1",
                       "timeLimitMs=0", "shapeOpenFour=10000000"):
            with self.subTest(params=params), self.assertRaises(ValueError):
                patch_header(self.path, params=params)
            self.assertEqual(before, self.path.read_bytes())

    def test_candidate_generation_inherits_winner(self):
        a = gen_candidates(3, 42, DEFAULT_WEIGHTS, "innerPoints=16 threatExtension=false")
        self.assertEqual(a, gen_candidates(3, 42, DEFAULT_WEIGHTS,
                                          "innerPoints=16 threatExtension=false"))
        b = gen_candidates(3, 42, "1,40,200,2000,6000,40000,60000,100000")
        self.assertNotEqual(a[0]["weights"], b[0]["weights"])
        self.assertIn("innerPoints=16", a[0]["search_params"])
        self.assertIn("threatExtension=false", a[0]["search_params"])


if __name__ == "__main__":
    unittest.main()
