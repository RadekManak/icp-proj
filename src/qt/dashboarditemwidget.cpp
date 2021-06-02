/** @file dashboarditemwidget.cpp
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */
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
        connect(ui->MultilineSendpushButton, &QPushButton::clicked, this, &DashboardItemWidget::sendButtonClicked);
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

/**
 * Update shown widgets
 */
void DashboardItemWidget::updateWidget() {
    switch (ui->stackedWidgetContent->currentIndex()) {
        case 0: //Centered
            updateCentered();
            break;
        case 1: //Multiline send
            break;
        case 2: //Multiline
            appendMultiline();
            break;
        case 3: //OnOff
            updateOnOff();
            break;
    }
}

/**
 * Update centered widget with new data
 */
void DashboardItemWidget::updateCentered(){
    auto* message = topicDataPtr->latest;
    if (message->mime_type == "image/png"){
        QPixmap pixmap;
        uint len = message->payload.length()*sizeof(uchar);
        pixmap.loadFromData((uchar*)message->payload.data(), len);
        ui->centeredLabel->setPixmap(pixmap.scaled(ui->centeredLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->centeredLabel->setText(message->payload.data());
    }
}

/**
 * Send on/off command when button is clicked
 */
void DashboardItemWidget::button_clicked() {
    QObject* obj = sender();
    if (obj == ui->OnOffpushButton_on){
        client->send_message(data.controlTopic, data.turnOnCommand);
    } else if (obj == ui->OnOffpushButton_off){
        client->send_message(data.controlTopic, data.turnOffCommand);
    }
}

/**
 * Update on/off widget with its user friendly message
 */
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

/**
 * Send message and clear input TextEdit
 */
void DashboardItemWidget::sendButtonClicked() {
    if (!data.stateTopic.empty()){
        client->send_message(data.stateTopic, ui->MultilineSendTextEdit->toPlainText().toStdString());
    }
    ui->MultilineSendTextEdit->clear();
}

/**
 * Append new message to multiline widget
 */
void DashboardItemWidget::appendMultiline() {
    ui->MultilineTextEdit->append(topicDataPtr->latest->payload.data());
}
