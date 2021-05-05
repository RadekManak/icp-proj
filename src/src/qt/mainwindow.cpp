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
}

/**
 * Slot for actions to be run when treeview topic changes.
 * @param selected topic
 * @param deselected topic
 */
void MainWindow::newSelection(const QItemSelection &selected, const QItemSelection &deselected) {
    QModelIndexList indexes = selected.indexes();
    if (!indexes.empty()) {
        Topicdata* ptr = indexes.at(0).data(Qt::UserRole + 1).value<Topicdata*>();
        if (ptr != nullptr){
            ui->valueTextEdit->setPlainText(QString(ptr->value.data()));
        } else {
            ui->valueTextEdit->setPlainText("");
        }
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
