#ifndef PLAYER_H
#define PLAYER_H

#include"Chequer.h"

class QPoint;
class Chequer;
class Player
{
public:
    Player();
    void play1();
    void play2();
    void ourFirstPlay();
    void oppoFirstPlay();
    void ourplay();
    void oppoplay();
    bool isEnd();
    void showWhoWin();
    void set(QPoint QPointation);
    void add(QPoint QPointation, std::vector<QPoint>& lastAdd);
    void swap();
    int negamax(int depth, int alpha, int beta);
    void retract(QPoint QPointation);
    void make(QPoint, QPoint);
    void unmake(QPoint, QPoint);
    void AnalyHH();
    void AnalyVV();
    void AnalyRR();
    void AnalyLL();
    int estimate();
    //void Comb(vector<struct QPoint>a, Move list[], int n);
    //查找搜索队列中是否包含指定值
   // bool contain(vector<QPoint>, QPoint);
    int type() const;

    void setType(int newType);
private:
    int m_type;//当前走棋方
    int m_step;//走两次落点中的第几步
    int m_win;//谁赢
    QPoint m_last;//
    QPoint m_llast;//记录上两次落子点
    std::pair< QPoint, QPoint>m_best;//最好的着法
    std::vector< QPoint>search;//搜索队列
};

#endif // PLAYER_H
