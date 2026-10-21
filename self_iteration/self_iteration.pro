QT += core gui widgets testlib
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = self_iteration
isEmpty(GAME_DIR): GAME_DIR = $$PWD/..
GAME_SOURCE_DIR = $$clean_path($$absolute_path($$GAME_DIR, $$PWD))
# Candidate headers must precede production fallback headers (Chesswoman.h).
INCLUDEPATH += $$GAME_SOURCE_DIR $$PWD/..
SOURCES += self_iteration.cpp $$GAME_SOURCE_DIR/Game.cpp $$GAME_SOURCE_DIR/Move.cpp
HEADERS += $$GAME_SOURCE_DIR/Game.h $$GAME_SOURCE_DIR/Move.h $$PWD/openings.h

# Regenerate when an engine input or runner changes, including make-only builds.
MANIFEST_INPUTS = $$GAME_SOURCE_DIR/Game.cpp $$GAME_SOURCE_DIR/Game.h \
                  $$GAME_SOURCE_DIR/Move.cpp $$GAME_SOURCE_DIR/Move.h \
                  $$PWD/self_iteration.cpp $$PWD/openings.h $$PWD/build_manifest.py $$PWD/config_tools.py
MANIFEST_COMMAND = python3 $$shell_quote($$PWD/build_manifest.py) $$shell_quote($$GAME_SOURCE_DIR) $$shell_quote($$OUT_PWD/build_manifest.h)
!system($$MANIFEST_COMMAND): error(Cannot generate worker source manifest)
manifest.input = MANIFEST_INPUTS
manifest.output = $$OUT_PWD/build_manifest.h
manifest.commands = $$MANIFEST_COMMAND
manifest.CONFIG += combine no_link target_predeps
QMAKE_EXTRA_COMPILERS += manifest
HEADERS += $$OUT_PWD/build_manifest.h
INCLUDEPATH += $$OUT_PWD
