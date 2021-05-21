#ifndef DASHBOARDITEMDELEGATE_H
#define DASHBOARDITEMDELEGATE_H

#include <QDialog>

namespace Ui {
class DashboardItemFormDialog;
}

class DashboardItemFormDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DashboardItemFormDialog(QWidget *parent = nullptr);
    ~DashboardItemFormDialog();

private:
    Ui::DashboardItemFormDialog *ui;
};

#endif // DASHBOARDITEMDELEGATE_H
