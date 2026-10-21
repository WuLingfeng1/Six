#!/usr/bin/env python3
"""Fail before an experiment if the worker does not match its candidate inputs."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

from config_tools import FIELD, config_block


def verify_worker(binary, source):
    source = Path(source)
    result = subprocess.run([str(binary), "--mode", "config"], check=True,
                            env=dict(os.environ, QT_QPA_PLATFORM="offscreen"),
                            capture_output=True, text=True)
    metadata = json.loads(result.stdout)
    for name in ("Game.cpp", "Game.h", "Move.cpp", "Move.h"):
        expected = hashlib.sha256((source / name).read_bytes()).hexdigest()
        if metadata["compiled_sources"][name] != expected:
            raise ValueError("compiled source mismatch: " + name)
    expected_config = {}
    for field in FIELD.finditer(config_block((source / "Game.h").read_bytes())[1]):
        expected_config[field[2].decode()] = json.loads(field[3].decode().strip())
    if metadata["search_config"] != expected_config:
        raise ValueError("runtime defaults differ from candidate Game.h")
    return metadata


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("source", type=Path)
    args = parser.parse_args()
    try:
        print(json.dumps(verify_worker(args.binary, args.source), indent=2))
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.exit(2, "worker verification failed: " + str(error) + "\n")
