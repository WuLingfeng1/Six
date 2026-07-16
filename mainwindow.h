#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class Chequer;
class Options;
class QWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);
signals:
    void save_signal();
    void undo_signal();
    void new_signal();
    void open();
    void loadfile(QString );
private slots:
    void on_undoAction_triggered();

    void on_saveAction_triggered();

    void on_newAction_triggered();

    void on_exitAction_triggered();

    void on_openAction_triggered();

    void on_aboutAction_triggered();


private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
