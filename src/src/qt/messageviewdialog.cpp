/**
 *  @file messageviewdialog.cpp
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */

#include "messageviewdialog.h"
#include <iostream>
#include "ui_messageviewdialog.h"

/** Constructor */
MessageViewDialog::MessageViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageViewDialog)
{
    ui->setupUi(this);
}

/** Destructor */
MessageViewDialog::~MessageViewDialog()
{
    delete ui;
}

/**
 * Displays message in a new window
 * @param message Object containing message data
 */
void MessageViewDialog::setMessage(TopicMessage *message) {
    time_t time = std::chrono::system_clock::to_time_t(message->received_time);
    char * timestamptext = ctime(&time);
    ui->timestamp->setText(timestamptext);
    auto * image = new QPixmap();
    uint len = message->payload.length()*sizeof(uchar);
    std::cout << len << std::endl;
    const uchar *buf = (uchar*)message->payload.data();
    if (image->loadFromData(buf, len, nullptr, Qt::AutoColor)){
        ui->stackedWidget->setCurrentIndex(1);
        ui->imageLabel->setPixmap(*image);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
        ui->textEdit->setText(message->payload.data());
    }
    delete image;
}
