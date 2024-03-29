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

#include <QDesktopServices>
#include <QListWidgetItem>
#include <QLinearGradient>
#include <QApplication>
#include <QFontMetrics>
#include <QStyleOption>
#include <QFileDialog>
#include <QInputEvent>
#include <QMessageBox>
#include <QTextStream>
#include <QMouseEvent>
#include <QStringList>
#include <QClipboard>
#include <QScrollBar>
#include <QDateTime>
#include <QFileInfo>
#include <QKeyEvent>
#include <QPalette>
#include <QPainter>
#include <QStyle>
#include <QTimer>
#include <QLabel>
#include <QEvent>
#include <QColor>
#include <QMenu>
#include <QSize>
#include <QUrl>
#include <QPen>

#include "listdetails.h"
#include "settings.h"
#include "utils.h"
#include "list.h"

#include "ui_list.h"

List::List(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::List)
    , m_ignoreInput(false)
{
    ui->setupUi(this);

    connect(ui->widgetInput, &MiniInput::focusUp,  this, &List::slotFocusUp);
    connect(ui->widgetInput, &MiniInput::addText,  this, &List::addText);
    connect(ui->widgetInput, &MiniInput::loadText, this, &List::loadText);

    m_numbers = new ListDetails(window());

    // "List" menu
    QMenu *menu = new QMenu(this);
    //: This is the label on a menu item that user clicks to issue the command
    menu->addAction(QIcon(":/images/clear.png"), tr("Clear") + '\t' + QKeySequence(QKeySequence::New).toString(), this, SLOT(clear()));
    menu->addAction(tr("Undo clear") + '\t' + QKeySequence(QKeySequence::Undo).toString(), this, SLOT(undo()));
    menu->addSeparator();
    //: This is the label on a menu item that user clicks to issue the command
    menu->addAction(tr("Sort") + "\tR", this, SLOT(sort()));

    ui->pushList->setMenu(menu);

    QIcon file_icon(":/images/file.png");

    // "Add" menu
    menu = new QMenu(this);
    //: This is the label on a menu item that user clicks to issue the command
    menu->addAction(file_icon, tr("Add from file...") + "\tA", this, SLOT(slotAddFromFile()));
    ui->pushAdd->setMenu(menu);

    setFocusProxy(ui->list);

    load();
    numberOfItemsChanged();

    m_numbers->show();

    // catch keyboard events
    ui->list->installEventFilter(this);
    ui->list->viewport()->installEventFilter(this);
    ui->labelListHeader->installEventFilter(this);
}

List::~List()
{
    delete m_numbers;
    delete ui;
}

bool List::hasLines() const
{
    return ui->list->count();
}

void List::addText(const QString &text)
{
    if(addItem(text))
    {
        numberOfItemsChanged();
        save();
    }
}

void List::initialSelect(bool alsoSetFocus)
{
    if(alsoSetFocus)
        setFocus();

    QListWidgetItem *item = ui->list->item(0);

    if(!item)
        return;

    ui->list->setCurrentItem(item);
}

void List::removeDuplicates()
{
    qDebug("Looking for duplicates");

    int row = 0;
    QListWidgetItem *i;
    QList<QListWidgetItem *> list;
    bool changed = false;

    while((i = ui->list->item(row++)))
    {
        list = ui->list->findItems(i->text(), Qt::MatchFixedString);

        if(list.size() > 1)
        {
            changed = true;

            for(int i = 1;i < list.size();i++)
                delete list[i];
        }
    }

    if(changed)
    {
        numberOfItemsChanged();
        save();
    }
}

bool List::eventFilter(QObject *obj, QEvent *event)
{
    const QEvent::Type type = event->type();

    // eat input events
    if(m_ignoreInput && obj == ui->list &&
                            (type == QEvent::KeyPress
                            || type == QEvent::KeyRelease
                            || type == QEvent::ShortcutOverride
                            || type == QEvent::MouseButtonPress
                            || type == QEvent::MouseButtonRelease
                            || type == QEvent::MouseButtonDblClick
                            || type == QEvent::MouseMove
                            || type == QEvent::Wheel
                            || type == QEvent::TabletPress
                            || type == QEvent::TabletRelease
                            || type == QEvent::TabletMove
                            || type == QEvent::TouchBegin
                            || type == QEvent::TouchEnd
                            || type == QEvent::TouchUpdate
                         ))
        return true;

    if(obj == ui->list)
    {
        if(type == QEvent::KeyPress)
        {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);

            bool ate = true;

            if(ke->matches(QKeySequence::New))
                clear();
            else if(ke->matches(QKeySequence::Open))
                slotAddFromFile();
            else if(ke->matches(QKeySequence::Save))
                save();
            else if(ke->matches(QKeySequence::Undo))
                undo();
            else if(ke->modifiers() == Qt::NoModifier
                    || ke->modifiers() == Qt::KeypadModifier) // disallow all modifiers except keypad
            {
                switch(ke->key())
                {
                    case Qt::Key_A:
                        slotAddFromFile();
                    break;

                    case Qt::Key_O:
                    case Qt::Key_Insert:
                        focusMiniEntry();
                    break;

                    case Qt::Key_Delete:
                        deleteCurrent();
                    break;

                    case Qt::Key_Return:
                    case Qt::Key_Enter:
                        loadItem(Load::Current);
                    break;

                    case Qt::Key_R:
                        sort();
                    break;

                    case Qt::Key_Up:
                        loadItem(Load::Previous);
                    break;

                    case Qt::Key_Down:
                        loadItem(Load::Next);
                    break;

                    case Qt::Key_Home:
                        loadItem(Load::First);
                    break;

                    case Qt::Key_End:
                        loadItem(Load::Last);
                    break;

                    case Qt::Key_PageUp:
                        loadItem(Load::PageUp);
                    break;

                    case Qt::Key_PageDown:
                        loadItem(Load::PageDown);
                    break;

                    // default processing
                    case Qt::Key_Tab:
                        return QObject::eventFilter(obj, event);

                    default:
                        ate = false;
                    break;
                } // switch
            }
            else if(ke->modifiers() == Qt::ControlModifier)
            {
                switch(ke->key())
                {
                    case Qt::Key_Up:
                        moveItem(Move::Previuos);
                    break;

                    case Qt::Key_Down:
                        moveItem(Move::Next);
                    break;

                    case Qt::Key_Home:
                        moveItem(Move::First);
                    break;

                    case Qt::Key_End:
                        moveItem(Move::Last);
                    break;

                    case Qt::Key_PageUp:
                        moveItem(Move::PageUp);
                    break;

                    case Qt::Key_PageDown:
                        moveItem(Move::PageDown);
                    break;

                    default:
                        ate = false;
                    break;
                }
            }
            else
                ate = false;

            if(ate)
                return true;
        }
        else if(type == QEvent::FocusIn)
            ui->list->setAlternatingRowColors(true);
        else if(type == QEvent::FocusOut)
            ui->list->setAlternatingRowColors(false);
        else if(type == QEvent::Resize || type == QEvent::Move)
            moveNumbersLabel();
    }
    else if(obj == ui->list->viewport())
    {
        if(type == QEvent::Resize || type == QEvent::Move)
            moveNumbersLabel();
    }

    return QObject::eventFilter(obj, event);
}

void List::moveEvent(QMoveEvent *)
{
    moveNumbersLabel();
}

void List::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void List::numberOfItemsChanged()
{
    m_numbers->setTotal(ui->list->count());

    // also change the current row
    slotCurrentRowChanged(ui->list->currentRow());

    resizeNumberLabel();

    if(ui->list->count())
        m_oldLines.clear();
}

QStringList List::toStringList()
{
    int i = 0;
    QStringList items;
    QListWidgetItem *item;

    while((item = ui->list->item(i++)))
    {
        items.append(item->text());
    }

    return items;
}

void List::save()
{
    qDebug("Saving section");

    qint64 t = QDateTime::currentMSecsSinceEpoch();

    // TODO
    //Settings::instance()->setTickersForGroup(m_section, toStringList());

    qDebug("Saved in %ld ms.", static_cast<long int>(QDateTime::currentMSecsSinceEpoch() - t));
}

void List::load()
{
    qDebug("Loading lines");
    // TODO
    //addLines(Settings::instance()->tickersForGroup(m_section));
}

void List::addLines(const QStringList &nlines)
{
    QStringList lines = nlines;

    // nothing to paste
    if(lines.isEmpty())
    {
        qDebug("Nothing to add");
        return;
    }

    qint64 v = QDateTime::currentMSecsSinceEpoch();

    if(!ui->list->count())
        lines.removeDuplicates();

    ui->list->setUpdatesEnabled(false);

    bool changed = false;

    foreach(const QString &line, lines)
    {
        if(addItem(line))
            changed = true;
    }

    ui->list->setUpdatesEnabled(true);

    qDebug("Added in %ld ms.", static_cast<long int>(QDateTime::currentMSecsSinceEpoch() - v));

    if(changed)
    {
        numberOfItemsChanged();
        save();
    }
}

bool List::addItem(const QString &text)
{
    if(ui->list->findItems(text, Qt::MatchFixedString).size())
        return false;

    // check if empty
    bool wasEmpty = !ui->list->count();

    ui->list->addItem(new QListWidgetItem(text, ui->list));

    if(wasEmpty)
        initialSelect(ui->list->hasFocus());

    return true;
}

void List::resizeNumberLabel()
{
    m_numbers->ensurePolished();

    // basic size
    QSize size = QFontMetrics(m_numbers->font()).size(0, m_numbers->totalText());
    size.setHeight(size.height()*2);

    // plus margins
    int left = 0, top = 0, right = 0, bottom = 0;

    m_numbers->layout()->getContentsMargins(&left, &top, &right, &bottom);
    m_numbers->resize(size + QSize(left + right + 2, top + bottom + m_numbers->layout()->spacing() + 2));

    moveNumbersLabel();
}

void List::moveNumbersLabel()
{
    const QWidget *w = ui->list->viewport();

    m_numbers->move(w->mapTo(window(), QPoint(w->width(), 0)).x() - m_numbers->width(),
                   w->mapTo(window(), QPoint(0, w->height())).y() - m_numbers->height()/2 + 1);
}

void List::undo()
{
    if(ui->list->count() || m_oldLines.isEmpty())
    {
        qDebug("Nothing to undo");
        return;
    }

    qDebug("Undo");

    addLines(m_oldLines);
}

QString List::currentText() const
{
    QListWidgetItem *item = ui->list->currentItem();
    return item ? item->text() : QString();
}

void List::deleteCurrent()
{
    QListWidgetItem *i = ui->list->currentItem();

    if(!i)
        return;

    delete i;
    numberOfItemsChanged();
    save();
    loadItem(Load::Current);
}

void List::loadItem(Load litem)
{
    QListWidgetItem *item = ui->list->currentItem();

    if(!item)
        item = ui->list->item(0);

    QListWidgetItem *citem = item;

    switch(litem)
    {
        case Load::Current:
        break;

        case Load::Next:
            item = ui->list->item(ui->list->row(item)+1);
        break;

        case Load::Previous:
            item = ui->list->item(ui->list->row(item)-1);
        break;

        case Load::First:
            item = ui->list->item(0);
        break;

        case Load::Last:
            item = ui->list->item(ui->list->count()-1);
        break;

        case Load::PageUp:
        case Load::PageDown:
            item = ui->list->itemAt(ui->list->visualItemRect(item).adjusted(
                                        0,
                                        (litem == Load::PageUp ? -ui->list->viewport()->height() : ui->list->viewport()->height()),
                                        0,
                                        0
                                        ).topLeft());

            if(!item)
                item = (litem == Load::PageUp) ? ui->list->item(0) : ui->list->item(ui->list->count()-1);
        break;
    }

    if(!item)
    {
        qDebug("Cannot find item to load");
        return;
    }

    if(item == citem && litem != Load::Current)
    {
        qDebug("Will not load the same item");
        return;
    }

    ui->list->setCurrentItem(item);
    emit loadText(item->text());
}

void List::moveItem(Move mi)
{
    QListWidgetItem *item = ui->list->currentItem();

    if(!item)
    {
        qDebug("Cannot find the line to move");
        return;
    }

    int crow = ui->list->currentRow();
    int row = -1;

    switch(mi)
    {
        case Move::Next:
            row = ui->list->row(item)+1;

            if(row >= ui->list->count())
                row = ui->list->count()-1;
        break;

        case Move::Previuos:
            row = ui->list->row(item)-1;

            if(row <= 0)
                row = 0;
        break;

        case Move::First:
            row = 0;
        break;

        case Move::Last:
            row = ui->list->count()-1;
        break;

        case Move::PageUp:
        case Move::PageDown:
            item = ui->list->itemAt(ui->list->visualItemRect(item).adjusted(
                                        0,
                                        (mi == Move::PageUp ? -ui->list->viewport()->height() : ui->list->viewport()->height()),
                                        0,
                                        0
                                        ).topLeft());

            if(!item)
                item = (mi == Move::PageUp) ? ui->list->item(0) : ui->list->item(ui->list->count()-1);

            row = ui->list->row(item);
        break;
    }

    if(row == crow)
    {
        qDebug("Won't move to the same position");
        return;
    }

    if(row < 0)
    {
        qDebug("Won't move to the invalid position");
        return;
    }

    qDebug("Moving line from position %d to %d", crow, row);

    item = ui->list->takeItem(crow);

    if(!item)
        return;

    ui->list->insertItem(row, item);
    ui->list->setCurrentItem(item);

    save();
}

void List::focusMiniEntry()
{
    ui->widgetInput->setFocusAndSelect();
}

void List::slotAddFromFile()
{
    qDebug("Adding new lines from file");

    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          //: Appeal to the user. Means "Please choose a file:"
                                                          tr("Choose a file"),
                                                          SETTINGS_GET_STRING(SETTING_LAST_FILE_DIRECTORY),
                                                          tr("Text files (*.txt)")
                                                              + ";;"
                                                              + tr("All files (*.*)"));

    if(fileNames.isEmpty())
        return;

    bool error = false;
    QStringList errorFiles, lines;

    SETTINGS_SET_STRING(SETTING_LAST_FILE_DIRECTORY, QFileInfo(fileNames.at(0)).absolutePath());

    foreach(QString fileName, fileNames)
    {
        QString line;
        QFile file(fileName);

        if(!file.open(QIODevice::ReadOnly))
        {
            qWarning("Cannot open file \"%s\" for reading", qPrintable(fileName));
            error = true;
            errorFiles.append(fileName);
            continue;
        }

        while(!file.atEnd())
        {
            line = file.readLine();

            if(!line.isEmpty())
                lines.append(line);
        }
    }

    addLines(lines);

    if(error)
        QMessageBox::warning(this,
                             Utils::errorTitle(),
                             //: Message displayed to the user. %1 will be replaced with the list of files by the application
                             tr("Cannot open the following files: %1").arg(errorFiles.join(",")));
}

void List::clear()
{
    qDebug("Clear section");

    // nothing to do
    if(!ui->list->count())
        return;

    m_oldLines = toStringList();

    ui->list->clear();
    numberOfItemsChanged();
    save();
}

void List::sort()
{
    ui->list->sortItems();
    save();
}

void List::slotFocusUp()
{
    setFocus();
}

void List::slotCurrentRowChanged(int row)
{
    m_numbers->setCurrent(row+1);
}
