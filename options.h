#ifndef OPTIONS_H
#define OPTIONS_H

#include <QWidget>
class Chesswoman;
namespace Ui {
class Options;
}

class Options : public QWidget
{
    Q_OBJECT

public:
    explicit Options(QWidget *parent = nullptr);
    ~Options(); 
signals:
    void type(int type);
    void undo_signal();
    void open_type(int type);
    void start();
public slots:

    void on_black_clicked();

    void on_white_clicked();

    void on_start_clicked();

    void on_undo_clicked();
    void undoenable();
    void on_open();
    void newBuilt();//Game新建后信号和槽要重新连接
private:
    Ui::Options *ui;
    bool openjudge;
};

#endif // OPTIONS_H
