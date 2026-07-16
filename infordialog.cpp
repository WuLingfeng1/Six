#include "infordialog.h"
#include "ui_infordialog.h"
#include <QFile>
#include <QDateTime>
#include <QMessageBox>

InforDialog::InforDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InforDialog)
{
    ui->setupUi(this);
}

InforDialog::~InforDialog()
{
    delete ui;
}

void InforDialog::accept()
{
    //获取对话框中选择的信息
    QString former=ui->former->text();
    QString latter=ui->latter->text();
    QString result=ui->result->currentText();
    if(former.isEmpty()||latter.isEmpty()||result.isEmpty())
    {
        QMessageBox::warning(this,"warning","保存失败");
        return;
    }
    QString place=ui->place->text();
    QString time=QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss");
    QString filename="C6-"+former+"Bvs"+latter+"W-"+result+"-"+place+".txt";
    filepath = "./棋谱文件/" + filename;
    QFile *file=new QFile(filepath);
    bool ret = file->open(QIODevice::WriteOnly|QIODevice::Truncate);    //创建文件
    if(!ret)
    {
        QMessageBox::warning(this,"warning","打开失败");
        return;
    }
    QString data = "{[C6][" + former + " B][" + latter + " W][" + result + "][" + time + " " + place
                   + "][2024 CCGC]";
    file->write(data.toUtf8());
    file->close();
    QDialog::accept();
}
//传递棋谱文件的路径信息
const QString &InforDialog::getFilepath() const
{
    return filepath;
}
