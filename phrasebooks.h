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

#ifndef PHRASEBOOKS_H
#define PHRASEBOOKS_H

#include <QByteArray>
#include <QWidget>
#include <QString>
#include <QList>

#include "platform.h"

class QGridLayout;
class QPoint;
class QTimer;
class QMenu;

namespace Ui
{
    class Phrasebooks;
}

class Phrasebooks : public QWidget
{
    Q_OBJECT

public:
    Phrasebooks();
    ~Phrasebooks();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent *e);
    virtual void dragLeaveEvent(QDragLeaveEvent *e);
    virtual void dropEvent(QDropEvent *e);

private:
    struct Link
    {
        Link(Platform::WindowId h = static_cast<Platform::WindowId>(0))
            : hwnd(h)
            , processId(0)
            , threadId(0)
            , subControl(0)
            , subControlSupportsClearing(false)
        {}

        Platform::WindowId hwnd;
        Platform::ResourceId processId;
        Platform::ResourceId threadId;
        QPoint dropPoint;
        Platform::WindowId subControl;
        bool subControlSupportsClearing;
    };

    void sendString(const QString &text);
    Link checkTargetWindow(const QPoint &);
    void checkWindows();
    void nextLoadableWindowIndex(int delta = 0);
    void loadNextWindow();
    void busy(bool);
    void loadText(const QString &text);
    bool setForeignFocus(const Link &);
    void rebuildLinks();
    bool isBusy() const;
    bool detectForegroundWindowAndActivate();

public slots:
    void activate();

private slots:
    void slotAbout();
    void slotAboutQt();
    void slotQuit();
    void slotOptions();
    void slotRestoreLastLinks();
    void slotClearLinks();
    void slotLockLinks();
    void slotTargetMoving(const QPoint &);
    void slotTargetCancelled();
    void targetDropped(const QPoint &);

// slots
private:
    void slotCheckActive();
    void slotLoadToNextWindow();
    void slotLoadText(const QString &text);
    void slotCurrentIndexChanged(int current, int total);
    void slotMessageReceived(const QString &);
    void slotFoolsDay();
    void slotSelectorClosed(const QString &bookAndChapter);

private:
    Ui::Phrasebooks *ui;

    QList<Link> m_windows;
    int m_currentWindow;
    bool m_running;
    QTimer *m_timerCheckActive;
    QTimer *m_timerLoadToNextWindow;
    qint64 m_startupTime;
    QMenu *m_menu;
    QGridLayout *m_layout;
    QString m_text;
    bool m_wasVisible;
    bool m_useKeyboardInRegion;
    bool m_locked;

#ifdef Q_OS_WIN32
    Platform::WindowId m_drawnWindow;
#endif

    bool m_justTitle;
};

#endif // Phrasebooks_H
