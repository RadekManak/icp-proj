#ifndef MESSAGEVIEWDIALOG_H
#define MESSAGEVIEWDIALOG_H

#include <QDialog>
#include "Mqttclient.h"

namespace Ui {
class MessageViewDialog;
}

class MessageViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageViewDialog(QWidget *parent = nullptr);
    ~MessageViewDialog();
    void setMessage(TopicMessage* message);

private:
    Ui::MessageViewDialog *ui;
};

#endif // MESSAGEVIEWDIALOG_H
