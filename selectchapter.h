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

#ifndef SELECTCHAPTER_H
#define SELECTCHAPTER_H

#include <QDialog>
#include <QDir>

class QFileSystemModel;

namespace Ui
{
    class SelectChapter;
}

class SelectChapter : public QDialog
{
    Q_OBJECT

public:
    explicit SelectChapter(const QDir &root, QWidget *parent = 0);
    ~SelectChapter();

    void createDefault();

private:
    enum CreateStatus { Ok, Exists, Error };

    CreateStatus addBook(const QString &book) const;
    CreateStatus addChapter(const QString &chapter) const;

private slots:
    void slotAddBook();
    void slotAddChapter();
    void slotDelete();

signals:
    void selected(const QString &book, const QString &chapter);

// slots
private:
    void slotActivated();
    void slotSelectionChanged();

private:
    Ui::SelectChapter *ui;
    QDir m_root;
    QFileSystemModel *m_model;
    QString m_currentBook, m_currentChapter;
};

#endif // SELECTCHAPTER_H
