#ifndef TIMER_H
#define TIMER_H

#include "qelapsedtimer.h"
#include <QWidget>
#include <QTime>

namespace Ui {
class Timer;
}

class Timer : public QWidget
{
    Q_OBJECT

public:
    explicit Timer(QWidget *parent = nullptr);
    ~Timer();
public slots:
    void on_start();//点击开始键后，计时器开始运行
    void on_restart(int);//重新开始计时
    void on_stop();//我方走完就停止，我方开始走就重新开始计时
    void update();
    //vodi ProcessEvent();
    void init();

private:
    Ui::Timer *ui;
    QTimer * timer;    //定时器 每秒更新时间
    //QElapsedTimer *timer;
    QTime time;
    QTime num1; //记录最后一个数
    QTime num2;
    int timetype;
};

#endif // TIMER_H
