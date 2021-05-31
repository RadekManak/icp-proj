/** @mainpage Explorer
 * This program is a MQTT client with GUI that provides structured overview of topics and allows publishing.
 *
 * Unimplemented features:
 * 		- History
 * 		- Non-text messages recognition
 * 		- File saving
 * 		- Dashboard
 */

/** @file main.cpp
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */

#include "mainwindow.h"
#include "QtWidgets"

/**
 * Main body of the program
 */
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    std::shared_ptr<Mqttclient> mqttclient = std::make_shared<Mqttclient>();
    MainWindow mainWindow;
    mainWindow.show();
    mainWindow.setClient(mqttclient);
    return QApplication::exec();
}
