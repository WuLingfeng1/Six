#pragma once
#include <QApplication>
#include <QMessageBox>
#include <QTimer>

inline void closeResultDialogs(QTimer &timer) {
    QObject::connect(&timer, &QTimer::timeout, [] {
        for (QWidget *widget : QApplication::topLevelWidgets())
            if (auto *box = qobject_cast<QMessageBox *>(widget))
                box->done(QMessageBox::Discard);
    });
    timer.start(10);
}
