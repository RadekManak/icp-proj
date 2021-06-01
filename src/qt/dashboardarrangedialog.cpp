#include "dashboardarrangedialog.h"
#include "ui_dashboardarrangedialog.h"
#include "dashboarditemformdialog.h"

DashboardArrangeDialog::DashboardArrangeDialog(QWidget *parent, std::shared_ptr<QStandardItemModel> dashboardModel) :
    QDialog(parent),
    ui(new Ui::DashboardArrangeDialog)
{
    ui->setupUi(this);
    this->model = dashboardModel;
    ui->tableView->setModel(dashboardModel.get());
    connect(ui->tableView, &QTableView::doubleClicked, this, &DashboardArrangeDialog::tableDoubleClick);

}

DashboardArrangeDialog::~DashboardArrangeDialog()
{
    delete ui;
}

void DashboardArrangeDialog::tableDoubleClick(QModelIndex index) {
    auto* dialog = new DashboardItemFormDialog(this, model, index);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->show();
}
