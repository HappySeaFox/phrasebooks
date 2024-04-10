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

class BookFileIconProvider;

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

private:
    void expandParent(const QString &name);

private slots:
    void slotAddBook();
    void slotAddChapter();
    void slotDelete();
    void slotExplorer();

signals:
    void selected(const QString &bookAndChapter);

// slots
private:
    void slotActivated();
    void slotSelectionChanged();
    void slotRowsInserted(const QModelIndex &parent, int first, int last);

private:
    Ui::SelectChapter *ui;
    QDir m_root;
    QFileSystemModel *m_model;
    QString m_currentBook, m_currentChapter;
    QString m_nameToEdit;
    BookFileIconProvider *m_iconProvider;
};

#endif // SELECTCHAPTER_H
