/** @file dashboardarrangedialog.cpp
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */

#include "dashboardarrangedialog.h"
#include "ui_dashboardarrangedialog.h"
#include "dashboarditemformdialog.h"
#include "mainwindow.h"

DashboardArrangeDialog::DashboardArrangeDialog(QWidget *parent, std::shared_ptr<QStandardItemModel> dashboardModel) :
    QDialog(parent),
    ui(new Ui::DashboardArrangeDialog)
{
    ui->setupUi(this);
    this->model = dashboardModel;
    ui->tableView->setModel(dashboardModel.get());
    connect(ui->tableView, &QTableView::doubleClicked, this, &DashboardArrangeDialog::tableDoubleClick);
    connect(ui->toolButton, &QToolButton::clicked, this, &DashboardArrangeDialog::deleteButtonClicked);
}

DashboardArrangeDialog::~DashboardArrangeDialog()
{
    delete ui;
}

/**
 * Opens full message in dialog
 * @param index
 */
void DashboardArrangeDialog::tableDoubleClick(QModelIndex index) {
    auto* dialog = new DashboardItemFormDialog(this, model, index);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->show();
}

/**
 * Removes currently selected dashboard widgets
 */
void DashboardArrangeDialog::deleteButtonClicked() {
    auto mainWindow = MainWindow::getMainWindow();
    for (auto index: ui->tableView->selectionModel()->selectedIndexes()){
        if (index.isValid()){
            mainWindow->removeDashboardWidget(index.row(), index.column());
            mainWindow->removeDashboardItemSettings(index.row(), index.column());
            model->setItem(index.row(), index.column(), nullptr);
        }
    }
}
