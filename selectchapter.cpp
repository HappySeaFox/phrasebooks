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

#include <QItemSelectionModel>
#include <QFileSystemModel>
#include <QInputDialog>
#include <QModelIndex>
#include <QMessageBox>
#include <QShortcut>
#include <QDebug>
#include <QFile>

#include "selectchapter.h"
#include "utils.h"
#include "ui_selectchapter.h"

SelectChapter::SelectChapter(const QDir &root, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectChapter)
    , m_root(root)
{
    ui->setupUi(this);

    m_model = new QFileSystemModel(this);
    m_model->setRootPath(m_root.absolutePath());
    m_model->setReadOnly(false);

    // TODO root item?
    ui->treeView->setModel(m_model);
    ui->treeView->setRootIndex(m_model->index(m_model->rootPath()));

    for(int i = 1;i < m_model->columnCount();i++)
        ui->treeView->setColumnHidden(i, true);

    connect(ui->treeView, &QTreeView::activated, this, &SelectChapter::slotActivated);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SelectChapter::slotSelectionChanged);

    new QShortcut(QKeySequence::Delete, this, SLOT(slotDelete()));
}

SelectChapter::~SelectChapter()
{
    delete ui;
}

void SelectChapter::slotDelete()
{
    QModelIndexList selected = ui->treeView->selectionModel()->selectedIndexes();

    if(selected.isEmpty())
        return;

    const QString element = selected.at(0).data().toString();

    if(QMessageBox::question(this, tr("Delete"), tr("Really delete \"%1\"?").arg(element)) == QMessageBox::Yes)
    {
        QString path = m_model->filePath(selected.at(0));
        QFileInfo info(path);
        bool removed;

        if(info.isDir())
            removed = QDir(path).removeRecursively();
        else
            removed = QFile::remove(path);

        if(!removed)
            QMessageBox::warning(this, Utils::errorTitle(), tr("Cannot delete \"%1\"").arg(element));
    }
}

void SelectChapter::slotActivated()
{
    if(m_currentChapter.isEmpty())
        return;

    emit selected(m_currentBook, m_currentChapter);
    accept();
}

void SelectChapter::slotSelectionChanged()
{
    const QModelIndexList selected = ui->treeView->selectionModel()->selectedIndexes();

    ui->toolAddChapter->setEnabled(!selected.isEmpty());
    ui->toolDelete->setEnabled(!selected.isEmpty());

    if(selected.isEmpty())
    {
        m_currentBook.clear();
        m_currentChapter.clear();
    }
    else
    {
        const QModelIndex index = selected.at(0);

        if(m_model->fileInfo(index).isDir())
        {
            m_currentBook = m_root.relativeFilePath(m_model->filePath(index));
            m_currentChapter.clear();
            qDebug("Selected book \"%s\"", qPrintable(m_currentBook));
        }
        else
        {
            m_currentBook = m_root.relativeFilePath(m_model->filePath(index.parent()));
            m_currentChapter = index.data().toString();
            qDebug("Selected book \"%s\", chapter \"%s\"", qPrintable(m_currentBook), qPrintable(m_currentChapter));
        }
    }
}

SelectChapter::CreateStatus SelectChapter::addBook(const QString &book) const
{
    if(m_root.exists(book))
        return CreateStatus::Exists;

    return m_root.mkdir(book) ? CreateStatus::Ok : CreateStatus::Error;
}

SelectChapter::CreateStatus SelectChapter::addChapter(const QString &bookAndChapter) const
{
    int index = bookAndChapter.indexOf('/');

    if(index < 1)
        return CreateStatus::Error;

    const QString book = bookAndChapter.left(index);

    if(!m_root.exists(book) && !m_root.mkdir(book))
        return CreateStatus::Error;

    QFile chapter(m_root.absoluteFilePath(bookAndChapter));

    if(chapter.exists())
        return CreateStatus::Exists;

    if(!chapter.open(QFile::WriteOnly | QFile::Truncate))
    {
        qWarning("Cannot open chapter %s for writing: %s", qPrintable(bookAndChapter), qPrintable(chapter.errorString()));
        return CreateStatus::Error;
    }

    return CreateStatus::Ok;
}

void SelectChapter::slotAddBook()
{
    qDebug("Adding book");

    QString book = QInputDialog::getText(this, tr("New book"), tr("Book:"));

    if(book.isEmpty())
        return;

    if(addBook(book) != CreateStatus::Ok)
        QMessageBox::warning(this, Utils::errorTitle(), tr("Cannot add this book"));
}

void SelectChapter::slotAddChapter()
{
    qDebug("Adding chapter");

    if(m_currentBook.isEmpty())
        return;

    QString chapter = QInputDialog::getText(this, tr("New chapter"), tr("Chapter:"));

    if(chapter.isEmpty())
        return;

    if(addChapter(m_currentBook + '/' + chapter) != CreateStatus::Ok)
        QMessageBox::warning(this, Utils::errorTitle(), tr("Cannot add this chapter"));
}
