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
#include <QMetaObject>
#include <QCursor>
#include <QPoint>
#include <QLabel>
#include <QTimer>

#include "numericlabel.h"
#include "target.h"

static const int PHRASEBOOKS_MAX_BLINKS = 2;

Target::Target(QWidget *parent)
    : QWidget(parent)
    , m_dragging(false)
    , m_blinkStep(0)
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

    m_label->installEventFilter(this);

    m_timerRevert = new QTimer(this);
    m_timerRevert->setInterval(200);
    connect(m_timerRevert, SIGNAL(timeout()), this, SLOT(slotRevertIcon()));
}

void Target::setNumberOfLinks(uint n)
{
    m_timerRevert->stop();
    m_number->setValue(n, NumericLabel::DontForce);
}

void Target::locked(bool l)
{
    m_number->setEnabled(!l);
}

void Target::blink()
{
    qDebug("Blinking");

    m_blinkStep = 0;
    m_timerRevert->start();

    QMetaObject::invokeMethod(m_timerRevert, "timeout");
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

void Target::slotRevertIcon()
{
    qDebug("Blink step %d", m_blinkStep);

    if(m_blinkStep == PHRASEBOOKS_MAX_BLINKS*2)
    {
        qDebug("Stopping blinking");
        m_timerRevert->stop();
        return;
    }

    if((m_blinkStep) % 2)
        m_number->setValue(m_number->value(), NumericLabel::Force);
    else
        m_number->setPixmap(QPixmap());

    m_blinkStep++;
}
