/** @file messageviewwidget.cpp
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */
#include "messageviewwidget.h"
#include "ui_messageviewwidget.h"

MessageViewWidget::MessageViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageViewWidget)
{
    ui->setupUi(this);
    ui->textEdit->setReadOnly(true);
}

MessageViewWidget::~MessageViewWidget()
{
    delete ui;
}

void MessageViewWidget::setMessage(TopicMessage *message) {
    if (message == nullptr){
        ui->textEdit->setText("");
        ui->stackedWidget->setCurrentIndex(0);
        return;
    }
    auto * image = new QPixmap();
    uint len = message->payload.length()*sizeof(uchar);
    const uchar *buf = (uchar*)message->payload.data();
    if (image->loadFromData(buf, len, nullptr, Qt::AutoColor)){
        ui->stackedWidget->setCurrentIndex(1);
        ui->imgLabel->setPixmap(*image);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
        ui->textEdit->setText(message->payload.data());
    }
    delete image;
}
