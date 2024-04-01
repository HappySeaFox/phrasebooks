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
#include <QDir>

#include "books.h"

Books::Books()
    : m_root(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
{
    m_root.mkpath("English");
    m_root.cd("English");
}

QStringList Books::books() const
{
    m_root.refresh();
    return m_root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
}

bool Books::addBook(const QString &book) const
{
    if(m_root.exists(book))
        return false;

    return m_root.mkdir(book);
}
