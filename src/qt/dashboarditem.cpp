#include "dashboarditem.h"
#include "ui_dashboarditem.h"

DashBoardItem::DashBoardItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DashBoardItem)
{
    ui->setupUi(this);
}

DashBoardItem::~DashBoardItem()
{
    delete ui;
}
