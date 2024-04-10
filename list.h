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
#include <QString>

class QMenu;

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

    int count() const;

    void initialSelect(bool alsoSetFocus = true);

    void removeDuplicates();

    void focusMiniEntry();

    void addLines(const QStringList &nlines);

    bool setCurrentChapterPath(const QString &path);
    QString currentChapterPath() const;

    QString currentText() const;

    void reset();

    enum class Load { Current,
                    Next, Previous,
                    First, Last,
                    PageUp, PageDown };

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

private:
    void deleteCurrent();
    void numberOfItemsChanged();
    QStringList toStringList();
    void save();
    bool addItem(const QString &text);

    enum class Move { Next, Previuos,
                    First, Last,
                    PageUp, PageDown };

    void moveItem(Move);

signals:
    void loadText(const QString &);
    void currentIndexChanged(int current, int total);

public slots:
    void clear();
    void loadItem(Load litem = Load::Current);
    void sort();

private slots:
    void slotAddText(const QString &text);
    void slotAddFromFile();
    void slotCurrentRowChanged(int);
    void undo();

// slots
private:
    void slotFocusUp();

private:
    Ui::List *ui;
    bool m_ignoreInput;
    QStringList m_oldLines;
    QMenu *m_menu;
    QString m_currentChapterPath;
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

inline
QString List::currentChapterPath() const
{
    return m_currentChapterPath;
}

#endif // LIST_H
