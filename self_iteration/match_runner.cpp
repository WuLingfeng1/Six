#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QTextStream>
#include <QVector>
#include <array>
#include <optional>
#include <algorithm>

namespace {
using Board = std::array<std::array<int, 19>, 19>;
struct Referee {
    Board board{}; int stones = 0; int winner = 0; int lastColor = 0; int runLength = 0;
    bool six(int color, int x, int y) const {
        const int dx[] = {1, 0, 1, 1}, dy[] = {0, 1, 1, -1};
        for (int d = 0; d < 4; ++d) { int n = 1;
            for (int s : {-1, 1}) for (int k = 1; k < 6; ++k) { int a=x+s*k*dx[d], b=y+s*k*dy[d]; if (a<0||a>=19||b<0||b>=19||board[a][b]!=color) break; ++n; }
            if (n >= 6) return true;
        } return false;
    }
    bool hasSix(int color) const {
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
            if (board[x][y] == color && six(color, x, y)) return true;
        return false;
    }
    bool put(int x, int y, int color) {
        const int expected = stones == 0 ? 1 : (((stones - 1) / 2) % 2 == 0 ? 2 : 1);
        const bool sameTurn = color == lastColor && runLength < 2;
        if (x<0||x>=19||y<0||y>=19||board[x][y]||winner
            || (color != expected && !sameTurn)) return false;
        board[x][y]=color; ++stones;
        if (color == lastColor) ++runLength; else { lastColor = color; runLength = 1; }
        if (six(color,x,y)) winner=color;
        return true;
    }
};
struct Fixture {
    QString name, source, currentColor, oldLabel, actualHistorical;
    Referee referee;
    int prefix = 0;
    QVector<QPair<int,int>> prefixMoves;
};
QString option(const QStringList &a, const QString &key, const QString &fallback = {}) { int i=a.indexOf(key); return i>=0&&i+1<a.size()?a[i+1]:fallback; }
QString notation(int x, int y) { return QString(QChar('A'+y))+QString::number(19-x); }
QByteArray boardLine(const Referee &r, int color) { QByteArray out="BOARD "; out += color==1 ? "B" : "W"; for (int x=0;x<19;++x) for(int y=0;y<19;++y) { out += ' '; out += QByteArray::number(r.board[x][y]); } return out+'\n'; }
QVector<QPair<int,int>> opening(int id) { if(id==0)return {{9,8},{7,10}}; if(id==1)return {{12,11},{5,7}}; return {}; }
QJsonArray moveJson(const QVector<QPair<int,int>> &m) { QJsonArray a; for(auto p:m)a.append(notation(p.first,p.second)); return a; }

std::optional<Fixture> loadFixture(const QString &name, const QString &source, int prefix) {
    QFile file(source);
    if (!file.open(QIODevice::ReadOnly)) return std::nullopt;
    const QString text = QString::fromUtf8(file.readAll());
    const QRegularExpression re(R"(([BW])\(\s*([A-S])\s*,\s*(1[0-9]|[1-9])\s*\))");
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    Fixture f; f.name = name; f.source = source; f.prefix = prefix;
    QVector<QPair<int,int>> allMoves;
    QVector<int> allColors;
    while (it.hasNext()) {
        const auto m = it.next();
        const int y = m.captured(2).at(0).unicode() - 'A';
        const int x = 19 - m.captured(3).toInt();
        allMoves.append({x, y});
        allColors.append(m.captured(1) == "B" ? 1 : 2);
    }
    if (allMoves.size() < prefix) return std::nullopt;
    Referee actual;
    bool actualLegal = true;
    for (int i = 0; i < allMoves.size(); ++i)
        actualLegal = actual.put(allMoves[i].first, allMoves[i].second, allColors[i]) && actualLegal;
    f.actualHistorical = !actualLegal ? "illegal" : actual.winner == 1 ? "black_win" : actual.winner == 2 ? "white_win" : "unfinished";
    f.oldLabel = source.contains("先手胜") ? "black_win" : source.contains("后手胜") ? "white_win" : source.contains("平局") ? "draw" : "unknown";
    int index = 0;
    while (index < prefix) {
        const QString expected = index == 0 ? "B" : (((index - 1) / 2) % 2 == 0 ? "W" : "B");
        if (allColors[index] != (expected == "B" ? 1 : 2) || !f.referee.put(allMoves[index].first, allMoves[index].second, allColors[index])) return std::nullopt;
        f.prefixMoves.append(allMoves[index]);
        ++index;
    }
    if (f.referee.winner) return std::nullopt;
    f.currentColor = index == 0 ? "B" : (((index - 1) / 2) % 2 == 0 ? "W" : "B");
    return f;
}

bool readProtocol(QProcess &p, QString &line, int timeout) {
    QElapsedTimer timer; timer.start();
    while (!p.canReadLine()) {
        const int remaining = timeout - int(timer.elapsed());
        if (remaining <= 0 || !p.waitForReadyRead(remaining)) return false;
    }
    line = QString::fromUtf8(p.readLine()).trimmed(); return !line.isEmpty();
}
QJsonObject play(const QString &blackPath, const QString &whitePath, int game, int maxTurns, int openingSet, const Fixture *fixture = nullptr) {
    QProcess black, white; black.start(blackPath, {"--mode","worker","--color","B"}); white.start(whitePath, {"--mode","worker","--color","W"});
    QJsonObject result{{"game",game+1},{"max_double_turns",maxTurns}}; QJsonArray transcript;
    if (fixture) { result["fixture"] = fixture->name; result["source"] = fixture->source; result["prefix_plies"] = fixture->prefix; result["current_color"] = fixture->currentColor; result["forced_color"] = fixture->currentColor; result["initial_stones"] = fixture->referee.stones; result["prefix_moves"] = moveJson(fixture->prefixMoves); }
    else result["opening_id"] = game % openingSet;
    if(!black.waitForStarted(1000)||!white.waitForStarted(1000)){ result["result"]="worker_start_failed"; return result; }
    Referee r; int color = 1;
    if (fixture) r = fixture->referee;
    else { r.put(9,9,1); const auto op=opening(game%openingSet); r.put(op[0].first,op[0].second,2); r.put(op[1].first,op[1].second,2); }
    if (fixture) color = fixture->currentColor == "B" ? 1 : 2;
    int illegal = 0, firstDirectWin = 0, firstDirectLoss = 0, directWins = 0, directLosses = 0;
    for(int turn=0;turn<maxTurns && !r.winner && r.stones<361;++turn){
        const bool opponentDirect = r.hasSix(3 - color);
        QProcess &worker=color==1?black:white; worker.write(boardLine(r,color)); worker.waitForBytesWritten(1000);
        QString line; QJsonObject item{{"turn",turn+1},{"color",color==1?"B":"W"}};
        if(!readProtocol(worker,line,4000)){ result["result"]="timeout"; result["winner"]=r.winner; result["transcript"]=transcript; result["illegal"]=illegal; black.kill(); white.kill(); return result; }
        const QStringList t=line.split(QRegularExpression("\\s+"),Qt::SkipEmptyParts); QVector<QPair<int,int>> move; bool ok=t.size()>=2&&t[0]=="MOVE"; int n=ok?t[1].toInt(&ok):-1;
        if(!ok||n<1||n>2||(t.size()!=2+2*n)){ ++illegal; result["result"]="illegal"; item["raw"]=line; transcript.append(item); break; }
        for(int i=0;i<n;++i){ bool xok=false,yok=false; int x=t[2+2*i].toInt(&xok),y=t[3+2*i].toInt(&yok); ok&=xok&&yok; move.append({x,y}); }
        bool firstWins = ok && r.board[move[0].first][move[0].second] == 0;
        if (firstWins) { Referee after = r; firstWins = after.put(move[0].first, move[0].second, color) && after.winner == color; }
        if (n == 1 && !firstWins) ok = false;
        for(auto p:move) { if (!r.put(p.first,p.second,color)) { ok = false; break; } }
        item["move"]=moveJson(move); item["direct_win"] = r.winner == color; item["opponent_direct_available"] = opponentDirect; transcript.append(item);
        if (r.winner == color) { ++directWins; if (!firstDirectWin) firstDirectWin = turn + 1; }
        if (opponentDirect) { ++directLosses; if (!firstDirectLoss) firstDirectLoss = turn + 1; }
        if(!ok){ ++illegal; result["result"]="illegal";break;}
        color = 3 - color;
     }
    if (!result.contains("result")) result["result"] = r.winner ? (r.winner==1 ? "black_win" : "white_win") : "unfinished";
     result["winner"]=r.winner; result["stones"]=r.stones; result["illegal"]=illegal; result["direct_wins"]=directWins; result["direct_losses"]=directLosses; result["first_direct_win_turn"]=firstDirectWin; result["first_direct_loss_turn"]=firstDirectLoss; result["transcript"]=transcript; black.kill(); white.kill(); black.waitForFinished(500); white.waitForFinished(500); return result;
}

QJsonObject historyReport(const QString &recordsDir, const QString &out, const QString &blackWorker,
                         const QString &whiteWorker, int prefix, int games, int plies) {
    const QFileInfoList files = QDir(recordsDir).entryInfoList(QStringList() << "*.txt", QDir::Files | QDir::Readable, QDir::Name);
    QJsonArray records;
    QByteArray csv = "file,old_label,actual_historical,prefix_moves,current_result,winner,illegal,stones,first_direct_win_turn,first_direct_loss_turn\n";
    QString md = "# History self-play\n\n";
    md += QString("- records: `%1`\n- prefix: %2\n- games per record: %3\n- maximum double turns: %4\n\n")
        .arg(recordsDir).arg(prefix).arg(games).arg(plies);
    for (const QFileInfo &info : files) {
        const auto fixture = loadFixture(info.fileName(), info.absoluteFilePath(), prefix);
        if (!fixture) continue;
        QJsonArray currentGames;
        for (int i = 0; i < games; ++i) {
            QJsonObject game = play(blackWorker, whiteWorker, i, plies, 0, &*fixture);
            game["file"] = info.fileName();
            game["old_label"] = fixture->oldLabel;
            game["actual_historical"] = fixture->actualHistorical;
            QJsonObject critical{{"prefix_valid", true}, {"next_color", fixture->currentColor},
                                 {"next_color_is_black", fixture->currentColor == "B"},
                                 {"historical_not_terminal", !fixture->referee.winner},
                                 {"first_direct_win_turn", game["first_direct_win_turn"]},
                                 {"first_direct_loss_turn", game["first_direct_loss_turn"]}};
            game["critical"] = critical;
            currentGames.append(game);
            const QStringList fields = {info.fileName(), fixture->oldLabel, fixture->actualHistorical,
                QString::fromLatin1(QJsonDocument(game["prefix_moves"].toArray()).toJson(QJsonDocument::Compact)).replace(',', '|'),
                game["result"].toString(), QString::number(game["winner"].toInt()), QString::number(game["illegal"].toInt()),
                QString::number(game["stones"].toInt()), QString::number(game["first_direct_win_turn"].toInt()), QString::number(game["first_direct_loss_turn"].toInt())};
            csv += fields.join(',').toUtf8() + '\n';
        }
        QJsonObject record{{"file", info.fileName()}, {"old_label", fixture->oldLabel},
            {"actual_historical", fixture->actualHistorical}, {"prefix_moves", moveJson(fixture->prefixMoves)},
            {"prefix_plies", prefix}, {"current_color", fixture->currentColor}, {"games", currentGames}};
        if (currentGames.size() == 1) {
            const QJsonObject game = currentGames.first().toObject();
            for (auto it = game.begin(); it != game.end(); ++it) record[it.key()] = it.value();
        }
        records.append(record);
        md += QString("- `%1`: historical `%2` (label `%3`), current games: %4\n").arg(info.fileName(), fixture->actualHistorical, fixture->oldLabel).arg(currentGames.size());
    }
    QJsonObject root{{"mode", "history-selfplay"}, {"records_dir", recordsDir}, {"prefix", prefix},
                     {"games", games}, {"plies", plies}, {"records", records}};
    QFile json(QDir(out).filePath("history-selfplay.json"));
    QFile csvFile(QDir(out).filePath("history-selfplay.csv"));
    QFile mdFile(QDir(out).filePath("history-selfplay.md"));
    if (!json.open(QIODevice::WriteOnly) || !csvFile.open(QIODevice::WriteOnly) || !mdFile.open(QIODevice::WriteOnly)) return {};
    json.write(QJsonDocument(root).toJson(QJsonDocument::Indented)); csvFile.write(csv); mdFile.write(md.toUtf8());
    return root;
}

QJsonObject compareTranscript(const QJsonObject &control, const QJsonObject &candidate,
                              const QString &candidateSide) {
    const auto controlMoves = control["transcript"].toArray();
    const auto candidateMoves = candidate["transcript"].toArray();
    const int count = qMin(controlMoves.size(), candidateMoves.size());
    const QString expectedColor = candidateSide == "black" ? "B" : "W";

    for (int i = 0; i < count; ++i) {
        const auto controlTurn = controlMoves[i].toObject();
        const auto candidateTurn = candidateMoves[i].toObject();
        const bool sameTurn = controlTurn["turn"] == candidateTurn["turn"]
            && controlTurn["color"] == candidateTurn["color"];
        const bool sameMove = sameTurn && controlTurn["move"] == candidateTurn["move"];
        if (sameMove)
            continue;

        QJsonObject difference{{"turn", i + 1},
                               {"color", controlTurn["color"]},
                               {"control_move", controlTurn.value("move")},
                               {"candidate_move", candidateTurn.value("move")}};
        if (controlTurn["color"] != expectedColor || !sameTurn)
            difference["kind"] = "unknown/board_diverged";
        return difference;
    }

    if (controlMoves.size() != candidateMoves.size()) {
        const int turn = count + 1;
        const QJsonObject controlTurn = count < controlMoves.size()
            ? controlMoves[count].toObject() : QJsonObject();
        const QJsonObject candidateTurn = count < candidateMoves.size()
            ? candidateMoves[count].toObject() : QJsonObject();
        const QJsonValue color = controlTurn.contains("color")
            ? controlTurn["color"] : candidateTurn.value("color");
        return QJsonObject{{"turn", turn},
                           {"color", color},
                           {"control_move", controlTurn.value("move")},
                           {"candidate_move", candidateTurn.value("move")},
                           {"kind", "unknown/board_diverged"}};
    }
    return {};
}
}
int main(int argc,char **argv){ QCoreApplication app(argc,argv); const auto a=app.arguments();
      const QString recordsDir = option(a, "--records-dir", "");
      if (!recordsDir.isEmpty()) {
          const QString worker = option(a, "--worker", "");
          const QString bp = option(a, "--black-worker", worker), wp = option(a, "--white-worker", worker);
          const QString out = option(a, "--out", "/tmp/history-selfplay");
          const int prefix = option(a, "--prefix", "3").toInt();
          const int games = option(a, "--games", "1").toInt();
          const int plies = option(a, "--plies", "30").toInt();
          if (bp.isEmpty() || wp.isEmpty() || !QDir(recordsDir).exists() || prefix < 1 || games < 1 || plies < 1 || plies > 30) return 2;
          QDir().mkpath(out);
          return historyReport(recordsDir, out, bp, wp, prefix, games, plies).isEmpty() ? 3 : 0;
      }
      const QString bp=option(a,"--black-worker"),wp=option(a,"--white-worker"),out=option(a,"--out","match.json"); int games=option(a,"--games","2").toInt(),turns=option(a,"--plies","5").toInt(),set=option(a,"--opening-set","4").toInt(); if(bp.isEmpty()||wp.isEmpty()||games<1||turns<1||set<1||set>4)return 2;
     const QString fixtureMode = option(a, "--history-fixtures", "");
     QList<Fixture> fixtures;
     if (!fixtureMode.isEmpty()) {
         if (fixtureMode != "white15" && fixtureMode != "anhui" && fixtureMode != "all") return 2;
         if (fixtureMode == "white15" || fixtureMode == "all") { auto f = loadFixture("white15", "/wlf/Six/server/six_server/build/Desktop_Qt_6_11_1-Debug/棋谱文件/C6-serverBvscilentW-先手胜-.txt", 57); if (!f || !f->referee.board[8][6] || !f->referee.board[4][7]) return 2; fixtures.append(*f); }
         if (fixtureMode == "anhui" || fixtureMode == "all") { auto f = loadFixture("anhui", "/wlf/Six/程序原版压缩包/six/棋谱文件/C6-安徽大学Bvs重师W-先手胜-f.txt", 51); if (!f) return 2; fixtures.append(*f); }
     }
     if (fixtures.isEmpty()) {
         QJsonArray scenarios; for(const auto &s: {QString("control"),QString("candidate-black"),QString("candidate-white")}){ QJsonObject x{{"scenario",s}}; QJsonArray gs; for(int i=0;i<games;++i){ const bool candidateBlack=s=="candidate-black", candidateWhite=s=="candidate-white"; gs.append(play(candidateBlack?wp:bp,candidateWhite?wp:bp,i,turns,set)); } x["games"]=gs; scenarios.append(x); }
         QJsonArray comparisons; const auto controlGames=scenarios[0].toObject()["games"].toArray(), blackGames=scenarios[1].toObject()["games"].toArray(), whiteGames=scenarios[2].toObject()["games"].toArray();
          for(int i=0;i<games;++i){ for(const auto &side : {QString("black"),QString("white")}){ const QJsonObject candidate=side=="black"?blackGames[i].toObject():whiteGames[i].toObject(); const QJsonObject diff=compareTranscript(controlGames[i].toObject(),candidate,side); comparisons.append(QJsonObject{{"game",i+1},{"opening_id",controlGames[i].toObject()["opening_id"]},{"candidate_side",side},{"first_difference",diff.isEmpty()?QJsonValue(QJsonValue::Null):QJsonValue(diff)}}); } }
          QJsonObject root{{"mode","baseline-vs-candidate"},{"games",games},{"plies",turns},{"opening_set",set},{"scenarios",scenarios},{"comparisons",comparisons}}; QFile f(out); if(!f.open(QIODevice::WriteOnly))return 3; f.write(QJsonDocument(root).toJson(QJsonDocument::Indented)); f.close(); QFile csv(QFileInfo(out).path()+"/match.csv"); if(csv.open(QIODevice::WriteOnly)){csv.write("scenario,game,result,winner\n"); for(auto v:scenarios){auto x=v.toObject();for(auto g:x["games"].toArray()){auto o=g.toObject();csv.write(x["scenario"].toString().toUtf8()+","+QByteArray::number(o["game"].toInt())+","+o["result"].toString().toUtf8()+","+QByteArray::number(o["winner"].toInt())+"\n");}}}
          QFile md(QFileInfo(out).path()+"/match.md"); if(md.open(QIODevice::WriteOnly)) md.write(QString("# Baseline vs candidate\n\n- 场景：control、candidate-black、candidate-white（共 %1 组）\n- 每场最多双子回合：%2\n- 固定开局数：%3\n").arg(games*3).arg(turns).arg(set).toUtf8());
         return 0;
     }
     QJsonArray scenarios, comparisons;
     for (const Fixture &fixture : fixtures) {
         const QString candidateSide = fixture.currentColor == "B" ? "black" : "white";
         QJsonArray fixtureGames[3];
         for (int i = 0; i < games; ++i) {
             fixtureGames[0].append(play(bp, wp, i, turns, set, &fixture));
             fixtureGames[1].append(play(candidateSide == "black" ? wp : bp, candidateSide == "white" ? wp : bp, i, turns, set, &fixture));
         }
          for (int n = 0; n < 2; ++n) { const QString scenario = n == 0 ? "control" : "candidate-" + candidateSide; QJsonObject x{{"scenario", scenario}, {"fixture", fixture.name}, {"source", fixture.source}, {"prefix_plies", fixture.prefix}, {"current_color", fixture.currentColor}, {"forced_color", fixture.currentColor}, {"games", fixtureGames[n]}}; scenarios.append(x); }
          for (int i = 0; i < games; ++i) { const QJsonObject diff = compareTranscript(fixtureGames[0][i].toObject(), fixtureGames[1][i].toObject(), candidateSide); comparisons.append(QJsonObject{{"fixture",fixture.name},{"game",i+1},{"candidate_side",candidateSide},{"first_difference",diff.isEmpty()?QJsonValue(QJsonValue::Null):QJsonValue(diff)}}); }
     }
     QJsonObject root{{"mode","baseline-vs-candidate-history"},{"games",games},{"plies",turns},{"history_fixtures",fixtureMode},{"scenarios",scenarios},{"comparisons",comparisons}}; QFile f(out); if(!f.open(QIODevice::WriteOnly))return 3; f.write(QJsonDocument(root).toJson(QJsonDocument::Indented)); f.close();
     return 0;
      #if 0
      /* legacy output path is above; history mode returns before it. */
     for(int i=0;i<games;++i){
        const int openingId=controlGames[i].toObject()["opening_id"].toInt();
        for(const auto &side : {QString("black"),QString("white")}){
            const QJsonObject candidate=side=="black"?blackGames[i].toObject():whiteGames[i].toObject();
            const QJsonObject diff=compareTranscript(controlGames[i].toObject(),candidate,side);
            comparisons.append(QJsonObject{{"game",i+1},{"opening_id",openingId},{"candidate_side",side},{"first_difference",diff.isEmpty()?QJsonValue(QJsonValue::Null):QJsonValue(diff)}});
        }
    }
    QJsonObject root{{"mode","baseline-vs-candidate"},{"games",games},{"plies",turns},{"opening_set",set},{"scenarios",scenarios},{"comparisons",comparisons}}; QFile f(out); if(!f.open(QIODevice::WriteOnly))return 3; f.write(QJsonDocument(root).toJson(QJsonDocument::Indented)); f.close(); QFile csv(QFileInfo(out).path()+"/match.csv"); if(csv.open(QIODevice::WriteOnly)){csv.write("scenario,game,result,winner\n"); for(auto v:scenarios){auto x=v.toObject();for(auto g:x["games"].toArray()){auto o=g.toObject();csv.write(x["scenario"].toString().toUtf8()+","+QByteArray::number(o["game"].toInt())+","+o["result"].toString().toUtf8()+","+QByteArray::number(o["winner"].toInt())+"\n");}}}
     QFile md(QFileInfo(out).path()+"/match.md"); if(md.open(QIODevice::WriteOnly)) md.write(QString("# Baseline vs candidate\n").toUtf8());
     #endif
     return 0;
 }
