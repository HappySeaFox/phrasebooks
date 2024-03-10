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

#include <QApplication>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QCursor>
#include <QPoint>
#include <QLabel>

#include "numericlabel.h"
#include "target.h"
#include "utils.h"

Target::Target(QWidget *parent)
    : QWidget(parent)
    , m_dragging(false)
{
    QHBoxLayout *l = new QHBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    setLayout(l);

    m_label = new QLabel(this);
    m_label->setFixedSize(32, 32);
    m_label->setToolTip(tr("Drag and drop this target to the window you need to create a link to"));
    m_label->setPixmap(QPixmap(":/images/drag.png"));

    m_number = new NumericLabel(this);

    l->addWidget(m_label);
    l->addStretch(1);

    setFixedWidth(m_label->width() + 14);

    // move the number to the bottom
    m_number->move(width() - m_number->width(), height() - m_number->height());

    setMouseTracking(true);

    // TODO
    setWhatsThis(QString("<a href=\"http://www.youtube.com/playlist?list=PL5FURm9nDau8oTXumieXJl3DNDRTUlBSm\">%1</a>")
                 .arg(Utils::openYoutubeTutorialTitle()));

    m_label->installEventFilter(this);
}

void Target::setNumberOfLinks(uint n)
{
    m_number->setValue(n);

    if(!n)
        setNumberToolTip(QString());
}

void Target::setNumberToolTip(const QString &tip)
{
    m_number->setToolTip(tip);
}

void Target::locked(bool l)
{
    m_number->setEnabled(!l);
}

void Target::mousePressEvent(QMouseEvent *event)
{
    if(!m_label->rect().contains(event->pos()))
        return;

    if(event->button() == Qt::LeftButton)
    {
        qDebug("Start dragging");
        QPixmap pixmap(m_label->size());
        pixmap.fill(Qt::transparent);
        m_label->render(&pixmap, QPoint(), QRegion(m_label->rect()), QWidget::DrawChildren);
        QApplication::setOverrideCursor(QCursor(pixmap));
        m_dragging = true;
    }
    else if(event->button() == Qt::MiddleButton)
        emit middleClick();
}

void Target::mouseMoveEvent(QMouseEvent *event)
{
    if(m_dragging)
    {
        if(event->buttons() == Qt::NoButton)
        {
            QApplication::restoreOverrideCursor();
            m_dragging = false;
            emit cancelled();
        }
        else
            emit moving(event->globalPos());
    }
}

void Target::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton || !m_dragging)
        return;

    m_dragging = false;

    QApplication::restoreOverrideCursor();

    QPoint p = QCursor::pos();

    qDebug("Dropped at %d,%d", p.x(), p.y());

    emit dropped(p);
}

void Target::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)

    m_number->move(width() - m_number->width(), height() - m_number->height());
}
