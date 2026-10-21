"""Independent Connect6 rules used by experiment selection and report replay."""
import hashlib
import json
import re


def coordinate(point):
    if not isinstance(point, str) or not re.fullmatch(r"[A-S](?:[1-9]|1[0-9])", point):
        raise ValueError("invalid coordinate: " + str(point))
    return 19 - int(point[1:]), ord(point[0]) - ord("A")


def prefix_hash(moves):
    return hashlib.sha256(" ".join(moves).encode("ascii")).hexdigest()


class Board:
    def __init__(self):
        self.cells = {}
        self.winner = 0

    @property
    def count(self):
        return len(self.cells)

    @property
    def next_color(self):
        return 1 if self.count == 0 else (2 if ((self.count - 1) // 2) % 2 == 0 else 1)

    def put(self, point, color=None):
        row, col = coordinate(point)
        color = self.next_color if color is None else color
        if self.winner or (row, col) in self.cells or color != self.next_color:
            raise ValueError("occupied, wrong-color or post-terminal move")
        self.cells[row, col] = color
        for dr, dc in ((0, 1), (1, 0), (1, 1), (1, -1)):
            length = 1
            for sign in (-1, 1):
                k = 1
                while self.cells.get((row + sign * k * dr, col + sign * k * dc)) == color:
                    length += 1
                    k += 1
            if length >= 6:
                self.winner = color

    def completions(self, color):
        """All six-windows completable by at most two stones, without top-K."""
        found = set()
        for row in range(19):
            for col in range(19):
                for dr, dc in ((0, 1), (1, 0), (1, 1), (1, -1)):
                    if not (0 <= row + 5 * dr < 19 and 0 <= col + 5 * dc < 19):
                        continue
                    points = [(row + k * dr, col + k * dc) for k in range(6)]
                    if any(self.cells.get(p, 0) == 3 - color for p in points):
                        continue
                    holes = tuple(p for p in points if p not in self.cells)
                    if len(holes) <= 2:
                        found.add(holes)
        return found


def load_prefix(moves):
    if not isinstance(moves, list) or not moves or moves[0] != "J10" or len(moves) % 2 != 1:
        raise ValueError("prefix must start at J10 and end at a complete turn")
    board = Board()
    for point in moves:
        board.put(point)
    if board.winner:
        raise ValueError("terminal starting position")
    return board


def load_suite(path):
    return validate_suite(json.loads(path.read_text()))


def validate_suite(data):
    if data.get("schema") != "connect6-midgames-v1" or not data.get("positions"):
        raise ValueError("invalid midgame suite")
    ids, hashes = set(), set()
    for position in data["positions"]:
        name = position["id"]
        moves = position["prefix_moves"]
        board = load_prefix(moves)
        digest = prefix_hash(moves)
        if name in ids or digest in hashes:
            raise ValueError("duplicate position id or prefix")
        ids.add(name)
        hashes.add(digest)
        if position["prefix_sha256"] != digest or position["stones"] != board.count:
            raise ValueError("prefix fingerprint/count mismatch")
        if position["current_color"] != ("B" if board.next_color == 1 else "W"):
            raise ValueError("prefix current-color mismatch")
        if position.get("quiet") is True and (board.completions(1) or board.completions(2)):
            raise ValueError("quiet position has an immediate two-stone completion")
    return data
