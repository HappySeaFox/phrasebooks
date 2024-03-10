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
#include <QPointer>
#include <QDebug>
#include <QUrl>

#include "networkaccess.h"

class NetworkAccessPrivate
{
public:
    NetworkAccessPrivate()
        : error(QNetworkReply::NoError)
        , manager(nullptr)
        , reply(nullptr)
    {}

    QUrl url;
    QNetworkReply::NetworkError error;
    QNetworkAccessManager *manager;
    QPointer<QNetworkReply> reply;
    QByteArray data;
};

/*******************************************/

NetworkAccess::NetworkAccess(QObject *parent)
    : QObject(parent)
{
    d = new NetworkAccessPrivate;

    d->manager = new QNetworkAccessManager(this);
}

NetworkAccess::~NetworkAccess()
{
    abort();

    delete d;
}

void NetworkAccess::abort()
{
    if(d->reply)
    {
        d->reply->blockSignals(true);
        d->reply->abort();
        d->reply->deleteLater();
        d->reply = 0;
    }
}

QByteArray NetworkAccess::data() const
{
    return d->data;
}

void NetworkAccess::clearBuffer()
{
    d->data.clear();
}

QNetworkReply::NetworkError NetworkAccess::error() const
{
    return d->error;
}

bool NetworkAccess::get(const QUrl &url)
{
    abort();

    d->error = QNetworkReply::NoError;
    d->data.clear();

    qDebug("Starting a new network request to \"%s\"", qPrintable(url.toString(QUrl::RemovePassword)));

    QNetworkRequest request(url);

    request.setRawHeader("Dnt", "1");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT rv:45.0) Gecko/20100101 Firefox/45.0");

    d->reply = d->manager->get(request);

    if(!d->reply)
        return false;

    // cache data
    d->url = url;

    connect(d->reply, &QNetworkReply::downloadProgress, this, &NetworkAccess::downloadProgress);

    connect(d->reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &NetworkAccess::slotNetworkError);

    connect(d->reply, &QNetworkReply::sslErrors,        this, &NetworkAccess::slotSslErrors);
    connect(d->reply, &QNetworkReply::finished,         this, &NetworkAccess::slotNetworkDone);
    connect(d->reply, &QNetworkReply::readyRead,        this, &NetworkAccess::slotNetworkData);

    return true;
}

void NetworkAccess::slotNetworkError(QNetworkReply::NetworkError err)
{
    qWarning("Network error #%d", err);

    d->error = err;
}

void NetworkAccess::slotSslErrors(const QList<QSslError> &errors)
{
    const QList<QSslError> allowed = QList<QSslError>()
                                        << QSslError::SelfSignedCertificate
                                        << QSslError::SelfSignedCertificateInChain;
    foreach(const QSslError &e, errors)
    {
        if(allowed.indexOf(e.error()) < 0)
        {
            qWarning("SSL error \"%s\" is fatal", qPrintable(e.errorString()));
            return;
        }
    }

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    if(reply)
        reply->ignoreSslErrors();
}

void NetworkAccess::slotNetworkData()
{
    d->data += d->reply->readAll();
}

void NetworkAccess::slotNetworkDone()
{
    qDebug("Network request done");

    QUrl redirect;

    if(d->reply->error() == QNetworkReply::NoError)
    {
        QVariant v = d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if(v.isValid())
        {
            redirect = v.toUrl();

            if(!redirect.isEmpty() && redirect.isRelative())
                redirect = d->reply->request().url().resolved(redirect);
        }
    }

    d->reply->deleteLater();
    d->reply = 0;

    if(redirect.isEmpty())
        emit finished();
    else
    {
        qDebug("Redirecting");
        get(redirect.isRelative() ? d->url.resolved(redirect) : redirect);
    }
}
