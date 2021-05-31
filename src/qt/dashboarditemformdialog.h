#ifndef DASHBOARDITEMDELEGATE_H
#define DASHBOARDITEMDELEGATE_H

#include <QDialog>

namespace Ui {
class DashboardItemFormDialog;
}

class DashboardItemFormDialog : public QDialog
{
    Q_OBJECT
    int row;
    int column;

public:
    explicit DashboardItemFormDialog(QWidget *parent, const QModelIndex& index);
    ~DashboardItemFormDialog();
public slots:
    void nextButton();
    void previousButton();

private:
    Ui::DashboardItemFormDialog *ui;
};

#endif // DASHBOARDITEMDELEGATE_H
