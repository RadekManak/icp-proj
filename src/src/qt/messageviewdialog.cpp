#include "messageviewdialog.h"
#include "ui_messageviewdialog.h"

MessageViewDialog::MessageViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageViewDialog)
{
    ui->setupUi(this);
}

MessageViewDialog::~MessageViewDialog()
{
    delete ui;
}

void MessageViewDialog::setMessage(TopicMessage *message) {
    time_t time = std::chrono::system_clock::to_time_t(message->received_time);
    char * timestamptext = ctime(&time);
    ui->timestamp->setText(timestamptext);
    if (message->mime_type == "text/plain"){
        ui->stackedWidget->setCurrentIndex(0);
        ui->textEdit->setText(message->payload.data());
    } else if(message->mime_type == "image/png"){
        ui->stackedWidget->setCurrentIndex(1);
        QPixmap * mypix = new QPixmap();
        uint len = message->payload.length();
        const uchar *buf = reinterpret_cast<const uchar *>(QString(message->payload.data()).toLocal8Bit().constData());
        mypix->loadFromData(buf, len, nullptr, Qt::AutoColor);
        ui->imageLabel->setPixmap(*mypix);
        delete mypix;
    }
}
