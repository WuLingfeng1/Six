#include <QtTest>
#include <QApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QRadioButton>
#include <iostream>
#include <vector>
#define private public
#include "Game.h"
#undef private
#include "Chequer.h"
#include "Chesswoman.h"
#include "mainwindow.h"
#include "options.h"
#include "dialog_closer.h"
#include "referee.h"

namespace {
Referee snapshot(const Game &game) {
    Referee result;
    for (int x = 0; x < 19; ++x)
        for (int y = 0; y < 19; ++y) {
            result.board[x][y] = game.m_chess[x][y];
            result.stones += game.m_chess[x][y] != 0;
        }
    return result;
}
void seed(Game &game, const Referee &position, int color, int step = 0) {
    for (int x = 0; x < 19; ++x)
        for (int y = 0; y < 19; ++y) game.m_chess[x][y] = position.board[x][y];
    game.m_type = color;
    game.m_step = step;
    game.m_last = {2, 3};
    game.m_llast = {3, 2};
    game.rebuildSearch();
}
QPoint recordPoint(const QString &name) {
    return {19 - name.mid(1).toInt(), name[0].unicode() - 'A'};
}
bool white15Position(Referee &position, QString &error) {
    const QString path = qEnvironmentVariable("SIX_WHITE15_RECORD");
    QFile file(path);
    if (!QFileInfo(path).isAbsolute() || !file.open(QIODevice::ReadOnly)) {
        error = "SIX_WHITE15_RECORD must be the user's complete absolute record path: " + file.errorString();
        return false;
    }
    const QByteArray bytes = file.readAll();
    qInfo().noquote() << "record" << path << "sha256"
                     << QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex();
    auto matches = QRegularExpression(";\\s*([BW])\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)")
                       .globalMatch(QString::fromUtf8(bytes));
    QPoint last[2];
    while (position.stones < 57 && matches.hasNext()) {
        const auto match = matches.next();
        QPoint p = recordPoint(match.captured(2) + match.captured(3));
        last[position.stones % 2] = p;
        if (!position.put(p, match.captured(1) == "B" ? 1 : 2) || position.winner) {
            error = "invalid or terminal prefix: " + position.error;
            return false;
        }
    }
    if (position.stones != 57 || last[1] != recordPoint("G11") || last[0] != recordPoint("H15")) {
        error = "expected 57-stone prefix ending black15 G11 H15";
        return false;
    }
    return true;
}
bool anqingPrefix15(Referee &position, QString &error) {
    const QString path = qEnvironmentVariable("SIX_ANQING_RECORD",
        "/wlf/Six/程序原版压缩包/six/棋谱文件/C6-重师Bvs安庆师范大学W-后手胜-祁门一中.txt");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { error = file.errorString(); return false; }
    auto matches = QRegularExpression(";\\s*([BW])\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)")
                       .globalMatch(QString::fromUtf8(file.readAll()));
    while (position.stones < 15 && matches.hasNext()) {
        const auto match = matches.next();
        const QPoint p = recordPoint(match.captured(2) + match.captured(3));
        if (!position.put(p, match.captured(1) == "B" ? 1 : 2)) {
            error = position.error; return false;
        }
    }
    if (position.stones != 15 || position.winner) { error = "invalid anqing prefix15"; return false; }
    return true;
}
bool turn17Prefix31(Referee &position, QString &error) {
    const QString path = qEnvironmentVariable("SIX_TURN17_RECORD",
        "/tmp/opencode/all-human-six-records/C6-sixBvshumanW-先手胜-06.txt");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { error = file.errorString(); return false; }
    auto matches = QRegularExpression(";\\s*([BW])\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)")
                       .globalMatch(QString::fromUtf8(file.readAll()));
    while (position.stones < 31 && matches.hasNext()) {
        const auto match = matches.next();
        if (!position.put(recordPoint(match.captured(2) + match.captured(3)),
                          match.captured(1) == "B" ? 1 : 2)) {
            error = position.error; return false;
        }
    }
    if (position.stones != 31 || position.winner) { error = "invalid turn17 prefix31"; return false; }
    return true;
}
bool exactSafe(const Referee &before, QPoint a, QPoint b, int color) {
    const int enemy = 3 - color;
    if (before.board[a.x()][a.y()] || before.board[b.x()][b.y()] || a == b) return false;
    Referee after = before;
    after.board[a.x()][a.y()] = after.board[b.x()][b.y()] = color;
    if (after.six(color)) return true;
    const auto counter = after.completions(color);
    std::vector<QPoint> legal;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
        if (!after.board[x][y]) legal.push_back({x, y});
    for (size_t i = 0; i < legal.size(); ++i) for (size_t j = i + 1; j < legal.size(); ++j) {
        Referee reply = after;
        reply.board[legal[i].x()][legal[i].y()] = reply.board[legal[j].x()][legal[j].y()] = enemy;
        if (reply.six(enemy)) return false;
        bool counterWin = false;
        for (const auto &window : counter)
            if (std::find(window.begin(), window.end(), legal[i]) == window.end()
                && std::find(window.begin(), window.end(), legal[j]) == window.end()) {
                counterWin = true; break;
            }
        const auto threats = reply.completions(enemy);
        if (!counterWin && !Referee::coverable(threats)) return false;
    }
    return true;
}
int fullScore(const Referee &position, int color, const int *weights) {
    int score = 0;
    for (int x = 0; x < 19; ++x)
        for (int y = 0; y < 19; ++y)
            for (QPoint d : {QPoint(1, 0), QPoint(0, 1), QPoint(1, 1), QPoint(1, -1)}) {
                if (!Referee::inside(QPoint(x, y) + d * 5)) continue;
                int own = 0, enemy = 0;
                for (int k = 0; k < 6; ++k) {
                    QPoint p = QPoint(x, y) + d * k;
                    own += position.board[p.x()][p.y()] == color;
                    enemy += position.board[p.x()][p.y()] == 3 - color;
                }
                if (!own || !enemy) score += weights[own] - weights[enemy];
            }
    return score;
}
}

class OfflineTests : public QObject {
    Q_OBJECT
private slots:
    void blackOpening() {
        Game game;
        QSignalSpy changes(&game, &Game::chessChange);
        QSignalSpy undo(&game, &Game::undoenable);
        game.whoFirst(1);
        QCOMPARE(changes.size(), 1);
        QCOMPARE(changes[0][0].toPoint(), QPoint(9, 9));
        QCOMPARE(changes[0][1].toInt(), 1);
        QCOMPARE(snapshot(game).stones, 1);
        QCOMPARE(game.type(), 2);
        QCOMPARE(game.step(), 0);
        QCOMPARE(undo.size(), 1);
    }
    void turns_data() {
        QTest::addColumn<int>("color");
        QTest::addColumn<int>("step");
        QTest::addColumn<int>("requested");
        for (int color : {1, 2})
            for (int step : {0, 1})
                QTest::newRow(qPrintable(QString("color%1-step%2").arg(color).arg(step))) << color << step << 2;
        QTest::newRow("explicit-one") << 2 << 0 << 1;
    }
    void turns() {
        QFETCH(int, color); QFETCH(int, step); QFETCH(int, requested);
        Referee initial;
        initial.board[9][9] = 1;
        initial.board[8][9] = initial.board[10][10] = 2;
        Game game;
        seed(game, initial, color, step);
        const QPoint last = game.m_last, llast = game.m_llast;
        QSignalSpy changes(&game, &Game::chessChange);
        QSignalSpy saves(&game, &Game::save_signal);
        QElapsedTimer wall; wall.start();
        game.bestPlay(requested);
        const int count = step % 2 || requested == 1 ? 1 : 2;
        QVERIFY(wall.elapsed() <= 2300);
        QCOMPARE(game.m_timeLimitMs, 2000);
        QCOMPARE(changes.size(), count);
        for (const auto &change : changes) {
            QCOMPARE(change[1].toInt(), color);
            QVERIFY(initial.put(change[0].toPoint(), color, false));
        }
        QVERIFY(snapshot(game).board == initial.board);
        QCOMPARE(game.type(), 3 - color);
        QCOMPARE(game.step(), 0);
        QCOMPARE(game.m_last, last); QCOMPARE(game.m_llast, llast);
        QCOMPARE(saves.size(), 0);
        if (count == 1) QCOMPARE(game.m_chosenMove.step1(), game.m_chosenMove.step2());
        else QVERIFY(game.m_chosenMove.step1() != game.m_chosenMove.step2());
        qInfo() << "turn color" << color << "step" << step << "stones" << count << "elapsed_ms" << wall.elapsed();
    }
    void tactical_data() {
        QTest::addColumn<int>("color"); QTest::addColumn<QString>("scenario");
        for (int color : {1, 2})
            for (const char *name : {"single-six", "double-six", "live-four", "jump-four", "shared-defense", "uncoverable", "win-first"})
                QTest::newRow(qPrintable(QString("%1-color%2").arg(name).arg(color))) << color << QString(name);
    }
    void tactical() {
        QFETCH(int, color); QFETCH(QString, scenario);
        const int enemy = 3 - color;
        Referee initial;
        if (scenario == "single-six") {
            for (int x = 4; x <= 8; ++x) initial.board[x][9] = color;
            initial.board[3][9] = enemy;
        } else if (scenario == "double-six" || scenario == "jump-four") {
            const int line = scenario == "double-six" ? color : enemy;
            for (int x : {4, 5, 7, 9}) initial.board[x][9] = line;
            initial.board[3][9] = initial.board[10][9] = 3 - line;
        } else if (scenario == "live-four") {
            for (int x = 7; x <= 10; ++x) initial.board[x][9] = enemy;
        } else if (scenario == "shared-defense") {
            for (int i = 4; i <= 8; ++i) initial.board[i][9] = initial.board[9][i] = enemy;
            initial.board[3][9] = initial.board[9][3] = color;
            for (int x = 4; x <= 7; ++x) initial.board[x][14] = enemy;
            initial.board[3][14] = color;
        } else {
            for (int y : {4, 9, 14}) {
                for (int x = 4; x <= 8; ++x) initial.board[x][y] = enemy;
                initial.board[3][y] = color;
            }
            QVERIFY(!Referee::coverable(initial.completions(enemy)));
            if (scenario == "win-first") {
                for (int x = 10; x <= 14; ++x) initial.board[x][0] = color;
                initial.board[9][0] = enemy;
            }
        }
        const bool win = scenario.endsWith("six") || scenario == "win-first";
        Game game; seed(game, initial, color);
        QSignalSpy changes(&game, &Game::chessChange), saves(&game, &Game::save_signal), stop(&game, &Game::stop);
        game.bestPlay();
        QVERIFY(game.m_timer.elapsed() <= 2300);
        const int count = scenario == "single-six" || scenario == "win-first" ? 1 : 2;
        QCOMPARE(changes.size(), count);
        for (int i = 0; i < changes.size(); ++i) {
            QCOMPARE(changes[i][1].toInt(), color);
            QVERIFY(initial.put(changes[i][0].toPoint(), color, false));
            QCOMPARE(initial.six(color), win && i == count - 1);
        }
        QVERIFY(snapshot(game).board == initial.board);
        QCOMPARE(saves.size(), 0);
        QVERIFY(!stop.empty());
        QCOMPARE(game.type(), win ? color : enemy);
        QCOMPARE(game.step(), win ? count : 0);
        if (count == 1) {
            QPoint unused = game.m_chosenMove.step2();
            QVERIFY(Referee::inside(unused));
            QVERIFY(unused != changes[0][0].toPoint());
            QCOMPARE(game.m_chess[unused.x()][unused.y()], 0);
        }
        if (!win && scenario != "uncoverable") QVERIFY(initial.completions(enemy).empty());
        if (scenario == "uncoverable") QCOMPARE(game.m_chosenMove.score(), -100000000);
        if (win) QCOMPARE(game.m_chosenMove.score(), 100000000);
        if (scenario == "shared-defense") QCOMPARE(initial.board[9][9], color);
    }
    void independentSharedDefenseSymmetry() {
        // Three real six-cell windows share one defensive point.  The same
        // position is checked after swapping colors; no production evaluator
        // or candidate generator is involved in either oracle assertion.
        for (int color : {1, 2}) {
            Referee position;
            const int enemy = 3 - color;
            for (int i = 4; i <= 8; ++i) {
                position.board[9][i] = color;
                position.board[i][9] = color;
                position.board[i][i] = color;
            }
            position.board[8][8] = color;
            position.board[9][9] = 0;
            position.board[3][9] = position.board[9][3] = enemy;
            const auto threats = position.completions(color);
            std::vector<std::vector<QPoint>> sharedThreats;
            int shared = 0;
            for (const auto &window : threats)
                if (window.size() == 1 && window.front() == QPoint(9, 9)) {
                    ++shared;
                    sharedThreats.push_back(window);
                }
            QVERIFY2(shared >= 3, qPrintable(QString("shared windows: %1").arg(shared)));
            QVERIFY(Referee::coverable(sharedThreats));
            QVERIFY(!position.six(color));
            QVERIFY(!position.six(enemy));
        }
    }
    void white15Oracle() {
        Referee initial; QString error;
        QVERIFY2(white15Position(initial, error), qPrintable(error));
        Referee losing = initial;
        for (const char *p : {"E9", "G18"}) QVERIFY(losing.put(recordPoint(p), 2));
        for (const char *p : {"F11", "F13"}) QVERIFY(losing.put(recordPoint(p), 1));
        QVERIFY(!losing.six(1));
        QVERIFY(losing.completions(2).empty());
        QVERIFY(!Referee::coverable(losing.completions(1)));
        Referee defending = initial;
        for (const char *p : {"E9", "F14"}) QVERIFY(defending.put(recordPoint(p), 2));
        for (const char *p : {"F11", "F13"}) QVERIFY(defending.put(recordPoint(p), 1));
        QVERIFY(Referee::coverable(defending.completions(1)));
    }
    void halfTurnTactics_data() {
        QTest::addColumn<int>("color"); QTest::addColumn<QString>("scenario");
        for (int color : {1, 2})
            for (const char *name : {"win", "defense", "two-holes"})
                QTest::newRow(qPrintable(QString("%1-color%2").arg(name).arg(color))) << color << QString(name);
    }
    void halfTurnTactics() {
        QFETCH(int, color); QFETCH(QString, scenario);
        Referee initial;
        const int line = scenario == "defense" ? 3 - color : color;
        if (scenario == "two-holes") {
            for (int x : {4, 5, 7, 9}) initial.board[x][9] = line;
            initial.board[10][9] = 3 - line;
        } else {
            for (int x = 4; x <= 8; ++x) initial.board[x][9] = line;
        }
        initial.board[3][9] = 3 - line;
        Game game; seed(game, initial, color, 1);
        QSignalSpy changes(&game, &Game::chessChange), saves(&game, &Game::save_signal);
        game.ourplay();
        QCOMPARE(changes.size(), 1);
        QCOMPARE(changes[0][1].toInt(), color);
        QVERIFY(initial.put(changes[0][0].toPoint(), color, false));
        QVERIFY(snapshot(game).board == initial.board);
        QCOMPARE(game.m_chosenMove.step1(), game.m_chosenMove.step2());
        QCOMPARE(initial.six(color), scenario == "win");
        QCOMPARE(game.type(), scenario == "win" ? color : 3 - color);
        QCOMPARE(game.step(), scenario == "win" ? 2 : 0);
        QCOMPARE(saves.size(), 0);
        QVERIFY(game.m_timer.elapsed() <= 2300);
        if (scenario == "defense") QVERIFY(initial.completions(3 - color).empty());
    }
    void white15Search() {
        Referee initial; QString error;
        QVERIFY2(white15Position(initial, error), qPrintable(error));
        Game game; seed(game, initial, 2);
        game.m_last = recordPoint("H15"); game.m_llast = recordPoint("G11");
        QSignalSpy changes(&game, &Game::chessChange);
        QElapsedTimer wall; wall.start();
        game.bestPlay();
        const qint64 elapsed = wall.elapsed();
        QCOMPARE(game.m_timeLimitMs, 2000);
        QVERIFY(elapsed <= 2300);
        QCOMPARE(changes.size(), 2);
        for (const auto &change : changes) {
            QCOMPARE(change[1].toInt(), 2);
            QVERIFY2(initial.put(change[0].toPoint(), 2), qPrintable(initial.error));
        }
        QVERIFY(snapshot(game).board == initial.board);
        QCOMPARE(game.type(), 1); QCOMPARE(game.step(), 0);
        QCOMPARE(game.m_last, recordPoint("H15")); QCOMPARE(game.m_llast, recordPoint("G11"));
        for (const char *bad : {"E9", "D8"}) {
            QPoint a = recordPoint(bad), b = recordPoint("G18");
            QVERIFY(!((changes[0][0].toPoint() == a && changes[1][0].toPoint() == b)
                      || (changes[0][0].toPoint() == b && changes[1][0].toPoint() == a)));
        }
        QVERIFY(initial.completions(1).empty());
        QPoint a, b; size_t checked = 0;
        bool forced = initial.forcingPair(1, a, b, checked);
        qInfo() << "white15 elapsed_ms" << elapsed << "pair" << changes[0][0] << changes[1][0]
                << "all-legal-black-pairs" << checked << "forced" << forced << "witness" << a << b;
        QCOMPARE(checked, size_t(45451));
        QVERIFY2(!forced, "black can make six or uncoverable threats without a white counter-win");
    }
    void anqingPrefix15ExactDefense() {
        Referee initial; QString error;
        QVERIFY2(anqingPrefix15(initial, error), qPrintable(error));
        const auto direct = initial.completions(2);
        std::vector<QPoint> legal;
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
            if (!initial.board[x][y]) legal.push_back({x, y});
        int exactCovers = 0;
        for (size_t i = 0; i < legal.size(); ++i) for (size_t j = i + 1; j < legal.size(); ++j) {
            bool covers = true;
            for (const auto &window : direct)
                if (std::find(window.begin(), window.end(), legal[i]) == window.end()
                    && std::find(window.begin(), window.end(), legal[j]) == window.end()) {
                    covers = false; break;
                }
            exactCovers += covers;
        }
        QCOMPARE(exactCovers, 689);
        Game game; seed(game, initial, 1);
        QSignalSpy changes(&game, &Game::chessChange);
        QElapsedTimer wall; wall.start();
        game.bestPlay();
        const qint64 searchElapsed = wall.elapsed();
        QCOMPARE(changes.size(), 2);
        const QPoint a = changes[0][0].toPoint(), b = changes[1][0].toPoint();
        const bool oracleSafe = exactSafe(initial, a, b, 1);
        qInfo() << "anqing selected oracle_safe" << oracleSafe << "pair" << a << b;
        QVERIFY(oracleSafe);
        QVERIFY(initial.put(a, 1, false));
        QVERIFY(initial.put(b, 1, false));
        QVERIFY(snapshot(game).board == initial.board);
        QVERIFY(searchElapsed <= 2300);
        qInfo() << "anqing prefix15 elapsed_ms" << searchElapsed << "pair" << a << b;
    }
    void turn17Prefix31ExactDefense() {
        Referee initial; QString error;
        QVERIFY2(turn17Prefix31(initial, error), qPrintable(error));
        Game game; seed(game, initial, 1);
        std::vector<Move> candidates;
        game.generateCandidates(1, candidates, 40, 25, 2, 200);
        QCOMPARE(candidates.size(), size_t(3));

        seed(game, initial, 1);
        QSignalSpy changes(&game, &Game::chessChange);
        QElapsedTimer wall; wall.start();
        game.bestPlay();
        const qint64 searchElapsed = wall.elapsed();
        QCOMPARE(changes.size(), 2);
        const QPoint a = changes[0][0].toPoint(), b = changes[1][0].toPoint();
        QVERIFY(game.m_chosenMove.score() != -100000000);
        QVERIFY(exactSafe(initial, a, b, 1));
        QVERIFY(initial.put(a, 1, false));
        QVERIFY(initial.put(b, 1, false));
        QVERIFY(snapshot(game).board == initial.board);
        QVERIFY(searchElapsed <= 2300);
    }
    void pairDeltaAndAntisymmetry() {
        Referee initial;
        initial.board[7][9] = initial.board[8][9] = initial.board[9][9] = 1;
        initial.board[10][7] = 1;
        initial.board[9][7] = initial.board[8][8] = initial.board[6][10] = 2;
        Game game; seed(game, initial, 2);
        const auto search = game.m_search;
        const QPoint last = game.m_last, llast = game.m_llast;
        const QPoint pairs[][2] = {
            {{10, 9}, {11, 9}}, {{7, 8}, {7, 10}}, {{10, 10}, {11, 11}},
            {{10, 8}, {11, 7}}, {{0, 0}, {18, 18}}, {{10, 9}, {10, 9}},
            {{7, 9}, {10, 9}}, {{7, 9}, {8, 9}}
        };
        for (int color : {1, 2}) {
            game.setType(color);
            const int before = fullScore(initial, color, game.scoreOfMyRoad);
            QCOMPARE(game.evaluateBoard(), before);
            for (const auto &pair : pairs) {
                Referee after = initial;
                for (QPoint p : pair) if (!after.board[p.x()][p.y()]) after.board[p.x()][p.y()] = color;
                const int delta = fullScore(after, color, game.scoreOfMyRoad) - before;
                QCOMPARE(game.evaluatePair(pair[0], pair[1], color), delta);
                QCOMPARE(game.evaluatePair(pair[1], pair[0], color), delta);
                QCOMPARE(fullScore(after, 3 - color, game.scoreOfMyRoad)
                         - fullScore(initial, 3 - color, game.scoreOfMyRoad), -delta);
                if (pair[0] == pair[1]) QCOMPARE(game.evaluatePoint(pair[0].x(), pair[0].y(), color), delta);
                QVERIFY(snapshot(game).board == initial.board);
                QVERIFY(game.m_search == search);
                QCOMPARE(game.type(), color); QCOMPARE(game.step(), 0);
                QCOMPARE(game.m_last, last); QCOMPARE(game.m_llast, llast);
            }
        }
        game.setType(1); const int black = game.evaluateBoard();
        game.setType(2); QCOMPARE(game.evaluateBoard(), -black);
    }
    void timeoutRestoration() {
        Referee initial;
        initial.board[9][9] = 1;
        initial.board[8][9] = initial.board[10][10] = 2;
        Game game; seed(game, initial, 2, 1);
        const auto search = game.m_search;
        const QPoint last = game.m_last, llast = game.m_llast;
        game.m_timer.start(); game.m_timeLimitMs = 2000;
        game.negamax(6, -1000000000, 1000000000, 1);
        QVERIFY(game.m_timedOut); QVERIFY(game.m_timer.elapsed() <= 2300);
        qInfo() << "timeout restoration elapsed_ms" << game.m_timer.elapsed();
        QVERIFY(snapshot(game).board == initial.board); QVERIFY(game.m_search == search);
        QCOMPARE(game.type(), 2); QCOMPARE(game.step(), 1);
        QCOMPARE(game.m_last, last); QCOMPARE(game.m_llast, llast);
    }
    void undoRebuild() {
        Game game; game.whoFirst(1);
        const auto opening = snapshot(game);
        std::vector<QPoint> added;
        game.set({0, 0}); game.add({0, 0}, added);
        game.gameundo(0, 0); game.stepdec();
        QVERIFY(snapshot(game).board == opening.board);
        // rebuildSearch uses a full radius-two square, unlike legacy add's lines.
        std::vector<QPoint> expected;
        for (int x = 7; x <= 11; ++x)
            for (int y = 7; y <= 11; ++y)
                if (QPoint(x, y) != QPoint(9, 9)) expected.push_back({x, y});
        QVERIFY(game.m_search == expected);
        QCOMPARE(game.type(), 2); QCOMPARE(game.step(), 0);
        game.gameundo(9, 9);
        QVERIFY(game.m_search.empty()); QCOMPARE(snapshot(game).stones, 0);
    }
    void windowReplacementAndReconnect() {
        MainWindow window; window.show(); QTest::qWait(50);
        auto *chequer = window.findChild<Chequer *>("chequer");
        auto *options = window.findChild<Options *>("options");
        QVERIFY(chequer); QVERIFY(options);
        auto *black = options->findChild<QRadioButton *>("black");
        auto *undo = options->findChild<QPushButton *>("undo");
        QVERIFY(black); QVERIFY(undo); black->setChecked(true);
        for (int repeat = 0; repeat < 2; ++repeat) {
            QPointer<Game> old = chequer->getGame();
            QVERIFY(QMetaObject::invokeMethod(&window, "on_newAction_triggered"));
            QVERIFY(old.isNull());
            Game *fresh = chequer->getGame(); QVERIFY(fresh);
            QCOMPARE(snapshot(*fresh).stones, 0); QVERIFY(fresh->m_search.empty());
            QCOMPARE(chequer->getChesswoman().size(), size_t(0));
            QSignalSpy changes(fresh, &Game::chessChange);
            QSignalSpy restarts(fresh, &Game::restart);
            options->on_start_clicked();
            QCOMPARE(changes.size(), 1); QCOMPARE(restarts.size(), 1);
            QCOMPARE(snapshot(*fresh).stones, 1);
            QCOMPARE(fresh->type(), 2); QCOMPARE(fresh->step(), 0);
            QCOMPARE(chequer->getChesswoman().size(), size_t(1)); QVERIFY(undo->isEnabled());
            options->on_undo_clicked();
            QCOMPARE(snapshot(*fresh).stones, 0); QVERIFY(fresh->m_search.empty());
            QCOMPARE(chequer->getChesswoman().size(), size_t(0)); QVERIFY(!undo->isEnabled());
            Referee resumed;
            for (int x : {4, 5, 7, 9}) resumed.board[x][9] = 2;
            resumed.board[3][9] = resumed.board[10][9] = 1;
            seed(*fresh, resumed, 2);
            changes.clear(); restarts.clear();
            QSignalSpy gameSaves(fresh, &Game::save_signal);
            options->open_type(2);
            QCOMPARE(changes.size(), 2); QCOMPARE(restarts.size(), 1);
            QCOMPARE(chequer->getChesswoman().size(), size_t(2));
            QVERIFY(snapshot(*fresh).six(2)); QCOMPARE(gameSaves.size(), 0);
        }
        QSignalSpy saves(&window, &MainWindow::save_signal);
        QVERIFY(window.close()); QCOMPARE(saves.size(), 0);
        // Chequer's production destructor does not own/delete its Game.
        delete chequer->getGame();
    }
    void productionGuiSmoke() {
        QString executable = qEnvironmentVariable("SIX_GUI_BINARY");
        QVERIFY2(QFileInfo(executable).isAbsolute(), "SIX_GUI_BINARY must name the full GUI executable");
        QProcess gui;
        gui.setWorkingDirectory(qEnvironmentVariable("SIX_GUI_WORKDIR"));
        gui.setProcessChannelMode(QProcess::MergedChannels);
        gui.start(executable);
        QVERIFY2(gui.waitForStarted(5000), qPrintable(gui.errorString()));
        QTest::qWait(1000);
        const bool running = gui.state() == QProcess::Running;
        gui.terminate();
        if (!gui.waitForFinished(3000)) { gui.kill(); gui.waitForFinished(3000); }
        qInfo().noquote() << "production GUI" << executable << "alive_after_1000ms" << running << gui.readAll();
        QVERIFY(running);
    }
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QTimer closer; closeResultDialogs(closer);
    OfflineTests tests;
    return QTest::qExec(&tests, argc, argv);
}
#include "moc_offline_tests.cpp"
