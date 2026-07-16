#ifndef INFORDIALOG_H
#define INFORDIALOG_H

#include <QDialog>

namespace Ui {
class InforDialog;
}

class InforDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InforDialog(QWidget *parent = nullptr);
    ~InforDialog();
    void accept();
    const QString &getFilepath() const;

signals:
    void savechess();
private:
    Ui::InforDialog *ui;
    QString filepath;
};

#endif // INFORDIALOG_H
