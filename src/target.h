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

#ifndef TARGET_H
#define TARGET_H

#include <QWidget>
#include <QPixmap>

class QTimer;
class QLabel;

class NumericLabel;

class Target : public QWidget
{
    Q_OBJECT

public:
    explicit Target(QWidget *parent = 0);

    void setNumberOfLinks(uint n);

    void locked(bool);

    void blink();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *);

signals:
    void moving(const QPoint &);
    void dropped(const QPoint &);
    void cancelled();
    void middleClick();

private slots:
    void slotRevertIcon();

private:
    QLabel *m_label;
    NumericLabel *m_number;
    bool m_dragging;
    QTimer *m_timerRevert;
    uint m_blinkStep;
    QPixmap m_dragPixmap;
};

#endif // TARGET_H
