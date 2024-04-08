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
#include <QDebug>
#include <QDir>

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
{
    ui->setupUi(this);

    m_root.mkdir("English");
    m_root.cd("English");

    installEventFilter(this);
}

BooksAndChapters::~BooksAndChapters()
{
    delete ui;
}

QString BooksAndChapters::chapterFullPath(const QString &book, const QString &chapter) const
{
    return m_root.absoluteFilePath(book + '/' + chapter);
}

void BooksAndChapters::setChapter(const QString &chapter)
{
    ui->chapter->setText(chapter);
}

bool BooksAndChapters::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
        m_wasMousePress = true;
    else if(event->type() == QEvent::MouseButtonRelease && m_wasMousePress)
    {
        m_wasMousePress = false;

        if(!m_selectChapter)
        {
            m_selectChapter = new SelectChapter(m_root, this);
            connect(m_selectChapter, &SelectChapter::selected, this, &BooksAndChapters::selected);

            if(SETTINGS_CONTAINS(SETTING_SELECT_CHAPTER_SIZE))
                m_selectChapter->resize(SETTINGS_GET_SIZE(SETTING_SELECT_CHAPTER_SIZE));
        }

        if(m_selectChapter->exec() == SelectChapter::Accepted)
            SETTINGS_SET_SIZE(SETTING_SELECT_CHAPTER_SIZE, m_selectChapter->size());
    }

    return QObject::eventFilter(obj, event);
}
