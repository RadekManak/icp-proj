#pragma once

#include <QStyledItemDelegate>

class TopicHistoryItemDelegate : public QStyledItemDelegate{
    Q_OBJECT
public:
    explicit TopicHistoryItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
