#ifndef GAME_H
#define GAME_H
#include <QObject>
#include<iostream>
#include"Move.h"

#define WIN_BLACK 1
#define WIN_WHITE 2
#define TIE 0
#define OTHER 3

class Game:public QObject
{
    Q_OBJECT
public:
    Game();
    void ourFirstPlay();
    void ourplay();
    bool isEnd();
    void swap();
    void set(QPoint location);
    void add(QPoint location, std::vector<QPoint>& lastAdd);//下棋后扩展搜索队列，第二个参数保存这一步添加的扩展区域，在后来模拟走法的时候方便撤回
    void retract(QPoint location,std::vector<QPoint>&lastAdd);//传add的中第二个参数代表的这一步加入的扩展队列，在search中做删除
    void make(QPoint, QPoint);//模拟落子，即不扩展
    void unmake(QPoint, QPoint);
    void mergeSort(Move list[], int n);//归并排序
    void mergePass(Move list[], Move t[], int s, int n);
    void merge(Move list[], Move t[], int l, int m, int r);
    void AnalyHH();//评估，下之后的值（四个方向）-下之前的值（四个方向）
    void AnalyVV();
    void AnalyRR();
    void AnalyLL();
    void AnalyFourConnect();//判断是活四还是眠四，数组对应增加
    void resetFourVariable();//重置四个全局变量
    bool CheckWinningPatterns();//检查必胜棋型
    int estimate();
    void Comb(Move list[], int n);
    bool contain(std::vector<QPoint>, QPoint);
    int type() const;
    void setType(int newType);
    void stepadd();//步数的增加和减少，在chequer对手操作时调用
    void stepdec();
    int step() const;

signals:
    void chessChange(QPoint ,int);
    void undoenable();//撤回键可用信号
    void save_signal();
    void restart(int);//给时钟发送的重新计时
    void stop();//停止计时，显示结果
//    void clickoff();//向窗口发送点击失效的信号
//    void clickon();//向窗口发送可以点击的信号
public slots:
    void whoFirst(int type);
    void whoPlay(int type);
    void gameundo(int x,int y);
    void whoWin();
private:
    int numberOfMyRoad[7]{ 0 };
    int numberOfEnemyRoad[7]{ 0 };
    int scoreOfMyRoad[7]{ 0,1,5,30,110,20,100000 };
    int scoreOfEnemyRoad[7]{ 0,10,40,80,5000,5000,100000 };
    int scoreOfWinningChessPattern[2]{0, 2000};//形成必胜棋型的步额外加2000
    int numberOfFourConnect[2]{0};//第一个数为眠四数，第二个数为活四数
    int numberOfHHfourConnect[2]{0};//不同方向上四连的个数
    int numberOfVVfourConnect[2]{0};
    int numberOfRRfourConnect[2]{0};
    int numberOfLLfourConnect[2]{0};

    int m_chess[19][19];//棋盘信息
    std::vector< QPoint>m_search;//搜索队列
    int m_step;//下的步数
    int m_win;//记录谁赢
    int m_type;//记录我方的类型
    int m_count;//保存一共生成多少中组合走法
    QPoint m_last;//
    QPoint m_llast;//记录上两次落子点


};

#endif // GAME_H
