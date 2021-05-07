/** @file mainwindow.cpp
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */

#include "mainwindow.h"
#include <QMessageBox>
#include <utility>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->connect_button, &QPushButton::clicked, this, [&](){connectAction();});
    connect(ui->pushButton_disconnect, &QPushButton::clicked, this, [&](){disconnectAction();});
    connect(ui->pushButton_disconnect_2, &QPushButton::clicked, this, [&](){disconnectAction();});
    connect(ui->pushButton_explorer, &QPushButton::clicked, this,
            [&](){ui->stackedWidget->setCurrentWidget(ui->explorer);});
    connect(ui->pushButton_dashboard, &QPushButton::clicked, this,
            [&](){ui->stackedWidget->setCurrentWidget(ui->dashboard);});
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->pushButton_publish, &QPushButton::clicked, this, &MainWindow::publishAction);
    topicHistoryItemDelegate = new TopicHistoryItemDelegate(this);
}

/**
 * Slot for actions to be run when treeview topic changes.
 * Disconnects deselected topic signal
 * Connects selected topic signal and updates view
 * @param selected topic
 * @param deselected topic
 */
void MainWindow::newSelection(const QItemSelection &selected, const QItemSelection &deselected) {
    QModelIndexList deselected_indexes = deselected.indexes();
    if (!deselected_indexes.empty()){
        QModelIndex item = deselected_indexes.at(0);
        auto* ptr = item.data(Qt::UserRole + 1).value<Topicdata*>();
        if (ptr != nullptr){
            disconnect(ptr, &Topicdata::data_changed, this, &MainWindow::updateSelected);
        }
    }
    QModelIndexList selected_indexes = selected.indexes();
    if (!selected_indexes.empty()) {
        QModelIndex item = selected_indexes.at(0);
        auto* ptr = item.data(Qt::UserRole + 1).value<Topicdata*>();
        updateSelected();
        if (ptr != nullptr){
            ui->listView->setModel(&ptr->messages);
            ui->listView->setItemDelegate(topicHistoryItemDelegate);
            // Build topic path by traversing tree to root node
            std::string topic_path;
            if (item.isValid()){
                topic_path = item.data(Qt::DisplayRole).toString().toStdString();
                item = item.parent();
                while (item.isValid()){
                    topic_path = item.data(Qt::DisplayRole).toString().toStdString() + '/' + topic_path;
                    item = item.parent();
                }
            }
            ui->topicLineEdit->setText(topic_path.data());
            connect(ptr, &Topicdata::data_changed, this, &MainWindow::updateSelected);
        }
    }
}

/**
 * Updates explorer view with selected topic
 */
void MainWindow::updateSelected(){
    QModelIndexList selected_indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (!selected_indexes.empty()) {
        QModelIndex item = selected_indexes.at(0);
        auto* ptr = item.data(Qt::UserRole + 1).value<Topicdata*>();
        if (ptr != nullptr){
            ui->viewLabel->setText(QString(ptr->latest->payload.data()));
        } else {
            ui->viewLabel->setText("");
        }
    }
}

/**
 * Gather message information and forwawrd to client
 */
void MainWindow::publishAction(){
    try{
        mqttclient->send_message(ui->topicLineEdit->text().toStdString(),
                                 ui->sendTextEdit->toPlainText().toStdString());
    } catch (mqtt::exception& error) {
        QMessageBox errorBox;
        std::string message = std::string("Error sending message\n") + error.what();
        errorBox.setText(message.c_str());
        errorBox.setIcon(QMessageBox::Critical);
        errorBox.exec();
    } catch (std::invalid_argument& error){
        QMessageBox errorBox;
        std::string message = std::string("Error sending message\n") + error.what();
        errorBox.setText(message.c_str());
        errorBox.setIcon(QMessageBox::Critical);
        errorBox.exec();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::setClient(std::shared_ptr<Mqttclient> client) {
    mqttclient = std::move(client);
    ui->treeView->setModel(mqttclient->itemModel.get());
    return true;
}

void MainWindow::connectAction() {
    try {
        mqttclient->connect(ui->lineEdit_host->text().toStdString(), ui->lineEdit_port->text().toStdString());
        ui->treeView->setModel(mqttclient->itemModel.get());
        connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::newSelection);
        ui->stackedWidget->setCurrentWidget(ui->explorer);
    } catch (mqtt::exception& error){
        QMessageBox errorBox;
        std::string message = std::string("Unable to connect to MQTT server\n") + error.what();
        errorBox.setText(message.c_str());
        errorBox.setIcon(QMessageBox::Critical);
        errorBox.exec();
    }
}

/**
 * Closes any active connection and switches back to login view.
 */
void MainWindow::disconnectAction()
{
    mqttclient->stop();
    ui->stackedWidget->setCurrentWidget(ui->login);
}
/**
 * Closes any active connection on window close.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    mqttclient->stop();
    QMainWindow::closeEvent(event);
}
