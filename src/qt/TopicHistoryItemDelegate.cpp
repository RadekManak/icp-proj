/**
 *  https://stackoverflow.com/questions/53105343/is-it-possible-to-add-a-custom-widget-into-a-qlistview
 *  This file is based on scopchanov's response
 *
 *  @file TopicHistoryItemDelegate.cpp
 *  @author Radek Manak (xmanak20)
 *  @author Branislav Brezani (xbreza01)
 */

#include "TopicHistoryItemDelegate.h"
#include "messageviewdialog.h"
#include <QLabel>
#include <QPainter>
#include <Mqttclient.h>
#include <QDialog>
#include <QTextEdit>

void TopicHistoryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    const QPalette &palette(opt.palette);
    const QRect &rect(opt.rect);
    const QRect &contentRect(rect.adjusted(100,100,100,100));
    const bool lastIndex = (index.model()->rowCount() - 1) == index.row();
    const int bottomEdge = rect.bottom();
    QFont f(opt.font);

    f.setPointSize(1);

    painter->save();
    painter->setClipping(true);
    painter->setClipRect(rect);
    painter->setFont(opt.font);

    // Draw background
    painter->fillRect(rect, opt.state & QStyle::State_Selected ?
                            palette.highlight().color() :
                            palette.base().color());

    // Draw bottom line
    painter->setPen(lastIndex ? palette.dark().color()
                              : palette.mid().color());
    painter->drawLine(lastIndex ? rect.left() : 0,
                      bottomEdge, rect.right(), bottomEdge);

    painter->setFont(opt.font);
    painter->setPen(palette.text().color());
    TopicMessage* message = index.data(Qt::UserRole+1).value<TopicMessage*>();
    if (message != nullptr){
        if (message->mime_type == "image/png"){
            painter->drawText(rect, "Image");
        } else {
            painter->drawText(rect, message->payload.data());
        }
    }

    painter->restore();
}

QSize TopicHistoryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

TopicHistoryItemDelegate::TopicHistoryItemDelegate(QObject *parent)
{
    setParent(parent);
}
