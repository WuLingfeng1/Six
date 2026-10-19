#include "Chequer.h"
#include "Chesswoman.h"
#include "Move.h"
#include "options.h"
#include "Game.h"
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QDebug>
#include <QMouseEvent>
#include <QPaintEvent>
#include <iostream>
#include <algorithm>
#include <vector>
#include <QPushButton>
#include <QMessageBox>
#include <QPoint>
#include <QInputDialog>
#include <QFile>
#include "infordialog.h"
using std::vector;



Chequer::Chequer(QWidget *parent)
    :QWidget{parent}
    ,m_chessX{-1}
    ,m_chessY{-1}
    ,game{new Game()}
    ,m_mouse{-100000,-100000}
{
    game->setObjectName("game");
    //鼠标移动时（哪怕没有按着鼠标按键），widget也能接受鼠标移动事件
    setMouseTracking(true);  //If mouse tracking is enabled, the widget receives mouse move events even if no buttons are pressed
    setAttribute(Qt::WA_TransparentForMouseEvents,true);
    QObject::connect(game,&Game::chessChange,this,&Chequer::onChessChange);
    QObject::connect(game,&Game::save_signal,this,&Chequer::save);
    QObject::connect(this,&Chequer::gameundo,game,&Game::gameundo);
    QObject::connect(this,&Chequer::win,game,&Game::whoWin);
//    QObject::connect(game,&Game::clickoff,this,&Chequer::clickoff);
//    QObject::connect(game,&Game::clickon,this,&Chequer::clickon);
}
Chequer::~Chequer()
{

}
//void Chequer::clickoff()
//{
//    setAttribute(Qt::WA_TransparentForMouseEvents,true);
//}

//void Chequer::clickon()
//{
//    setAttribute(Qt::WA_TransparentForMouseEvents,false);
//}
void Chequer::paintEvent(QPaintEvent *event){
    m_grid = height()/19.5;
    //棋盘起点坐标
    m_startX = m_grid;
    m_startY = m_grid;

    QPainter painter(this);  //设置Qpainter,关联当前窗口
    painter.drawPixmap(rect(), QPixmap(":/six/bg.png")); //棋盘底图由Qt资源系统提供

    drawGrid(&painter);//绘制网格
    draw9Point(&painter);//绘制九个点
    drawPointer(&painter);//绘制指示器，显示鼠标指示的位置
    drawQizi(&painter);//绘制棋子
    drawSign(&painter);//绘制最新的下棋点
}
void Chequer::drawGrid(QPainter *painter)
{
    //设置画笔
    QPen pen;
    pen.setWidth(1);  //线宽
    painter->setPen(pen);

    //画横线
    for(int i=0;i<19;i++)
        painter->drawLine(m_startX,m_startY+i*m_grid,m_startX+18*m_grid,m_startY+i*m_grid);
    //画竖线
    for(int i=0;i<19;i++)
        painter->drawLine(m_startX+i*m_grid,m_startY,m_startX+i*m_grid,m_startY+18*m_grid);
    for (int i = 1; i <= 19; i++) {
        int j = 20 - i;
        QString str = QString::number(j);
        painter->drawText(m_startX - 20, m_startY + (i - 1) * m_grid + 5, str);
    }
    for (int i = 0; i < 19; i++) {
        QString str = QChar(i + 65);
        painter->drawText(m_startX + i * m_grid - 3, m_startY + height() - 23, str);
        painter->drawText(m_startX + i * m_grid - 3, m_startY - 5, str);
    }
}
void Chequer::draw9Point(QPainter *painter)
{
    //设置画笔
    QPen pen;
    pen.setWidth(7);  //线宽
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    int num[3]={3,9,15};
    for(auto i:num)
        for(auto j:num)
            painter->drawPoint(m_startX+i*m_grid,m_startY+j*m_grid);
}
void Chequer::drawPointer(QPainter *painter)
{
    QPen pen;
    pen.setWidth(2);  //线宽
    pen.setColor("green");
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    painter->drawRect(m_mouse.x(),m_mouse.y(),m_grid,m_grid);//鼠标移动到哪儿，绿色框就到哪儿
}
void Chequer::drawSign(QPainter *painter)
{
    QPen pen;
    pen.setWidth(2);  //线宽
    pen.setColor("red");
    painter->setPen(pen);
    //获取最后两个棋子的坐标
    for(int i=chesswoman.size()-1;i>(int)chesswoman.size()-3&&i>=0;i--)
        painter->drawRect(chesswoman[i].x()*m_grid+0.5*m_grid,chesswoman[i].y()*m_grid+0.5*m_grid,m_grid,m_grid);

}
void Chequer::drawQizi(QPainter *painter)
{
    for(auto e:chesswoman)
        e.draw(painter,m_grid);
}
void Chequer::mouseMoveEvent(QMouseEvent *event)
{

    //获取鼠标的坐标
    int XPos = event->pos().x();
    int YPos = event->pos().y();

    //保证点击在棋盘范围内
    if(XPos >= m_startX && XPos <= m_startX + 18*m_grid
            &&  YPos >= m_startY && YPos <= m_startY+18*m_grid){
        //棋盘位置转化为数组坐标值
        m_chessX = round(1.0*(XPos-m_startX) / m_grid);
        m_chessY = round(1.0*(YPos-m_startY) / m_grid);
        //qDebug() << m_chessX <<m_chessY;
    }
    m_mouse={m_chessX*m_grid+0.5*m_grid,m_chessY*m_grid+0.5*m_grid};
    //更新窗口，间接调用paintEvent
    update();
    //repaint();

}

Game *Chequer::getGame() const
{
    return game;
}

bool Chequer::save()
{
    InforDialog*dialog=new InforDialog(this);
    dialog->show();
    int ret=dialog->exec();
    if(ret==1)
    {
        QFile *file=new QFile(dialog->getFilepath());
        bool ret = file->open(QIODevice::WriteOnly|QIODevice::Append);    //创建文件
        if(!ret)
        {
            QMessageBox::warning(this,"warning","打开失败");
            return false;
        }
        for(auto e:chesswoman)
        {
            file->write(";");
            if(e.type()==1)
                file->write("B");
            else
                file->write("W");
            QString x=(QChar)(e.x()+65);
            QString y=QString::number(19-e.y());
            QString data="("+x+","+y+")";
            file->write(data.toUtf8());
        }
        file->write("}");
        file->close();
    }
    return true;
}

bool Chequer::undo()
{
    vector<QPoint>temp;
    emit gameundo(chesswoman.back().y(),chesswoman.back().x());
    chesswoman.pop_back();
    update();
    if(game->step()==0)
        game->swap();
    game->stepdec();
    return true;

}

bool Chequer::newBuilt()
{
    Game *p;
    p=game;
    game=new Game();
    delete p;
    QObject::connect(game,&Game::chessChange,this,&Chequer::onChessChange);
    QObject::connect(game,&Game::save_signal,this,&Chequer::save);
    QObject::connect(this,&Chequer::gameundo,game,&Game::gameundo);
    QObject::connect(this,&Chequer::win,game,&Game::whoWin);
//    QObject::connect(game,&Game::clickoff,this,&Chequer::clickoff);
//    QObject::connect(game,&Game::clickon,this,&Chequer::clickon);
    chesswoman.clear();
    m_chessX=-1;
    m_chessY=-1;
    setAttribute(Qt::WA_TransparentForMouseEvents,true);
    update();
    return true;
}

bool Chequer::loadfile(QString filename)
{
    QChar endtype;
    QChar lasttype;//保存最后两个棋子的类型
    vector<QPoint>no_use;
    //从棋谱文件中读出所有字符串
    QFile *file=new QFile(filename);
    bool ret = file->open(QIODevice::ReadOnly);
    if(!ret)
    {
        QMessageBox::warning(this,"warning","打开失败");
        return false;
    }
    QByteArray temp=file->readAll();
    QString data=temp;
    for(int i=0;i<data.size();i++)
    {
        QString woman;

        if(data[i]==';')
        {
            if(data[i+6]==')')
            {
                lasttype=endtype;
                woman=data.mid(i+1,6);
                endtype=woman[0];
                if(woman[0]=='B')
                    chesswoman.push_back({woman[2].toLatin1()-65,19-woman[4].digitValue(),1});
                else
                    chesswoman.push_back({woman[2].toLatin1()-65,19-woman[4].digitValue(),2});
            }

            else if(data[i+7]==')')
            {
                lasttype=endtype;
                woman=data.mid(i+1,7);
                endtype=woman[0];
                if(woman[0]=='B')
                    chesswoman.push_back({woman[2].toLatin1()-65,19-woman.mid(4,2).toInt(),1});
                else
                    chesswoman.push_back({woman[2].toLatin1()-65,19-woman.mid(4,2).toInt(),2});
            }
            //读出每一个棋子信息
        }
        //自己点开始按钮，否则棋盘还是不能点击
    }
    file->close();
    //棋盘更新
    update();
    //game更新
    //第一步
    auto e=chesswoman[0];
    game->set({e.y(),e.x()});
    game->add({e.y(),e.x()},no_use);
    game->swap();
    //其他步
    for(int i=1;i<chesswoman.size();i++)
    {
        game->set({chesswoman[i].y(),chesswoman[i].x()});
        game->add({chesswoman[i].y(),chesswoman[i].x()},no_use);
        if(game->step()==2)
            game->swap();
    }
    //判断是否结束
    if(game->isEnd())
        emit win();
    //给options发送当前是打开的文件
    emit open();
    return true;
}
void Chequer::mousePressEvent(QMouseEvent *event){

    //获取点击的坐标
    bool judge=false;//判断该点是否已经下了棋
    int clickXPos = event->position().x();
    int clickYPos = event->position().y();

    //保证点击在棋盘范围内
    if(clickXPos >= m_startX && clickXPos <= m_startX + 18*m_grid
            &&  clickYPos >= m_startY && clickYPos <= m_startY+18*m_grid){
        //棋盘位置转化为数组坐标值
        m_chessX = round(1.0*(clickXPos-m_startX) / m_grid);
        m_chessY = round(1.0*(clickYPos-m_startY) / m_grid);
        //qDebug() << m_chessX <<m_chessY;
    }
    for(auto e:chesswoman)
        if(e.x()==m_chessX&&e.y()==m_chessY)
            judge=true;
    if(!judge)
    {
        vector<QPoint>temp;
        if(chesswoman.empty())//对手先行，第一次只下一子
        {
            chesswoman.push_back({m_chessX,m_chessY,game->type()});
            repaint();
            game->set({m_chessY,m_chessX});
            game->add({m_chessY,m_chessX}, temp);
            game->swap();
            game->ourplay();
            this->parent()->findChild<Options *>("options")->findChild<QPushButton *>("undo")->setEnabled(true);
        }
        else//除了第一次下->之后几次
        {
            chesswoman.push_back({m_chessX,m_chessY,game->type()});
            //更新窗口，间接调用paintEvent
            //update();
            repaint();//不使用update，会遭到阻塞不立即绘制棋子，repaint直接绘制
            //在game中添加对方的下棋点
            game->set({m_chessY,m_chessX});
            game->add({m_chessY,m_chessX}, temp);
            //如果结束了就显示结果
            if(game->isEnd())
                emit win();
            else//没结束
            {
                //bool isover = false;
                if(game->step()==2){
                    game->swap();
                    //如果对方下完比赛没结束就我方下
                    //isover = true;
                    game->ourplay();
                }
            }
            /*if (isover == true) {
                game->ourplay();
            isover = false;}*/
            //撤回键可以用了
            this->parent()->findChild<Options *>("options")->findChild<QPushButton *>("undo")->setEnabled(true);
        }
    }
}
std::vector<Chesswoman> &Chequer::getChesswoman()
{
    return chesswoman;
}

void Chequer::onChessChange(QPoint location,int type)
{

    chesswoman.push_back({location.y(),location.x(),type});
    repaint();
}

