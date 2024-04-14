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

#include <QFontMetrics>

#include "chapterlabel.h"

ChapterLabel::ChapterLabel(QWidget *parent)
    : QLabel(parent)
{}

void ChapterLabel::setText(const QString &text)
{
    m_text = text;
    updateText();
}

QSize ChapterLabel::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}

void ChapterLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateText();
}

void ChapterLabel::updateText()
{
    QFontMetrics metrics(font());
    QLabel::setText(metrics.elidedText(m_text, Qt::ElideLeft, width()));
}
