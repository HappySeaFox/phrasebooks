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

#ifndef NETWORKACCESS_H
#define NETWORKACCESS_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QSslError>
#include <QObject>
#include <QString>
#include <QList>

class QHttpMultiPart;
class QUrl;

class NetworkAccessPrivate;

/*
 *  Class to download/upload data and report the status
 */
class NetworkAccess : public QObject
{
    Q_OBJECT

public:
    explicit NetworkAccess(QObject *parent = 0);
    virtual ~NetworkAccess();

    /*
     *  Abort downloading. Doesn't emit any signal
     */
    void abort();

    /*
     *  The downloaded data. Call this after finished()
     */
    QByteArray data() const;

    /*
     *  Clear the internal buffer. data() will now return an empty array
     */
    void clearBuffer();

    /*
     *  Status of the network operation. Call this after finished()
     */
    QNetworkReply::NetworkError error() const;

    /*
     *  Start a new network GET request
     */
    bool get(const QUrl &url);

signals:
    /*
     *  The network operation is done. Call error() to check for errors
     */
    void finished();

    /*
     *  Same as QNetworkReply::downloadProgress()
     */
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

// slots
private:
    void slotNetworkError(QNetworkReply::NetworkError err);
    void slotSslErrors(const QList<QSslError> &errors);
    void slotNetworkData();
    void slotNetworkDone();

private:
    NetworkAccessPrivate *d;
};

#endif // NETWORKACCESS_H
