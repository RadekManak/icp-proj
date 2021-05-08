/** @file messageviewwidget.h
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */
#ifndef MESSAGEVIEWWIDGET_H
#define MESSAGEVIEWWIDGET_H

#include <QWidget>
#include <Mqttclient.h>

namespace Ui {
class MessageViewWidget;
}

class MessageViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MessageViewWidget(QWidget *parent = nullptr);
    ~MessageViewWidget();
    void setMessage(TopicMessage *message);

private:
    Ui::MessageViewWidget *ui;
};

#endif // MESSAGEVIEWWIDGET_H
