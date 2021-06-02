/** @file dashboarditemformdialog.h
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */
#ifndef DASHBOARDITEMDELEGATE_H
#define DASHBOARDITEMDELEGATE_H

#include <QDialog>
#include <QModelIndex>
#include <QStandardItemModel>

namespace Ui {
class DashboardItemFormDialog;
}

class DashboardItemFormDialog : public QDialog
{
    Q_OBJECT
    QModelIndex index;
    std::shared_ptr<QStandardItemModel> model;

public:
    explicit DashboardItemFormDialog(QWidget *parent, std::shared_ptr<QStandardItemModel> dashboardModel, QModelIndex index);
    ~DashboardItemFormDialog();
public slots:
    void nextButton();
    void previousButton();

private:
    Ui::DashboardItemFormDialog *ui{};
};

#endif // DASHBOARDITEMDELEGATE_H
