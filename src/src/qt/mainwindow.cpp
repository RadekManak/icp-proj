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
    ui->treeView->setHeaderHidden(true);
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
 * Closes any active connection on window close.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    mqttclient->stop();
    QMainWindow::closeEvent(event);
}
