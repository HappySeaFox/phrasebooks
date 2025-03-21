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

MiniInput::MiniInput(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::MiniInput)
{
    ui->setupUi(this);

    setFocusProxy(ui->lineMiniInput);

    ui->lineMiniInput->setValidator(new EnglishValidator(ui->lineMiniInput));
    ui->lineMiniInput->installEventFilter(this);

    ui->lineMiniInput->setToolTip(QString("<table>"
                                          "<tr>"
                                            "<td><nobr><b>%1</b></nobr></td>"
                                            "<td></td>"
                                            "<td>%2</td>"
                                          "</tr>"
                                          "<tr>"
                                            "<td><nobr><b>%3</b></nobr></td>"
                                            "<td></td>"
                                            "<td>%4</td>"
                                          "</tr>"
                                          "</table>")
                                  //: Means "Enter" key on a keyboard
                                  .arg(tr("Enter"))
                                  .arg(tr("Load the text into an external dictionary"))
                                  //: Means "Up Arrow" key on a keyboard
                                  .arg(tr("Up Arrow"))
                                  .arg(tr("Add the text to the current chapter")));
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
                    {
                        if(ui->lineMiniInput->text().isEmpty())
                            emit focusUp();
                        else
                        {
                            emit addText(ui->lineMiniInput->text());
                            ui->lineMiniInput->selectAll();
                        }
                    }
                    break;

                    case Qt::Key_Return:
                    case Qt::Key_Enter:
                    {
                        emit loadText(ui->lineMiniInput->text());
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
