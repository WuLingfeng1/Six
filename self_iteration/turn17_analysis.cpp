#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>
#include <QElapsedTimer>
#include <algorithm>
#include <array>
#include <cstdio>
#include <vector>

#define private public
#include "Game.h"
#undef private

namespace {
using Windows = std::vector<std::vector<QPoint>>;
constexpr int BLACK = 1;
constexpr int WHITE = 2;

struct Referee {
    std::array<std::array<int, 19>, 19> board{};
    int stones = 0;
    int winner = 0;

    static bool inside(QPoint p) { return p.x() >= 0 && p.x() < 19 && p.y() >= 0 && p.y() < 19; }
    Windows windows(int color, size_t maxHoles) const {
        Windows result;
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
            for (QPoint d : {QPoint(1, 0), QPoint(0, 1), QPoint(1, 1), QPoint(1, -1)}) {
                if (!inside(QPoint(x, y) + d * 5)) continue;
                std::vector<QPoint> holes;
                bool blocked = false;
                for (int k = 0; k < 6; ++k) {
                    const int cell = board[x + d.x() * k][y + d.y() * k];
                    if (!cell) holes.push_back(QPoint(x, y) + d * k);
                    else if (cell != color) blocked = true;
                }
                if (!blocked && holes.size() <= maxHoles) result.push_back(std::move(holes));
            }
        return result;
    }
    Windows completions(int color) const { return windows(color, 2); }
    bool six(int color) const { return !windows(color, 0).empty(); }
    bool put(QPoint p, int color) {
        if (!inside(p) || board[p.x()][p.y()] || winner) return false;
        board[p.x()][p.y()] = color; ++stones;
        if (six(color)) winner = color;
        return true;
    }
    int emptyCount() const {
        int n = 0;
        for (const auto &row : board) for (int cell : row) n += cell == 0;
        return n;
    }
    static bool coverable(const Windows &ws) {
        if (ws.empty()) return true;
        for (const auto &w : ws) if (w.empty()) return false;
        for (QPoint a : ws.front()) {
            const std::vector<QPoint> *missed = nullptr;
            for (const auto &w : ws) if (std::find(w.begin(), w.end(), a) == w.end()) { missed = &w; break; }
            if (!missed) return true;
            for (QPoint b : *missed) {
                bool all = true;
                for (const auto &w : ws)
                    if (std::find(w.begin(), w.end(), a) == w.end()
                        && std::find(w.begin(), w.end(), b) == w.end()) { all = false; break; }
                if (all) return true;
            }
        }
        return false;
    }
};

QString notation(QPoint p) { return QString(QChar('A' + p.y())) + QString::number(19 - p.x()); }
QJsonArray points(const std::vector<QPoint> &ps) {
    QJsonArray a; for (QPoint p : ps) a.append(notation(p)); return a;
}
QString pairText(QPoint a, QPoint b) { return notation(a) + " " + notation(b); }
QJsonArray pairJson(QPoint a, QPoint b) { return points({a, b}); }
bool samePair(QPoint a, QPoint b, QPoint c, QPoint d) { return (a == c && b == d) || (a == d && b == c); }
QJsonArray windowJson(const Windows &ws) {
    QJsonArray a; for (const auto &w : ws) a.append(points(w)); return a;
}

bool load(const QString &path, int prefix, Referee &r, std::vector<QPoint> &all) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    const QRegularExpression re(";\\s*([BW])\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)");
    auto it = re.globalMatch(QString::fromUtf8(f.readAll()));
    while (it.hasNext()) {
        const auto m = it.next();
        QPoint p(19 - m.captured(3).toInt(), m.captured(2)[0].unicode() - 'A');
        all.push_back(p);
        if (int(all.size()) <= prefix && !r.put(p, m.captured(1) == "B" ? BLACK : WHITE)) return false;
    }
    return int(all.size()) >= prefix && r.stones == prefix && !r.winner;
}

struct Production {
    QJsonObject json;
    QPoint a{-1, -1}, b{-1, -1};
};
int productionDepth = -1;
Production runProduction(const Referee &before) {
    Game game;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) game.m_chess[x][y] = before.board[x][y];
    game.m_type = BLACK; game.m_step = 0; game.m_win = TIE; game.rebuildSearch();
    std::vector<QPoint> emitted;
    QObject::connect(&game, &Game::chessChange, [&](QPoint p, int color) { if (color == BLACK) emitted.push_back(p); });
    QElapsedTimer timer; timer.start();
    game.bestPlay();
    Production p;
    p.json["move"] = points(emitted);
    p.json["elapsed_ms"] = double(timer.elapsed());
    p.json["depth"] = productionDepth;
    p.json["score"] = game.m_chosenMove.score();
    p.json["timed_out"] = game.m_timedOut;
    if (emitted.size() >= 2) { p.a = emitted[0]; p.b = emitted[1]; p.json["pair"] = pairText(p.a, p.b); }
    return p;
}

struct Assessment {
    QString status = "unknown";
    qint64 checked = 0, total = 0, unsafe = 0, safe = 0;
    QJsonObject witness;
    QJsonObject focusReply;
    qint64 elapsed = 0;
};
Assessment assess(const Referee &before, QPoint a, QPoint b, qint64 deadline, qint64 globalDeadline,
                  QPoint focusA = {-1, -1}, QPoint focusB = {-1, -1}) {
    QElapsedTimer timer; timer.start();
    Assessment r;
    Referee after = before;
    if (!after.put(a, BLACK)) { r.status = "unknown"; return r; }
    if (after.winner == BLACK) { r.status = "two_ply_no_immediate_forcing"; r.elapsed = timer.elapsed(); return r; }
    if (!after.put(b, BLACK)) { r.status = "unknown"; return r; }
    std::vector<QPoint> legal;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) if (!after.board[x][y]) legal.push_back({x, y});
    r.total = qint64(legal.size()) * (legal.size() - 1) / 2;
    for (size_t i = 0; i < legal.size(); ++i) for (size_t j = i + 1; j < legal.size(); ++j) {
        if (((r.checked & 127) == 0 && timer.elapsed() >= deadline)
            || globalDeadline <= 0) {
            r.status = "unknown"; r.elapsed = timer.elapsed(); return r;
        }
        ++r.checked;
        Referee reply = after;
        if (!reply.put(legal[i], WHITE)) continue;
        const bool focus = samePair(legal[i], legal[j], focusA, focusB);
        if (reply.winner == WHITE || !reply.put(legal[j], WHITE)) {
            ++r.unsafe;
            if (focus) r.focusReply = QJsonObject{{"status", "unsafe"}, {"kind", "white_direct_six"}};
            if (r.witness.isEmpty()) r.witness = QJsonObject{{"reply", pairJson(legal[i], legal[j])}, {"kind", "white_direct_six"}};
            continue;
        }
        const Windows direct = reply.completions(WHITE);
        const bool blackCanCover = reply.emptyCount() >= 2 && Referee::coverable(direct);
        if (focus) r.focusReply = QJsonObject{{"status", blackCanCover ? "safe" : "unsafe"},
            {"white_direct_completions", windowJson(direct)}};
        if (blackCanCover) ++r.safe;
        else {
            ++r.unsafe;
            if (r.witness.isEmpty()) r.witness = QJsonObject{{"reply", pairJson(legal[i], legal[j])},
                {"kind", "white_direct_completions_uncoverable"}, {"white_direct_completions", windowJson(direct)}};
        }
    }
    r.status = r.unsafe ? "refuted" : "two_ply_no_immediate_forcing";
    r.elapsed = timer.elapsed();
    return r;
}
QJsonObject assessmentJson(const Assessment &a) {
    return {{"status", a.status}, {"checked", double(a.checked)}, {"total", double(a.total)},
            {"unsafe_reply_witnesses", double(a.unsafe)}, {"safe_reply_count", double(a.safe)},
            {"unknown_reply_count", a.status == "unknown" ? 1 : 0}, {"witness", a.witness},
            {"focus_human_reply", a.focusReply}, {"elapsed_ms", double(a.elapsed)}};
}
QString option(const QStringList &args, const QString &key, const QString &fallback) {
    const int i = args.indexOf(key); return i >= 0 && i + 1 < args.size() ? args[i + 1] : fallback;
}
void closeDialogs(QTimer &timer) {
    QObject::connect(&timer, &QTimer::timeout, [] { for (QWidget *w : QApplication::topLevelWidgets())
        if (auto *box = qobject_cast<QMessageBox *>(w)) box->done(QMessageBox::Discard); });
    timer.start(10);
}
}

int main(int argc, char **argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &s) {
        const auto m = QRegularExpression("Search completedDepth: (\\d+)").match(s);
        if (m.hasMatch()) productionDepth = m.captured(1).toInt();
        fprintf(stderr, "%s\n", qPrintable(s));
    });
    QTimer dialogs; closeDialogs(dialogs);
    const QString source = option(app.arguments(), "--record", "/wlf/Six/程序原版压缩包/six/build/Desktop_Qt_6_11_1-Release/棋谱文件/C6-sixBvshumanW-先手胜-06.txt");
    const QString outDir = option(app.arguments(), "--out", "/tmp/turn17-analysis");
    const int perCandidateMs = option(app.arguments(), "--candidate-ms", "3000").toInt();
    const qint64 overallMs = option(app.arguments(), "--total-ms", "60000").toLongLong();
    if (!QDir(outDir).exists()) qFatal("Output directory must already exist");

    Referee before; std::vector<QPoint> history;
    QJsonObject report{{"source", source}, {"prefix_plies", 31}, {"next_color", "B"},
                       {"candidate_reply_deadline_ms", perCandidateMs}, {"overall_deadline_ms", double(overallMs)},
                       {"definition", "two_ply_no_immediate_forcing: every legal white double-stone reply is either not an immediate white six and has an exact legal black double-stone cover of all current white direct completions"}};
    if (!load(source, 31, before, history) || before.stones != 31) qFatal("Cannot restore prefix31");
    report["history_prefix"] = points(std::vector<QPoint>(history.begin(), history.begin() + 31));
    if (history.size() >= 33) report["historical_black_move"] = pairJson(history[31], history[32]);
    if (history.size() >= 35) report["actual_human_white_reply"] = pairJson(history[33], history[34]);
    report["before_black_direct_completions"] = windowJson(before.completions(WHITE));

    Production prod = runProduction(before);
    report["production_bestPlay"] = prod.json;
    Game rootGame;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) rootGame.m_chess[x][y] = before.board[x][y];
    rootGame.m_type = BLACK; rootGame.m_step = 0; rootGame.rebuildSearch();
    std::vector<Move> root;
    const int tactic = rootGame.generateCandidates(BLACK, root, 40, 25);
    report["generateCandidates_tactic"] = tactic;
    report["root_candidate_count"] = int(root.size());
    QJsonArray candidates;
    for (int i = 0; i < int(root.size()); ++i) {
        QJsonObject c{{"root_rank", i + 1}, {"pair", pairText(root[i].step1(), root[i].step2())},
                      {"move", pairJson(root[i].step1(), root[i].step2())}, {"score", root[i].score()}};
        c["production_move"] = samePair(root[i].step1(), root[i].step2(), prod.a, prod.b);
        c["historical_black_move"] = history.size() >= 33 && samePair(root[i].step1(), root[i].step2(), history[31], history[32]);
        candidates.append(c);
    }
    // The historical black move is expected to be in the production root list;
    // the following human white move is a focused reply, not another black candidate.
    QElapsedTimer overall; overall.start();
    QJsonArray analyzed; int twoPly = 0, refuted = 0, unknown = 0;
    for (const auto &v : candidates) {
        if (overall.elapsed() >= overallMs) { unknown += candidates.size() - analyzed.size(); break; }
        const auto c = v.toObject(); const auto move = c["move"].toArray();
        const QPoint a(19 - move[0].toString().mid(1).toInt(), move[0].toString()[0].unicode() - 'A');
        const QPoint b(19 - move[1].toString().mid(1).toInt(), move[1].toString()[0].unicode() - 'A');
        const bool isHistoricalBlack = history.size() >= 33 && samePair(a, b, history[31], history[32]);
        const QPoint focusA = history.size() >= 35 ? history[33] : QPoint(-1, -1);
        const QPoint focusB = history.size() >= 35 ? history[34] : QPoint(-1, -1);
        Assessment ar = assess(before, a, b, std::min<qint64>(perCandidateMs, overallMs - overall.elapsed()), overallMs - overall.elapsed(),
                               isHistoricalBlack ? focusA : QPoint(-1, -1), isHistoricalBlack ? focusB : QPoint(-1, -1));
        if (isHistoricalBlack && history.size() >= 35) {
            Referee focus = before;
            if (focus.put(a, BLACK) && !focus.winner && focus.put(b, BLACK)
                && focus.put(focusA, WHITE) && !focus.winner && focus.put(focusB, WHITE)) {
                const Windows direct = focus.completions(WHITE);
                ar.focusReply = QJsonObject{{"status", focus.emptyCount() >= 2 && Referee::coverable(direct) ? "safe" : "unsafe"},
                    {"white_direct_completions", windowJson(direct)}};
            } else {
                ar.focusReply = QJsonObject{{"status", "white_reply_not_legal_after_candidate"}};
            }
        }
        QJsonObject row = c; row["assessment"] = assessmentJson(ar); analyzed.append(row);
        if (ar.status == "two_ply_no_immediate_forcing") ++twoPly; else if (ar.status == "refuted") ++refuted; else ++unknown;
    }
    report["candidate_root_ranking"] = candidates;
    report["analyzed_candidates"] = analyzed;
    report["two_ply_no_immediate_forcing_count"] = twoPly;
    report["refuted_count"] = refuted; report["unknown_count"] = unknown;
    report["conclusion"] = twoPly ? "two_ply_safe_candidate_exists" : "turn17_no_avoiding_candidate_confirmed";
    const QByteArray json = QJsonDocument(report).toJson(QJsonDocument::Indented);
    QFile jf(QDir(outDir).filePath("turn17_analysis.json")); if (!jf.open(QIODevice::WriteOnly) || jf.write(json) != json.size()) qFatal("Cannot write JSON");
    QString md = "# Turn 17 Analysis\n\n";
    md += QString("Source: `%1`\n\nPrefix: 31 plies, next side **B**.\n\n").arg(source);
    md += QString("Production `bestPlay`: **%1**, elapsed %2 ms, depth %3.\n\n").arg(prod.json.value("pair").toString()).arg(prod.json.value("elapsed_ms").toInt()).arg(productionDepth);
    md += QString("Historical black move: **%1**; human white reply: **%2**.\n\n")
        .arg(history.size() >= 33 ? pairText(history[31], history[32]) : "unavailable")
        .arg(history.size() >= 35 ? pairText(history[33], history[34]) : "unavailable");
    md += QString("Candidates: %1; two-ply candidates: **%2**; refuted: %3; unknown: %4.\n\n").arg(candidates.size()).arg(twoPly).arg(refuted).arg(unknown);
    md += "`two_ply_no_immediate_forcing` is not a global proof: it only means every enumerated white double-stone reply has no immediate white six and has an exact black double-stone cover of white direct completions.\n\n";
    for (const auto &v : analyzed) { const auto c = v.toObject(); const auto a = c["assessment"].toObject();
        md += QString("- rank %1 `%2`: **%3**, unsafe witnesses %4, safe replies %5, unknown %6%7%8\n")
            .arg(c["root_rank"].toInt()).arg(c["pair"].toString()).arg(a["status"].toString()).arg(a["unsafe_reply_witnesses"].toInt())
            .arg(a["safe_reply_count"].toInt()).arg(a["unknown_reply_count"].toInt())
            .arg(c["production_move"].toBool() ? "; production" : "").arg(c["historical_black_move"].toBool() ? "; historical black" : "");
        if (!a["focus_human_reply"].toObject().isEmpty())
            md += QString("  Focus human reply `K12 J13`: **%1**.\n").arg(a["focus_human_reply"].toObject()["status"].toString());
    }
    if (!twoPly) md += "\n\n**Conclusion:** no two-ply candidate was confirmed; turn 17 cannot be confirmed avoidable under this analysis.\n";
    else md += "\n\n**Conclusion:** at least one concrete two-ply candidate was found; see the rows marked `two_ply_no_immediate_forcing`.\n";
    QFile mf(QDir(outDir).filePath("turn17_analysis.md")); if (!mf.open(QIODevice::WriteOnly) || mf.write(md.toUtf8()) != md.toUtf8().size()) qFatal("Cannot write Markdown");
    QTextStream(stdout) << json;
    return 0;
}
