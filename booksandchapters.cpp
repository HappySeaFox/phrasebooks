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

#include <QStandardPaths>
#include <QMessageBox>
#include <QMouseEvent>
#include <QShortcut>
#include <QTimer>

#include <cstdlib>

#include "booksandchapters.h"
#include "selectchapter.h"
#include "settings.h"
#include "utils.h"

#include "ui_booksandchapters.h"

BooksAndChapters::BooksAndChapters(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::BooksAndChapters)
    , m_root(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
    , m_wasMousePress(false)
    , m_selectChapter(nullptr)
    , m_shortcutOpen(new QShortcut(QKeySequence::Open, this, SLOT(openSelector())))
{
    ui->setupUi(this);

    setChapter(QString());

    if(!m_root.mkpath("English"))
    {
        QMessageBox::warning(this,
                             Utils::errorTitle(),
                             //: %1 will be replaced with the directory name by the application
                             tr("Cannot create the default directory. Check the file permissions under %1").arg(m_root.absolutePath()));
        ::exit(1);
    }

    m_root.cd("English");
}

BooksAndChapters::~BooksAndChapters()
{
    delete ui;
}

QString BooksAndChapters::chapterFullPath(const QString &bookAndChapter) const
{
    return m_root.absoluteFilePath(bookAndChapter);
}

void BooksAndChapters::setChapter(const QString &chapter)
{
    m_currentChapterRelativePath = chapter;

    if(m_currentChapterRelativePath.isEmpty())
        //: %1 will be replaced with the hotkey by the application
        ui->chapter->setText(tr("Click or press %1...").arg(m_shortcutOpen->key().toString()));
    else
        ui->chapter->setText(m_currentChapterRelativePath);
}

void BooksAndChapters::openSelector()
{
    // create dialog
    if(!m_selectChapter)
    {
        m_selectChapter = new SelectChapter(m_root, this);

        m_selectChapter->setInitialChapter(m_currentChapterRelativePath);

        connect(m_selectChapter, &SelectChapter::selected, this, &BooksAndChapters::slotSelected);

        if(SETTINGS_CONTAINS(SETTING_SELECT_CHAPTER_SIZE))
            m_selectChapter->resize(SETTINGS_GET_SIZE(SETTING_SELECT_CHAPTER_SIZE));
    }

    m_selectedChapterRelativePath.clear();

    if(m_selectChapter->exec() == SelectChapter::Accepted)
        SETTINGS_SET_SIZE(SETTING_SELECT_CHAPTER_SIZE, m_selectChapter->size());

    emit selectorClosed(m_selectedChapterRelativePath);
}

void BooksAndChapters::mousePressEvent(QMouseEvent *event)
{
    event->accept();

    if(event->button() == Qt::LeftButton)
        m_wasMousePress = true;
}

void BooksAndChapters::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();

    if(m_wasMousePress)
    {
        m_wasMousePress = false;
        QTimer::singleShot(0, this, &BooksAndChapters::openSelector);
    }
}

void BooksAndChapters::slotSelected(const QString &bookAndChapter)
{
    m_selectedChapterRelativePath = bookAndChapter;
}
