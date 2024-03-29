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

#ifndef LIST_H
#define LIST_H

#include <QStringList>
#include <QWidget>

class ListDetails;

namespace Ui
{
    class List;
}

class List : public QWidget
{
    Q_OBJECT

public:
    explicit List(QWidget *parent = 0);
    ~List();

    inline void setIgnoreInput(bool);
    inline bool ignoreInput() const;

    bool hasLines() const;

    void initialSelect(bool alsoSetFocus = true);

    void removeDuplicates();

    void focusMiniEntry();

    void addLines(const QStringList &nlines);

    QString currentText() const;

    enum class Load { Current,
                    Next, Previous,
                    First, Last,
                    PageUp, PageDown };

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void moveEvent(QMoveEvent *);
    virtual void paintEvent(QPaintEvent *event);

private:
    void deleteCurrent();
    void numberOfItemsChanged();
    QStringList toStringList();
    void save();
    void load();
    bool addItem(const QString &text);
    void resizeNumberLabel();
    void moveNumbersLabel();

    enum Move { Next, Previuos,
                    First, Last,
                    PageUp, PageDown };

    void moveItem(Move);

signals:
    void loadText(const QString &);

public slots:
    void addText(const QString &text);
    void clear();
    void loadItem(Load litem = Load::Current);
    void sort();

private slots:
    void slotAddFromFile();
    void slotCurrentRowChanged(int);
    void undo();

// slots
private:
    void slotFocusUp();

private:
    Ui::List *ui;
    bool m_ignoreInput;
    ListDetails *m_numbers;
    QStringList m_oldLines;
};

inline
void List::setIgnoreInput(bool ignore)
{
    m_ignoreInput = ignore;
}

inline
bool List::ignoreInput() const
{
    return m_ignoreInput;
}

#endif // LIST_H
