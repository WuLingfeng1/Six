//一开始不能点击棋盘，要点开始才行，棋盘上没有棋子也不能点击撤回
//撤回为空就等于新建了一个棋盘，又不能点击了要点开始才行

//todo::打开已有的棋谱下残局
#include "mainwindow.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include "Chequer.h"
#include "Game.h"
#include "options.h"
#include "timer.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QObject::connect(this,&MainWindow::save_signal,findChild<Chequer*>("chequer"),&Chequer::save);
    QObject::connect(this,&MainWindow::undo_signal,findChild<Chequer*>("chequer"),&Chequer::undo);
    QObject::connect(this,&MainWindow::new_signal,findChild<Chequer*>("chequer"),&Chequer::newBuilt);
    QObject::connect(this,&MainWindow::new_signal,findChild<Options*>("options"),&Options::newBuilt);
    QObject::connect(this, &MainWindow::new_signal, findChild<Timer *>("timer"), &Timer::init);
    QObject::connect(this,&MainWindow::loadfile,findChild<Chequer*>("chequer"),&Chequer::loadfile);

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox msgBox;
    msgBox.setInformativeText("是否保存棋谱文件");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Save:
        emit save_signal();
        break;
    case QMessageBox::Discard:
        // Don't Save was clicked
        break;
    case QMessageBox::Cancel:
        // Cancel was clicked
        break;
    default:
        // should never be reached
        break;
    }
}
void MainWindow::resizeEvent(QResizeEvent *event)
{
    Chequer *chequer=this->findChild<Chequer*>("chequer");
    QWidget *centralwidget=this->findChild<QWidget*>("centralwidget");
    chequer->resize(centralwidget->height()-9,centralwidget->height()-9);
}

void MainWindow::on_undoAction_triggered()
{
    emit undo_signal();
}


void MainWindow::on_saveAction_triggered()
{
    emit save_signal();
}


void MainWindow::on_newAction_triggered()
{
    //清空棋盘，一切归零，撤回不可用棋盘不可点击，要点开始才行
    emit new_signal();
}


void MainWindow::on_exitAction_triggered()
{

}


void MainWindow::on_openAction_triggered()
{
    //和普通新建不一样（因为不会有第一步单下了）
    emit new_signal();
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开棋谱文件"),
                                                    "../sixsixsix/棋谱文件/",
                                                    tr("棋谱 files (*.txt)")
                                                    );
    if (!fileName.isEmpty())
        emit loadfile(fileName);
    //emit open();
}


void MainWindow::on_aboutAction_triggered()
{
    QMessageBox::about(this, tr("About 666"),
                       tr("<h2>六子棋2.0</h2>"
                          "<p>感谢我的倾情奉献."
                          "<p>sixsixsix is a small application that "
                          "demonstrates QAction, QMainWindow, QMenuBar, "
                          "QPainter, QFile, QDilag, and many other "
                          "Qt classes."));
}



