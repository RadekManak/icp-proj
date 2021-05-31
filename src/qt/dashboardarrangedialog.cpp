#include "dashboardarrangedialog.h"
#include "ui_dashboardarrangedialog.h"
#include "dashboarditemformdialog.h"

DashboardArrangeDialog::DashboardArrangeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DashboardArrangeDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Edit Dashboard Layout");
    ui->tableWidget->setRowCount(10);
    ui->tableWidget->setColumnCount(4);
    connect(ui->tableWidget, &QTableWidget::doubleClicked, this, &DashboardArrangeDialog::tableDoubleClick);

}

DashboardArrangeDialog::~DashboardArrangeDialog()
{
    delete ui;
}

void DashboardArrangeDialog::tableDoubleClick(QModelIndex model) {
    auto* dialog = new DashboardItemFormDialog(this, model);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->show();
}
