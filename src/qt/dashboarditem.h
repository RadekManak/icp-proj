#ifndef DASHBOARDITEM_H
#define DASHBOARDITEM_H

#include <QWidget>

namespace Ui {
class DashBoardItem;
}

class DashBoardItem : public QWidget
{
    Q_OBJECT

public:
    explicit DashBoardItem(QWidget *parent = nullptr);
    ~DashBoardItem();

private:
    Ui::DashBoardItem *ui{};
};

#endif // DASHBOARDITEM_H
