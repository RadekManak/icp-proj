#include "Mqttclient.h"
#include <QtGlobal>
#include <utility>

Mqttclient::Mqttclient(){
        connOpts = mqtt::connect_options_builder()
        .clean_session(true)
        .finalize();
}

const int QOS = 1;
const std::string TOPIC = "#";

void action_listener::on_failure(const mqtt::token& tok) {
    std::cout << name_ << " failure";
    if (tok.get_message_id() != 0)
        std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    std::cout << std::endl;
}

void action_listener::on_success(const mqtt::token& tok) {
    std::cout << name_ << " success";
    if (tok.get_message_id() != 0)
        std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    auto top = tok.get_topics();
    if (top && !top->empty())
        std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
    std::cout << std::endl;
}

action_listener::action_listener(std::string name) : name_(std::move(name)) {}

void Callback::on_failure(const mqtt::token &asyncActionToken) {
}

void Callback::on_success(const mqtt::token &asyncActionToken) {
}

void Callback::connection_lost(const std::string& cause){
    std::cerr << "Connection lost\n";
    if (!cause.empty()){
        std::cout << "\tcause: " << cause << "\n";
    }
}

// Callback for when a message arrives.
void Callback::message_arrived(mqtt::const_message_ptr msg) {
    std::cout << "Message arrived" << std::endl;
    std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
    std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
}

Callback::Callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
        : nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}

bool Mqttclient::connect(const std::string& server_address, std::string server_port) {
    std::string client_id = "icp-mqtt-explorer-vut-fit";
    if (server_port.empty()){
        server_port = "1883";
    }
    client = std::make_unique<mqtt::async_client>(server_address+":"+server_port, client_id);

    callback = std::make_unique<Callback>(*client, connOpts);
    client->set_callback(*callback);


    try {
        std::cout << "Connecting to the MQTT server..." << std::flush;
        client->connect(connOpts, nullptr, *callback)->wait();
        client->subscribe(TOPIC, QOS);
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "\nERROR: Unable to connect to MQTT server: '"
                  << server_address << "'" << exc << std::endl;
        throw;
    }
    return true;
}

bool Mqttclient::stop() {
    if (client){
        client->stop_consuming();
    }
    return true;
}
