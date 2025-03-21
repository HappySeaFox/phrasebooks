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

#include <QNetworkRequest>
#include <QRegExp>
#include <QTimer>

#include "networkaccess.h"
#include "updatechecker.h"
#include "utils.h"

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_lastVersion(VERSION_STRING)
    , m_rxVersion("^(\\d+)\\.(\\d+)\\.(\\d+)$")
    , m_rxNewLine("\\r?\\n")
    , m_url(VCSROOT_FOR_DOWNLOAD "/" TARGET_STRING "-version.tag?format=raw")
{
    m_net = new NetworkAccess(this);

    connect(m_net, &NetworkAccess::finished, this, &UpdateChecker::slotFinished);
}

void UpdateChecker::start()
{
    m_net->get(m_url);
}

void UpdateChecker::slotFinished()
{
    if(m_net->error() != QNetworkReply::NoError)
    {
        emit error(Utils::networkErrorTitle().arg(m_net->error()));
        return;
    }

    QString version = m_net->data().trimmed();

    if(version.isEmpty() || !m_rxVersion.exactMatch(version))
    {
        qWarning("Update checker: answer is broken");
        emit error(tr("Server answer is broken"));
        return;
    }

    qDebug("Update checker: version on the server is %s", qPrintable(version));

    if(m_lastVersion != version)
    {
        m_lastVersion = version;

        bool okmajor, okminor, okpatch;

        int major = m_rxVersion.cap(1).toInt(&okmajor);
        int minor = m_rxVersion.cap(2).toInt(&okminor);
        int patch = m_rxVersion.cap(3).toInt(&okpatch);

        if(okmajor && okminor && okpatch)
        {
            if(major > VERSION_MAJOR
                    || (major == VERSION_MAJOR && minor > VERSION_MINOR)
                    || (major == VERSION_MAJOR && minor == VERSION_MINOR && patch > VERSION_PATCH)
                    )
            {
                qDebug("Update checker: new version is \"%s\"", qPrintable(m_lastVersion));
                emit newVersion(m_lastVersion);
            }
            else
            {
                qDebug("Update checker: current version is better than \"%s\"", qPrintable(m_lastVersion));
                emit newVersion(QString());
            }
        }
        else
        {
            qDebug("Update checker: new version is \"%s\"", qPrintable(m_lastVersion));
            emit newVersion(m_lastVersion);
        }
    }
    else
    {
        qDebug("Update checker: version is up-to-date");
        emit newVersion(QString());
    }
}
