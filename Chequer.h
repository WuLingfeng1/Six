#ifndef CHEQUER_H
#define CHEQUER_H

#include <QWidget>
#include<iostream>
class Chesswoman;
class Game;
class Chequer:public QWidget
{
    Q_OBJECT
public:
    Chequer(QWidget *parent=0);
    ~Chequer();
    void drawGrid(QPainter *painter);
    void draw9Point(QPainter *painter);
    void drawPointer(QPainter *painter);
    void drawQizi(QPainter *painter);
    void drawSign(QPainter *painter);
    std::vector<Chesswoman> &getChesswoman();
    Game *getGame() const;

public   slots:
    void onChessChange(QPoint,int);
    //保存棋谱
    bool save();
    //撤回
    bool undo();
    //新建
    bool newBuilt();
    //打开
    bool loadfile(QString);
//    void clickoff();
//    void clickon();
signals:
    void gameundo(int x,int y);//撤回操作中让game中也做出对应的撤回操作
    void win();//发送对手赢的信息，和game的显示谁赢对话框相连
    void open();//给边框栏发送这是以打开的棋谱下残局，让他点击开始按钮后不从第一步开始下（根据具体情况判断
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
private:
    int m_grid;//格子尺寸
    int m_startX;//棋盘起点X
    int m_startY;//棋盘起点Y
    int m_chessX;//棋盘坐标
    int m_chessY;
    std::vector<Chesswoman>chesswoman;//保存所有棋子
    Game *game;
    QPointF m_mouse;//保存鼠标的位置
    //bool isover = false;
};

#endif // CHEQUER_H
