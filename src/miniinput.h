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

#ifndef MINIINPUT_H
#define MINIINPUT_H

#include <QFrame>

namespace Ui
{
    class MiniInput;
}

class MiniInput : public QFrame
{
    Q_OBJECT

public:
    explicit MiniInput(QWidget *parent = 0);
    ~MiniInput();

    void setFocusAndSelect();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *);

signals:
    void focusUp();
    void addText(const QString &);
    void loadText(const QString &);

private:
    Ui::MiniInput *ui;
};

#endif // TICKERMINIINPUT_H
