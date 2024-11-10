#include "datatable_item_delegate.h"
#include <QApplication>
#include <QPainter>

DataTableItemDelegate::DataTableItemDelegate(QObject *parent)
  :QStyledItemDelegate(parent)
{

}

void DataTableItemDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
  Q_ASSERT(index.isValid());

  auto op = option;

  op.showDecorationSelected = true;

  //for proper working, styles for [QTree/QTable]View have to miss the background and color(text color) settings
  if(!op.state.testFlag(QStyle::State_Selected))
  {
    if(index.data(Qt::ForegroundRole).isValid() && !index.data(Qt::ForegroundRole).isNull())
      op.palette.setColor(QPalette::Text, index.data(Qt::ForegroundRole).value<QColor>());

    //for proper working, styled background must be "none" or "transparent"!!!
    if(index.data(Qt::BackgroundRole).isValid() && !index.data(Qt::BackgroundRole).isNull())
    {
      QColor color = index.data(Qt::BackgroundRole).value<QColor>();
      painter->fillRect(op.rect/*.marginsAdded(QMargins(-1,-1,-1,-1))*/, color);
    }
  }

  QStyledItemDelegate::paint(painter, op, index);
}
