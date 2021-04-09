#include "Mqttclient.h"
#include <QtGlobal>
#include <utility>
#include <sstream>

Mqttclient::Mqttclient(){
    itemModel = std::make_unique<QStandardItemModel>();
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

// Re-connection failure
void Mqttclient::on_failure(const mqtt::token &asyncActionToken) {
}

void Mqttclient::on_success(const mqtt::token &asyncActionToken) {
}

void Mqttclient::connected(const std::string& what) {
    client->subscribe(TOPIC, QOS);
}

void Mqttclient::connection_lost(const std::string& cause){
    std::cerr << "Connection lost\n";
    if (!cause.empty()){
        std::cout << "\tcause: " << cause << "\n";
    }
}

// Callback for when a message arrives.
void Mqttclient::message_arrived(mqtt::const_message_ptr msg) {
    std::stringstream s(msg->get_topic());
    std::string token;
    QStandardItem *item = itemModel->invisibleRootItem();
    while (std::getline(s, token, '/')) {
        if (item->rowCount() != 0){
            for (int i = 0; i < item->rowCount();i++){
                if (item->child(i)->text().toStdString() == token){
                    item = item->child(i);
                    break;
                } else if (i == item->rowCount()-1){
                    auto *newitem = new QStandardItem(token.c_str());
                    item->appendRow(newitem);
                    item = newitem;
                    break;
                }
            }
        } else {
            auto *newitem = new QStandardItem(token.c_str());
            item->appendRow(newitem);
            item = newitem;
        }
    }
}

bool Mqttclient::connect(const std::string& server_address, std::string server_port) {
    std::string client_id = "icp-mqtt-explorer-vut-fit";
    if (server_port.empty()){
        server_port = "1883";
    }
    client = std::make_unique<mqtt::async_client>(server_address+":"+server_port, client_id);
    client->set_callback(*this);

    try {
        std::cout << "Connecting to the MQTT server..." << std::flush;
        client->connect(connOpts, nullptr, *this)->wait();
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "\nERROR: Unable to connect to MQTT server: '"
                  << server_address << "'" << exc << std::endl;
        throw;
    }
    return true;
}

void Mqttclient::stop() {
    if (client){
        client->stop_consuming();
    }
}
