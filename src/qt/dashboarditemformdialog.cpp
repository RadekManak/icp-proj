#include <iostream>
#include <QMainWindow>
#include "dashboarditemformdialog.h"
#include "ui_dashboarditemformdialog.h"
#include "mainwindow.h"

DashboardItemFormDialog::DashboardItemFormDialog(QWidget *parent, const QModelIndex& index) :
    QDialog(parent),
    ui(new Ui::DashboardItemFormDialog)
{
    row = index.row();
    column = index.column();
    ui->setupUi(this);
    ui->pushButton_previous->setVisible(false);
    connect(ui->pushButton_next, &QPushButton::clicked, this, &DashboardItemFormDialog::nextButton);
    connect(ui->pushButton_previous, &QPushButton::clicked, this, &DashboardItemFormDialog::previousButton);
}

DashboardItemFormDialog::~DashboardItemFormDialog()
{
    delete ui;
}

MainWindow* getMainWindow()
{
    for (QWidget * window: QApplication::topLevelWidgets()){
        if (auto* mainWindow = qobject_cast<MainWindow*>(window))
            return mainWindow;
    }
    return nullptr;
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
        DashboardItemData dashboardItemData;
        dashboardItemData.row = row;
        dashboardItemData.column = column;
        dashboardItemData.name = ui->lineEdit_name->text().toStdString();
        dashboardItemData.type = ui->comboBox_type->currentText().toStdString();
        if (ui->formPageWidget->currentWidget() == ui->topic_only){
            dashboardItemData.stateTopic = ui->subscribe_topic->text().toStdString();
        } else if (ui->formPageWidget->currentWidget() == ui->onOff){
            dashboardItemData.onOffType = ui->onoff_type_comboBox->currentText().toStdString();
            dashboardItemData.stateTopic = ui->onoff_state_topic->text().toStdString();
            dashboardItemData.offStateMessage = ui->onoff_off_message->text().toStdString();
            dashboardItemData.onStateMessage = ui->onoff_on_message->text().toStdString();
            dashboardItemData.controllable = ui->controllable_checkbox->isChecked();
            if (ui->controllable_checkbox->isChecked()){
                dashboardItemData.controlTopic = ui->onoff_control_topic->text().toStdString();
                dashboardItemData.turnOnCommand = ui->onoff_on_command->text().toStdString();
                dashboardItemData.turnOffCommand = ui->onoff_turnoff_command->text().toStdString();
            }
        }
        getMainWindow()->addDashBoardWidget(dashboardItemData);
        this->close();
    }
}

void DashboardItemFormDialog::previousButton() {
    ui->formPageWidget->setCurrentWidget(ui->main);
    ui->pushButton_next->setText("Next");
    ui->pushButton_previous->setVisible(false);
}