#include "dashboarditemformdialog.h"
#include "ui_dashboarditemformdialog.h"

DashboardItemFormDialog::DashboardItemFormDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DashboardItemFormDialog)
{
    ui->setupUi(this);
}

DashboardItemFormDialog::~DashboardItemFormDialog()
{
    delete ui;
}
