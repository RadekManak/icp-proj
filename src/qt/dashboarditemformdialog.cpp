#include <iostream>
#include <QMainWindow>
#include <utility>
#include "dashboarditemformdialog.h"
#include "ui_dashboarditemformdialog.h"
#include "mainwindow.h"


DashboardItemFormDialog::DashboardItemFormDialog(QWidget *parent, std::shared_ptr<QStandardItemModel> dashboardModel, QModelIndex index):
    QDialog(parent), ui(new Ui::DashboardItemFormDialog)
{
    this->index = index;
    model = std::move(dashboardModel);
    ui->setupUi(this);
    ui->pushButton_previous->setVisible(false);
    connect(ui->pushButton_next, &QPushButton::clicked, this, &DashboardItemFormDialog::nextButton);
    connect(ui->pushButton_previous, &QPushButton::clicked, this, &DashboardItemFormDialog::previousButton);
    auto* dashboardItemData = model->data(index, Qt::UserRole+1).value<DashboardItemData*>();
    if (dashboardItemData != nullptr){
        ui->lineEdit_name->setText(dashboardItemData->name.data());
    }
}

DashboardItemFormDialog::~DashboardItemFormDialog()
{
    delete ui;
}

void DashboardItemFormDialog::nextButton() {
    if (ui->formPageWidget->currentWidget() == ui->main){
        if (ui->comboBox_type->currentText() == "On/Off"){
            ui->formPageWidget->setCurrentWidget(ui->onOff);
        } else {
            ui->formPageWidget->setCurrentWidget(ui->topic_only);
        }
        ui->pushButton_next->setText("Done");
        ui->pushButton_previous->setVisible(true);
    } else {
        auto* dashboardItemData = new DashboardItemData();
        dashboardItemData->row = index.row();
        dashboardItemData->column = index.column();
        dashboardItemData->name = ui->lineEdit_name->text().toStdString();
        dashboardItemData->type = ui->comboBox_type->currentText().toStdString();
        if (ui->formPageWidget->currentWidget() == ui->topic_only){
            dashboardItemData->stateTopic = ui->subscribe_topic->text().toStdString();
        } else if (ui->formPageWidget->currentWidget() == ui->onOff){
            dashboardItemData->onOffType = ui->onoff_type_comboBox->currentText().toStdString();
            dashboardItemData->stateTopic = ui->onoff_state_topic->text().toStdString();
            dashboardItemData->offStateMessage = ui->onoff_off_message->text().toStdString();
            dashboardItemData->onStateMessage = ui->onoff_on_message->text().toStdString();
            dashboardItemData->controllable = ui->controllable_checkbox->isChecked();
            if (ui->controllable_checkbox->isChecked()){
                dashboardItemData->controlTopic = ui->onoff_control_topic->text().toStdString();
                dashboardItemData->turnOnCommand = ui->onoff_on_command->text().toStdString();
                dashboardItemData->turnOffCommand = ui->onoff_turnoff_command->text().toStdString();
            }
        }
        MainWindow::getMainWindow()->addDashBoardWidget(*dashboardItemData);
        MainWindow::getMainWindow()->saveDashboardItemSettings(*dashboardItemData);
        QStandardItem *item = model->itemFromIndex(index);
        QVariant variant;
        variant.setValue(dashboardItemData);
        item->setData(variant, Qt::UserRole+1);
        item->setText(QString(dashboardItemData->name.c_str()));
        this->close();
    }
}

void DashboardItemFormDialog::previousButton() {
    ui->formPageWidget->setCurrentWidget(ui->main);
    ui->pushButton_next->setText("Next");
    ui->pushButton_previous->setVisible(false);
}