/** @file dashboardarrangedialog.h
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */
#ifndef DASHBOARDARRANGEDIALOG_H
#define DASHBOARDARRANGEDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class DashboardArrangeDialog;
}

class DashboardArrangeDialog : public QDialog
{
    Q_OBJECT
    std::shared_ptr<QStandardItemModel> model;

public:
    explicit DashboardArrangeDialog(QWidget *parent, std::shared_ptr<QStandardItemModel> dashboardModel);
    ~DashboardArrangeDialog();
public slots:
    void tableDoubleClick(QModelIndex index);
    void deleteButtonClicked();

private:
    Ui::DashboardArrangeDialog *ui;
};

#endif // DASHBOARDARRANGEDIALOG_H
