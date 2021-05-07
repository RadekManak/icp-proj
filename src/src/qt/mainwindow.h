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
#include "TopicHistoryItemDelegate.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    std::shared_ptr<Mqttclient> mqttclient;
    TopicHistoryItemDelegate* topicHistoryItemDelegate;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool setClient(std::shared_ptr<Mqttclient> ptr);
    void connectAction();
    ~MainWindow();

public slots:
    void newSelection(const QItemSelection &selected, const QItemSelection &deselected);
    void updateSelected();

private:
    Ui::MainWindow *ui;

    void closeEvent(QCloseEvent *event) override;

    void disconnectAction();

    void publishAction();
};

#endif // MAINWINDOW_H
