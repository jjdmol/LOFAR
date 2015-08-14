#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QtGui/QTableView>
#include <QSet>

class TableView : public QTableView
{
    Q_OBJECT

public:
    TableView(QWidget *parent = 0);
    ~TableView();

    void mousePressEvent(QMouseEvent *event);
//    Qt::KeyboardModifiers getModifierKeys(void) const {return modifierKeys;}
    virtual QModelIndexList selectedIndexes(void) const {return QTableView::selectedIndexes();}
    QSet<int> selectedRows(void) const;

protected:
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void mouseClick(const QModelIndex &index, QMouseEvent *event);
    void tableContextMenuRequest(const QModelIndex &index, QContextMenuEvent *event);

private:
//    Qt::KeyboardModifiers modifierKeys;
};

#endif // TABLEVIEW_H
