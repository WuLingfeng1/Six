#include"Chesswoman.h"
Chesswoman::Chesswoman(int x,int y,int type)
    :m_x{x}
    ,m_y{y}
    ,m_type{type}
{}

Chesswoman::Chesswoman(QPoint pos, int type)
    :m_x{pos.x()}
    ,m_y{pos.y()}
    ,m_type{type}
{}

int Chesswoman::x() const
{
    return m_x;
}

int Chesswoman::y() const
{
    return m_y;
}
void Chesswoman::draw(QPainter *painter,int grid)
{
    if(m_type==1)
        painter->drawPixmap(QRect(m_x * grid + 0.5 * grid, m_y * grid + 0.5 * grid, grid, grid),
                            QPixmap(":/six/black.png"));
    else  if(m_type==2)
        painter->drawPixmap(QRect(m_x * grid + 0.5 * grid, m_y * grid + 0.5 * grid, grid, grid),
                            QPixmap(":/six/white.png"));
}

int Chesswoman::type() const
{
    return m_type;
}
