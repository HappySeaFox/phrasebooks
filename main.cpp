/*
 * This file is part of phrasebooks.
 *
 * phrasebooks is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * phrasebooks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with phrasebooks.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QStandardPaths>
#include <QLibraryInfo>
#include <QTranslator>
#include <QByteArray>
#include <QSslSocket>
#include <QDateTime>
#include <QLocale>
#include <QIcon>
#include <QDir>

#include <cstdlib>
#include <cstdio>

#include "qtsingleapplication.h"

#ifndef PHRASEBOOKS_NO_LOG
#include "qtlockedfile.h"
#endif

#include "phrasebooks.h"
#include "settings.h"
#include "utils.h"

static char messageTypeToChar(QtMsgType type)
{
    switch(type)
    {
        case QtDebugMsg:    return 'D';
        case QtWarningMsg:  return 'W';
        case QtCriticalMsg: return 'C';
        case QtFatalMsg:    return 'F';
        case QtInfoMsg:     return 'I';

        default:
            return 'U';
    }
}

static void phrasebooksOutput(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    Q_UNUSED(context)

    QByteArray msg = message.toLatin1();

    const QString currentTime = Utils::currentTime();

    const char messageChar = messageTypeToChar(type);

    fprintf(stderr, "%s %c %s\n", qPrintable(currentTime), messageChar, static_cast<const char *>(msg));

#ifndef PHRASEBOOKS_NO_LOG
    static bool noLog = qgetenv("PHRASEBOOKS_NO_LOG") == "1";

    if(noLog)
        return;

    static QtLockedFile log(
                     QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                     + QDir::separator()
                     + QCoreApplication::applicationName().toLower()
                     + ".log");

    static bool failed = false;

    if(!log.isOpen() && !failed)
    {
        failed = (!log.open(QIODevice::ReadWrite | QIODevice::Append)
                  || log.isLocked()
                  || !log.lock(QtLockedFile::WriteLock, false)
                  || !log.resize(0)
                  );

        if(failed)
            fprintf(stderr, "Web2video: Log file is unavailable\n");
    }

    if(!failed)
    {
        // truncate
        if(log.size() > 1*1024*1024) // 1 Mb
        {
            fprintf(stderr, "Web2video: Truncating log\n");

            log.seek(0);

            char buf[1024];
            int lines = 0;
            char c = 'x';

            while(log.readLine(buf, sizeof(buf)) > 0 && lines++ < 30)
            {}

            log.resize(log.pos());

            // check if '\n' is last
            if(log.seek(log.pos()-1))
            {
                log.getChar(&c);

                if(c != '\n')
                    log.write("\n");
            }

            log.write("...\n<overwrite>\n...\n");
        }

        log.write(currentTime.toLatin1());
        log.write(" ");
        log.write(&messageChar, sizeof(decltype(messageChar)));
        log.write(" ");
        log.write(static_cast<const char *>(msg));
        log.write("\n");
    }
#endif // PHRASEBOOKS_NO_LOG

    if(type == QtFatalMsg)
        abort();
}

/******************************************/

int main(int argc, char *argv[])
{
    setbuf(stderr, 0);

    const qint64 v = QDateTime::currentMSecsSinceEpoch();

    QCoreApplication::setApplicationName("Phrasebooks");
    QCoreApplication::setOrganizationName("Phrasebooks");
    QCoreApplication::setApplicationVersion(NVER_STRING);

    qInstallMessageHandler(phrasebooksOutput);

    qDebug("Starting at %s", qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));

    QtSingleApplication app(argc, argv);

    if(app.sendMessage("activate-window"))
        return 0;

#ifdef Q_OS_UNIX
    QApplication::setWindowIcon(QIcon(":/images/phrasebooks.ico"));
#endif

    // workaround to speed up the SSL initialization
    const bool haveSsl = QSslSocket::supportsSsl();

    if(haveSsl)
        qDebug("SSL engine: %s", qPrintable(QSslSocket::sslLibraryVersionString()));
    else
        qDebug("SSL is not supported");

    // load translations
    const QString locale = QLocale::system().name();

    const QString translationsDir =
#ifdef Q_OS_WIN32
            QCoreApplication::applicationDirPath() + QDir::separator() + "translations";
#else
            "/usr/share/phrasebooks/translations";
#endif

    const QString qtTranslationsDir =
#ifdef Q_OS_WIN32
            translationsDir;
#else
            QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    QString ts = SETTINGS_GET_STRING(SETTING_TRANSLATION);

    qDebug("Locale \"%s\", translation \"%s\"", qPrintable(locale), qPrintable(ts));

    ts = ts.isEmpty() ? locale : (ts + ".qm");

    QTranslator translator_qt;
    qDebug("Loading Qt translation: %s", translator_qt.load("qt_" + ts, qtTranslationsDir) ? "ok" : "failed");

    QTranslator translator_qtbase;
    qDebug("Loading Qt Base translation: %s", translator_qtbase.load("qtbase_" + ts, qtTranslationsDir) ? "ok" : "failed");

    QTranslator translator;
    qDebug("Loading translation: %s", translator.load(ts, translationsDir) ? "ok" : "failed");

    app.installTranslator(&translator_qt);
    app.installTranslator(&translator_qtbase);
    app.installTranslator(&translator);

    // main window
    Phrasebooks w;
    w.show();

    qDebug("Initialized in %ld ms.", static_cast<long int>(QDateTime::currentMSecsSinceEpoch() - v));

    int code = app.exec();

    qDebug("Goodbye at %s (exit code %d)", qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")), code);

    return code;
}
