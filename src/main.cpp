#include "mainwindow.h"
#include "Mqttclient.h"
#include "QtWidgets"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    std::shared_ptr<Mqttclient> mqttclient = std::make_shared<Mqttclient>();
    MainWindow mainWindow;
    mainWindow.show();
    mainWindow.setClient(mqttclient);
    return QApplication::exec();
}