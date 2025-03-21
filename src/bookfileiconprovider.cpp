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

#include "bookfileiconprovider.h"

BookFileIconProvider::BookFileIconProvider()
    : QFileIconProvider()
    , m_dirIcon(":/images/book.png")
    , m_chapterIcon(":/images/chapter.png")
{}

QIcon BookFileIconProvider::icon(const QFileInfo &info) const
{
    if(info.isDir())
        return m_dirIcon;
    else
        return m_chapterIcon;
}
