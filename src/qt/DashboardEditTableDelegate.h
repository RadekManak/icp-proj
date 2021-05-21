#pragma once

#include <QStyledItemDelegate>

class DashboardEditTableDelegate : public QStyledItemDelegate{
    Q_OBJECT

public:
    explicit DashboardEditTableDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

};