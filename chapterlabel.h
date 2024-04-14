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

#ifndef CHAPTERLABEL_H
#define CHAPTERLABEL_H

#include <QLabel>

class ChapterLabel : public QLabel
{
public:
    ChapterLabel(QWidget *parent = 0);

    void setText(const QString &text);

    virtual QSize minimumSizeHint() const;

protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    void updateText();

private:
    QString m_text;
};

#endif // CHAPTERLABEL_H
