QT = core gui widgets
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = history_analysis
INCLUDEPATH += ..
SOURCES += history_analysis.cpp ../Game.cpp ../Move.cpp
HEADERS += ../Game.h ../Move.h referee.h dialog_closer.h
