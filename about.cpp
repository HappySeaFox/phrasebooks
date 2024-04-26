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

#include <QDesktopServices>
#include <QKeySequence>
#include <QMouseEvent>
#include <QShortcut>
#include <QTimer>
#include <QMovie>
#include <QIcon>
#include <QUrl>

#include "updatechecker.h"
#include "settings.h"
#include "utils.h"
#include "about.h"

#include "ui_about.h"

About::About(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::About)
    , m_wasMousePress(false)
    , m_newVersionAvailable(false)
{
    setWindowTitle(Utils::aboutPhrasebooks());

    ui->setupUi(this);

    ui->pushOk->setText(Utils::oKTitle());

    ui->label->setPixmap(QIcon(":/images/phrasebooks.ico").pixmap(48, 48));
    ui->labelVersion->setText(QString("Phrasebooks %1").arg(NVER_STRING));

    ui->labelCopyright->setText(QString("(C) 2016 %1 &lt;<a href=\"mailto:linuxsquirrel.dev@gmail.com\">"
                                        "linuxsquirrel.dev@gmail.com</a>&gt;").arg(tr("Dmitry Baryshev")));

    ui->labelUrl->setText(QString("<a href=\"%1\">%2</a> <a href=\"%3\">%4</a> <a href=\"%5\">%6</a> <a href=\"%7\">%8</a>")
                            .arg(HTTPROOT "/wiki/howto")
                            .arg(tr("Documentation"))
                            .arg(DOWNLOADROOT "/files")
                            //: Noun in the plural
                            .arg(tr("Downloads"))
                            .arg("http://www.transifex.com/projects/p/phrasebooks")
                            .arg(tr("Translations"))
                            .arg(HTTPROOT "/youtube")
                            .arg(tr("Youtube"))
                            );

    // set "wait" icon
    m_timer = new QTimer(this);

    m_timer->setSingleShot(true);
    m_timer->setInterval(750);

    connect(m_timer, &QTimer::timeout, this, &About::slotSetWaitIcon);

    // update checker
    m_checker = new UpdateChecker(this);

    connect(m_checker, &UpdateChecker::newVersion, this, &About::slotNewVersion);
    connect(m_checker, &UpdateChecker::error,      this, &About::slotError);

    m_checker->start();
    m_timer->start();

    // catch mouse events
    ui->labelUpdate->installEventFilter(this);

    adjustSize();
}

About::~About()
{
    delete ui;
}

bool About::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);

        if(me->button() == Qt::LeftButton)
            m_wasMousePress = true;
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);

        if(me->button() == Qt::LeftButton && m_wasMousePress && m_newVersionAvailable)
        {
            m_wasMousePress = false;
            QDesktopServices::openUrl(QUrl(DOWNLOADROOT));
        }
    }

    return QObject::eventFilter(obj, event);
}

void About::slotNewVersion(const QString &newVersion)
{
    QString tooltip;

    m_timer->stop();

    if(!newVersion.isEmpty())
    {
        //: Means "A new version of Phrasebooks is availabe on the website". %1 will be replaced with the version available by the application
        tooltip = tr("Update available (%1)").arg(newVersion);

        ui->labelUpdate->setPixmap(QIcon::fromTheme("software-update-available", QIcon(":/images/update.png")).pixmap(16, 16));
        ui->labelUpdate->setCursor(Qt::PointingHandCursor);
        m_newVersionAvailable = true;
    }
    else
    {
        //: "Phrasebooks" is the name of the application. Phrasebooks will check for updates and show this message when there are no new version available
        tooltip = tr("Phrasebooks is up to date");
        ui->labelUpdate->setPixmap(QIcon(":/images/ok.png").pixmap(16, 16));
    }

    delete ui->labelUpdate->movie();

    ui->labelUpdate->setToolTip(tooltip);
}

void About::slotError(const QString &err)
{
    m_timer->stop();

    delete ui->labelUpdate->movie();

    ui->labelUpdate->setPixmap(QIcon::fromTheme("error",
                                                QIcon::fromTheme("dialog-error", QIcon(":/images/error.png"))).pixmap(16, 16));
    //: %1 will be replaced with the error code by the application. It will look like "Cannot check for updates (Server is unavailable)"
    ui->labelUpdate->setToolTip(tr("Cannot check for updates (%1)").arg(err));
}

void About::slotSetWaitIcon()
{
    QMovie *movie = new QMovie(":/images/wait.gif", "GIF", this);
    ui->labelUpdate->setMovie(movie);
    movie->start();

    ui->labelUpdate->setToolTip(tr("Checking for updates..."));
}
