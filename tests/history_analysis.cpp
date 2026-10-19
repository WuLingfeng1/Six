#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>
#include <cstdio>
#define private public
#include "Game.h"
#undef private
#include "referee.h"
#include "dialog_closer.h"

namespace {
int depth = -1;
void messages(QtMsgType, const QMessageLogContext &, const QString &text) {
    const auto match = QRegularExpression("Search completedDepth: (\\d+)").match(text);
    if (match.hasMatch()) depth = match.captured(1).toInt();
    fprintf(stderr, "%s\n", qPrintable(text));
}
QString point(QPoint p) { return QString(QChar('A' + p.y())) + QString::number(19 - p.x()); }
QJsonArray points(const std::vector<QPoint> &ps) {
    QJsonArray a; for (QPoint p : ps) a.append(point(p)); return a;
}
QJsonArray windows(const std::vector<std::vector<QPoint>> &ws) {
    QJsonArray a; for (const auto &w : ws) a.append(points(w)); return a;
}
int ordinal(QPoint p) { return p.x() * 19 + p.y(); }
bool write(const QString &path, const QByteArray &data) {
    QFile f(path); return f.open(QIODevice::WriteOnly) && f.write(data) == data.size();
}
struct Turn { Referee before; int color; std::vector<QPoint> move; };
struct Record { QJsonObject meta; std::vector<Turn> turns; int own = 0; bool loss = false; QString duplicate; };

// Same independent exhaustive oracle as Referee::forcingPair. Only windows
// incident to either ordinal change; unchanged <=2-hole windows remain threats.
QJsonObject oracle(const Referee &p, int attacker, int deadlineMs) {
    QElapsedTimer timer; timer.start();
    const auto attack = p.emptyWindows(attacker, 4);
    const auto counter = p.completions(3 - attacker);
    std::array<std::vector<int>, 361> incidence;
    std::vector<int> base;
    for (size_t k = 0; k < attack.size(); ++k) {
        if (attack[k].size() <= 2) base.push_back(int(k));
        for (QPoint h : attack[k]) incidence[ordinal(h)].push_back(int(k));
    }
    std::vector<QPoint> legal;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
        if (!p.board[x][y]) legal.push_back({x, y});
    const qint64 total = qint64(legal.size()) * (legal.size() - 1) / 2;
    qint64 checked = 0, refutations = 0;
    QJsonObject witness;
    std::vector<int> seen(attack.size(), 0), affected;
    std::vector<std::vector<QPoint>> threats;
    int stamp = 0;
    for (size_t i = 0; i < legal.size(); ++i) for (size_t j = i + 1; j < legal.size(); ++j) {
        if ((checked & 127) == 0 && timer.elapsed() >= deadlineMs) {
            return {{"status", "unknown"}, {"checked", double(checked)}, {"total", double(total)},
                    {"refutations", double(refutations)}, {"witness", witness}, {"elapsed_ms", double(timer.elapsed())}};
        }
        ++checked; ++stamp;
        const QPoint a = legal[i], b = legal[j];
        affected.clear(); threats.clear();
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
        for (const auto &holes : counter)
            if (std::find(holes.begin(), holes.end(), a) == holes.end()
                && std::find(holes.begin(), holes.end(), b) == holes.end()) { counterWin = true; break; }
        if (wins || (!counterWin && !Referee::coverable(threats))) {
            ++refutations;
            if (witness.isEmpty()) witness = {{"reply", points({a, b})},
                {"kind", wins ? "opponent_six" : "uncoverable_no_counterwin"},
                {"threats", windows(threats)}, {"own_immediate_counterwin", counterWin}};
        }
    }
    return {{"status", refutations ? "refuted" : "safe_one_reply"}, {"checked", double(checked)},
            {"total", double(total)}, {"refutations", double(refutations)}, {"witness", witness},
            {"elapsed_ms", double(timer.elapsed())}};
}
QJsonObject assess(const Referee &before, int color, const std::vector<QPoint> &move, int deadline) {
    Referee p = before;
    bool legal = !move.empty() && move.size() <= size_t(before.stones == 0 ? 1 : std::min(2, 361 - before.stones));
    for (QPoint q : move) if (!p.put(q, color)) { legal = false; break; }
    if (!p.winner && p.stones < 361 && move.size() != size_t(before.stones == 0 ? 1 : 2)) {
        legal = false; p.error = "incomplete nonterminal turn";
    }
    QJsonObject r{{"move", points(move)}, {"legal", legal}, {"error", legal ? "" : p.error}};
    if (!legal) { r["status"] = "illegal"; return r; }
    r["own_six"] = p.winner == color;
    if (p.winner == color) { r["status"] = "direct_win"; return r; }
    if (p.stones == 361) { r["status"] = "full_draw"; return r; }
    const auto completion = p.completions(3 - color);
    r["opponent_completions"] = windows(completion);
    r["covers"] = Referee::coverable(completion);
    r["immediate_losing"] = !completion.empty();
    if (!completion.empty()) {
        r["status"] = "direct_loss";
        r["witness"] = QJsonObject{{"reply", points(completion.front())}, {"kind", "opponent_completion"}};
    }
    const auto o = oracle(p, 3 - color, deadline);
    r["oracle"] = o;
    if (completion.empty()) r["status"] = o["status"];
    return r;
}

void verify(const QString &out, const QString &white15) {
    QFile f(white15);
    if (!f.open(QIODevice::ReadOnly)) qFatal("White15 reference record missing");
    const auto bytes = f.readAll();
    auto matches = QRegularExpression(";\\s*([BW])\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)").globalMatch(QString::fromUtf8(bytes));
    Referee prefix;
    while (prefix.stones < 57 && matches.hasNext()) {
        const auto m = matches.next();
        if (!prefix.put({19 - m.captured(3).toInt(), m.captured(2)[0].unicode() - 'A'}, m.captured(1) == "B" ? 1 : 2)) qFatal("Invalid white15 prefix");
    }
    if (prefix.stones != 57 || prefix.winner) qFatal("Invalid white15 reference");
    const auto parse = [](QString s) { return QPoint(19 - s.mid(1).toInt(), s[0].unicode() - 'A'); };
    QJsonArray checks;
    for (const QString &second : {QString("G18"), QString("F14")}) {
        Referee position = prefix;
        if (!position.put(parse("E9"), 2) || !position.put(parse(second), 2)) qFatal("Cannot apply white15 move");
        const auto fast = oracle(position, 1, 10000);
        QPoint a, b; size_t checked = 0;
        QElapsedTimer timer; timer.start();
        const bool forced = position.forcingPair(1, a, b, checked);
        const bool agree = fast["status"] != "unknown" && forced == (fast["status"] == "refuted") && checked == size_t(fast["checked"].toDouble());
        Referee known = position;
        if (!known.put(parse("F11"), 1) || !known.put(parse("F13"), 1)) qFatal("Cannot apply known reply");
        const bool knownRefuted = !Referee::coverable(known.completions(1)) && known.completions(2).empty();
        if (!agree || knownRefuted != (second == "G18")) qFatal("Optimized oracle disagrees with independent original helper");
        checks.append(QJsonObject{{"white_move", points({parse("E9"), parse(second)})}, {"optimized", fast},
            {"original_forced", forced}, {"original_checked", double(checked)}, {"original_ms", double(timer.elapsed())},
            {"known_reply", points({parse("F11"), parse("F13")})}, {"known_reply_refutes", knownRefuted}, {"agreement", agree}});
    }
    // A deliberately expired exhaustive run must be unknown, never safe.
    Referee stress;
    for (int y = 0; y < 19; y += 2) for (int x = 0; x < 19; x += 3) { stress.board[x][y] = 1; ++stress.stones; }
    const auto interrupted = oracle(stress, 1, 0);
    if (interrupted["status"] != "unknown" || interrupted["checked"].toDouble() != 0) qFatal("Unknown deadline handling failed");
    QJsonArray terminalChecks;
    for (int color : {1, 2}) {
        Referee p;
        for (int x = 3; x <= 7; ++x) { p.board[x][9] = color; ++p.stones; }
        p.board[2][9] = 3 - color; ++p.stones;
        for (int x = 0; x < (color == 1 ? 1 : 3); ++x) { p.board[x][0] = 3 - color; ++p.stones; }
        Game game;
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) game.m_chess[x][y] = p.board[x][y];
        game.m_type = color; game.m_step = 0; game.rebuildSearch();
        std::vector<QPoint> emitted;
        QObject::connect(&game, &Game::chessChange, [&](QPoint q, int) { emitted.push_back(q); });
        game.bestPlay();
        const auto single = assess(p, color, emitted, 1000);
        if (emitted.size() != 1 || single["status"] != "direct_win") qFatal("First-stone terminal did not stop");
        Referee terminal = p;
        if (!terminal.put(emitted[0], color) || !terminal.winner || terminal.put({18, 18}, color)) qFatal("Referee allowed orphan after six");
        auto extra = emitted; extra.push_back({18, 18});
        if (assess(p, color, extra, 1000)["legal"].toBool()) qFatal("Orphan second stone accepted");
        const auto shortTurn = assess(p, color, {{18, 18}}, 1000);
        if (shortTurn["legal"].toBool() || shortTurn["error"] != "incomplete nonterminal turn") qFatal("Nonterminal single stone treated as full turn");
        terminalChecks.append(QJsonObject{{"color", color}, {"production_emitted", points(emitted)},
            {"first_stone_six", single}, {"orphan_rejected", true}, {"nonterminal_single", shortTurn}});
    }
    QJsonObject result{{"qt_version", qVersion()}, {"white15_source", white15},
        {"source_sha256", QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex())},
        {"checks", checks}, {"expired_deadline", interrupted}, {"terminal_checks", terminalChecks}, {"scope", "oracle agreement and terminal behavior only, not a game win"}};
    if (!write(QDir(out).filePath("history_oracle_verification.json"), QJsonDocument(result).toJson())) qFatal("Cannot write verification");
    QTextStream(stdout) << QJsonDocument(result).toJson();
}

std::vector<Record> load(const QString &dir) {
    std::vector<Record> result;
    QMap<QByteArray, QString> seen;
    for (const QString &name : QDir(dir).entryList({"*.txt"}, QDir::Files, QDir::Name)) {
        QFile f(QDir(dir).filePath(name));
        if (!f.open(QIODevice::ReadOnly)) qFatal("Cannot open record");
        const QByteArray bytes = f.readAll();
        const QString text = QString::fromUtf8(bytes);
        Record r;
        r.own = name.contains(QString::fromUtf8("重师B")) ? 1 : name.contains(QString::fromUtf8("重师W")) ? 2 : 0;
        QString label = name.contains(QString::fromUtf8("先手胜")) ? "B" : name.contains(QString::fromUtf8("后手胜")) ? "W" : "draw";
        const QRegularExpression re(";\\s*([BW])\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)");
        auto matches = re.globalMatch(text);
        std::vector<std::pair<int, QPoint>> moves;
        QByteArray canonical;
        while (matches.hasNext()) {
            const auto m = matches.next();
            const int color = m.captured(1) == "B" ? 1 : 2;
            QPoint p(19 - m.captured(3).toInt(), m.captured(2)[0].unicode() - 'A');
            moves.push_back({color, p});
            canonical += QByteArray::number(color) + ':' + point(p).toLatin1() + ';';
        }
        const int semicolons = text.count(';');
        bool valid = semicolons == int(moves.size()) && !moves.empty();
        QString error = valid ? "" : "unparsed move token or empty record";
        Referee p;
        size_t index = 0;
        while (valid && index < moves.size() && !p.winner && p.stones < 361) {
            Turn turn{p, moves[index].first, {}};
            const int count = p.stones == 0 ? 1 : 2;
            for (int k = 0; k < count && index < moves.size() && !p.winner && p.stones < 361; ++k) {
                const auto &mv = moves[index++];
                if (!p.put(mv.second, mv.first)) { valid = false; error = p.error; break; }
                turn.move.push_back(mv.second);
            }
            if (valid) r.turns.push_back(turn);
        }
        const QString actual = !valid ? "invalid" : p.winner == 1 ? "B" : p.winner == 2 ? "W" : p.stones == 361 ? "draw" : "unfinished";
        const QByteArray digest = QCryptographicHash::hash(canonical, QCryptographicHash::Sha256).toHex();
        if (seen.contains(digest)) r.duplicate = seen[digest]; else seen[digest] = name;
        r.loss = r.own && p.winner && p.winner != r.own && valid;
        r.meta = {{"file", name}, {"sha256", QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex())},
            {"canonical_sha256", QString::fromLatin1(digest)}, {"own_color", r.own == 1 ? "B" : r.own == 2 ? "W" : "both_non_chongshi"},
            {"label", label}, {"actual", actual}, {"label_mismatch", (actual == "B" || actual == "W" || actual == "draw") && actual != label},
            {"valid_prefix", valid}, {"error", error}, {"parsed_stones", int(moves.size())}, {"validated_stones", p.stones},
            {"terminal_suffix_stones", int(moves.size() - index)}, {"turns", int(r.turns.size())},
            {"partial_last_turn", !r.turns.empty() && r.turns.back().before.stones != 0 && r.turns.back().move.size() == 1 && !p.winner && p.stones < 361},
            {"duplicate_of", r.duplicate}, {"chongshi_loss", r.loss}};
        result.push_back(r);
    }
    return result;
}
bool selected(const Record &r, const Turn &t) { return r.loss || !r.own || t.color == r.own; }
void report(const QString &out, const std::vector<Record> &records) {
    QJsonArray meta, rows;
    QString markdown = QString::fromUtf8("# 历史棋谱离线分析\n\n实际结果由独立 Referee 逐颗核验。未终局短谱不判胜负；重复谱不重复计样本。败局双方全部回合、其他重师谱我方全部回合、非重师谱双方全部回合列入待分析集合。每个新着法只替换当前回合，下一样本恢复原历史，绝非伪重赛。\n\n`direct_win` 是即时六连；`direct_loss` 是对手直接 completion；`refuted` 有对手双子造不可 cover 且我无即时 counterwin 的反例；`safe_one_reply` 仅排除这一个回复层的战术反例，不表示深层安全或全局获胜；`unknown` 是 oracle 截止中断，不能当安全。原记录与新动作差异不是胜率。搜索保持生产 2000ms 预算，战术命中可提前返回。黑方首子走正常中央开局，未调用双子搜索。\n\n");
    QString csv = "file,turn,color,role,stones,old_move,new_move,legal,elapsed_ms,budget_ms,completedDepth,old_status,new_status,witness\n";
    int losses = 0, mismatches = 0, duplicates = 0, expected = 0, completed = 0, black = 0, white = 0, non = 0;
    qint64 searchMs = 0, maxOracleMs = 0, enumerated = 0;
    int oracleUnknown = 0, illegalNew = 0, openings = 0, budgetErrors = 0;
    int unavoidable = 0, keyCount = 0;
    QString keySamples = QString::fromUtf8("# 关键历史局面\n\n以下仅是独立原历史局面的着法替换与一回复战术检查，不是重赛或整局胜率。`当前直接无法防` 指对手现有 completion 无两子 cover，且我方无即时六连着法；新动作 refuted 不自动等价于所有动作都输。\n\n|文件/回合|色/角色|当前直接无法防|旧动作/结论|新动作/结论|具体反例|\n|---|---|---|---|---|---|\n");
    QMap<QString, int> transitions;
    QMap<QString, QJsonObject> roles;
    QMap<QString, int> oldCounts, newCounts;
    const auto quote = [](QString s) { s.replace('"', "\"\""); return '"' + s + '"'; };
    for (const auto &r : records) {
        meta.append(r.meta); losses += r.loss && r.duplicate.isEmpty(); mismatches += r.meta["label_mismatch"].toBool(); duplicates += !r.duplicate.isEmpty();
        markdown += "## " + r.meta["file"].toString() + "\n\n";
        markdown += QString::fromUtf8("颜色：%1；文件标签：%2；实际结果：%3；标签不符：%4；合法前缀：%5；核验 %6/%7 子；终局后忽略 %8 子；重复来源：%9。\n\n")
            .arg(r.meta["own_color"].toString(), r.meta["label"].toString(), r.meta["actual"].toString(), r.meta["label_mismatch"].toBool() ? "是" : "否", r.meta["valid_prefix"].toBool() ? "是" : "否")
            .arg(r.meta["validated_stones"].toInt()).arg(r.meta["parsed_stones"].toInt()).arg(r.meta["terminal_suffix_stones"].toInt()).arg(r.duplicate);
        if (r.meta["actual"] == "unfinished") markdown += QString::fromUtf8("未终局/短谱：无法依据文件标签判胜负。\n\n");
        if (!r.duplicate.isEmpty()) continue;
        int planned = 0, done = 0;
        markdown += QString::fromUtf8("|回合|色/对照|原着法|新着法|合法|ms/深度|原/新战术|具体反例|\n|---|---|---|---|---|---|---|---|\n");
        for (size_t t = 0; t < r.turns.size(); ++t) {
            if (!selected(r, r.turns[t])) continue;
            ++planned;
            const QString path = QDir(out).filePath(QString("row-%1-%2.json").arg(r.meta["canonical_sha256"].toString()).arg(t));
            QFile f(path); if (!f.open(QIODevice::ReadOnly)) continue;
            auto row = QJsonDocument::fromJson(f.readAll()).object();
            if (row["source_sha256"] != r.meta["sha256"] || row["turn"].toInt() != int(t + 1)) continue;
            const auto &position = r.turns[t].before;
            const auto ownCompletion = position.completions(r.turns[t].color);
            const auto enemyCompletion = position.completions(3 - r.turns[t].color);
            const bool cannotDefend = ownCompletion.empty() && !Referee::coverable(enemyCompletion);
            row["position_tactics"] = QJsonObject{{"own_completions", windows(ownCompletion)},
                {"opponent_completions", windows(enemyCompletion)}, {"opponent_coverable", Referee::coverable(enemyCompletion)},
                {"unavoidable_direct_loss", cannotDefend}};
            row["side_turn"] = int(t / 2 + 1);
            if (row["blackFirst"].toBool()) row["production_score"] = QJsonValue();
            rows.append(row); ++done; ++completed;
            unavoidable += cannotDefend;
            if (!r.own) ++non; else if (r.own == 1) ++black; else ++white;
            const auto old = row["old"].toObject(), fresh = row["new"].toObject();
            ++oldCounts[old["status"].toString()]; ++newCounts[fresh["status"].toString()];
            ++transitions[old["status"].toString() + " -> " + fresh["status"].toString()];
            searchMs += row["elapsed_ms"].toInteger();
            openings += row["blackFirst"].toBool();
            budgetErrors += !row["blackFirst"].toBool() && row["budget_ms"].toInt() != 2000;
            illegalNew += !fresh["legal"].toBool();
            for (const auto &action : {old, fresh}) {
                const auto o = action["oracle"].toObject();
                maxOracleMs = std::max(maxOracleMs, o["elapsed_ms"].toInteger());
                enumerated += o["checked"].toInteger();
                oracleUnknown += o["status"] == "unknown";
            }
            const QString role = row["role"].toString() + "_" + row["color"].toString();
            auto rs = roles[role];
            rs["samples"] = rs["samples"].toInt() + 1;
            for (const auto &kind : {QString("old"), QString("new")}) {
                const QString key = kind + "_" + row[kind].toObject()["status"].toString();
                rs[key] = rs[key].toInt() + 1;
            }
            roles[role] = rs;
            const auto notation = [](const QJsonValue &v) { QStringList s; for (const auto &p : v.toArray()) s << p.toString(); return s.join(' '); };
            const auto wit = [](const QJsonObject &a) { QJsonObject w = a["witness"].toObject(); if (w.isEmpty()) w = a["oracle"].toObject()["witness"].toObject(); return w.isEmpty() ? QString("-") : QString::fromUtf8(QJsonDocument(w).toJson(QJsonDocument::Compact)); };
            const QString witness = "old=" + wit(old) + "; new=" + wit(fresh);
            if (keyCount < 40 && (cannotDefend || old["status"] != fresh["status"] || fresh["status"] == "refuted" || fresh["status"] == "direct_win")) {
                ++keyCount;
                keySamples += QString("|%1 / %2 (%3 %4)|%5/%6|%7|%8 / %9|%10 / %11|%12|\n")
                    .arg(r.meta["file"].toString()).arg(t + 1).arg(row["color"].toString()).arg(row["side_turn"].toInt())
                    .arg(row["color"].toString(), row["role"].toString(), cannotDefend ? "yes" : "no", notation(old["move"]), old["status"].toString(), notation(fresh["move"]), fresh["status"].toString(), witness);
            }
            markdown += QString("|%1|%2/%3|%4|%5|%6|%7/%8|%9/%10|%11|\n").arg(t + 1).arg(row["color"].toString(), row["role"].toString(), notation(old["move"]), notation(fresh["move"]), fresh["legal"].toBool() ? "yes" : "NO").arg(row["elapsed_ms"].toInt()).arg(row["completedDepth"].toInt()).arg(old["status"].toString(), fresh["status"].toString(), witness);
            QStringList fields{r.meta["file"].toString(), QString::number(t + 1), row["color"].toString(), row["role"].toString(), QString::number(row["stones"].toInt()), notation(old["move"]), notation(fresh["move"]), fresh["legal"].toBool() ? "true" : "false", QString::number(row["elapsed_ms"].toInt()), QString::number(row["budget_ms"].toInt()), QString::number(row["completedDepth"].toInt()), old["status"].toString(), fresh["status"].toString(), witness};
            for (auto &s : fields) s = quote(s);
            csv += fields.join(',') + '\n';
        }
        expected += planned;
        markdown += QString::fromUtf8("\n已完成 %1/%2 个独立历史局面；其余未运行，不作推断。\n\n").arg(done).arg(planned);
    }
    QJsonObject oldStats, newStats; for (auto i = oldCounts.begin(); i != oldCounts.end(); ++i) oldStats[i.key()] = i.value(); for (auto i = newCounts.begin(); i != newCounts.end(); ++i) newStats[i.key()] = i.value();
    QJsonObject totals{{"files", int(records.size())}, {"duplicates", duplicates}, {"unique_losses", losses}, {"label_mismatches", mismatches}, {"planned", expected}, {"completed", completed}, {"remaining", expected - completed}, {"chongshi_B_samples", black}, {"chongshi_W_samples", white}, {"non_chongshi_samples", non}, {"old_tactics", oldStats}, {"new_tactics", newStats}};
    QJsonObject roleStats; for (auto i = roles.begin(); i != roles.end(); ++i) roleStats[i.key()] = i.value();
    totals["role_color_tactics"] = roleStats;
    totals["search_wall_ms"] = double(searchMs); totals["max_oracle_ms"] = double(maxOracleMs);
    totals["enumerated_reply_pairs"] = double(enumerated); totals["oracle_unknown"] = oracleUnknown;
    totals["illegal_new"] = illegalNew; totals["budget_errors"] = budgetErrors; totals["normal_black_openings"] = openings;
    totals["unavoidable_direct_loss_positions"] = unavoidable;
    QJsonObject transitionStats; for (auto i = transitions.begin(); i != transitions.end(); ++i) transitionStats[i.key()] = i.value();
    totals["tactical_transitions_not_win_rates"] = transitionStats;
    keySamples += QString::fromUtf8("\n当前已核验直接无法防局面 %1；战术转移计数（绝非胜率）：`%2`。表格最多列 40 个关键例子；全部回合和全部 witness 见 history_details.json / history_details.csv / history_summary.md。\n")
        .arg(unavoidable).arg(QString::fromUtf8(QJsonDocument(transitionStats).toJson(QJsonDocument::Compact)));
    if (!write(QDir(out).filePath("history_key_samples.md"), keySamples.toUtf8())) qFatal("Cannot write key samples");
    markdown += QString::fromUtf8("## 审计\n\n新动作非法 %1；生产预算错误 %2；正常黑方单子开局 %3；累计搜索墙钟 %4 ms；完整/已尝试双子枚举 %5；单 oracle 最大 %6 ms；oracle unknown %7。\n\n双方/颜色战术控制：`%8`\n\n深度 -1 仅表示正常首子不进行搜索；深度 0 为生产战术快速返回或未完成迭代，不是深层证明。\n")
        .arg(illegalNew).arg(budgetErrors).arg(openings).arg(searchMs).arg(enumerated).arg(maxOracleMs).arg(oracleUnknown).arg(QString::fromUtf8(QJsonDocument(roleStats).toJson(QJsonDocument::Compact)));
    markdown.insert(markdown.indexOf("## "), QString::fromUtf8("## 总计\n\n文件 %1；重复 %2；独立重师败局 %3；已证实标签不符 %4；计划样本 %5；已完成 %6；未运行 %7。颜色分类按谱中重师颜色：B %8、W %9、非重师 %10（实际落子颜色与我方/对手对照见逐行 JSON）。\n\n原动作战术：`%11`\n\n新动作战术：`%12`\n\n")
        .arg(records.size()).arg(duplicates).arg(losses).arg(mismatches).arg(expected).arg(completed).arg(expected - completed).arg(black).arg(white).arg(non)
        .arg(QString::fromUtf8(QJsonDocument(oldStats).toJson(QJsonDocument::Compact)), QString::fromUtf8(QJsonDocument(newStats).toJson(QJsonDocument::Compact))));
    if (!write(QDir(out).filePath("history_summary.md"), markdown.toUtf8()) || !write(QDir(out).filePath("history_details.csv"), csv.toUtf8())
        || !write(QDir(out).filePath("history_details.json"), QJsonDocument(QJsonObject{{"totals", totals}, {"records", meta}, {"positions", rows}}).toJson())) qFatal("Cannot write reports");
    QTextStream(stdout) << QJsonDocument(totals).toJson(QJsonDocument::Compact) << '\n';
}
}

int main(int argc, char **argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    qInstallMessageHandler(messages);
    QTimer closer; closeResultDialogs(closer);
    const QStringList args = app.arguments();
    const auto option = [&](const QString &key, const QString &fallback) { int i = args.indexOf(key); return i >= 0 && i + 1 < args.size() ? args[i + 1] : fallback; };
    const QString dir = option("--records", "/wlf/Six/程序原版压缩包/six/棋谱文件");
    const QString out = option("--out", "/wlf/Six/程序原版压缩包/six/tests/history_reports");
    if (!QDir(out).exists()) qFatal("Output directory must already exist");
    const auto records = load(dir);
    if (args.contains("--verify")) {
        verify(out, option("--white15", "/wlf/Six/server/six_server/build/Desktop_Qt_6_11_1-Debug/棋谱文件/C6-serverBvscilentW-先手胜-.txt"));
        return 0;
    }
    if (args.contains("--scan") || args.contains("--report")) {
        for (size_t i = 0; i < records.size(); ++i) QTextStream(stdout) << i << ' ' << QJsonDocument(records[i].meta).toJson(QJsonDocument::Compact) << '\n';
        report(out, records); return 0;
    }
    const int file = option("--file", "-1").toInt();
    const int from = option("--from", "0").toInt();
    const int count = option("--count", "40").toInt();
    const int deadline = option("--oracle-ms", "1000").toInt();
    if (file < 0 || file >= int(records.size()) || from < 0 || count < 1 || deadline < 1) qFatal("Use --scan, then --file INDEX --from ZERO_BASED_TURN --count N");
    const auto &r = records[file];
    if (!r.duplicate.isEmpty() || !r.meta["valid_prefix"].toBool()) qFatal("Duplicate or invalid record not analyzed");
    for (int t = from; t < std::min(from + count, int(r.turns.size())); ++t) {
        const auto &turn = r.turns[t]; if (!selected(r, turn)) continue;
        const QString path = QDir(out).filePath(QString("row-%1-%2.json").arg(r.meta["canonical_sha256"].toString()).arg(t));
        if (QFile::exists(path) && !args.contains("--rerun")) continue;
        Game game;
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) game.m_chess[x][y] = turn.before.board[x][y];
        game.m_type = turn.color; game.m_step = 0; game.m_win = TIE;
        game.m_last = {2, 3}; game.m_llast = {3, 2}; game.rebuildSearch();
        std::vector<QPoint> move;
        bool signalColors = true; int saves = 0;
        QObject::connect(&game, &Game::chessChange, [&](QPoint p, int color) { move.push_back(p); signalColors &= color == turn.color; });
        QObject::connect(&game, &Game::save_signal, [&] { ++saves; });
        depth = -1; QElapsedTimer timer; timer.start();
        const bool opening = turn.before.stones == 0;
        if (opening) game.whoFirst(1); else game.bestPlay();
        const qint64 elapsed = timer.elapsed();
        const int completedDepth = depth;
        Referee emitted = turn.before;
        bool boardMatches = true;
        for (QPoint p : move) if (!emitted.put(p, turn.color)) boardMatches = false;
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y) boardMatches &= game.m_chess[x][y] == emitted.board[x][y];
        const auto old = assess(turn.before, turn.color, turn.move, deadline);
        auto fresh = assess(turn.before, turn.color, move, deadline);
        fresh["legal"] = fresh["legal"].toBool() && signalColors && boardMatches && !saves;
        QJsonObject row{{"file", r.meta["file"]}, {"source_sha256", r.meta["sha256"]}, {"turn", t + 1},
            {"color", turn.color == 1 ? "B" : "W"}, {"role", !r.own ? "non_chongshi" : turn.color == r.own ? "own" : "opponent_control"},
            {"stones", turn.before.stones}, {"m_type_seed", turn.color}, {"m_step_seed", 0}, {"blackFirst", opening},
            {"elapsed_ms", double(elapsed)}, {"budget_ms", opening ? 0 : int(game.m_timeLimitMs)},
            {"completedDepth", completedDepth}, {"timed_out", game.m_timedOut}, {"production_score", opening ? QJsonValue() : QJsonValue(game.m_chosenMove.score())},
            {"board_matches_signals", boardMatches}, {"save_signals", saves}, {"next_type", game.m_type}, {"next_step", game.m_step},
            {"old", old}, {"new", fresh}};
        if (!write(path, QJsonDocument(row).toJson())) qFatal("Cannot write row");
        QTextStream(stdout) << "file=" << file << " turn=" << t + 1 << " ms=" << elapsed << " depth=" << completedDepth << " old=" << old["status"].toString() << " new=" << fresh["status"].toString() << '\n';
        if ((!opening && game.m_timeLimitMs != 2000) || !fresh["legal"].toBool()) qFatal("Production budget or legality violation; row retained");
    }
    report(out, records);
    return 0;
}
