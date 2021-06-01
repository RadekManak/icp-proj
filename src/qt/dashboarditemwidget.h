#ifndef DASHBOARDITEM_H
#define DASHBOARDITEM_H

#include <QWidget>
#include <Mqttclient.h>
#include "Mqttclient.h"

struct DashboardItemData{
    uint row;
    uint column;
    std::string name;
    std::string type;

    std::string onOffType;
    std::string stateTopic;
    std::string offStateMessage;
    std::string onStateMessage;
    bool controllable;
    std::string controlTopic;
    std::string turnOffCommand;
    std::string turnOnCommand;
};

Q_DECLARE_METATYPE(DashboardItemData*)

namespace Ui {
class DashboardItemWidget;
}

class DashboardItemWidget : public QWidget
{
    Q_OBJECT
    DashboardItemData data;
    Topicdata* topicDataPtr;
    std::shared_ptr<Mqttclient> client;

public:
    explicit DashboardItemWidget(QWidget *parent, DashboardItemData data, QStandardItem* TopicDataItem,
                                 std::shared_ptr<Mqttclient> mqttclient);
    ~DashboardItemWidget();

private:
    Ui::DashboardItemWidget *ui{};
public slots:
    void updateWidget();

    void updateCentered();
    void button_clicked();

    void updateOnOff();
};

#endif // DASHBOARDITEM_H
