﻿/*
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
#include <QMetaType>
#include <QDir>

#include "settings.h"

class SettingsPrivate
{
public:
    QSettings *settings;
    QMap<QString, QString> translations;
};

/*******************************************************/

Settings::Settings()
{
    d = new SettingsPrivate;

    qRegisterMetaTypeStreamOperators<QList<QPoint>>("QList<QPoint>");

    d->settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                    + QDir::separator()
                                    + QCoreApplication::applicationName()
                                    + ".ini",
                                QSettings::IniFormat);

    d->settings->setFallbacksEnabled(false);

    // save version for future changes
    d->settings->setValue("version", NVER_STRING);
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

void Settings::fillTranslations()
{
    d->translations.insert("en", "English");

    // http://www.loc.gov/standards/iso639-2/php/code_list.php
    d->translations.insert("ru", QString::fromUtf8("Русский"));
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
    if(d->translations.isEmpty())
        fillTranslations();

    return d->translations;
}
