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

#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

class QTimer;

class UpdateChecker;

namespace Ui
{
    class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = 0);
    ~About();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

// slots
private:
    void slotSetWaitIcon();
    void slotNewVersion(const QString &);
    void slotError(const QString &);

private:
    Ui::About *ui;
    UpdateChecker *m_checker;
    QTimer *m_timer;
    bool m_wasMousePress;
    bool m_newVersionAvailable;
};

#endif // ABOUT_H
