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

#include <QFileSystemModel>
#include <QStandardPaths>
#include <QInputDialog>
#include <QMessageBox>
#include <QTreeView>
#include <QDebug>
#include <QDir>

#include "booksandchapters.h"
#include "utils.h"

#include "ui_booksandchapters.h"

BooksAndChapters::BooksAndChapters(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BooksAndChapters)
    , m_root(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
{
    ui->setupUi(this);

    m_root.mkdir("English");
    m_root.cd("English");

    QFileSystemModel *m = new QFileSystemModel(this);
    m->setRootPath(m_root.absolutePath());

    m_view = new QTreeView;
    m_view->setModel(m);
    m_view->setRootIndex(m->index(m->rootPath()));
    m_view->setWindowFlags(m_view->windowFlags() | Qt::ToolTip);
    m_view->setHeaderHidden(true);

    for(int i = 1;i < m->columnCount();i++)
        m_view->setColumnHidden(i, true);

    ui->label->installEventFilter(this);
}

BooksAndChapters::~BooksAndChapters()
{
    delete m_view;
    delete ui;
}

bool BooksAndChapters::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        m_view->move(ui->label->mapToGlobal(QPoint(0, 0)) + QPoint(0, ui->label->height()));
        m_view->show();
    }

    return QObject::eventFilter(obj, event);
}

BooksAndChapters::CreateStatus BooksAndChapters::addBook(const QString &book) const
{
    if(m_root.exists(book))
        return CreateStatus::Exists;

    return m_root.mkdir(book) ? CreateStatus::Ok : CreateStatus::Error;
}

BooksAndChapters::CreateStatus BooksAndChapters::addChapter(const QString &bookAndChapter) const
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

void BooksAndChapters::slotAddBook()
{
    qDebug("Adding book");

    QString book = QInputDialog::getText(this, tr("New book"), tr("Book:"));

    if(book.isEmpty())
        return;

    if(addBook(book) != CreateStatus::Ok)
        QMessageBox::warning(this, Utils::errorTitle(), tr("Cannot add this book"));
}

void BooksAndChapters::slotAddChapter()
{
    qDebug("Adding chapter");

    QString chapter = QInputDialog::getText(this, tr("New chapter"), tr("Chapter:"));

    // TODO calculate book+chapter path
    if(chapter.isEmpty())
        return;

    if(addChapter(chapter) != CreateStatus::Ok)
        QMessageBox::warning(this, Utils::errorTitle(), tr("Cannot add this chapter"));
}

void BooksAndChapters::slotDelete()
{
    qDebug("Deleting");
}
