#include <QApplication>
#include <QCryptographicHash>
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
#include <vector>

#define private public
#include "Game.h"
#undef private

namespace {
using Windows = std::vector<std::vector<QPoint>>;
int productionDepth = -1;
struct Stone { QPoint p; int color; };
struct Position { std::array<std::array<int, 19>, 19> b{}; int stones = 0; int winner = 0; };
struct Fixture { QString file; int color; int knownRefutedTurn; };
struct Assessment { QString status; qint64 checked = 0, total = 0, refutations = 0, elapsed = 0; QJsonObject witness; };

bool inside(QPoint p) { return p.x() >= 0 && p.x() < 19 && p.y() >= 0 && p.y() < 19; }
QString note(QPoint p) { return QString(QChar('A' + p.y())) + QString::number(19 - p.x()); }
QJsonArray jsonPoints(const std::vector<QPoint> &p) { QJsonArray a; for (auto x : p) a.append(note(x)); return a; }
QJsonArray jsonWindows(const Windows &ws) { QJsonArray a; for (const auto &w : ws) a.append(jsonPoints(w)); return a; }
QString pairText(QPoint a, QPoint b) { return note(a) + " " + note(b); }
int id(QPoint p) { return p.x() * 19 + p.y(); }

Windows windows(const Position &p, int color, size_t maxHoles) {
    Windows out;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
        for (QPoint d : {QPoint(1,0), QPoint(0,1), QPoint(1,1), QPoint(1,-1)}) {
            if (!inside(QPoint(x,y) + d * 5)) continue;
            std::vector<QPoint> holes; bool blocked = false;
            for (int k = 0; k < 6; ++k) { const int c = p.b[x+d.x()*k][y+d.y()*k]; if (!c) holes.push_back(QPoint(x,y)+d*k); else if (c != color) blocked = true; }
            if (!blocked && holes.size() <= maxHoles) out.push_back(std::move(holes));
        }
    return out;
}
Windows completions(const Position &p, int c) { return windows(p, c, 2); }
bool put(Position &p, QPoint q, int c) {
    if (!inside(q) || p.b[q.x()][q.y()] || p.winner) return false;
    p.b[q.x()][q.y()] = c; ++p.stones;
    if (!windows(p, c, 0).empty()) p.winner = c;
    return true;
}
bool apply(const Position &before, QPoint a, QPoint b, int c, Position &after) {
    after = before; if (!put(after, a, c)) return false; if (after.winner) return b == QPoint(-1,-1); return put(after, b, c);
}
bool coverable(const Windows &ws) {
    if (ws.empty()) return true;
    for (const auto &w : ws) if (w.empty()) return false;
    for (QPoint a : ws.front()) for (QPoint b : ws.front()) {
        bool all = true; for (const auto &w : ws) if (std::find(w.begin(),w.end(),a)==w.end() && std::find(w.begin(),w.end(),b)==w.end()) { all=false; break; }
        if (all) return true;
    }
    return false;
}

bool load(const QString &path, std::vector<Stone> &stones) {
    QFile f(path); if (!f.open(QIODevice::ReadOnly)) return false;
    auto it = QRegularExpression(";\\s*([BW])\\s*\\(\\s*([A-S])\\s*,\\s*(1[0-9]|[1-9])\\s*\\)").globalMatch(QString::fromUtf8(f.readAll()));
    while (it.hasNext()) { auto m=it.next(); stones.push_back({QPoint(19-m.captured(3).toInt(),m.captured(2)[0].unicode()-'A'),m.captured(1)=="B"?1:2}); }
    return !stones.empty();
}
bool prefix(const std::vector<Stone> &s, int n, Position &p) { p={}; for (int i=0;i<n;++i) if (!put(p,s[i].p,s[i].color)) return false; return !p.winner; }

QJsonObject assess(const Position &after, int attacker, int limitMs) {
    QElapsedTimer clock; clock.start(); Assessment r;
    std::vector<QPoint> legal; for(int x=0;x<19;++x) for(int y=0;y<19;++y) if(!after.b[x][y]) legal.push_back({x,y});
    r.total=qint64(legal.size())*(legal.size()-1)/2;
    const Windows attack=windows(after,attacker,4), counter=completions(after,3-attacker);
    std::array<std::vector<int>,361> incidence; std::vector<int> base;
    for(size_t k=0;k<attack.size();++k){ if(attack[k].size()<=2) base.push_back(int(k)); for(auto q:attack[k]) incidence[id(q)].push_back(int(k)); }
    std::vector<int> seen(attack.size()); int stamp=0;
    for(size_t i=0;i<legal.size();++i) for(size_t j=i+1;j<legal.size();++j){
        if((r.checked&127)==0 && clock.elapsed()>=limitMs){r.status="unknown";r.elapsed=clock.elapsed();return { {"status",r.status},{"checked",double(r.checked)},{"total",double(r.total)},{"refutations",double(r.refutations)},{"witness",r.witness},{"elapsed_ms",double(r.elapsed)} };}
        ++r.checked; ++stamp; std::vector<int> affected; std::vector< std::vector<QPoint> > threats;
        auto add=[&](int k){if(seen[k]!=stamp){seen[k]=stamp;affected.push_back(k);}}; for(int k:base)add(k); for(auto q:{legal[i],legal[j]})for(int k:incidence[id(q)])add(k);
        bool win=false; for(int k:affected){std::vector<QPoint> left;for(auto q:attack[k])if(q!=legal[i]&&q!=legal[j])left.push_back(q);if(left.empty())win=true;if(left.size()<=2)threats.push_back(std::move(left));}
        bool counterWin=false; for(const auto&w:counter) if(std::find(w.begin(),w.end(),legal[i])==w.end()&&std::find(w.begin(),w.end(),legal[j])==w.end()){counterWin=true;break;}
        if(win||(!counterWin&&!coverable(threats))){++r.refutations;if(r.witness.isEmpty())r.witness={{"reply",jsonPoints({legal[i],legal[j]})},{"kind",win?"opponent_six":"uncoverable_no_counterwin"},{"threats",jsonWindows(threats)},{"own_immediate_counterwin",counterWin}};}
    }
    r.status=r.refutations?"refuted":"safe_one_reply";r.elapsed=clock.elapsed();
    return {{"status",r.status},{"checked",double(r.checked)},{"total",double(r.total)},{"refutations",double(r.refutations)},{"witness",r.witness},{"elapsed_ms",double(r.elapsed)}};
}

QJsonObject production(const Position &p, int color) {
    Game g; for(int x=0;x<19;++x)for(int y=0;y<19;++y)g.m_chess[x][y]=p.b[x][y]; g.m_type=color;g.m_step=0;g.m_win=TIE;g.rebuildSearch();
    productionDepth = -1; QObject::connect(&g,&Game::chessChange,[&](QPoint,int){ }); QElapsedTimer t;t.start(); g.bestPlay();
    QJsonObject o={{"move",jsonPoints({g.m_chosenMove.step1(),g.m_chosenMove.step2()})},{"pair",pairText(g.m_chosenMove.step1(),g.m_chosenMove.step2())},{"elapsed_ms",double(t.elapsed())},{"depth",productionDepth},{"score",g.m_chosenMove.score()},{"timed_out",g.m_timedOut}};
    return o;
}

QJsonObject analyzeTurn(const std::vector<Stone> &s, int prefixN, int color, int oracleMs) {
    Position before; QJsonObject out={{"prefix_plies",prefixN},{"color",color==1?"B":"W"},{"history_prefix",jsonPoints([&]{std::vector<QPoint>v;for(int i=0;i<prefixN;++i)v.push_back(s[i].p);return v;}())}};
    if(!prefix(s,prefixN,before)){out["status"]="invalid_prefix";return out;}
    const Windows direct=completions(before,3-color); out["opponent_direct_completions"]=jsonWindows(direct); out["production"]=production(before,color);
    std::vector<QPoint> legal;for(int x=0;x<19;++x)for(int y=0;y<19;++y)if(!before.b[x][y])legal.push_back({x,y}); QJsonArray candidates; int exact=0,safe=0,ref=0,unknown=0;
    const QString productionPair=out["production"].toObject()["pair"].toString(); int productionSafe=0;
    auto evaluate=[&](QPoint a,QPoint b,bool isExact){Position after; if(!apply(before,a,b,color,after))return;const bool isProduction=pairText(a,b)==productionPair||pairText(b,a)==productionPair;QJsonObject row={{"move",jsonPoints({a,b})},{"pair",pairText(a,b)},{"exact_cover",isExact},{"production_move",isProduction}};QJsonObject ar=after.winner==color?QJsonObject{{"status","safe_one_reply"},{"checked",0},{"total",0},{"refutations",0},{"elapsed_ms",0}}:assess(after,3-color,oracleMs);row["assessment"]=ar;candidates.append(row);if(ar["status"]=="safe_one_reply"){++safe;if(isProduction)productionSafe=1;}else if(ar["status"]=="refuted")++ref;else ++unknown;};
    for(size_t i=0;i<legal.size();++i)for(size_t j=i+1;j<legal.size();++j){bool ok=true;for(const auto&w:direct)if(std::find(w.begin(),w.end(),legal[i])==w.end()&&std::find(w.begin(),w.end(),legal[j])==w.end()){ok=false;break;}if(ok){++exact;evaluate(legal[i],legal[j],true);}}
    const auto pm=out["production"].toObject()["move"].toArray(); QJsonObject productionAssessment{{"status","invalid"}};
    if(pm.size()==2){auto decode=[](const QJsonValue&v){const QString t=v.toString();return QPoint(19-t.mid(1).toInt(),t[0].unicode()-'A');};QPoint a=decode(pm[0]),b=decode(pm[1]);Position after;if(apply(before,a,b,color,after))productionAssessment=after.winner==color?QJsonObject{{"status","safe_one_reply"},{"checked",0},{"total",0},{"refutations",0},{"elapsed_ms",0}}:assess(after,3-color,oracleMs);}
    out["production_assessment"]=productionAssessment;out["exact_cover_count"]=exact;out["safe_count"]=safe;out["refuted_count"]=ref;out["unknown_count"]=unknown;out["safe_alternative_count"]=safe-productionSafe;out["safe_alternative_exists"]=(safe-productionSafe)>0;out["candidates"]=candidates;out["status"]="complete";return out;
}

QString option(const QStringList&a,const QString&k,const QString&d){int i=a.indexOf(k);return i>=0&&i+1<a.size()?a[i+1]:d;}
QString hashFile(const QString&p){QFile f(p);if(!f.open(QIODevice::ReadOnly))return {};return QString(QCryptographicHash::hash(f.readAll(),QCryptographicHash::Sha256).toHex());}
void closeDialogs(QTimer&t){QObject::connect(&t,&QTimer::timeout,[]{for(QWidget*w:QApplication::topLevelWidgets())if(auto*b=qobject_cast<QMessageBox*>(w))b->done(QMessageBox::Discard);});t.start(10);}
}

int main(int argc,char**argv){qputenv("QT_QPA_PLATFORM","offscreen");QApplication app(argc,argv);qInstallMessageHandler([](QtMsgType,const QMessageLogContext&,const QString&m){auto x=QRegularExpression("Search completedDepth: (\\d+)").match(m);if(x.hasMatch())productionDepth=x.captured(1).toInt();});QTimer dialogs;closeDialogs(dialogs);const auto a=app.arguments();const QString records=option(a,"--records","/wlf/Six/程序原版压缩包/six/棋谱文件"),outDir=option(a,"--out","/tmp/backward-analysis"),sourceDir=QDir(records).filePath("..");const int maxTurns=option(a,"--max-turns","10").toInt(),window=option(a,"--window","5").toInt(),oracle=option(a,"--oracle-ms","2000").toInt();if(!QDir(outDir).exists())qFatal("Output directory must already exist");
    const std::array<Fixture,6> fs={{{"C6-东北秦皇岛分校Bvs重师W-先手胜-祁门一中.txt",2,16},{"C6-安庆师范大学Bvs重师W-先手胜-祁门一中.txt",2,18},{"C6-安徽大学Bvs重师W-先手胜-fff.txt",2,26},{"C6-重师Bvs东北大学秦皇岛分校W-后手胜-祁门一中.txt",1,35},{"C6-重师Bvs吉林大学一队W-后手胜-祁门一中.txt",1,19},{"C6-重师Bvs安庆师范大学W-后手胜-祁门一中.txt",1,13}}};QJsonArray games;int found=0;
    QJsonObject earliest;for(const auto&f:fs){std::vector<Stone>s;if(!load(QDir(records).filePath(f.file),s))continue;std::vector<int> turns;Position p;for(int i=0;i<int(s.size());++i){if(s[i].color==f.color&&(i==0||s[i-1].color!=f.color))turns.push_back(i);if(!put(p,s[i].p,s[i].color))break;}std::vector<int> selected;for(int i=int(turns.size())-1;i>=0&&int(selected.size())<maxTurns;--i){const int turn=turns[i]==0?1:turns[i]/2+2;if(turn>=std::max(1,f.knownRefutedTurn-window+1))selected.push_back(turns[i]);}std::sort(selected.begin(),selected.end());QJsonArray rows;for(int n:selected){auto r=analyzeTurn(s,n,f.color,oracle);const int turn=n==0?1:n/2+2;r["turn"]=turn;r["historical_followup"]=jsonPoints([&]{std::vector<QPoint>v;for(int i=n;i<int(s.size())&&i<n+4;++i)v.push_back(s[i].p);return v;}());rows.append(r);if(r["production_assessment"].toObject()["status"]=="refuted"&&r["safe_alternative_exists"].toBool()){found++;if(earliest.isEmpty()||turn<earliest["turn"].toInt())earliest=QJsonObject{{"source",f.file},{"turn",turn},{"production",r["production"]},{"production_assessment",r["production_assessment"]},{"safe_alternative_count",r["safe_alternative_count"]}};}}games.append(QJsonObject{{"source",f.file},{"self_color",f.color==1?"B":"W"},{"known_refuted_turn",f.knownRefutedTurn},{"turns_analyzed",rows}});}
    QJsonObject report={{"tool","backward_analysis"},{"oracle_ms_per_candidate",oracle},{"max_turns",maxTurns},{"window",window},{"safe_one_reply_definition","one complete legal opponent double-stone reply layer; not a global win"},{"source_sha256_game",hashFile(QDir(sourceDir).filePath("Game.cpp"))},{"source_sha256_move",hashFile(QDir(sourceDir).filePath("Move.cpp"))},{"config_sha256",hashFile(QDir(sourceDir).filePath("six.pro"))},{"games",games},{"earliest_avoidable_found",found>0},{"earliest_avoidable",earliest}};const QByteArray json=QJsonDocument(report).toJson(QJsonDocument::Indented);QFile jf(QDir(outDir).filePath("backward_analysis.json"));if(!jf.open(QIODevice::WriteOnly))qFatal("cannot write JSON");jf.write(json);QString md="# Backward Analysis\n\n`safe_one_reply` is not a global win. The six Chongshi losses are scanned from the terminal side; only complete, known oracle results count as safe.\n\n- max turns: "+QString::number(maxTurns)+"\n- window: "+QString::number(window)+"\n- earliest avoidable found: **"+(found?"yes":"no")+"**\n";if(!earliest.isEmpty())md+="- earliest: turn "+QString::number(earliest["turn"].toInt())+" in "+earliest["source"].toString()+"\n";md+="\n";for(auto v:games){auto g=v.toObject();md+="## "+g["source"].toString()+"\n\n";for(auto x:g["turns_analyzed"].toArray()){auto r=x.toObject();md+="- turn "+QString::number(r["turn"].toInt())+": production "+r["production"].toObject()["pair"].toString()+", production status "+r["production_assessment"].toObject()["status"].toString()+", exact covers "+QString::number(r["exact_cover_count"].toInt())+", safe alternatives "+QString::number(r["safe_alternative_count"].toInt())+", unknown "+QString::number(r["unknown_count"].toInt())+"\n";}md+="\n";}QFile mf(QDir(outDir).filePath("backward_analysis.md"));if(!mf.open(QIODevice::WriteOnly))qFatal("cannot write Markdown");mf.write(md.toUtf8());QTextStream(stdout)<<json;return 0;}
