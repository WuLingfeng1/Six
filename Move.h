#ifndef MOVE_H
#define MOVE_H

#include<QPoint>

class Move
{
public:
    Move();
    Move(QPoint a,QPoint b,int s);
    QPoint step1() const;
    void setStep1(QPoint newStep1);
    QPoint step2() const;
    void setStep2(QPoint newStep2);
    int score() const;
    void setScore(int newScore);

protected:
    QPoint m_step1;
    QPoint m_step2;
    int m_score;

};

inline QPoint Move::step1() const
{
    return m_step1;
}

inline void Move::setStep1(QPoint newStep1)
{
    m_step1 = newStep1;
}

inline QPoint Move::step2() const
{
    return m_step2;
}

inline void Move::setStep2(QPoint newStep2)
{
    m_step2 = newStep2;
}

inline int Move::score() const
{
    return m_score;
}

inline void Move::setScore(int newScore)
{
    m_score = newScore;
}

#endif // MOVE_H
