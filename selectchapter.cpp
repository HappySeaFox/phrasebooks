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
#include <QDesktopServices>
#include <QInputDialog>
#include <QModelIndex>
#include <QMessageBox>
#include <QShortcut>
#include <QDebug>
#include <QFile>
#include <QUrl>

#include "bookfileiconprovider.h"
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

    m_iconProvider = new BookFileIconProvider;
    m_model->setIconProvider(m_iconProvider);

    m_model->setRootPath(m_root.absolutePath());
    m_model->setReadOnly(false);

    ui->treeView->setModel(m_model);
    ui->treeView->setRootIndex(m_model->index(m_model->rootPath()));

    for(int i = 1;i < m_model->columnCount();i++)
        ui->treeView->setColumnHidden(i, true);

    connect(m_model, &QFileSystemModel::rowsInserted, this, &SelectChapter::slotRowsInserted);
    connect(ui->treeView, &QTreeView::activated, this, &SelectChapter::slotActivated);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SelectChapter::slotSelectionChanged);

    new QShortcut(QKeySequence::Delete, this, SLOT(slotDelete()));
}

SelectChapter::~SelectChapter()
{
    delete m_iconProvider;
    m_iconProvider = nullptr;

    delete ui;
}

void SelectChapter::expandParent(const QString &name)
{
    QModelIndex index = m_model->index(QFileInfo(m_root.absoluteFilePath(name)).absoluteDir().absolutePath());

    if(!index.isValid())
        return;

    ui->treeView->expand(index);
}

void SelectChapter::slotDelete()
{
    QModelIndexList selected = ui->treeView->selectionModel()->selectedIndexes();

    if(selected.isEmpty())
        return;

    const QString element = selected.at(0).data().toString();

    if(QMessageBox::question(this,
                             tr("Delete"),
                             //: %1 will be replaced with the directory name by the application
                             tr("Really delete \"%1\"?").arg(element)) == QMessageBox::Yes)
    {
        QString path = m_model->filePath(selected.at(0));
        QFileInfo info(path);
        bool removed;

        if(info.isDir())
        {
            const int tries = 20;

            int i = 0;
            removed = false;

            // dirty workaround for bug https://bugreports.qt.io/browse/QTBUG-52470
            while(!removed && i++ < tries)
                removed = QDir(path).removeRecursively();
        }
        else
            removed = QFile::remove(path);

        if(!removed)
            QMessageBox::warning(this, Utils::errorTitle(), tr("Cannot delete \"%1\"").arg(element));
    }
}

void SelectChapter::slotExplorer()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_root.absolutePath()));
}

void SelectChapter::slotActivated()
{
    if(m_currentChapter.isEmpty())
        return;

    emit selected(m_currentBook.isEmpty() ? m_currentChapter : (m_currentBook + '/' + m_currentChapter));

    accept();
}

void SelectChapter::slotSelectionChanged()
{
    const QModelIndexList selected = ui->treeView->selectionModel()->selectedIndexes();

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
        }
        else
        {
            m_currentBook = m_root.relativeFilePath(m_model->filePath(index.parent()));
            m_currentChapter = index.data().toString();

            if(m_currentBook == ".")
                m_currentBook.clear();
        }
    }
}

void SelectChapter::slotRowsInserted(const QModelIndex &parent, int first, int last)
{
    if(m_nameToEdit.isEmpty())
        return;

    for(int i = first;i <= last;i++)
    {
        QModelIndex index = parent.child(i, 0);

        if(index.isValid())
        {
            if(m_root.relativeFilePath(m_model->filePath(index)) == m_nameToEdit)
            {
                ui->treeView->setCurrentIndex(index);
                ui->treeView->edit(index);
                break;
            }
        }
    }

    m_nameToEdit.clear();
}

void SelectChapter::slotAddBook()
{
    qDebug("Adding book");

    const QString book = tr("New book");
    const QString baseName = m_currentBook.isEmpty() ? book : (m_currentBook + '/' + book);

    QString name = baseName;
    int index = 1;

    while(m_root.exists(name))
    {
        name = QString("%1 %2").arg(baseName).arg(index++);
    }

    expandParent(name);

    if(!m_root.mkpath(name))
        QMessageBox::warning(this, Utils::errorTitle(), tr("Cannot add a new book"));
    else
        m_nameToEdit = name;
}

void SelectChapter::slotAddChapter()
{
    qDebug("Adding chapter");

    const QString chapter = tr("New chapter");
    const QString baseName = m_currentBook.isEmpty() ? chapter : (m_currentBook + '/' + chapter);

    QString name = baseName;
    int index = 1;

    while(m_root.exists(name))
    {
        name = QString("%1 %2").arg(baseName).arg(index++);
    }

    bool error = false;

    if(!m_currentBook.isEmpty() && !m_root.exists(m_currentBook) && !m_root.mkpath(m_currentBook))
        error = true;
    else
    {
        expandParent(name);

        QFile file(m_root.absoluteFilePath(name));

        if(!file.open(QFile::WriteOnly | QFile::Truncate))
        {
            qWarning("Cannot open chapter %s for writing: %s", qPrintable(name), qPrintable(file.errorString()));
            error = true;
        }
    }

    if(error)
        QMessageBox::warning(this, Utils::errorTitle(), tr("Cannot add a new chapter"));
    else
        m_nameToEdit = name;
}
