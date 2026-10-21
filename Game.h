#ifndef GAME_H
#define GAME_H
#include <QObject>
#include <iostream>
#include <vector>
#include <QElapsedTimer>
#include "Move.h"

#define WIN_BLACK 1
#define WIN_WHITE 2
#define TIE 0
#define OTHER 3

struct TTEntry {
    quint64 key = 0;
    int depth = -1;
    int score = 0;
    int flag = 0; // 1=EXACT, 2=LOWER, 3=UPPER
};

class Game:public QObject
{
    Q_OBJECT
public:
    Game();

    struct SearchConfig {
        qint64 timeLimitMs = 2000;
        qint64 probeBudgetMs = 1100;
        qint64 probeDiscoveryMs = 200;
        int probePoints = 28;
        int probeReplies = 32;
        int qMaxPly = 2;
        int qMaxNodes = 1024;
        int qMaxDefenses = 16;
        int maxDepth = 6;
        int rootMaxPoints = 40;
        int rootMaxPairs = 25;
        int innerPoints = 18;
        int innerPairsDeep = 35;
        int innerPairsShallow = 20;
        bool randomize = false;
        int randomTopN = 5;
        double randomTemp = 2000.0;
        int randomSeed = 0;
        bool ttEnabled = true;
        bool incrementalTactics = false; // Maintain six-window counts during search.
        bool threatExtension = true;
        int threatExtendMax = 2;
        int threatExtendBudget = 64;
        // Actual lineShape weights; configure before constructing/seeding Game.
        int shapeSingle = 1;
        int shapeOpenTwo = 20;
        int shapeClosedThree = 100;
        int shapeOpenThree = 1000;
        int shapeClosedFour = 3000;
        int shapeOpenFour = 20000;
        int shapeFive = 60000;
        int shapeSix = 100000;
    };
    struct SearchIteration {
        int depth = 0;
        bool complete = false;
        quint64 nodes = 0;
        quint64 ttHits = 0;
        int rootMoves = 0;
        int extensionsUsed = 0;
        int qNodes = 0;
        qint64 elapsedMs = 0;
    };
    struct SearchStats {
        quint64 nodes = 0;
        quint64 ttProbes = 0;
        quint64 ttHits = 0;
        quint64 ttStores = 0;
        int completedDepth = 0;
        int extensionsUsed = 0;
        int rootCandidates = 0;
        int rootSurvivors = 0;
        int rootExcluded = 0;
        int rootUnknown = 0;
        qint64 precheckMs = 0;
        qint64 elapsedMs = 0;
        const char* stopReason = "not_started";
        std::vector<SearchIteration> iterations;
    };
    const SearchStats& searchStats() const { return m_searchStats; }
    static SearchConfig searchConfig;
    static void seedRandom(unsigned seed);
    void ourFirstPlay();
    void ourplay();
    bool isEnd();
    void swap();
    void set(QPoint location);
    void add(QPoint location, std::vector<QPoint>& lastAdd);
    void retract(QPoint location,std::vector<QPoint>&lastAdd);
    void make(QPoint, QPoint);
    void unmake(QPoint, QPoint);
    void analyzeDir(int px, int py, int dx, int dy,
                    int skipX, int skipY, int dirIdx);
    void AnalyFourConnect();
    void resetFourConnect();
    bool CheckWinningPatterns();
    int estimate();
    int evaluateBoard();
    quint64 boardHash() const;
    int evaluatePoint(int x, int y, int piece);
    int evaluatePair(QPoint a, QPoint b, int piece);
    bool hasSix(int x, int y, int piece);
    bool createsThreat(QPoint a, QPoint b, int piece) const;
    void rebuildSearch(bool rebuildCaches = true);
    // 1: immediate win, 2: required defenses, -1: undefendable, 0: quiet.
    // -2: interrupted/unknown. enumerate=false only checks existence of a cover.
    int tacticalMoves(int pieceType, std::vector<Move>& out, int stonesRemaining = 2,
                      bool enumerate = true, qint64 deadlineMs = -1);
    // Interrupted ranking returns -2 with out cleared, never a partial quiet list.
    int generateCandidates(int pieceType, std::vector<Move>& out,
                           int maxPoints, int maxPairs, int stonesRemaining = 2,
                           qint64 deadlineMs = -1);
    int negamax(int depth, int alpha, int beta, int pieceType, int qply = 0, quint64 hash = 0);
    void bestPlay(int stonesRemaining = 2);
    bool timedOut();
    bool contain(const std::vector<QPoint>&, QPoint) const;
    int type() const;
    void setType(int newType);
    void stepadd();
    void stepdec();
    int step() const;

signals:
    void chessChange(QPoint ,int);
    void undoenable();
    void save_signal();
    void restart(int);
    void stop();
public slots:
    void whoFirst(int type);
    void whoPlay(int type);
    void gameundo(int x,int y);
    void whoWin();

private:
    int numberOfMyRoad[7]{ 0 };
    int numberOfEnemyRoad[7]{ 0 };
    int scoreOfMyRoad[7]{ 0,1,20,300,2000,6000,100000 };
    int scoreOfEnemyRoad[7]{ 0,1,20,300,2000,6000,100000 };
    int scoreOfWinningChessPattern[2]{0, 2000};
    int m_fourConnect[4][2]{};

    int m_chess[19][19];
    std::vector<QPoint>m_search;
    int m_step;
    int m_win;
    int m_type;
    QPoint m_last;
    QPoint m_llast;

    QElapsedTimer m_timer;
    qint64 m_timeLimitMs;
    bool m_timedOut;
    int m_qNodes = 0; // Cumulative extension budget for one bestPlay, not per iteration.
    int m_threatBudget = 0;
    int m_extensions = 0;
    SearchStats m_searchStats;
    Move m_chosenMove;
    std::vector<TTEntry> m_tt;
    int m_lineScore[2][112]{};
    unsigned char m_windowCounts[924][3]{}; // black, white, occupied
    int m_forcingWindowCount = 0;
    void updateTacticalWindows(int x, int y, int color, int delta);
    void rebuildTacticalWindows();
    void recomputeLineScores();
    void placeStone(int x, int y, int color);
    void clearStone(int x, int y);
};

#endif // GAME_H
