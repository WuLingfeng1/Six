QT = core gui widgets testlib
CONFIG += testcase console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = offline_tests
MOC_DIR = $$OUT_PWD
INCLUDEPATH += ..
SOURCES += offline_tests.cpp ../Game.cpp ../Move.cpp ../Chesswoman.cpp \
           ../Chequer.cpp ../mainwindow.cpp ../options.cpp ../timer.cpp ../infordialog.cpp
HEADERS += referee.h dialog_closer.h ../Game.h ../Move.h ../Chesswoman.h \
           ../Chequer.h ../mainwindow.h ../options.h ../timer.h ../infordialog.h \
           offline_tests.cpp
FORMS += ../mainwindow.ui ../options.ui ../timer.ui ../infordialog.ui

# Keep the offline target resource-complete, including the production board art.
RESOURCES += ../six.qrc
