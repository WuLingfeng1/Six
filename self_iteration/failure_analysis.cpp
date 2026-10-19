#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>
#include <algorithm>
#include <array>
#include <cstdio>
#include <tuple>
#include <vector>

#define private public
#include "Game.h"
#undef private

namespace {
using Windows = std::vector<std::vector<QPoint>>;
int productionDepth = -1;

struct Referee {
    std::array<std::array<int, 19>, 19> board{};
    int stones = 0;
    int winner = 0;
    QString error;

    static bool inside(QPoint p) { return p.x() >= 0 && p.x() < 19 && p.y() >= 0 && p.y() < 19; }
    Windows windows(int color, size_t maxHoles) const {
        Windows out;
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
                if (!blocked && holes.size() <= maxHoles) out.push_back(std::move(holes));
            }
        return out;
    }
    Windows completions(int color) const { return windows(color, 2); }
    bool six(int color) const { return !windows(color, 0).empty(); }
    static bool coverable(const Windows &ws) {
        if (ws.empty()) return true;
        for (const auto &w : ws) if (w.empty()) return false;
        for (QPoint a : ws.front()) {
            const std::vector<QPoint> *missed = nullptr;
            for (const auto &w : ws)
                if (std::find(w.begin(), w.end(), a) == w.end()) { missed = &w; break; }
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
    bool put(QPoint p, int color) {
        const int expected = stones == 0 ? 1 : (((stones - 1) / 2) % 2 == 0 ? 2 : 1);
        if (!inside(p) || (color != 1 && color != 2) || winner || color != expected || board[p.x()][p.y()]) {
            error = "invalid coordinate, color, turn, terminal position, or occupied point";
            return false;
        }
        board[p.x()][p.y()] = color; ++stones;
        if (six(color)) winner = color;
        return true;
    }
};

QString notation(QPoint p) { return QString(QChar('A' + p.y())) + QString::number(19 - p.x()); }
QJsonArray points(const std::vector<QPoint> &ps) {
    QJsonArray out; for (QPoint p : ps) out.append(notation(p)); return out;
}
QJsonArray windows(const Windows &ws) {
    QJsonArray out; for (const auto &w : ws) out.append(points(w)); return out;
}
QString pairText(QPoint a, QPoint b) { return notation(a) + " " + notation(b); }
QJsonArray pairArray(QPoint a, QPoint b) { return points({a, b}); }
int ordinal(QPoint p) { return p.x() * 19 + p.y(); }

bool loadPrefix(const QString &path, int prefix, Referee &r) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    auto matches = QRegularExpression(";\\s*([BW])\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)")
                       .globalMatch(QString::fromUtf8(file.readAll()));
    int loaded = 0;
    while (matches.hasNext() && loaded < prefix) {
        const auto m = matches.next();
        const QPoint p(19 - m.captured(3).toInt(), m.captured(2)[0].unicode() - 'A');
        if (!r.put(p, m.captured(1) == "B" ? 1 : 2)) return false;
        ++loaded;
    }
    return loaded == prefix && !r.winner;
}

bool apply(const Referee &before, QPoint a, QPoint b, int color, Referee &after) {
    after = before;
    if (!after.put(a, color)) return false;
    if (after.winner) return b == QPoint(-1, -1);
    return after.put(b, color);
}

struct ReplyResult { QString status; qint64 checked = 0; qint64 total = 0; qint64 refutations = 0; QJsonObject witness; qint64 elapsed = 0; };

ReplyResult replies(const Referee &position, int attacker, int deadlineMs) {
    QElapsedTimer timer; timer.start();
    const Windows attack = position.windows(attacker, 4);
    const Windows counter = position.completions(3 - attacker);
    std::vector<QPoint> legal;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
        if (!position.board[x][y]) legal.push_back({x, y});
    ReplyResult result;
    result.total = qint64(legal.size()) * (legal.size() - 1) / 2;
    std::array<std::vector<int>, 361> incidence;
    std::vector<int> base;
    for (size_t k = 0; k < attack.size(); ++k) {
        if (attack[k].size() <= 2) base.push_back(int(k));
        for (QPoint h : attack[k]) incidence[ordinal(h)].push_back(int(k));
    }
    std::vector<int> seen(attack.size());
    std::vector<int> affected;
    Windows threats;
    int stamp = 0;
    for (size_t i = 0; i < legal.size(); ++i) for (size_t j = i + 1; j < legal.size(); ++j) {
        if ((result.checked & 127) == 0 && timer.elapsed() >= deadlineMs) {
            result.status = "unknown"; result.elapsed = timer.elapsed(); return result;
        }
        ++result.checked; ++stamp; affected.clear(); threats.clear();
        const QPoint a = legal[i], b = legal[j];
        const auto include = [&](int k) { if (seen[k] != stamp) { seen[k] = stamp; affected.push_back(k); } };
        for (int k : base) include(k);
        for (int k : incidence[ordinal(a)]) include(k);
        for (int k : incidence[ordinal(b)]) include(k);
        bool wins = false;
        for (int k : affected) {
            std::vector<QPoint> left;
            for (QPoint h : attack[k]) if (h != a && h != b) left.push_back(h);
            wins |= left.empty();
            if (left.size() <= 2) threats.push_back(std::move(left));
        }
        bool counterWin = false;
        for (const auto &w : counter)
            if (std::find(w.begin(), w.end(), a) == w.end() && std::find(w.begin(), w.end(), b) == w.end()) {
                counterWin = true; break;
            }
        if (wins || (!counterWin && !Referee::coverable(threats))) {
            ++result.refutations;
            if (result.witness.isEmpty()) result.witness = QJsonObject{
                {"reply", pairArray(a, b)}, {"kind", wins ? "opponent_six" : "uncoverable_no_counterwin"},
                {"threats", windows(threats)}, {"own_immediate_counterwin", counterWin}};
        }
    }
    result.status = result.refutations ? "refuted" : "safe_one_reply";
    result.elapsed = timer.elapsed();
    return result;
}

QJsonObject replyJson(const ReplyResult &r) {
    return {{"status", r.status}, {"checked", double(r.checked)}, {"total", double(r.total)},
            {"refutations", double(r.refutations)}, {"witness", r.witness}, {"elapsed_ms", double(r.elapsed)}};
}

QJsonObject production(const Referee &before, int color) {
    Game game;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) game.m_chess[x][y] = before.board[x][y];
    game.m_type = color; game.m_step = 0; game.m_win = TIE; game.rebuildSearch();
    std::vector<QPoint> move;
    QObject::connect(&game, &Game::chessChange, [&](QPoint p, int c) { if (c == color) move.push_back(p); });
    productionDepth = -1;
    QElapsedTimer timer; timer.start();
    game.bestPlay();
    const qint64 elapsed = timer.elapsed();
    QJsonObject out{{"move", points(move)}, {"elapsed_ms", double(elapsed)}, {"depth", productionDepth},
                    {"score", game.m_chosenMove.score()}, {"timed_out", game.m_timedOut}};
    if (move.size() == 2) out["pair"] = pairText(move[0], move[1]);
    return out;
}

QJsonObject analyze(const QString &source, int prefix, int expectedColor, int oracleMs, const QString &root) {
    Referee before;
    QJsonObject out{{"source", source}, {"prefix_plies", prefix}, {"expected_color", expectedColor == 1 ? "B" : "W"}};
    if (!loadPrefix(QDir(root).filePath(source), prefix, before)) { out["status"] = "invalid_fixture"; return out; }
    const int color = ((before.stones - 1) / 2) % 2 == 0 ? 2 : 1;
    out["current_color"] = color == 1 ? "B" : "W";
    if (color != expectedColor) { out["status"] = "wrong_turn"; return out; }
    const int enemy = 3 - color;
    const Windows direct = before.completions(enemy);
    out["opponent_direct_completions"] = windows(direct);
    out["production"] = production(before, color);
    std::vector<QPoint> legal;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) if (!before.board[x][y]) legal.push_back({x, y});
    QJsonArray candidates;
    int safe = 0, refuted = 0, unknown = 0, exact = 0;
    const QJsonArray productionMove = out["production"].toObject().value("move").toArray();
    bool productionIsCandidate = false;
    for (size_t i = 0; i < legal.size(); ++i) for (size_t j = i + 1; j < legal.size(); ++j) {
        const QPoint a = legal[i], b = legal[j];
        bool covers = true;
        for (const auto &w : direct)
            if (std::find(w.begin(), w.end(), a) == w.end() && std::find(w.begin(), w.end(), b) == w.end()) { covers = false; break; }
        if (!covers) continue;
        ++exact;
        Referee after;
        if (!apply(before, a, b, color, after)) continue;
        QJsonObject row{{"move", pairArray(a, b)}, {"pair", pairText(a, b)}, {"direct_win", after.winner == color}};
        ReplyResult rr;
        if (after.winner == color) { rr.status = "safe_one_reply"; rr.elapsed = 0; }
        else rr = replies(after, enemy, oracleMs);
        row["assessment"] = replyJson(rr);
        if (rr.status == "safe_one_reply") ++safe; else if (rr.status == "refuted") ++refuted; else ++unknown;
        const bool isProduction = productionMove == pairArray(a, b)
            || productionMove == pairArray(b, a);
        row["production_move"] = isProduction;
        productionIsCandidate |= isProduction;
        candidates.append(row);
    }
    out["exact_cover_count"] = exact;
    out["safe_count"] = safe; out["refuted_count"] = refuted; out["unknown_count"] = unknown;
    out["production_is_exact_cover_candidate"] = productionIsCandidate;
    out["production_candidate_unique"] = productionIsCandidate && exact == 1;
    out["safe_alternative_count"] = std::max(0, safe - (productionIsCandidate ? 1 : 0));
    out["safe_alternative_exists"] = std::max(0, safe - (productionIsCandidate ? 1 : 0)) > 0;
    out["candidates"] = candidates;
    out["status"] = "complete_candidate_set";
    return out;
}

void closeDialogs(QTimer &timer) {
    QObject::connect(&timer, &QTimer::timeout, [] {
        for (QWidget *w : QApplication::topLevelWidgets())
            if (auto *box = qobject_cast<QMessageBox *>(w)) box->done(QMessageBox::Discard);
    });
    timer.start(10);
}
QString option(const QStringList &args, const QString &key, const QString &fallback) {
    const int i = args.indexOf(key); return i >= 0 && i + 1 < args.size() ? args[i + 1] : fallback;
}
}

int main(int argc, char **argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &message) {
        const auto match = QRegularExpression("Search completedDepth: (\\d+)").match(message);
        if (match.hasMatch()) productionDepth = match.captured(1).toInt();
        fprintf(stderr, "%s\n", qPrintable(message));
    });
    QTimer dialogs; closeDialogs(dialogs);
    const QStringList args = app.arguments();
    const QString root = option(args, "--records", "/wlf/Six/程序原版压缩包/six/棋谱文件");
    const QString outDir = option(args, "--out", "/wlf/Six/程序原版压缩包/six/self_iteration/failure_reports");
    const int oracleMs = option(args, "--oracle-ms", "2000").toInt();
    if (!QDir(outDir).exists()) qFatal("Output directory must already exist");
    const std::array<std::tuple<QString, int, int>, 3> fixtures = {
        std::make_tuple(QString::fromUtf8("C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt"), 67, 1),
        std::make_tuple(QString::fromUtf8("C6-重师Bvs吉林大学一队W-后手胜-祁门一中.txt"), 35, 1),
        std::make_tuple(QString::fromUtf8("C6-东北秦皇岛分校Bvs重师W-先手胜-祁门一中.txt"), 29, 2)};
    QJsonArray results;
    for (const auto &[file, prefix, color] : fixtures) results.append(analyze(file, prefix, color, oracleMs, root));
    QJsonObject report{{"oracle_ms_per_candidate", oracleMs}, {"safe_one_reply_definition", "only one complete opponent double-stone reply layer; not a global proof"}, {"positions", results}};
    const QByteArray json = QJsonDocument(report).toJson(QJsonDocument::Indented);
    QFile jf(QDir(outDir).filePath("failure_analysis.json"));
    if (!jf.open(QIODevice::WriteOnly) || jf.write(json) != json.size()) qFatal("Cannot write JSON report");
    QString md = "# Failure Analysis\n\n`safe_one_reply` 只是 1 层，不是全局证明。候选集合是所有能覆盖当前对手 direct completions 的合法双子；对手回复枚举不使用 production candidate pruning。\n\n";
    for (const auto &value : results) {
        const auto r = value.toObject();
        if (r["status"].toString() != "complete_candidate_set") {
            md += QString("## %1 prefix %2\n\n- status: **%3**\n\n")
                .arg(r["source"].toString()).arg(r["prefix_plies"].toInt()).arg(r["status"].toString());
            continue;
        }
        md += QString("## %1 prefix %2 (%3)\n\n- exact covers: %4\n- safe: %5\n- refuted: %6\n- unknown: %7\n- production move: `%8`\n- production candidate unique: **%9**\n- safe alternatives: %10\n- safe alternative exists: **%11**\n- production depth: %12\n- production time: %13 ms\n\n")
            .arg(r["source"].toString()).arg(r["prefix_plies"].toInt()).arg(r["current_color"].toString())
            .arg(r["exact_cover_count"].toInt()).arg(r["safe_count"].toInt()).arg(r["refuted_count"].toInt()).arg(r["unknown_count"].toInt())
            .arg(r["production"].toObject().value("pair").toString())
            .arg(r["production_candidate_unique"].toBool() ? "yes" : "no").arg(r["safe_alternative_count"].toInt())
            .arg(r["safe_alternative_exists"].toBool() ? "yes" : "no")
            .arg(r["production"].toObject().value("depth").toInt())
            .arg(r["production"].toObject().value("elapsed_ms").toInt());
        for (const auto &candidate : r["candidates"].toArray()) {
            const auto c = candidate.toObject();
            const auto assessment = c["assessment"].toObject();
            md += QString("- `%1`: %2%3, checked %4/%5, refutations %6\n")
                .arg(c["pair"].toString())
                .arg(assessment["status"].toString())
                .arg(c["production_move"].toBool() ? "; **production**" : "")
                .arg(assessment["checked"].toInt())
                .arg(assessment["total"].toInt())
                .arg(assessment["refutations"].toInt());
        }
        md += "\n";
    }
    QFile mf(QDir(outDir).filePath("failure_analysis.md"));
    if (!mf.open(QIODevice::WriteOnly) || mf.write(md.toUtf8()) != md.toUtf8().size()) qFatal("Cannot write Markdown report");
    QTextStream(stdout) << json;
    return 0;
}
