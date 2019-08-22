# vim: ts=4:expandtab

TARGET      = ./lingualeo

HEADERS     = mainWidget.hpp        \
                leo.hpp             \
                main.hpp            \
                trayWidget.hpp      \
                filedownloader.hpp  \
                tableWidget.hpp     \
                translateItem.hpp

SOURCES     = main.cpp              \
                mainWidget.cpp      \
                leo.cpp             \
                trayWidget.cpp      \
                filedownloader.cpp  \
                tableWidget.cpp     \
                translateItem.cpp

RESOURCES   = resources.qrc
FORMS       = trayWidget.ui

QT          +=  core                \
                widgets             \
                network             \
                multimedia          \
                multimediawidgets

QMAKE_CXXFLAGS += -g3 -O3 -Wall -Werror -Wfatal-errors -Wextra

