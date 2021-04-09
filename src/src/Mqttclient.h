#pragma once
#include "mqtt/async_client.h"
#include "QStandardItemModel"

class action_listener : public virtual mqtt::iaction_listener {
    std::string name_;

    void on_failure(const mqtt::token &tok) override;

    void on_success(const mqtt::token &tok) override;
public:
    explicit action_listener(std::string name);
};

class Mqttclient : public virtual mqtt::callback, public virtual mqtt::iaction_listener, public virtual QObject{
    std::unique_ptr<mqtt::async_client> client;
    mqtt::connect_options connOpts;
    std::thread client_thread;

public:
    std::unique_ptr<QStandardItemModel> itemModel;
    explicit Mqttclient();
    bool connect(const std::string& server_address, std::string server_port);
    void stop();

    void message_arrived(mqtt::const_message_ptr msg) override;
    void on_success(const mqtt::token &asyncActionToken) override;
    void on_failure(const mqtt::token &asyncActionToken) override;
    void connection_lost(const std::string& cause) override;
    void connected(const std::string &what) override;
};
