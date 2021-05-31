/** @file mainwindow.h
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Mqttclient.h"
#include <QStandardItemModel>
#include <QItemSelection>
#include <QSettings>
#include "TopicHistoryItemDelegate.h"
#include "dashboarditemformdialog.h"
#include "dashboarditemwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    std::shared_ptr<Mqttclient> mqttclient;
    TopicHistoryItemDelegate* topicHistoryItemDelegate;
    QSettings settings{"xmanak20-xbreza01", "MQTT Explorer"};

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool setClient(std::shared_ptr<Mqttclient> ptr);
    void connectAction();
    ~MainWindow();

public slots:
    void newSelection(const QItemSelection &selected, const QItemSelection &deselected);
    void updateSelected();
    void disconnectAction();
    void publishAction();
    void saveButtonAction();
    void inputTypeComboBoxChanged(int index);
    void filePickerAction();
    void dashBoardEditButtonAction();
    void addDashBoardWidget(const DashboardItemData& data);

private:
    Ui::MainWindow *ui;

    void closeEvent(QCloseEvent *event) override;

};

#endif // MAINWINDOW_H
