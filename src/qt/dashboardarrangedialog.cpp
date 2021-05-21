#include "dashboardarrangedialog.h"
#include "ui_dashboardarrangedialog.h"
#include "DashboardEditTableDelegate.h"

DashboardArrangeDialog::DashboardArrangeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DashboardArrangeDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Edit Dashboard Layout");
    ui->tableWidget->setRowCount(10);
    ui->tableWidget->setColumnCount(5);
    auto* delegate = new DashboardEditTableDelegate();
    ui->tableWidget->setItemDelegate(delegate);

}

DashboardArrangeDialog::~DashboardArrangeDialog()
{
    delete ui;
}
