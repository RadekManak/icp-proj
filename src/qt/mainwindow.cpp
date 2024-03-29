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
    Q_INIT_RESOURCE(resources);

    // Load demo dashboard if config file does not exist
    QFile file(dashboardSettings.fileName());
    if (!file.exists()){
        QFile demo_dashboard(":/demo-dashboard.conf");
        demo_dashboard.open(QIODevice::ReadOnly);
        file.open(QIODevice::WriteOnly);
        file.write(demo_dashboard.readAll());
        file.flush();
        dashboardSettings.sync();
    }
    file.close();

    connect(ui->connect_button, &QPushButton::clicked, this, [&](){connectAction();});
    connect(ui->pushButton_disconnect, &QPushButton::clicked, this, [&](){disconnectAction();});
    connect(ui->pushButton_dashboard, &QPushButton::clicked, this,
            [&](){ui->stackedWidget->setCurrentWidget(ui->dashboard);});
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->pushButton_publish, &QPushButton::clicked, this, &MainWindow::publishAction);
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

    dashboardModel = std::make_shared<QStandardItemModel>();
    dashboardModel->setRowCount(10);
    dashboardModel->setColumnCount(4);
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
        loadDashboard();
    } catch (mqtt::exception& error){
        QMessageBox errorBox;
        std::string message = std::string("Unable to connect to MQTT server\n") + error.what();
        errorBox.setText(message.c_str());
        errorBox.setIcon(QMessageBox::Critical);
        errorBox.exec();
    }
}

/**
 * Stores login data in configuration file
 */
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

/**
 * Create or raise dashboard edit dialog
 */
void MainWindow::dashBoardEditButtonAction(){
    if (dashboardDialog == nullptr){
        dashboardDialog = new DashboardArrangeDialog(this, dashboardModel);
        dashboardDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dashboardDialog->show();
    } else {
        dashboardDialog->raise();
        dashboardDialog->setFocus();
    }
}

/**
 * Add new widget to dashboard layout or replace old
 * @param data for widget
 */
void MainWindow::addDashBoardWidget(const DashboardItemData& data) {
    QStandardItem* topicItem = mqttclient->getTopicItem(mqttclient->itemModel.get(), data.stateTopic);
    if (topicItem->data().isNull()){
        QVariant variant;
        auto* topicData = new Topicdata();
        variant.setValue(topicData);
        topicItem->setData(variant);
    }
    auto* item = new DashboardItemWidget(ui->dashboardGridWidget, data, topicItem, mqttclient);
    if (ui->dashboardGridlayout->itemAtPosition(data.row, data.column) != nullptr){
        auto item = ui->dashboardGridlayout->itemAtPosition(data.row, data.column);
        ui->dashboardGridlayout->removeItem(item);
        item->widget()->deleteLater();
    }
    ui->dashboardGridlayout->addWidget(item, data.row, data.column, 1, 1);
}

/**
 * Remove widget from layout and delete it
 * @param row
 * @param collumn
 */
void MainWindow::removeDashboardWidget(int row, int collumn){
    if (ui->dashboardGridlayout->itemAtPosition(row, collumn) != nullptr){
        auto item = ui->dashboardGridlayout->itemAtPosition(row, collumn);
        ui->dashboardGridlayout->removeItem(item);
        item->widget()->deleteLater();
    }
}

/**
 * Open dialog with full message
 * @param index
 */
void MainWindow::historyItemClicked(const QModelIndex& index) {
    MessageViewDialog* messageViewDialog = new MessageViewDialog(this);
    auto* ptr = index.data(Qt::UserRole + 1).value<TopicMessage*>();
    messageViewDialog->setMessage(ptr);
    messageViewDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    messageViewDialog->show();
}

/**
 * Load full dashboard from configuration file
 */
void MainWindow::loadDashboard(){
    DashboardItemData* data;
    for (const auto& group: dashboardSettings.childGroups()){
        data = new DashboardItemData();
        dashboardSettings.beginGroup(group);
        auto position = group.split('-');
        data->row = position[0].toUInt();
        data->column = position[1].toUInt();
        data->name = dashboardSettings.value("name").toString().toStdString();
        data->type = dashboardSettings.value("type").toString().toStdString();

        data->onOffType = dashboardSettings.value("onOffType").toString().toStdString();
        data->stateTopic = dashboardSettings.value("stateTopic").toString().toStdString();
        data->offStateMessage = dashboardSettings.value("offStateMessage").toString().toStdString();
        data->onStateMessage = dashboardSettings.value("onStateMessage").toString().toStdString();
        data->controllable = dashboardSettings.value("controllable").toBool();
        data->controlTopic = dashboardSettings.value("controlTopic").toString().toStdString();
        data->turnOffCommand = dashboardSettings.value("turnOffCommand").toString().toStdString();
        data->turnOnCommand = dashboardSettings.value("turnOnCommand").toString().toStdString();
        dashboardSettings.endGroup();
        auto *item = new QStandardItem(data->name.data());
        QVariant variant;
        variant.setValue(data);
        item->setData(variant, Qt::UserRole+1);
        dashboardModel->setItem(data->row, data->column, item);
        addDashBoardWidget(*data);
    }
}

/**
 * Save dashboard widget settings to configuration file
 * @param data
 */
void MainWindow::saveDashboardItemSettings(DashboardItemData data) {
    auto group = std::to_string(data.row) + "-" + std::to_string(data.column);
    dashboardSettings.beginGroup(group.data());
    dashboardSettings.setValue("name", data.name.data());
    dashboardSettings.setValue("type", data.type.data());

    dashboardSettings.setValue("onOffType", data.onOffType.data());
    dashboardSettings.setValue("stateTopic", data.stateTopic.data());
    dashboardSettings.setValue("offStateMessage", data.offStateMessage.data());
    dashboardSettings.setValue("onStateMessage", data.onStateMessage.data());
    dashboardSettings.setValue("controllable", data.controllable);
    dashboardSettings.setValue("controlTopic", data.controlTopic.data());
    dashboardSettings.setValue("turnOffCommand", data.turnOffCommand.data());
    dashboardSettings.setValue("turnOnCommand", data.turnOnCommand.data());
    dashboardSettings.endGroup();
}

/**
 * Remove dashboard widget data from configuration file
 * @param row
 * @param column
 */
void MainWindow::removeDashboardItemSettings(int row, int column) {
    auto group = std::to_string(row) + "-" + std::to_string(column);
    dashboardSettings.beginGroup(group.data());
    dashboardSettings.remove("");
    dashboardSettings.endGroup();
}

/**
 * Get pointer to MainWindow
 * @return MainWindow*
 */
MainWindow *MainWindow::getMainWindow() {
    for (QWidget * window: QApplication::topLevelWidgets()){
        if (auto* mainWindow = qobject_cast<MainWindow*>(window))
            return mainWindow;
    }
    return nullptr;
}
