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

#include <QCoreApplication>
#include <QStandardPaths>
#include <QTextStream>
#include <QMetaType>
#include <QFile>
#include <QDir>

#include "settings.h"

class SettingsPrivate
{
public:
    SettingsPrivate()
        : translationsFilled(false)
    {}

    QSettings *settings;
    QMap<QString, QString> translations;
    bool translationsFilled;
};

/*******************************************************/

Settings::Settings()
{
    d = new SettingsPrivate;

    qRegisterMetaTypeStreamOperators<QList<QPoint>>("QList<QPoint>");

    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
#ifdef Q_OS_UNIX
            + QDir::separator()
            + QCoreApplication::applicationName().toLower()
#endif
            ;

    qDebug("Configuration directory: %s", qPrintable(configPath));

    QDir().mkpath(configPath);

    d->settings = new QSettings(configPath
                                + QDir::separator()
                                + QCoreApplication::applicationName().toLower()
                                + ".ini",
                                QSettings::IniFormat);

    d->settings->setFallbacksEnabled(false);

    // save version for future changes
    d->settings->setValue("version", VERSION_STRING);
}

Settings::~Settings()
{
    delete d->settings;
    delete d;
}

void Settings::sync()
{
    d->settings->sync();
}

void Settings::setCheckBoxState(const QString &checkbox, bool checked, SyncType sync)
{
    d->settings->setValue("settings/checkbox-" + checkbox, (int)checked);

    if(sync == Sync)
        d->settings->sync();
}

int Settings::checkBoxState(const QString &checkbox)
{
    return d->settings->value("settings/checkbox-" + checkbox, -1).toInt();
}

void Settings::setTickersForGroup(int group, const QStringList &tickers, SyncType sync)
{
    d->settings->beginGroup(QString("tickers-%1").arg(group));
    d->settings->setValue("tickers", tickers);
    d->settings->endGroup();

    if(sync == Sync)
        d->settings->sync();
}

QStringList Settings::tickersForGroup(int group)
{
    d->settings->beginGroup(QString("tickers-%1").arg(group));
    QStringList tickers = d->settings->value("tickers").toStringList();
    d->settings->endGroup();

    return tickers;
}

void Settings::removeTickers(int group, SyncType sync)
{
    d->settings->beginGroup(QString("tickers-%1").arg(group));
    d->settings->remove(QString());
    d->settings->endGroup();

    if(sync == Sync)
        d->settings->sync();
}

Settings* Settings::instance()
{
    static Settings *inst = new Settings;

    return inst;
}
#include <QMessageBox>
void Settings::fillTranslations()
{
    QFile translations(QCoreApplication::applicationDirPath()
                       + QDir::separator()
                       + "translations"
                       + QDir::separator()
                       + "translations.conf");

    d->translationsFilled = true;

    if(!translations.open(QFile::ReadOnly))
    {
        qWarning("Cannot open translations' configuration: %s", qPrintable(translations.errorString()));
        return;
    }

    QTextStream in(&translations);

    in.setCodec("UTF-8");

    while(!in.atEnd())
    {
        QString line = in.readLine().trimmed();

        if(line.startsWith("#"))
            continue;

        int index = line.indexOf(QChar('='));

        if(index < 0)
            continue;

        QString code = line.mid(index+1);

        if(code.isEmpty())
            continue;

        d->translations.insert(code, line.left(index));
    }
}

QSettings *Settings::settings()
{
    return d->settings;
}

bool Settings::contains(const QString &key) const
{
    return d->settings->contains(key.contains('/') ? key : ("settings/" + key));
}

void Settings::remove(const QString &key, Settings::SyncType sync)
{
    QSettings *s = settings();

    s->beginGroup("settings");
    s->remove(key);
    s->endGroup();

    if(sync == Sync)
        s->sync();
}

QMap<QString, QString> Settings::translations()
{
    if(!d->translationsFilled)
        fillTranslations();

    return d->translations;
}
