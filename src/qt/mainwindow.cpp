/** @file mainwindow.cpp
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */

#include "mainwindow.h"
#include <QMessageBox>
#include <utility>
#include <QSettings>
#include <QFileDialog>
#include <fstream>
#include "ui_mainwindow.h"
#include "dashboarditemwidget.h"
#include "dashboardarrangedialog.h"
#include "messageviewdialog.h"

/** Main window constructor */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->connect_button, &QPushButton::clicked, this, [&](){connectAction();});
    connect(ui->pushButton_disconnect, &QPushButton::clicked, this, [&](){disconnectAction();});
    connect(ui->pushButton_dashboard, &QPushButton::clicked, this,
            [&](){ui->stackedWidget->setCurrentWidget(ui->dashboard);});
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->pushButton_publish, &QPushButton::clicked, this, &MainWindow::publishAction);
    topicHistoryItemDelegate = new TopicHistoryItemDelegate(this);
    connect(ui->listView, &QListView::doubleClicked, this, &MainWindow::historyItemClicked);
    connect(ui->save_button, &QPushButton::clicked, this, &MainWindow::saveButtonAction);
    ui->lineEdit_host->setText(settings.value("login/hostname").toString());
    ui->lineEdit_port->setText(settings.value("login/port").toString());
    ui->lineEdit_username->setText(settings.value("login/username").toString());
    ui->lineEdit_password->setText(settings.value("login/password").toString());
    connect(ui->combobox_inputType, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::inputTypeComboBoxChanged);
    connect(ui->inputFileBrowseButton, &QPushButton::clicked, this, &MainWindow::filePickerAction);

    //Dashboard
    connect(ui->pushButton_explorer, &QPushButton::clicked, this,
            [&](){ui->stackedWidget->setCurrentWidget(ui->explorer);});
    connect(ui->pushButton_disconnect_2, &QPushButton::clicked, this, [&](){disconnectAction();});

    for (int i = 0; i<4;i++){
        ui->dashboardGridlayout->setColumnStretch(i,1);
    }
    for (int i = 0; i<5;i++){
        ui->dashboardGridlayout->setRowStretch(i,1);
    }
    connect(ui->editDashboardButton, &QToolButton::clicked, this, &MainWindow::dashBoardEditButtonAction);
}

/** Main window destructor */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * Slot for actions to be run when treeview topic changes, disconnects deselected topic signal and connects selected topic signal and updates view
 * @param selected Selected topic
 * @param deselected Deselected topic
 */
void MainWindow::newSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
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
        } else {
            ui->listView->setModel(nullptr);
        }
    }
}

/**
 * Updates explorer view with selected topic
 */
void MainWindow::updateSelected()
{
    QModelIndexList selected_indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (!selected_indexes.empty()) {
        QModelIndex item = selected_indexes.at(0);
        auto* ptr = item.data(Qt::UserRole + 1).value<Topicdata*>();
        if (ptr != nullptr){
            ui->messageView->setMessage(ptr->latest);
        } else {
            ui->messageView->setMessage(nullptr);
        }
    }
}

/**
 * Gather message information and forwawrd to client
 */
void MainWindow::publishAction()
{
    try{
        if (ui->combobox_inputType->currentIndex() == 1){
            if (ui->inputFilenameLineEdit->text().isEmpty()){
                throw std::invalid_argument("File path is empty");
            }
            std::ifstream infile(ui->inputFilenameLineEdit->text().toStdString(), std::ios::binary);
            std::string content((std::istreambuf_iterator<char>(infile)), (std::istreambuf_iterator<char>()));
            mqttclient->send_message(ui->topicLineEdit->text().toStdString(),
                                     content);
        } else {
            mqttclient->send_message(ui->topicLineEdit->text().toStdString(),
                                     ui->inputTextEdit->toPlainText().toStdString());
        }
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

/**
 * Set client
 */
bool MainWindow::setClient(std::shared_ptr<Mqttclient> client)
{
    mqttclient = std::move(client);
    ui->treeView->setModel(mqttclient->itemModel.get());
    return true;
}

/**
 * Connects GUI actions
 */
void MainWindow::connectAction()
{
    try {
        mqttclient->connect(ui->lineEdit_host->text().toStdString(), ui->lineEdit_port->text().toStdString(),
        ui->lineEdit_username->text().toStdString(), ui->lineEdit_password->text().toStdString());
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

void MainWindow::saveButtonAction(){
    settings.setValue("login/hostname", ui->lineEdit_host->text());
    settings.setValue("login/port", ui->lineEdit_port->text());
    settings.setValue("login/username", ui->lineEdit_username->text());
    settings.setValue("login/password", ui->lineEdit_password->text());
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
/**
 * Switch between text and file input widget
 */
void MainWindow::inputTypeComboBoxChanged(int index) {
    if (index == 1){
        ui->stackedWidget_input->setCurrentWidget(ui->fileInputPage);
    } else {
        ui->stackedWidget_input->setCurrentWidget(ui->textInputPage);
    }
}

/**
 * Open file picker and save resulting path
 */
void MainWindow::filePickerAction(){
    auto fileName = QFileDialog::getOpenFileName(this,
                                                            tr("Select file"));
    ui->inputFilenameLineEdit->setText(fileName);
}

void MainWindow::dashBoardEditButtonAction(){
    if (dashboardDialog == nullptr){
        dashboardDialog = new DashboardArrangeDialog(this);
        dashboardDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dashboardDialog->show();
    } else {
        dashboardDialog->raise();
        dashboardDialog->setFocus();
    }
}

void MainWindow::addDashBoardWidget(const DashboardItemData& data) {
    QStandardItem* topicItem = mqttclient->getTopicItem(mqttclient->itemModel.get(), data.stateTopic);
    if (topicItem->data().isNull()){
        QVariant variant;
        auto* topicData = new Topicdata();
        variant.setValue(topicData);
        topicItem->setData(variant);
    }
    auto* item = new DashboardItemWidget(ui->dashboardGridWidget, data, topicItem, mqttclient);
    ui->dashboardGridlayout->addWidget(item, data.row, data.column, 1, 1);
}

void MainWindow::historyItemClicked(const QModelIndex& index) {
    MessageViewDialog* messageViewDialog = new MessageViewDialog(this);
    auto* ptr = index.data(Qt::UserRole + 1).value<TopicMessage*>();
    messageViewDialog->setMessage(ptr);
    messageViewDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    messageViewDialog->show();
}
