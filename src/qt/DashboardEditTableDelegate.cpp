#include "DashboardEditTableDelegate.h"
#include "dashboarditemformdialog.h"

QWidget *DashboardEditTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const {
    DashboardItemFormDialog* form = new DashboardItemFormDialog();
    form->setMinimumSize(400,500);
    return form;
    //return QStyledItemDelegate::createEditor(parent, option, index);
}

DashboardEditTableDelegate::DashboardEditTableDelegate(QObject *parent)
{
    setParent(parent);
}
