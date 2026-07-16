#include"Move.h"
Move::Move()
{}
Move::Move(QPoint a,QPoint b,int s)
    :m_step1(a)
    ,m_step2(b)
    ,m_score(s)
{}
