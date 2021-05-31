/**
 *  @file TopicHistoryItemDelegate.h
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */

#pragma once

#include <QStyledItemDelegate>

/**
 * This class is based on scopchanov's response: https://stackoverflow.com/questions/53105343/is-it-possible-to-add-a-custom-widget-into-a-qlistview
 */
class TopicHistoryItemDelegate : public QStyledItemDelegate{
    Q_OBJECT
public:
    explicit TopicHistoryItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
