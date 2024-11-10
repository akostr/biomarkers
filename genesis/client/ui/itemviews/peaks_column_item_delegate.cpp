#include "peaks_column_item_delegate.h"
#include "common_table_view.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>

void PeaksColumnItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  auto colorData = index.data(Qt::DecorationRole);
//  QStyleOptionViewItem opt = option;
//  this->initStyleOption(&opt, index);
//  const QWidget *widget = opt.widget;
//  QStyle *style = widget ? widget->style() : QApplication::style();

  if(colorData.isValid() && !colorData.isNull())
  {
    painter->save();

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.window());
    else
      painter->fillRect(option.rect, Qt::white);
    QRect rect = option.rect;
    rect.moveLeft(option.fontMetrics.height() + 
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    option.fontMetrics.width(" ") 
#else
    option.fontMetrics.horizontalAdvance(" ") 
#endif
    * 2);
    painter->drawText(rect, option.displayAlignment, index.data(Qt::DisplayRole).toString());

    auto textRect = QRect(0, 0, option.fontMetrics.height(), option.fontMetrics.height());
    textRect.moveLeft(option.rect.left());
    textRect.moveCenter(QPoint(textRect.center().x() + 
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    option.fontMetrics.width(" ") 
#else
    option.fontMetrics.horizontalAdvance(" ") 
#endif
    , option.rect.center().y()));
    painter->setBackground(Qt::red);
    painter->setBrush(colorData.value<QColor>());
    painter->setPen(Qt::black);
    painter->drawRect(textRect);

    painter->restore();
  }
  else
  {
    QStyledItemDelegate::paint(painter, option, index);
  }
}

QSize PeaksColumnItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto colorData = index.data(Qt::DecorationRole);
    auto size = QStyledItemDelegate::sizeHint(option, index);
    if(colorData.isValid() && !colorData.isNull())
    {
      size.setWidth(size.width() + option.fontMetrics.height() + 
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    option.fontMetrics.width(" ") 
#else
    option.fontMetrics.horizontalAdvance(" ") 
#endif
      * 2);
    }
    return size;
}
