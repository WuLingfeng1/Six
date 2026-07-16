#include "options.h"
#include "ui_options.h"
#include "Chequer.h"
#include "Chesswoman.h"
#include"Game.h"

Options::Options(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Options),
    openjudge(false)
{
    ui->setupUi(this);
    findChild<QPushButton *>("undo")->setEnabled(false);
    QObject::connect(parent->findChild<Chequer*>("chequer"),&Chequer::open,
                     this,&Options::on_open);
    QObject::connect(this,&Options::type,
                     parent->findChild<Chequer*>("chequer")->getGame(),&Game::whoFirst);
    QObject::connect(this,&Options::open_type,
                     parent->findChild<Chequer*>("chequer")->getGame(),&Game::whoPlay);
    QObject::connect(parent->findChild<Chequer*>("chequer")->getGame(),&Game::undoenable,
                     this,&Options::undoenable);
    QObject::connect(this,&Options::undo_signal,parent->findChild<Chequer*>("chequer"),&Chequer::undo);
    QObject::connect(parent->findChild<Chequer*>("chequer")->getGame(),&Game::restart,
                     this->findChild<Timer*>("timer"),&Timer::on_restart);
    QObject::connect(parent->findChild<Chequer*>("chequer")->getGame(),&Game::stop,
                     this->findChild<Timer*>("timer"),&Timer::on_stop);
    QObject::connect(this,&Options::start,
                     this->findChild<Timer*>("timer"),&Timer::on_start);
}

Options::~Options()
{
    delete ui;
}

void Options::on_start_clicked()
{
    Chequer*chequer=this->parent()->findChild<Chequer *>("chequer");
    if(openjudge)//如果是打开的棋谱下残局
    {
        if(chequer->getChesswoman().empty())
        {
            this->parent()->findChild<Chequer *>("chequer")->setAttribute(Qt::WA_TransparentForMouseEvents,false);
            if(this->findChild<QRadioButton*>("black")->isChecked())
            {
                emit start();
                emit type(1);
            }
            else  if(this->findChild<QRadioButton*>("white")->isChecked())
            {
                emit type(2);
            }
            ui->start->setEnabled(false);
        }
        else
        {
            this->parent()->findChild<Chequer *>("chequer")->setAttribute(Qt::WA_TransparentForMouseEvents,false);
            if(this->findChild<QRadioButton*>("black")->isChecked())
            {
                emit start();
                emit open_type(1);
            }
            else  if(this->findChild<QRadioButton*>("white")->isChecked())
            {
                emit open_type(2);
            }
            ui->start->setEnabled(false);
        }
    }
    else
    {
        this->parent()->findChild<Chequer *>("chequer")->setAttribute(Qt::WA_TransparentForMouseEvents,false);
        if(this->findChild<QRadioButton*>("black")->isChecked())
        {
            emit start();
            emit type(1);
        }
        else  if(this->findChild<QRadioButton*>("white")->isChecked())
        {
            emit type(2);
        }
        ui->start->setEnabled(false);
    }

}


void Options::on_black_clicked()
{
    ui->start->setEnabled(true);
}


void Options::on_white_clicked()
{
    ui->start->setEnabled(true);
}


void Options::on_undo_clicked()
{
    bool judge=this->parent()->findChild<Chequer*>("chequer")->getChesswoman().size()==1;
    if(judge)
    {
        this->parent()->findChild<Options *>("options")->findChild<QPushButton *>("undo")->setEnabled(false);
        this->parent()->findChild<Chequer *>("chequer")->setAttribute(Qt::WA_TransparentForMouseEvents,true);
        ui->start->setEnabled(true);
    }
    emit undo_signal();
}

void Options::undoenable()
{
    ui->undo->setEnabled(true);
}

void Options::on_open()
{
    openjudge=true;
}

void Options::newBuilt()
{
    QObject::connect(this,&Options::type,
                     parent()->findChild<Chequer*>("chequer")->getGame(),&Game::whoFirst);
    QObject::connect(this,&Options::open_type,
                     parent()->findChild<Chequer*>("chequer")->getGame(),&Game::whoPlay);
    QObject::connect(parent()->findChild<Chequer*>("chequer")->getGame(),&Game::undoenable,
                     this,&Options::undoenable);;
    QObject::connect(parent()->findChild<Chequer*>("chequer")->getGame(),&Game::restart,
                     this->findChild<Timer*>("timer"),&Timer::on_restart);
    QObject::connect(parent()->findChild<Chequer*>("chequer")->getGame(),&Game::stop,
                     this->findChild<Timer*>("timer"),&Timer::on_stop);
}

