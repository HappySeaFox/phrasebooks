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
{
    ui->setupUi(this);

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
    ui->chapter->setText(chapter);
}

void BooksAndChapters::openSelector()
{
    // create dialog
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

void BooksAndChapters::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    m_wasMousePress = true;
}

void BooksAndChapters::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    if(m_wasMousePress)
    {
        m_wasMousePress = false;
        openSelector();
    }
}
