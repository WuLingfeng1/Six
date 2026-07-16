#ifndef CHESSWOMAN_H
#define CHESSWOMAN_H
#include<QPainter>
class Chesswoman
{
public:

    Chesswoman(int x,int y,int type);
    Chesswoman(QPoint pos,int type);
    int x() const;
    int y() const;
    int type() const;

    void draw(QPainter *painter,int gird);


private:
    int m_x;
    int m_y;
    int m_type;//黑棋1，白棋2
};
#endif // CHESSWOMAN_H
