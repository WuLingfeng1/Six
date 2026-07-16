#include "timer.h"
#include "ui_timer.h"
#include <QTimer>
#include <QThread>

Timer::Timer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Timer),
    timer(new QTimer()),
    time(0,0,0),
    num1(0,0,0),
    num2(0,0,0),
    timetype(0)
{
    ui->setupUi(this);
    // timer->start(1);
    // connect(timer, &QTimer::timeout, this,&Timer::update,Qt::DirectConnection );

    ui->Time->setDigitCount(9);
    ui->Time->setSegmentStyle(QLCDNumber::Flat);
    timer->setTimerType(Qt::PreciseTimer);//提高计时精度
    connect(timer, &QTimer::timeout, this, &Timer::update, Qt::DirectConnection);
}

Timer::~Timer()
{
    delete ui;
}

void Timer::on_start()
{
    //connect(timer, &QTimer::timeout, this, &Timer::update, Qt::DirectConnection);
    timer->start(33);//这里以及下面的on_restart,理论上这个这个数值设置越大，即刷新频率越小会让计时精度更高
    //connect(timer, &QTimer::timeout, this, &Timer::update, Qt::DirectConnection);
    //qDebug() << "on_start";
}

void Timer::on_restart(int type)
{

    //time=QTime(0,0,0);
    /*if (timer->isActive() == false) {
        timer->start(1);
        connect(timer, &QTimer::timeout, this, &Timer::update, Qt::DirectConnection);
    }*/
    //connect(timer, &QTimer::timeout, this, &Timer::update, Qt::DirectConnection);
    timetype = type;
    //qDebug() << "zheshi" << timetype;
    //qDebug() << "on_restart";
    if (timer->isActive() == false) {
        if (timetype == 0) {
            //time = QTime(0, 0, 0);
            timer->start(33);
        }
        if (timetype == 1) {
            time = num1;
            timer->start(33);
            //qDebug() << "time = num1";
        }
        if (timetype == 2) {
            time = num2;
            timer->start(33);
            //qDebug() << "time = num2";
        }
        //connect(timer, &QTimer::timeout, this, &Timer::update, Qt::DirectConnection);
    }
}

void Timer::on_stop()
{
    /*if (timer->isActive() == true)
        timer->stop();*/
    //qDebug() << "on_stop";
    if (timer->isActive() == true) {
        timer->stop();
        if (timetype == 1) { num1 = time; }
        if (timetype == 2) { num2 = time; }
        //qDebug() << num1 << num2;
        //qDebug() << time;
        //timer->stop();
    }
    //    connect(timer, &QTimer::timeout,[=](){
    //        ui->Time->display(num.toString("ss.zzz"));});

}

void Timer::update()
{
    time=time.addMSecs(33);//需要与上面的start同步修改
    ui->Time->display(time.toString("mm:ss.zzz"));//display只是输出括号中的值，不会做绑定
}
void Timer::init()
{
    // timer->stop();
    time.setHMS(0, 0, 0); // 将时间设置为00:00:00
    timer->isActive() == false;
    //time = QTime(0, 0, 0);
    ui->Time->display(time.toString("mm:ss.zzz"));
    qDebug() << time;

    // timer->start(1);
}
