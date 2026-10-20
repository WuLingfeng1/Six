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
    };
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
    void rebuildSearch();
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
    Move m_chosenMove;
    std::vector<TTEntry> m_tt;
    int m_lineScore[2][112]{};
    void recomputeLineScores();
    void placeStone(int x, int y, int color);
    void clearStone(int x, int y);
};

#endif // GAME_H
