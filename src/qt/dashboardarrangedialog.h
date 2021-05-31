#ifndef DASHBOARDARRANGEDIALOG_H
#define DASHBOARDARRANGEDIALOG_H

#include <QDialog>

namespace Ui {
class DashboardArrangeDialog;
}

class DashboardArrangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DashboardArrangeDialog(QWidget *parent = nullptr);
    ~DashboardArrangeDialog();
public slots:
    void tableDoubleClick(QModelIndex index);

private:
    Ui::DashboardArrangeDialog *ui;
};

#endif // DASHBOARDARRANGEDIALOG_H
