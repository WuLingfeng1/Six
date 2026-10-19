#pragma once
#include <QPoint>
#include <QString>
#include <algorithm>
#include <array>
#include <vector>

// Independent six-cell windows and exhaustive legal-pair oracle.
struct Referee {
    std::array<std::array<int, 19>, 19> board{};
    int stones = 0;
    int winner = 0;
    QString error;
    static bool inside(QPoint p) {
        return p.x() >= 0 && p.x() < 19 && p.y() >= 0 && p.y() < 19;
    }
    std::vector<std::vector<QPoint>> emptyWindows(int color, size_t maxHoles) const {
        std::vector<std::vector<QPoint>> result;
        for (int x = 0; x < 19; ++x)
            for (int y = 0; y < 19; ++y)
                for (QPoint d : {QPoint(1, 0), QPoint(0, 1), QPoint(1, 1), QPoint(1, -1)}) {
                    if (!inside(QPoint(x, y) + d * 5)) continue;
                    std::vector<QPoint> holes;
                    bool blocked = false;
                    for (int k = 0; k < 6; ++k) {
                        QPoint p = QPoint(x, y) + d * k;
                        int cell = board[p.x()][p.y()];
                        if (!cell) holes.push_back(p);
                        else if (cell != color) blocked = true;
                    }
                    if (!blocked && holes.size() <= maxHoles) result.push_back(holes);
                }
        return result;
    }
    std::vector<std::vector<QPoint>> completions(int color) const {
        return emptyWindows(color, 2);
    }
    bool six(int color) const { return !emptyWindows(color, 0).empty(); }
    static bool coverable(const std::vector<std::vector<QPoint>> &windows,
                          int stonesRemaining = 2) {
        if (windows.empty()) return true;
        for (const auto &holes : windows) if (holes.empty()) return false;
        for (QPoint a : windows.front()) {
            const std::vector<QPoint> *missed = nullptr;
            for (const auto &holes : windows)
                if (std::find(holes.begin(), holes.end(), a) == holes.end()) {
                    missed = &holes;
                    break;
                }
            if (!missed) return true;
            if (stonesRemaining < 2) continue;
            for (QPoint b : *missed) {
                bool covers = true;
                for (const auto &holes : windows)
                    if (std::find(holes.begin(), holes.end(), a) == holes.end()
                        && std::find(holes.begin(), holes.end(), b) == holes.end()) {
                        covers = false;
                        break;
                    }
                if (covers) return true;
            }
        }
        return false;
    }
    bool put(QPoint p, int color, bool turns = true) {
        int expected = stones == 0 ? 1 : (((stones - 1) / 2) % 2 == 0 ? 2 : 1);
        if (!inside(p) || (color != 1 && color != 2) || winner
            || (turns && color != expected)) {
            error = "invalid coordinate, color, turn or terminal position";
            return false;
        }
        if (board[p.x()][p.y()]) { error = "occupied point"; return false; }
        board[p.x()][p.y()] = color;
        ++stones;
        if (six(color)) winner = color;
        return true;
    }
    bool forcingPair(int color, QPoint &a, QPoint &b, size_t &checked) const {
        // Only <=4-hole windows can become <=2-hole threats in a dual turn.
        const auto attack = emptyWindows(color, 4);
        const auto counter = completions(3 - color);
        std::vector<QPoint> legal;
        for (int x = 0; x < 19; ++x)
            for (int y = 0; y < 19; ++y)
                if (!board[x][y]) legal.push_back({x, y});
        checked = 0;
        bool forced = false;
        for (size_t i = 0; i < legal.size(); ++i)
            for (size_t j = i + 1; j < legal.size(); ++j) {
                ++checked;
                const QPoint c = legal[i], d = legal[j];
                std::vector<std::vector<QPoint>> threats;
                bool wins = false;
                for (const auto &holes : attack) {
                    std::vector<QPoint> left;
                    for (QPoint p : holes) if (p != c && p != d) left.push_back(p);
                    if (left.empty()) wins = true;
                    if (left.size() <= 2) threats.push_back(left);
                }
                bool counterWin = false;
                for (const auto &holes : counter)
                    if (std::find(holes.begin(), holes.end(), c) == holes.end()
                        && std::find(holes.begin(), holes.end(), d) == holes.end()) {
                        counterWin = true;
                        break;
                    }
                if (wins || (!coverable(threats) && !counterWin)) {
                    if (!forced) { a = c; b = d; }
                    forced = true;
                }
            }
        return forced;
    }
};
