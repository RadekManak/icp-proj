#include <iostream>
#include <utility>
#include "dashboarditemwidget.h"

#include "ui_dashboarditemwidget.h"
#include "Mqttclient.h"

DashboardItemWidget::DashboardItemWidget(QWidget *parent, DashboardItemData in_data, QStandardItem* TopicDataItem, std::shared_ptr<Mqttclient> mqttclient) :
    QWidget(parent),  data(std::move(in_data)),
    ui(new Ui::DashboardItemWidget)
{
    client = std::move(mqttclient);
    ui->setupUi(this);
    if (data.type == "On/Off"){
        ui->stackedWidgetContent->setCurrentWidget(ui->OnOff);
        if (!data.controllable){
            ui->horizontalLayout_onoff->deleteLater();
        } else {
            connect(ui->OnOffpushButton_on, &QPushButton::clicked, this, &DashboardItemWidget::button_clicked);
            connect(ui->OnOffpushButton_off, &QPushButton::clicked, this, &DashboardItemWidget::button_clicked);
        }
    } else if (data.type == "Centered Oneline"){
        ui->stackedWidgetContent->setCurrentWidget(ui->Centered);
    } else if (data.type == "MultiLine Text"){
        ui->stackedWidgetContent->setCurrentWidget(ui->Multiline);
    } else if (data.type == "MultiLine Send"){
        ui->stackedWidgetContent->setCurrentWidget(ui->MultilineSend);
    } else {
        std::cout << data.type << std::endl;
    }
    ui->centeredLabel->setScaledContents(true);
    ui->label->setText(data.name.data());

    topicDataPtr = TopicDataItem->data(Qt::UserRole + 1).value<Topicdata*>();
    connect(topicDataPtr, &Topicdata::data_changed, this, &DashboardItemWidget::updateWidget);
}

DashboardItemWidget::~DashboardItemWidget()
{
    delete ui;
}

void DashboardItemWidget::updateWidget() {
    switch (ui->stackedWidgetContent->currentIndex()) {
        case 0: //Centered
            updateCentered();
            break;
        case 1: //Multiline send
            break;
        case 2: //Multiline
            break;
        case 3: //OnOff
            updateOnOff();
            break;
    }
}

void DashboardItemWidget::updateCentered(){
    auto* message = topicDataPtr->latest;
    QPixmap pixmap;
    uint len = message->payload.length()*sizeof(uchar);
    pixmap.loadFromData((uchar*)message->payload.data(), len);
    ui->centeredLabel->setPixmap(pixmap.scaled(ui->centeredLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void DashboardItemWidget::button_clicked() {
    QObject* obj = sender();
    if (obj == ui->OnOffpushButton_on){
        client->send_message(data.controlTopic, data.turnOnCommand);
    } else if (obj == ui->OnOffpushButton_off){
        client->send_message(data.controlTopic, data.turnOffCommand);
    }
}

void DashboardItemWidget::updateOnOff() {
    if (topicDataPtr->latest->payload == data.onStateMessage){
        if (data.onOffType == "Light"){
            ui->OnOffImage->setText("Light on");
        } else if (data.onOffType == "Door"){
            ui->OnOffImage->setText("Door open");
        } else if (data.onOffType == "Generic"){
            ui->OnOffImage->setText("Turned on");
        }
    } else if (topicDataPtr->latest->payload == data.offStateMessage) {
        if (data.onOffType == "Light") {
            ui->OnOffImage->setText("Light off");
        } else if (data.onOffType == "Door") {
            ui->OnOffImage->setText("Door closed");
        } else if (data.onOffType == "Generic") {
            ui->OnOffImage->setText("Turned off");
        }
    } else {
        // Unknown message
        ui->OnOffImage->setText("Unrecognized state");
    }
}
