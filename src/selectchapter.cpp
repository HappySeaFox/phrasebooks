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

    QShortcut *shortcutNewBook = new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_B), this, SLOT(slotAddBook()));
    QShortcut *shortcutNewChapter = new QShortcut(QKeySequence::New, this, SLOT(slotAddChapter()));
    QShortcut *shortcutDelete = new QShortcut(QKeySequence::Delete, this, SLOT(slotDelete()));

    // theme icons
    ui->toolDelete->setIcon(QIcon::fromTheme("edit-delete", QIcon(":/images/delete.png")));
    ui->toolExplorer->setIcon(QIcon::fromTheme("folder-open", QIcon(":/images/filebrowser.png")));

    //: %1 will be replaced with the hotkey by the application
    ui->toolAddBook->setToolTip(tr("Add a book (%1)").arg(shortcutNewBook->key().toString()));
    //: %1 will be replaced with the hotkey by the application
    ui->toolAddChapter->setToolTip(tr("Add a chapter (%1)").arg(shortcutNewChapter->key().toString()));
    //: %1 will be replaced with the hotkey by the application
    ui->toolDelete->setToolTip(tr("Delete (%1)").arg(shortcutDelete->key().toString()));

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
    connect(m_model, &QFileSystemModel::fileRenamed, this, &SelectChapter::slotFileRenamed);

    connect(ui->treeView, &QTreeView::activated, this, &SelectChapter::slotActivated);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SelectChapter::slotSelectionChanged);
}

SelectChapter::~SelectChapter()
{
    delete m_iconProvider;
    m_iconProvider = nullptr;

    delete ui;
}

void SelectChapter::setInitialChapter(const QString &chapterRelativePath)
{
    m_initialPaths.clear();

    QStringList initialPaths = QDir::fromNativeSeparators(chapterRelativePath).split(QChar('/'), QString::SkipEmptyParts);

    if(initialPaths.isEmpty())
        return;

    if(!m_root.exists(chapterRelativePath))
    {
        qWarning("Initial chapter \"%s\" doesn't exist, ignoring it", qPrintable(chapterRelativePath));
        return;
    }

    qDebug("Initial chapter: \"%s\"", qPrintable(chapterRelativePath));

    for(int i = 0;i < initialPaths.size();i++)
    {
        m_initialPaths.append(QStringList(initialPaths.mid(0, i+1)).join(QChar('/')));
    }

    qDebug() << "Inital chapter search index:" << m_initialPaths;
}

void SelectChapter::expandParent(const QString &name)
{
    const int lastSlash = name.lastIndexOf(QChar('/'));

    if(lastSlash < 0)
        return;

    const QModelIndex index = m_model->index(m_root.absoluteFilePath(name.left(lastSlash)));

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
        QFileInfo info(m_model->filePath(index));

        if(info.isDir())
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

    qDebug("Selected book/chapter: \"%s\" \"%s\"", qPrintable(m_currentBook), qPrintable(m_currentChapter));
}

void SelectChapter::slotRowsInserted(const QModelIndex &parent, int first, int last)
{
    if(!m_nameToEdit.isEmpty())
    {
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
    else if(!m_initialPaths.isEmpty())
    {
        for(int i = first;i <= last;i++)
        {
            QModelIndex index = parent.child(i, 0);

            if(index.isValid())
            {
                if(m_root.relativeFilePath(m_model->filePath(index)) == m_initialPaths.at(0))
                {
                    if(m_model->fileInfo(index).isDir())
                        ui->treeView->expand(index);
                    else
                        ui->treeView->setCurrentIndex(index);

                    m_initialPaths.takeFirst();
                    break;
                }
            }
        }
    }
}

void SelectChapter::slotFileRenamed(const QString &path, const QString &oldName, const QString &newName)
{
    qDebug("Renamed %s => %s", qPrintable(oldName), qPrintable(newName));

    const QString selected = m_currentBook.isEmpty()
                            ? m_currentChapter
                            : (m_currentChapter.isEmpty() ? m_currentBook : (m_currentBook + '/' + m_currentChapter));

    if(selected.isEmpty())
        return;

    const QString selectedPath = m_root.absoluteFilePath(selected);
    const QString old = m_root.absoluteFilePath(path + '/' + oldName);

    if(old == selectedPath)
    {
        qDebug("Updating the currently selected item");
        slotSelectionChanged();
    }
}

void SelectChapter::slotAddBook()
{
    qDebug("Adding book under \"%s\"", m_currentBook.isEmpty() ? "." : qPrintable(m_currentBook));

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
    qDebug("Adding chapter under \"%s\"", m_currentBook.isEmpty() ? "." : qPrintable(m_currentBook));

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
