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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCoreApplication>
#include <QStandardPaths>
#include <QStringList>
#include <QDataStream>
#include <QByteArray>
#include <QDateTime>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <QPoint>
#include <QList>
#include <QHash>
#include <QMap>
#include <QDir>

#define SETTINGS_CONTAINS Settings::instance()->contains

#define SETTINGS_GET_BOOL Settings::instance()->value<bool>
#define SETTINGS_SET_BOOL Settings::instance()->setValue<bool>

#define SETTINGS_GET_INT Settings::instance()->value<int>
#define SETTINGS_SET_INT Settings::instance()->setValue<int>

#define SETTINGS_GET_UINT Settings::instance()->value<uint>
#define SETTINGS_SET_UINT Settings::instance()->setValue<uint>

#define SETTINGS_GET_LONG Settings::instance()->value<long>
#define SETTINGS_SET_LONG Settings::instance()->setValue<long>

#define SETTINGS_GET_ULONG Settings::instance()->value<unsigned long>
#define SETTINGS_SET_ULONG Settings::instance()->setValue<unsigned long>

#define SETTINGS_GET_LONG_LONG Settings::instance()->value<qlonglong>
#define SETTINGS_SET_LONG_LONG Settings::instance()->setValue<qlonglong>

#define SETTINGS_GET_ULONG_LONG Settings::instance()->value<qulonglong>
#define SETTINGS_SET_ULONG_LONG Settings::instance()->setValue<qulonglong>

#define SETTINGS_GET_FLOAT Settings::instance()->value<float>
#define SETTINGS_SET_FLOAT Settings::instance()->setValue<float>

#define SETTINGS_GET_DOUBLE Settings::instance()->value<double>
#define SETTINGS_SET_DOUBLE Settings::instance()->setValue<double>

#define SETTINGS_GET_DATE Settings::instance()->value<QDate>
#define SETTINGS_SET_DATE Settings::instance()->setValue<QDate>

#define SETTINGS_GET_TIME Settings::instance()->value<QTime>
#define SETTINGS_SET_TIME Settings::instance()->setValue<QTime>

#define SETTINGS_GET_DATE_TIME Settings::instance()->value<QDateTime>
#define SETTINGS_SET_DATE_TIME Settings::instance()->setValue<QDateTime>

#define SETTINGS_GET_BYTE_ARRAY Settings::instance()->value<QByteArray>
#define SETTINGS_SET_BYTE_ARRAY Settings::instance()->setValue<QByteArray>

#define SETTINGS_GET_BIT_ARRAY Settings::instance()->value<QBitArray>
#define SETTINGS_SET_BIT_ARRAY Settings::instance()->setValue<QBitArray>

#define SETTINGS_GET_CHAR Settings::instance()->value<QChar>
#define SETTINGS_SET_CHAR Settings::instance()->setValue<QChar>

#define SETTINGS_GET_STRING Settings::instance()->value<QString>
#define SETTINGS_SET_STRING Settings::instance()->setValue<QString>

#define SETTINGS_GET_STRING_LIST Settings::instance()->value<QStringList>
#define SETTINGS_SET_STRING_LIST Settings::instance()->setValue<QStringList>

#define SETTINGS_GET_COLOR Settings::instance()->value<QColor>
#define SETTINGS_SET_COLOR Settings::instance()->setValue<QColor>

#define SETTINGS_GET_SIZE Settings::instance()->value<QSize>
#define SETTINGS_SET_SIZE Settings::instance()->setValue<QSize>

#define SETTINGS_GET_SIZEF Settings::instance()->value<QSizeF>
#define SETTINGS_SET_SIZEF Settings::instance()->setValue<QSizeF>

#define SETTINGS_GET_POINT Settings::instance()->value<QPoint>
#define SETTINGS_SET_POINT Settings::instance()->setValue<QPoint>

#define SETTINGS_GET_POINTF Settings::instance()->value<QPointF>
#define SETTINGS_SET_POINTF Settings::instance()->setValue<QPointF>

#define SETTINGS_GET_POINTS Settings::instance()->value<QList<QPoint>>
#define SETTINGS_SET_POINTS Settings::instance()->setValue<QList<QPoint>>

#define SETTINGS_GET_RECT Settings::instance()->value<QRect>
#define SETTINGS_SET_RECT Settings::instance()->setValue<QRect>

#define SETTINGS_GET_RECTF Settings::instance()->value<QRectF>
#define SETTINGS_SET_RECTF Settings::instance()->setValue<QRectF>

#define SETTINGS_GET_LINE Settings::instance()->value<QLine>
#define SETTINGS_SET_LINE Settings::instance()->setValue<QLine>

#define SETTINGS_GET_LINEF Settings::instance()->value<QLineF>
#define SETTINGS_SET_LINEF Settings::instance()->setValue<QLineF>

#define SETTINGS_GET_URL Settings::instance()->value<QUrl>
#define SETTINGS_SET_URL Settings::instance()->setValue<QUrl>

#define SETTINGS_REMOVE Settings::instance()->remove

#define SETTING_TRANSLATION         "translation"
#define SETTING_LAST_FILE_DIRECTORY "last-file-directory"
#define SETTING_SIZE                "size"
#define SETTING_POSITION            "position"
#define SETTING_SELECT_CHAPTER_SIZE "select-chapter-size"
#define SETTING_FOOLSDAY_SEEN       "foolsday-seen"
#define SETTING_LAST_CHAPTER        "last-chapter"
#define SETTING_LAST_LINK_POINTS    "last-link-points"
#define SETTING_XTEST_RETURN_DELAY  "xtest-return-delay"
#define SETTING_ON_TOP              "on-top"

class SettingsPrivate;

/*
 *  Class to query the application settings. You can use
 *  the macroses above to query the appropriate values
 *  like that:
 *
 *      qDebug() << SETTINGS_GET_STRING(SETTING_TRANSLATION);
 */
class Settings
{
public:
    static Settings* instance();

    ~Settings();

    static inline QString appDataLocation();

    enum SyncType { NoSync, Sync };

    /*
     *  Get the key value. If the value is not found, then
     *  the appropriate value from the known default values
     *  is returned. If the default value for the key is unknown,
     *  the C++ default value is returned ('0' for 'int', 'false' for 'bool' etc.)
     */
    template <typename T>
    T value(const QString &key);

    /*
     *  Set the value of the key. If 'sync' is 'Sync', then call sync()
     */
    template <typename T>
    void setValue(const QString &key, const T &value, SyncType sync = Sync);

    /*
     *  Returns 'true' if the setting 'key' exists. If 'key' starts with '/'
     *  the setting is returned from "settings" group (almost all settings are stored there).
     *
     *  For example:
     *
     *  contains("list")  - will look for the "list" key in the global section
     *  contains("/list") - will look for the "list" key in the "settings" section
     */
    bool contains(const QString &key) const;

    /*
     *  Remove the specified key from the section "settings"
     */
    void remove(const QString &key, SyncType sync = Sync);

    /*
     *  Sync with the storage
     */
    void sync();

    /*
     *  Available translations, hardcoded
     */
    QMap<QString, QString> translations();

private:
    Settings();

    void fillTranslations();

    QSettings *settings();

private:
    SettingsPrivate *d;
};

/**********************************/

inline
QString Settings::appDataLocation()
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + QDir::separator()
            + QCoreApplication::applicationName();
}

template <typename T>
T Settings::value(const QString &key)
{
    QSettings *s = settings();

    s->beginGroup("settings");
    QVariant value = s->value(key);
    s->endGroup();

    return value.value<T>();
}

template <typename T>
void Settings::setValue(const QString &key, const T &value, Settings::SyncType sync)
{
    QSettings *s = settings();

    s->beginGroup("settings");
    s->setValue(key, QVariant::fromValue(value));
    s->endGroup();

    if(sync == Sync)
        s->sync();
}

#endif // SETTINGS_H
