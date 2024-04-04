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

#include <QWidget>
#include <QDir>

class QTreeView;

namespace Ui
{
    class BooksAndChapters;
}

class BooksAndChapters : public QWidget
{
    Q_OBJECT

public:
    explicit BooksAndChapters(QWidget *parent = 0);
    ~BooksAndChapters();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    enum CreateStatus { Ok, Exists, Error };

    CreateStatus addBook(const QString &book) const;
    CreateStatus addChapter(const QString &bookAndChapter) const;

private slots:
    void slotAddBook();
    void slotAddChapter();
    void slotDelete();

private:
    Ui::BooksAndChapters *ui;
    QDir m_root;
    QTreeView *m_view;
};

#endif // BOOKSANDCHAPTERS_H
