#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Mqttclient.h"
#include <QStandardItemModel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    std::shared_ptr<Mqttclient> mqttclient;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool setClient(std::shared_ptr<Mqttclient>);
    void connectAction();
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
