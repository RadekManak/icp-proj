#pragma once
#include "mqtt/async_client.h"

class action_listener : public virtual mqtt::iaction_listener {
    std::string name_;

    void on_failure(const mqtt::token &tok) override;

    void on_success(const mqtt::token &tok) override;
public:
    explicit action_listener(std::string name);
};

class Callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener {

// Counter for the number of connection retries
    int nretry_;
    // The MQTT client
    mqtt::async_client& cli_;
    // Options to use if we need to reconnect
    mqtt::connect_options& connOpts_;
    // An action listener to display the result of actions.
    action_listener subListener_;

    void message_arrived(mqtt::const_message_ptr msg) override;

    void on_success(const mqtt::token &asyncActionToken) override;

    void on_failure(const mqtt::token &asyncActionToken) override;

    void connection_lost(const std::string& cause) override;
public:
    Callback(mqtt::async_client &cli, mqtt::connect_options &connOpts);

};

class Mqttclient {

    std::unique_ptr<mqtt::async_client> client;
    std::unique_ptr<Callback> callback;
    mqtt::connect_options connOpts;
    std::thread client_thread;

public:
    explicit Mqttclient();
    bool connect(const std::string& server_address, std::string server_port);
    bool stop();
};
