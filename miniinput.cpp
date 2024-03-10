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

#include <QApplication>
#include <QKeySequence>
#include <QKeyEvent>
#include <QEvent>

#include "englishvalidator.h"
#include "miniinput.h"
#include "settings.h"

#include "ui_miniinput.h"

MiniInput::MiniInput(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MiniInput)
{
    ui->setupUi(this);

    setFocusProxy(ui->lineMiniInput);

    ui->lineMiniInput->setValidator(new EnglishValidator(ui->lineMiniInput));
    ui->lineMiniInput->installEventFilter(this);

    // TODO
    //ui->lineMiniInput->setToolTip(tr(""));
}

MiniInput::~MiniInput()
{
    delete ui;
}

void MiniInput::setFocusAndSelect()
{
    setFocus();
    ui->lineMiniInput->selectAll();
}

bool MiniInput::eventFilter(QObject *watched, QEvent *e)
{
    if(watched == ui->lineMiniInput)
    {
        switch(e->type())
        {
            case QEvent::KeyPress:
            {
                QKeyEvent *ke = static_cast<QKeyEvent *>(e);
                bool ate = true;

                // search widget
                switch(ke->key())
                {
                    case Qt::Key_Up:
                        emit focusUp();
                    break;

                    case Qt::Key_Return:
                    case Qt::Key_Enter:
                    {
                        // FIXME
                        if(1)//ke->modifiers() & Qt::ControlModifier)
                            emit loadText(ui->lineMiniInput->text());
                        else
                            emit addText(ui->lineMiniInput->text());

                        ui->lineMiniInput->selectAll();
                    }
                    break;

                    case Qt::Key_Escape:
                        ui->lineMiniInput->clear();
                    break;

                    default:
                        ate = false;
                    break;
                }

                if(ate)
                    return true;
            }
            break; // event type

            default:
            break;
        }
    }

    return QWidget::eventFilter(watched, e);
}
