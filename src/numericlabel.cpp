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

#include <QImage>

#include "numericlabel.h"

static const uint PHRASEBOOKS_NUMERIC_LABEL_MAX = 20;

NumericLabel::NumericLabel(QWidget *parent)
    : QLabel(parent)
{
    init(0);
}

NumericLabel::NumericLabel(uint value, QWidget *parent)
    : QLabel(parent)
{
    init(value);
}

NumericLabel::~NumericLabel()
{}

void NumericLabel::setValue(uint val)
{
    if(m_value == val || (m_value > PHRASEBOOKS_NUMERIC_LABEL_MAX && val > PHRASEBOOKS_NUMERIC_LABEL_MAX))
        return;

    m_value = val;

    QHash<uint, QPixmap>::iterator it = m_cache.find(m_value > PHRASEBOOKS_NUMERIC_LABEL_MAX
                                                     ? (PHRASEBOOKS_NUMERIC_LABEL_MAX+1)
                                                     : m_value);
    QPixmap pixmap;

    if(it == m_cache.end())
    {
        pixmap = QPixmap(m_value > PHRASEBOOKS_NUMERIC_LABEL_MAX
                         ? QString(":/images/counters/%1+.png").arg(PHRASEBOOKS_NUMERIC_LABEL_MAX)
                         : QString(":/images/counters/%1.png").arg(m_value));

        m_cache.insert(m_value > PHRASEBOOKS_NUMERIC_LABEL_MAX
                       ? (PHRASEBOOKS_NUMERIC_LABEL_MAX+1)
                       : m_value, pixmap);
    }
    else
        pixmap = it.value();

    setPixmap(pixmap);
}

void NumericLabel::init(uint value)
{
    setObjectName("numericLabel");

    // initial invalid value
    m_value = PHRASEBOOKS_NUMERIC_LABEL_MAX + 1;

    setContentsMargins(0, 0, 0, 0);
    setValue(value);

    QImage im(QString(":/images/counters/%1+.png").arg(PHRASEBOOKS_NUMERIC_LABEL_MAX));

    if(!im.isNull())
        setFixedSize(im.size());
    else
    {
        qWarning("The reference \"%d+\" image is broken", PHRASEBOOKS_NUMERIC_LABEL_MAX);
        setFixedSize(15, 9);
    }
}
