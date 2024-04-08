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

#ifndef BOOKSANDCHAPTERS_H
#define BOOKSANDCHAPTERS_H

#include <QFrame>
#include <QDir>

class SelectChapter;

namespace Ui
{
    class BooksAndChapters;
}

class BooksAndChapters : public QFrame
{
    Q_OBJECT

public:
    explicit BooksAndChapters(QWidget *parent = 0);
    ~BooksAndChapters();

    QString chapterFullPath(const QString &book, const QString &chapter) const;
    QString chapterFullPath(const QString &bookAndChapter) const;

    void setChapter(const QString &chapter);

    void openSelector();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void selected(const QString &book, const QString &chapter);

private:
    Ui::BooksAndChapters *ui;
    QDir m_root;
    bool m_wasMousePress;
    SelectChapter *m_selectChapter;
    bool m_createDefault;
};

#endif // BOOKSANDCHAPTERS_H
