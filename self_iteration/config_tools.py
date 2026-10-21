#!/usr/bin/env python3
"""Shared, strict configuration patching for isolated candidate sources."""
import argparse
import json
import math
from pathlib import Path
import random
import re

SHAPE_FIELDS = (
    "shapeSingle", "shapeOpenTwo", "shapeClosedThree", "shapeOpenThree",
    "shapeClosedFour", "shapeOpenFour", "shapeFive", "shapeSix",
)
DEFAULT_WEIGHTS = "1,20,100,1000,3000,20000,60000,100000"
FIELD = re.compile(rb"\b(int|qint64|double|bool)\s+(\w+)\s*=\s*([^;]+);")


def config_block(data):
    match = re.search(rb"struct SearchConfig\s*\{(.*?)\n\s*\};", data, re.S)
    if not match:
        raise ValueError("SearchConfig not found")
    return match


def parse_weights(value):
    parts = value.split(",")
    if len(parts) != len(SHAPE_FIELDS) or any(not re.fullmatch(r"\d+", x) for x in parts):
        raise ValueError("weights require 8 integers: " + ",".join(SHAPE_FIELDS)
                         + "; the obsolete 7 road weights are not supported")
    weights = list(map(int, parts))
    if any(x < 1 or x > 100000 for x in weights):
        raise ValueError("shape weights must be in 1..100000")
    return weights


def parse_params(value):
    result = {}
    for item in value.split():
        if item.count("=") != 1:
            raise ValueError("expected key=value: " + item)
        key, val = item.split("=")
        if not re.fullmatch(r"[A-Za-z]\w*", key) or key in result:
            raise ValueError("invalid or duplicate parameter: " + key)
        result[key] = val
    return result


def checked_value(kind, key, value):
    if kind == b"bool":
        if value not in ("true", "false"):
            raise ValueError(key + " requires true or false")
        return value
    if kind == b"double":
        number = float(value)
        if not math.isfinite(number) or number <= 0 or number > 1e9:
            raise ValueError(key + " requires a finite positive number <= 1e9")
        return repr(number)
    if not re.fullmatch(r"\d+", value):
        raise ValueError(key + " requires a nonnegative integer")
    number = int(value)
    zero_ok = {"randomSeed", "probeBudgetMs", "probeDiscoveryMs", "qMaxPly",
               "qMaxNodes", "threatExtendMax", "threatExtendBudget"}
    upper = 100000 if key in SHAPE_FIELDS else 2147483647
    if number > upper or (number == 0 and key not in zero_ok):
        raise ValueError("out-of-range parameter: " + key)
    if key in {"rootMaxPoints", "innerPoints", "probePoints"} and not 2 <= number <= 361:
        raise ValueError(key + " must be in 2..361")
    if key == "maxDepth" and not 1 <= number <= 64:
        raise ValueError("maxDepth must be in 1..64")
    return str(number)


def patch_header(path, weights=None, params=""):
    """Validate the whole patch before writing; preserve original line endings."""
    path = Path(path)
    data = path.read_bytes()
    block = config_block(data)
    changes = parse_params(params)
    if weights is not None:
        for key, value in zip(SHAPE_FIELDS, parse_weights(weights)):
            if key in changes:
                raise ValueError("weight specified twice: " + key)
            changes[key] = str(value)
    fields = {m[2].decode(): m for m in FIELD.finditer(block[1])}
    for key in changes:
        if key not in fields:
            raise ValueError("unknown SearchConfig field: " + key)
        changes[key] = checked_value(fields[key][1], key, changes[key])
    body = block[1]
    for key in sorted(changes, key=lambda k: fields[k].start(), reverse=True):
        field = fields[key]
        body = body[:field.start(3)] + changes[key].encode() + body[field.end(3):]
    path.write_bytes(data[:block.start(1)] + body + data[block.end(1):])


def gen_candidates(count, seed, weights=DEFAULT_WEIGHTS, params=""):
    rng = random.Random(seed)
    base = parse_weights(weights)
    inherited = parse_params(params)
    result = []
    for i in range(1, count + 1):
        varied = list(base)
        for k in range(1, 7):
            varied[k] = min(100000, max(1, round(base[k] * 2 ** rng.uniform(-1, 1))))
        search = dict(inherited)
        search.update(maxDepth=str(rng.choice([5, 6, 7, 8])),
                      qMaxNodes=str(rng.choice([512, 1024, 2048, 4096])),
                      qMaxPly=str(rng.choice([1, 2, 3])))
        result.append({"id": "cand%d" % i, "weights": ",".join(map(str, varied)),
                       "search_params": " ".join(k + "=" + v for k, v in search.items())})
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("header", type=Path)
    parser.add_argument("--weights")
    parser.add_argument("--params", default="")
    args = parser.parse_args()
    try:
        patch_header(args.header, args.weights, args.params)
    except (ValueError, OSError) as error:
        parser.exit(2, str(error) + "\n")
