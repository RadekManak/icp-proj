/** @file Mqttclient.cpp
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */

#include "Mqttclient.h"
#include <QtGlobal>
#include <utility>
#include <sstream>
#include <stdexcept>

/** Connection options */
Mqttclient::Mqttclient() = default;
/** Quality of service */
const int QOS = 1;
/** Root topic name */
const std::string TOPIC = "#";

/** Client callback override for when action fails */
void Mqttclient::on_failure(const mqtt::token& asyncActionToken) {}

/** Client callback override for when action succeeds */
void Mqttclient::on_success(const mqtt::token& asyncActionToken) {}

/**
 * Callback for when client connects to a server
 * @param what Connection status message
 */
void Mqttclient::connected(const std::string& what)
{
    client->subscribe(TOPIC, QOS);
}

/**
 * Callback for when client loses connection to a server
 * @param cause Cause of connection loss
 */
void Mqttclient::connection_lost(const std::string& cause)
{
    std::cerr << "Connection lost\n";
    if (!cause.empty()){
        std::cout << "\tcause: " << cause << "\n";
    }
}

/**
 * Callback for when a message arrives
 * @param msg Pointer to received message
 */
void Mqttclient::message_arrived(mqtt::const_message_ptr msg)
{
    QStandardItem* topicItem = getTopicItem(itemModel.get(), msg->get_topic());
    create_or_update_topic(*topicItem, msg);
}

/**
 * This function implements tree traversal of StandardItemModel, if item for topic does not exits it gets allocated.
 * @param model Model representing current topic tree
 * @param topic_name Topic name
 * @return modelItem for topic
 */
QStandardItem* Mqttclient::getTopicItem(QStandardItemModel *model, const std::string &topic_name)
{
    std::stringstream s(topic_name);
    std::string token;
    QStandardItem *item = model->invisibleRootItem();
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
    return item;
}

/**
 * Updates topic with new data from message
 * @param topicItem modelItem of topic
 * @param msg Message containing new data
 */
void Mqttclient::create_or_update_topic(QStandardItem& topicItem, mqtt::const_message_ptr& msg)
{
    Topicdata* topic;
    if (topicItem.data().isNull()){
        QVariant variant;
        topic = new Topicdata();
        variant.setValue(topic);
        topicItem.setData(variant);
    } else {
        topic = topicItem.data().value<Topicdata*>();
    }
    TopicMessage* message = new TopicMessage();
    message->received_time = std::chrono::system_clock::now();
    message->payload = msg->get_payload();
    if (message->payload.rfind("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 0) == 0){
        message->mime_type = "image/png";
    } else {
        message->mime_type = "text/plain";
    }
    topic->add_message(message);
}

/**
 * Connects to a specified MQTT server
 * @param server_address Server address
 * @param server_port Server port
 * @return Returns true at success
 */
bool Mqttclient::connect(const std::string& server_address, std::string server_port, const std::string& username, const std::string& password)
{
    std::string client_id = "icp-mqtt-explorer-vut-fit";
    if (server_port.empty()){
        server_port = "1883";
    }
    client = std::make_unique<mqtt::async_client>(server_address+":"+server_port, client_id);
    if (username.empty()){
        connOpts = mqtt::connect_options_builder()
            .clean_session(true)
            .finalize();
    } else {
        connOpts = mqtt::connect_options_builder()
                .clean_session(true)
                .user_name(username)
                .password(password)
                .finalize();
    }
    client->set_callback(*this);
    itemModel = std::make_unique<QStandardItemModel>();

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

/**
 * Creates message object and sends it
 * @param topic Topic of message
 * @param value Value of message
 */
void Mqttclient::send_message(const std::string& topic,const std::string& value)
{
    if(topic.empty()){
        throw std::invalid_argument("Message topic is empty");
    }
    auto msg = mqtt::make_message(topic, value);
    client->publish(msg);
}

/**
 * Stops consuming messages, removing internal callback and discarding any unread messages
 */
void Mqttclient::stop()
{
    if (client){
        client->stop_consuming();
    }
}

/**
 * Adds message to topic history
 * @param message Pointer to message object
 */
void Topicdata::add_message(TopicMessage* message)
{
    auto *messageItem = new QStandardItem();
    QVariant variant;
    auto* topicMessage = message;
    variant.setValue(topicMessage);
    messageItem->setData(variant);
    messages.appendRow(messageItem);
    latest = topicMessage;
    emit data_changed();
}
