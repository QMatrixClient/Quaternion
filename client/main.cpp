/**************************************************************************
 *                                                                        *
 * Copyright (C) 2015 Felix Rohrbach <kde@fxrh.de>                        *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 3         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                        *
 **************************************************************************/

#include <QtWidgets/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QLibraryInfo>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>
#include <QtCore/QStandardPaths>

#include "networksettings.h"
#include "mainwindow.h"
#include "activitydetector.h"
#include <settings.h>

int main( int argc, char* argv[] )
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("QMatrixClient");
    QApplication::setApplicationName("quaternion");
    QApplication::setApplicationDisplayName("Quaternion");
    QApplication::setApplicationVersion("0.0.9.4-git");

    QMatrixClient::Settings::setLegacyNames("Quaternion", "quaternion");

    // We should not need to do the following, as quitOnLastWindowClosed is
    // set to "true" by default; might be a bug, see
    // https://forum.qt.io/topic/71112/application-does-not-quit
    QObject::connect(&app, &QApplication::lastWindowClosed, []{
        qDebug() << "Last window closed!";
        QApplication::postEvent(qApp, new QEvent(QEvent::Quit));
    });

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::translate("main",
            "Quaternion - an IM client for the Matrix protocol"));
    parser.addHelpOption();
    parser.addVersionOption();

    QList<QCommandLineOption> options;
    QCommandLineOption locale { "locale",
        QApplication::translate("main", "Override locale"),
        QApplication::translate("main", "locale") };
    options.append(locale);
    QCommandLineOption hideMainWindow { "hide-mainwindow",
        QApplication::translate("main", "Hide main window on startup") };
    options.append(hideMainWindow);
    QCommandLineOption debug { "debug",
        QApplication::translate("main", "Display debug information") };
    debug.setHidden(true); // FIXME, #415; also, setHidden is obsolete in Qt 5.11
    options.append(debug);
    // Add more command line options before this line

    if (!parser.addOptions(options))
        Q_ASSERT_X(false, __FUNCTION__,
                   "Command line options are improperly defined, fix the code");
    parser.process(app);

    const auto overrideLocale = parser.value(locale);
    if (!overrideLocale.isEmpty())
    {
        QLocale::setDefault(overrideLocale);
        qInfo() << "Using locale" << QLocale().name();
    }

    QTranslator qtTranslator;
    qtTranslator.load(QLocale(), "qt", "_",
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator appTranslator;
    if (!appTranslator.load(QLocale(), "quaternion", "_"))
        appTranslator.load(QLocale(), "quaternion", "_",
            QStandardPaths::locate(QStandardPaths::AppLocalDataLocation,
            "translations", QStandardPaths::LocateDirectory));
    app.installTranslator(&appTranslator);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    app.setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

    QMatrixClient::NetworkSettings().setupApplicationProxy();

    MainWindow window;
    if (parser.isSet(debug))
    {
        qInfo() << "Debug mode enabled";
        window.enableDebug();
    }

    ActivityDetector ad(app, window); Q_UNUSED(ad);
    if (!parser.isSet(hideMainWindow)) {
        qDebug() << "--- Show time!";
        window.show();
    }
    else {
      qDebug() << "--- Hide time!";
      window.hide();
    }

    return app.exec();
}

