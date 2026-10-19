#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>
#include <array>
#include <vector>
#include <algorithm>
#include <cstdlib>

#define private public
#include "Game.h"
#undef private

namespace {
int completedDepth = -1;
void captureDepth(QtMsgType, const QMessageLogContext &, const QString &message) {
    const auto match = QRegularExpression("Search completedDepth: (\\d+)").match(message);
    if (match.hasMatch()) completedDepth = match.captured(1).toInt();
}
void applySearchConfig(const QStringList &a) {
    const auto readInt = [&](const char *key, int &field) {
        const int i = a.indexOf(QString::fromLatin1(key));
        if (i >= 0 && i + 1 < a.size()) { bool ok = false; const int v = a[i + 1].toInt(&ok); if (ok) field = v; }
    };
    const auto readI64 = [&](const char *key, qint64 &field) {
        const int i = a.indexOf(QString::fromLatin1(key));
        if (i >= 0 && i + 1 < a.size()) { bool ok = false; const qint64 v = a[i + 1].toLongLong(&ok); if (ok) field = v; }
    };
    readI64("--time-limit", Game::searchConfig.timeLimitMs);
    readI64("--probe-budget", Game::searchConfig.probeBudgetMs);
    readI64("--probe-discovery", Game::searchConfig.probeDiscoveryMs);
    readInt("--probe-points", Game::searchConfig.probePoints);
    readInt("--probe-replies", Game::searchConfig.probeReplies);
    readInt("--q-max-ply", Game::searchConfig.qMaxPly);
    readInt("--q-max-nodes", Game::searchConfig.qMaxNodes);
    readInt("--q-max-defenses", Game::searchConfig.qMaxDefenses);
    readInt("--max-depth", Game::searchConfig.maxDepth);
    readInt("--root-max-points", Game::searchConfig.rootMaxPoints);
    readInt("--root-max-pairs", Game::searchConfig.rootMaxPairs);
    readInt("--inner-points", Game::searchConfig.innerPoints);
    readInt("--inner-pairs-deep", Game::searchConfig.innerPairsDeep);
    readInt("--inner-pairs-shallow", Game::searchConfig.innerPairsShallow);
    if (a.contains("--randomize")) Game::searchConfig.randomize = true;
    readInt("--random-top-n", Game::searchConfig.randomTopN);
    readInt("--seed", Game::searchConfig.randomSeed);
    {
        const int i = a.indexOf(QString::fromLatin1("--random-temp"));
        if (i >= 0 && i + 1 < a.size()) { bool ok = false; const double v = a[i + 1].toDouble(&ok); if (ok) Game::searchConfig.randomTemp = v; }
    }
    if (Game::searchConfig.randomSeed != 0)
        Game::seedRandom(static_cast<unsigned>(Game::searchConfig.randomSeed));
}
struct Referee {
    std::array<std::array<int, 19>, 19> board{};
    int stones = 0;
    int winner = 0;
    int lastColor = 0;
    int runLength = 0;
    QString error;
    static bool inside(QPoint p) { return p.x() >= 0 && p.x() < 19 && p.y() >= 0 && p.y() < 19; }
    std::vector<std::vector<QPoint>> windows(int color, size_t holes) const {
        std::vector<std::vector<QPoint>> out;
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
            for (QPoint d : {QPoint(1, 0), QPoint(0, 1), QPoint(1, 1), QPoint(1, -1)}) {
                if (!inside(QPoint(x, y) + d * 5)) continue;
                std::vector<QPoint> missing; bool blocked = false;
                for (int k = 0; k < 6; ++k) { const QPoint p = QPoint(x, y) + d * k; const int v = board[p.x()][p.y()];
                    if (!v) missing.push_back(p); else if (v != color) blocked = true; }
                if (!blocked && missing.size() <= holes) out.push_back(missing);
            }
        return out;
    }
    bool six(int color) const { return !windows(color, 0).empty(); }
    std::vector<std::vector<QPoint>> completions(int color) const { return windows(color, 2); }
    static bool coverable(const std::vector<std::vector<QPoint>> &ws) {
        if (ws.empty()) return true;
        for (const auto &w : ws) if (w.empty()) return false;
        for (QPoint a : ws.front()) {
            const std::vector<QPoint> *missed = nullptr;
            for (const auto &w : ws) if (std::find(w.begin(), w.end(), a) == w.end()) { missed = &w; break; }
            if (!missed) return true;
            for (QPoint b : *missed) {
                bool all = true;
                for (const auto &w : ws) if (std::find(w.begin(), w.end(), a) == w.end() && std::find(w.begin(), w.end(), b) == w.end()) { all = false; break; }
                if (all) return true;
            }
        }
        return false;
    }
    bool put(QPoint p, int color) {
        const int expected = stones == 0 ? 1 : (stones <= 2 ? 2 : (((stones - 1) / 2) % 2 == 0 ? 2 : 1));
        const bool sameTurn = color == lastColor && runLength < 2;
        if (!inside(p) || (color != 1 && color != 2) || winner || (color != expected && !sameTurn)) { error = "turn, terminal, or coordinate"; return false; }
        if (board[p.x()][p.y()]) { error = "occupied"; return false; }
        board[p.x()][p.y()] = color; ++stones; if (color == lastColor) ++runLength; else { lastColor = color; runLength = 1; } if (six(color)) winner = color; return true;
    }
    QString oneLayer(int attacker, int deadlineMs) const {
        QElapsedTimer timer; timer.start();
        const auto attack = windows(attacker, 4), counter = completions(3 - attacker);
        std::vector<QPoint> legal;
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) if (!board[x][y]) legal.push_back({x, y});
        for (size_t i = 0; i < legal.size(); ++i) for (size_t j = i + 1; j < legal.size(); ++j) {
            if (timer.elapsed() >= deadlineMs) return "unknown";
            const QPoint a = legal[i], b = legal[j]; std::vector<std::vector<QPoint>> threats; bool wins = false;
            for (const auto &w : attack) { std::vector<QPoint> left; for (QPoint p : w) if (p != a && p != b) left.push_back(p); wins |= left.empty(); if (left.size() <= 2) threats.push_back(left); }
            bool counterWin = false; for (const auto &w : counter) if (std::find(w.begin(), w.end(), a) == w.end() && std::find(w.begin(), w.end(), b) == w.end()) { counterWin = true; break; }
            if (wins || (!counterWin && !coverable(threats))) return "refuted";
        }
        return "safe_one_reply";
    }
};

QJsonArray points(const std::vector<QPoint> &ps);
QJsonObject exhaustiveOneLayer(const Referee &before, int attacker, int deadlineMs) {
    QElapsedTimer timer; timer.start();
    const auto attack = before.windows(attacker, 4);
    const auto counter = before.completions(3 - attacker);
    std::vector<QPoint> legal;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
        if (!before.board[x][y]) legal.push_back({x, y});
    const double total = legal.size() < 2 ? 0 : double(legal.size()) * (legal.size() - 1) / 2;
    int checked = 0, refutations = 0; QJsonObject witness;
    for (size_t i = 0; i < legal.size(); ++i) for (size_t j = i + 1; j < legal.size(); ++j) {
        if ((checked & 127) == 0 && timer.elapsed() >= deadlineMs)
            return QJsonObject{{"status", "unknown"}, {"checked", checked}, {"total", total},
                {"refutations", refutations}, {"witness", witness}, {"elapsed_ms", timer.elapsed()}};
        ++checked;
        const QPoint a = legal[i], b = legal[j];
        bool wins = false; std::vector<std::vector<QPoint>> threats;
        for (const auto &w : attack) {
            std::vector<QPoint> left;
            for (QPoint p : w) if (p != a && p != b) left.push_back(p);
            wins |= left.empty();
            if (left.size() <= 2) threats.push_back(left);
        }
        bool counterWin = false;
        for (const auto &w : counter)
            if (std::find(w.begin(), w.end(), a) == w.end() && std::find(w.begin(), w.end(), b) == w.end()) {
                counterWin = true; break;
            }
        if (wins || (!counterWin && !Referee::coverable(threats))) {
            ++refutations;
            if (witness.isEmpty()) {
                QJsonArray threatArray; for (const auto &w : threats) threatArray.append(points(w));
                witness = QJsonObject{{"reply", points({a, b})},
                    {"kind", wins ? "opponent_six" : "uncoverable_no_counterwin"},
                    {"threats", threatArray}, {"own_immediate_counterwin", counterWin}};
            }
        }
    }
    return QJsonObject{{"status", refutations ? "refuted" : "safe_one_reply"}, {"checked", checked},
        {"total", total}, {"refutations", refutations}, {"witness", witness}, {"elapsed_ms", timer.elapsed()}};
}

QString sha(const QString &path) { QFile f(path); if (!f.open(QIODevice::ReadOnly)) return {}; return QString::fromLatin1(QCryptographicHash::hash(f.readAll(), QCryptographicHash::Sha256).toHex()); }
QString pt(QPoint p) { return QString(QChar('A' + p.y())) + QString::number(19 - p.x()); }
QJsonArray points(const std::vector<QPoint> &ps) { QJsonArray a; for (auto p : ps) a.append(pt(p)); return a; }
QString status(const Referee &before, int color, const std::vector<QPoint> &move, int &illegal, int oracleMs) {
    Referee p = before; const size_t need = before.stones == 0 ? 1 : 2; bool ok = move.size() == need;
    for (QPoint q : move) {
        if (!Referee::inside(q) || p.board[q.x()][q.y()] || p.winner) { ok = false; continue; }
        p.board[q.x()][q.y()] = color; ++p.stones; if (p.six(color)) p.winner = color;
    }
    if (!ok) { ++illegal; return "illegal"; }
    if (p.winner == color) return "direct_win";
    if (!p.completions(3 - color).empty()) return "direct_loss";
    return p.oneLayer(3 - color, oracleMs);
}
QStringList args() { return QCoreApplication::arguments(); }
QString option(const QStringList &a, const QString &key, const QString &fallback) { const int i = a.indexOf(key); return i >= 0 && i + 1 < a.size() ? a[i + 1] : fallback; }
void closeResultDialogs(QTimer &timer) {
    QObject::connect(&timer, &QTimer::timeout, [] {
        for (QWidget *widget : QApplication::topLevelWidgets())
            if (auto *box = qobject_cast<QMessageBox *>(widget))
                box->done(QMessageBox::Discard);
    });
    timer.start(10);
}
bool loadPrefix(const QString &path, int plies, Referee &r) {
    QFile f(path); if (!f.open(QIODevice::ReadOnly)) return false;
    auto it = QRegularExpression(";\\s*([BW])\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)").globalMatch(QString::fromUtf8(f.readAll()));
    int n = 0; while (it.hasNext() && n < plies) { auto m = it.next(); const QPoint p(19 - m.captured(3).toInt(), m.captured(2)[0].unicode() - 'A'); if (!r.put(p, m.captured(1) == "B" ? 1 : 2)) return false; ++n; }
    return n == plies && !r.winner;
}
QJsonObject runPosition(const QString &file, int prefix, const QString &root) {
    Referee before; QJsonObject r{{"source", file}, {"prefix_plies", prefix}};
    if (!loadPrefix(QDir(root).filePath(file), prefix, before)) { r["status"] = "invalid_fixture"; return r; }
    Game game; for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) game.m_chess[x][y] = before.board[x][y];
    const int color = before.stones == 0 ? 1 : ((((before.stones - 1) / 2) % 2) == 0 ? 2 : 1); game.m_type = color; game.m_step = 0; game.rebuildSearch();
    std::vector<QPoint> move; QObject::connect(&game, &Game::chessChange, [&](QPoint p, int c) { if (c == color) move.push_back(p); });
    QElapsedTimer timer; timer.start(); completedDepth = -1; if (before.stones == 0) game.whoFirst(1); else game.bestPlay();
    int illegal = 0; const QString s = status(before, color, move, illegal, 2000);
    r["color"] = color == 1 ? "B" : "W"; r["move"] = points(move); r["status"] = s; r["elapsed_ms"] = double(timer.elapsed()); r["completed_depth"] = completedDepth; r["timed_out"] = game.m_timedOut; r["illegal"] = illegal; return r;
}

std::vector<QPoint> nextHistoricalMove(const QString &path, int prefix) {
    QFile f(path); if (!f.open(QIODevice::ReadOnly)) return {};
    auto it = QRegularExpression(";\\s*[BW]\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)").globalMatch(QString::fromUtf8(f.readAll()));
    std::vector<QPoint> out; int n = 0;
    while (it.hasNext()) { auto m = it.next(); if (n++ >= prefix && out.size() < 2) out.push_back({19 - m.captured(2).toInt(), m.captured(1)[0].unicode() - 'A'}); }
    return out;
}

QJsonObject runCritical(const QString &path, int prefix, int forcedColor, bool white15, const QString &knownLabel = {}) {
    Referee before; QJsonObject r{{"source", path}, {"prefix_plies", prefix}, {"color", forcedColor == 1 ? "B" : "W"}};
    if (!loadPrefix(path, prefix, before)) { r["status"] = "invalid_fixture"; r["failed"] = true; return r; }
    if (white15 && (before.board[8][6] != 1 || before.board[4][7] != 1)) {
        r["status"] = "invalid_fixture"; r["failed"] = true; r["error"] = "G11/H15 are not black"; return r;
    }
    Game game; for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) game.m_chess[x][y] = before.board[x][y];
    game.m_type = forcedColor; game.m_step = 0; game.rebuildSearch(); std::vector<QPoint> move;
    QObject::connect(&game, &Game::chessChange, [&](QPoint p, int c) { if (c == forcedColor) move.push_back(p); });
    if (before.stones == 0) game.whoFirst(1); else game.bestPlay();
    Referee after = before; bool legal = move.size() == 2;
    // Critical records may intentionally force a color at a historical boundary.
    for (QPoint p : move) {
        if (!Referee::inside(p) || after.board[p.x()][p.y()] || after.winner) { legal = false; continue; }
        after.board[p.x()][p.y()] = forcedColor; ++after.stones; if (after.six(forcedColor)) after.winner = forcedColor;
    }
    r["move"] = points(move); r["legal"] = legal;
    if (!legal) { r["status"] = "illegal"; r["failed"] = true; return r; }
    const bool unavoidable = !before.completions(3 - forcedColor).empty() && !Referee::coverable(before.completions(3 - forcedColor));
    r["unavoidable"] = unavoidable;
    const bool directLoss = !after.completions(3 - forcedColor).empty();
    const QJsonObject oracle = exhaustiveOneLayer(after, 3 - forcedColor, 2000);
    r["oracle"] = oracle; r["witness"] = oracle["witness"]; r["checked"] = oracle["checked"];
    r["status"] = unavoidable ? "unavoidable" : (directLoss ? "direct_loss" : oracle["status"].toString());
    r["failed"] = !unavoidable && (directLoss || oracle["status"] == "refuted" || oracle["status"] == "unknown");
    if (!knownLabel.isEmpty()) {
        const auto historical = nextHistoricalMove(path, prefix); r["known_historical_move"] = points(historical);
        if (historical.size() == 2) { Referee hp = before; bool ok = true; for (QPoint p : historical) { ok = Referee::inside(p) && !hp.board[p.x()][p.y()] && !hp.winner && ok; if (ok) { hp.board[p.x()][p.y()] = forcedColor; ++hp.stones; if (hp.six(forcedColor)) hp.winner = forcedColor; } } r["known_historical_legal"] = ok; if (ok) r["known_historical_oracle"] = exhaustiveOneLayer(hp, 3 - forcedColor, 2000); }
        r["known_historical_label"] = knownLabel;
    }
    return r;
}
QVector<QPoint> forcedOpening(int openingId) {
    if (openingId == 0) return {QPoint(9, 8), QPoint(7, 10)};    // I10, K12
    if (openingId == 1) return {QPoint(12, 11), QPoint(5, 7)};   // L7, H14
    if (openingId == 2) return {QPoint(8, 7), QPoint(10, 11)};   // H11, L9
    if (openingId == 3) return {QPoint(11, 6), QPoint(7, 12)};   // G8, M12
    return {};
}
bool validOpening(int openingId) {
    const QVector<QPoint> opening = forcedOpening(openingId);
    if (opening.size() != 2) return false;
    Referee p;
    if (!p.put(QPoint(9, 9), 1) || !p.put(opening[0], 2) || !p.put(opening[1], 2)) return false;
    return p.stones == 3 && !p.winner && p.board[9][9] == 1 && p.board[opening[0].x()][opening[0].y()] == 2 && p.board[opening[1].x()][opening[1].y()] == 2;
}
QJsonObject runGame(int index, int maxPlies, int openingSet, const QString &sourceHash, const QString &configHash) {
    const int openingId = openingSet ? index % openingSet : -1;
    const QVector<QPoint> opening = openingSet ? forcedOpening(openingId) : QVector<QPoint>();
    QJsonArray moves;
    Referee p; QJsonObject r{{"game", index + 1}, {"max_double_turns", maxPlies}, {"opening_id", openingId}, {"opening_moves", points(std::vector<QPoint>(opening.begin(), opening.end()))}, {"source_sha256_game", sourceHash}, {"config_sha256", configHash}}; int illegal = 0, directWin = 0, directLoss = 0, refuted = 0; qint64 elapsed = 0; bool complete = false;
    if (openingSet) {
        if (!validOpening(openingId)) { r["result"] = "invalid_opening"; r["winner"] = 0; r["illegal"] = 1; return r; }
        const bool valid = p.put(QPoint(9, 9), 1) && p.put(opening[0], 2) && p.put(opening[1], 2) && !p.winner;
        if (!valid) { r["result"] = "invalid_opening"; r["winner"] = p.winner; r["illegal"] = 1; return r; }
        moves.append(pt(QPoint(9, 9)));
        moves.append(pt(opening[0]));
        moves.append(pt(opening[1]));
    }
    for (int turn = 0; turn < maxPlies && !p.winner && p.stones < 361; ++turn) {
        const int color = p.stones == 0 ? 1 : ((((p.stones - 1) / 2) % 2) == 0 ? 2 : 1); Game game; for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) game.m_chess[x][y] = p.board[x][y]; game.m_type = color; game.m_step = 0; game.rebuildSearch();
        std::vector<QPoint> move; QObject::connect(&game, &Game::chessChange, [&](QPoint q, int c) { if (c == color) move.push_back(q); }); QElapsedTimer timer; timer.start(); completedDepth = -1; if (p.stones == 0) game.whoFirst(1); else game.bestPlay(); elapsed += timer.elapsed();
        // Self-play keeps the independent oracle bounded; unknown is never a pass.
        const QString s = status(p, color, move, illegal, 10); if (s == "direct_win") ++directWin; if (s == "direct_loss") ++directLoss; if (s == "refuted") ++refuted;
         bool ok = true; for (QPoint q : move) { moves.append(pt(q)); ok &= p.put(q, color); } if (!ok || s == "illegal") { r["result"] = "illegal"; break; }
        if (p.winner || p.stones == 361) complete = true;
    }
    r["result"] = complete ? (p.winner ? (p.winner == 1 ? "black_win" : "white_win") : "draw") : "unfinished"; r["winner"] = p.winner; r["illegal"] = illegal; r["direct_wins"] = directWin; r["direct_losses"] = directLoss; r["one_layer_refuted"] = refuted; r["elapsed_ms"] = double(elapsed); r["moves"] = moves; return r;
}
bool write(const QString &path, const QByteArray &data) { QFile f(path); return f.open(QIODevice::WriteOnly) && f.write(data) == data.size(); }

bool parseWorkerBoard(const QStringList &tokens, int &color, int board[19][19]) {
    if (tokens.size() != 363 || (tokens[1] != "B" && tokens[1] != "W")) return false;
    color = tokens[1] == "B" ? 1 : 2;
    int n = 2;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) {
        bool ok = false; const int value = tokens[n++].toInt(&ok);
        if (!ok || (value != 0 && value != 1 && value != 2)) return false;
        board[x][y] = value;
    }
    return true;
}

int runWorker(const QStringList &arguments) {
    const QString fixedColor = option(arguments, "--color", "");
    if (fixedColor != "B" && fixedColor != "W") {
        QTextStream(stderr) << "worker requires --color B or W\n";
        return 2;
    }
    QTextStream input(stdin), output(stdout);
    QString line;
    while (input.readLineInto(&line)) {
        const QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (tokens.isEmpty()) continue;
        if (tokens[0] != "BOARD") { output << "END\n" << Qt::flush; continue; }
        int color = 0; int board[19][19]{};
        if (!parseWorkerBoard(tokens, color, board) || (color == 1 ? "B" : "W") != fixedColor) {
            output << "END\n" << Qt::flush;
            continue;
        }
        Game game;
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) game.m_chess[x][y] = board[x][y];
        game.m_type = color; game.m_step = 0; game.rebuildSearch();
        std::vector<QPoint> move;
        QObject::connect(&game, &Game::chessChange, [&](QPoint p, int c) { if (c == color) move.push_back(p); });
        int stones = 0; for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) stones += board[x][y] != 0;
        completedDepth = -1;
        if (stones == 0) game.whoFirst(1); else game.bestPlay();
         // A first-child win is a legal one-stone turn; never emit a second child.
         if (move.size() == 2) {
             Referee afterFirst;
             for (int x = 0; x < 19; ++x)
                 for (int y = 0; y < 19; ++y)
                     afterFirst.board[x][y] = board[x][y];
             afterFirst.stones = stones;
             afterFirst.board[move[0].x()][move[0].y()] = color;
             if (afterFirst.six(color)) move.resize(1);
         }
         output << "MOVE " << move.size();
        for (const QPoint p : move) output << ' ' << p.x() << ' ' << p.y();
        output << '\n' << Qt::flush;
        bool terminal = false;
        for (const QPoint p : move) {
            board[p.x()][p.y()] = color;
            int dx[] = {1, 0, 1, 1}, dy[] = {0, 1, 1, -1};
            for (int d = 0; d < 4 && !terminal; ++d) {
                int count = 1;
                for (int sign : {-1, 1}) for (int step = 1; step < 6; ++step) {
                    const int x = p.x() + sign * step * dx[d], y = p.y() + sign * step * dy[d];
                    if (x < 0 || x >= 19 || y < 0 || y >= 19 || board[x][y] != color) break;
                    ++count;
                }
                terminal |= count >= 6;
            }
        }
        if (terminal) output << "END\n" << Qt::flush;
    }
    return 0;
}
}

int main(int argc, char **argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen"); QApplication app(argc, argv); QTimer dialogCloser; closeResultDialogs(dialogCloser); qInstallMessageHandler(captureDepth); const QStringList a = args();
    applySearchConfig(a);
    if (option(a, "--mode", "") == "worker") return runWorker(a);
    if (option(a, "--mode", "") != "baseline") { QTextStream(stderr) << "only --mode baseline is supported\n"; return 2; }
    const int games = option(a, "--games", "2").toInt(), plies = option(a, "--plies", "30").toInt(), openingSet = option(a, "--opening-set", "0").toInt(); const QString out = option(a, "--out", "");
    if (games < 1 || games > 40 || (openingSet != 0 && openingSet != 1 && openingSet != 2 && openingSet != 4) || plies < 1 || plies > 30 || out.isEmpty() || !QDir(out).exists()) { QTextStream(stderr) << "unsupported parameters or output directory\n"; return 2; }
    if (openingSet) for (int id = 0; id < openingSet; ++id) if (!validOpening(id)) { QTextStream(stderr) << "invalid fixed opening: " << id << '\n'; return 2; }
    const bool criticalRegression = !a.contains("--no-critical-regression");
    const QString fixtureRoot = option(a, "--fixture-root", "/wlf/Six/程序原版压缩包/six");
    const QString sourceRoot = option(a, "--source-root", fixtureRoot);
    const QStringList fixtures = {"棋谱文件/C6-重师Bvs北京信息科技大学W-先手胜-祁门一中.txt", "棋谱文件/C6-北京信息科技大学Bvs重师W-后手胜-祁门一中.txt", "棋谱文件/C6-重师Bvs吉林大学一队W-后手胜-祁门一中.txt", "棋谱文件/C6-重师Bvs安庆师范大学W-后手胜-祁门一中.txt"};
    const QList<int> prefixes = {8, 16, 24, 12}; QJsonArray positions; for (int i = 0; i < fixtures.size(); ++i) positions.append(runPosition(fixtures[i], prefixes[i], fixtureRoot));
    QJsonArray critical; bool criticalFailed = false;
    if (criticalRegression) {
        const QString white15 = "/wlf/Six/server/six_server/build/Desktop_Qt_6_11_1-Debug/棋谱文件/C6-serverBvscilentW-先手胜-.txt";
        const QString historical = "/wlf/Six/程序原版压缩包/six/棋谱文件/C6-安徽大学Bvs重师W-先手胜-f.txt";
        critical.append(runCritical(white15, 57, 2, true));
        critical.append(runCritical(historical, 51, 2, false, "historical_next_move"));
        for (const auto &v : critical) criticalFailed |= v.toObject()["failed"].toBool();
    }
    const QString gameHash = sha(QDir(sourceRoot).filePath("Game.cpp"));
    const QString headerHash = sha(QDir(sourceRoot).filePath("Game.h"));
    const QString moveHash = sha(QDir(sourceRoot).filePath("Move.cpp"));
    const QString configHash = QString::fromLatin1(QCryptographicHash::hash("default_game_params:time=2000;board=19;win=6;one_layer=2000", QCryptographicHash::Sha256).toHex());
    QJsonArray gamesOut; for (int i = 0; i < games; ++i) gamesOut.append(runGame(i, plies, openingSet, gameHash, configHash));
    int illegal = 0, directWins = 0, directLosses = 0, unfinished = 0;
    for (const auto &v : positions) { const auto o = v.toObject(); illegal += o["illegal"].toInt(); }
    for (const auto &v : gamesOut) { const auto o = v.toObject(); illegal += o["illegal"].toInt(); directWins += o["direct_wins"].toInt(); directLosses += o["direct_losses"].toInt(); unfinished += o["result"].toString() == "unfinished"; }
     QJsonObject result{{"mode", "baseline"}, {"position_tests", positions.size()}, {"games_requested", games}, {"plies_requested", plies}, {"opening_set", openingSet}, {"critical_regression", criticalRegression}, {"critical_failed", criticalFailed}, {"illegal_count", illegal}, {"direct_win_count", directWins}, {"direct_loss_count", directLosses}, {"unfinished_count", unfinished}, {"time_budget_ms", 2000}, {"source_sha256_game", gameHash}, {"source_sha256_header", headerHash}, {"source_sha256_move", moveHash}, {"config_sha256", configHash}, {"qt_version", qVersion()}, {"compiler", "not_recorded"}, {"build_abi", QSysInfo::buildAbi()}, {"positions", positions}, {"games", gamesOut}, {"critical_regressions", critical}};
    if (!write(QDir(out).filePath("baseline.json"), QJsonDocument(result).toJson(QJsonDocument::Indented))) return 3;
    QByteArray csv = "kind,id,status,elapsed_ms,illegal\n"; for (const auto &v : positions) { const auto o = v.toObject(); csv += "position," + QByteArray::number(o["prefix_plies"].toInt()) + "," + o["status"].toString().toLatin1() + "," + QByteArray::number(o["elapsed_ms"].toDouble()) + "," + QByteArray::number(o["illegal"].toInt()) + "\n"; } for (const auto &v : gamesOut) { const auto o = v.toObject(); csv += "game," + QByteArray::number(o["game"].toInt()) + "," + o["result"].toString().toLatin1() + "," + QByteArray::number(o["elapsed_ms"].toDouble()) + "," + QByteArray::number(o["illegal"].toInt()) + "\n"; } if (!write(QDir(out).filePath("baseline.csv"), csv)) return 3;
    const QString md = QString("# 自动迭代基线\n\n- 局面测试：%1\n- 自对弈：%2 局，每局最多 %3 双子回合\n- 预算：2000 ms/步\n- Game.cpp SHA256：`%4`\n- Move.cpp SHA256：`%5`\n- 配置 SHA256：`%6`\n\n完整明细见 `baseline.json`，表格见 `baseline.csv`。`unfinished`、`unknown`、超时和非法均不判胜。\n").arg(positions.size()).arg(games).arg(plies).arg(result["source_sha256_game"].toString(), result["source_sha256_move"].toString(), result["config_sha256"].toString()); if (!write(QDir(out).filePath("baseline.md"), md.toUtf8())) return 3;
    QTextStream(stdout) << QJsonDocument(result).toJson(QJsonDocument::Compact) << '\n'; return illegal || criticalFailed ? 4 : 0;
}
