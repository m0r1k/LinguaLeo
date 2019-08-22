// vim: ts=4:expandtab

#include <stdint.h>

#include <QApplication>
#include <QMessageBox>
#include <QLabel>
#include <QFile>
#include <QSharedMemory>
#include <QDebug>

#include "version.h"
#include "mainWidget.hpp"

int32_t main(
    int32_t argc,
    char    *argv[])
{
    int32_t err = -1;

    Q_INIT_RESOURCE(resources);

    QApplication    app(argc, argv);
    MainWidget      widget;

	{
	    // Load an application style
	    QFile styleFile(":/style.qss");
	    styleFile.open(QFile::ReadOnly);

	    // Apply the loaded stylesheet
	    QString style(styleFile.readAll());
	    app.setStyleSheet(style);
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()){
        QMessageBox::critical(
            0,
            QObject::tr("Systray"),
            QObject::tr("I couldn't detect any system tray"
                " on this system."
            )
        );
        goto fail;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    app.exec();

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

