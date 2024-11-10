#include "checkable_marker_headerview.h"
#include "common_table_view.h"
#include <QPainter>

CheckableMarkerHeaderView::CheckableMarkerHeaderView(Qt::Orientation orientation, QWidget * parent = nullptr)
  : CheckableHeaderView(orientation, parent)
{

}

void CheckableMarkerHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
  bool isMarker, isCheckable;
  if(!model())
  {
    QHeaderView::paintSection(painter, rect, logicalIndex);
    return;
  }
  else
  {
    isMarker = model()->headerData(logicalIndex, orientation(), CommonTableView::MarkerFlagRole).toBool();
    isCheckable = model()->headerData(logicalIndex, orientation(), CommonTableView::CheckableRole).toBool();

    if(!isMarker)
    {
      if(isCheckable)
      {
        CheckableHeaderView::paintSection(painter, rect, logicalIndex);
        return;
      }
      else
      {
        QHeaderView::paintSection(painter, rect, logicalIndex);
        return;
      }
    }
  }


  //calc current marker plate
  QRect plateRect;
  if(orientation() == Qt::Horizontal)
  {
    plateRect.setHeight(height()/3);
    plateRect.setWidth(height()/3);
    plateRect.moveTop(height()/3);
    if(isCheckable && !isCheckBoxesHidden)
      plateRect.moveLeft(rect.left() + CheckBoxLocalRect.right() + platesMargins);
    else
      plateRect.moveLeft(rect.left() + platesMargins);
  }
  else
  {//TODO this is just a stub
    plateRect.setHeight(width()/3);
    plateRect.setWidth(width()/3);
    plateRect.moveLeft(width()/3);
    plateRect.moveBottom(rect.bottom() - platesMargins);
  }

  //draw section with label offset
  defaultPaintWithLabelOffset(painter, rect, logicalIndex, QPoint(plateRect.right() - rect.left() + platesMargins, 0));

  //draw checkbox if needed
  if(isCheckable && !isCheckBoxesHidden)
  {
    QStyleOptionButton checkBoxOption = calcCheckboxOpt(rect, logicalIndex);
    style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter, this);
  }

  //draw marker plate
  if(isMarker)
  {
    painter->setPen(Qt::black);
    QBrush brush(model()->headerData(logicalIndex, orientation(), Qt::DecorationRole).value<QColor>());
    painter->setBrush(brush);
    painter->drawRect(plateRect);
  }
}


QSize CheckableMarkerHeaderView::sectionSizeFromContents(int logicalIndex) const
{
  auto sz = CheckableHeaderView::sectionSizeFromContents(logicalIndex);
  if(!model())
    return sz;
  if(!model()->headerData(logicalIndex, orientation(), CommonTableView::MarkerFlagRole).toBool())
    return sz;
  if(orientation() == Qt::Horizontal)
    sz.setWidth(sz.width() + height()/3);
  else
    sz.setHeight(sz.height() + width()/3);
  return sz;
}
