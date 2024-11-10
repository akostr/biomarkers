#include "data_checkable_headerview.h"

#include <QPainter>
#include <QMouseEvent>

#include "genesis_style/style.h"

namespace
{
  const int IconSize = 16;
  const int Pad = 10;
}

namespace Control
{
  DataCheckableHeaderView::DataCheckableHeaderView(Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent)
    , IconSortAsc(":/resource/styles/genesis/qheaderview/sort_asc@2x.png")
    , IconSortDes(":/resource/styles/genesis/qheaderview/sort_desc@2x.png")
  {
    setSectionsClickable(true);
    QStyleOptionButton opt;
    opt.state = QStyle::State_Enabled | QStyle::State_Active | QStyle::State_Off;

    CheckBoxLocalRect.setSize(style()->sizeFromContents(QStyle::ContentsType::CT_CheckBox, &opt, QSize(0, 0)));
    CheckBoxLocalRect.moveTopLeft(
      (orientation == Qt::Horizontal)
      ? QPoint{ platesMargins, (height() - CheckBoxLocalRect.height()) / 2 }
    : QPoint{ (width() - CheckBoxLocalRect.width()) / 2, 0 });
    setSelectionBehavior(QAbstractItemView::SelectItems);
  }

  void DataCheckableHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
  {
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->setClipRect(rect);

    QScopeGuard iconGuard([&]() { paintSortIcon(painter, rect, logicalIndex); });

    const auto tableModel = model();
    if (!tableModel || (tableModel && !tableModel->headerData(logicalIndex, orientation(), Qt::DecorationRole).toBool()))
      return;

    painter->save();
    QScopeGuard painterGuard([&]() { painter->restore(); });

    const auto headerStyle = style();

    QStyleOptionHeaderV2 opt;
    initStyleOptionForIndex(&opt, logicalIndex);
    opt.rect = rect;

    headerStyle->drawControl(QStyle::CE_HeaderSection, &opt, painter, this);

    QStyleOptionButton option;
    option.rect = CheckBoxLocalRect;
    option.state = QStyle::State_Enabled | QStyle::State_Active;
    switch(tableModel->headerData(logicalIndex, orientation(), Qt::CheckStateRole).value<Qt::CheckState>())
    {
    case Qt::Unchecked:
      option.state |= QStyle::State_Off;
      break;
    case Qt::PartiallyChecked:
      option.state |= QStyle::State_NoChange;
      break;
    case Qt::Checked:
      option.state |= QStyle::State_On;
      break;
    }

    {
      auto cursorPos = mapFromGlobal(QCursor::pos());
      auto rect = CheckBoxLocalRect;
      const int logicalIndex = logicalIndexAt(cursorPos);
      const auto position = sectionViewportPosition(logicalIndex);
      if (orientation() == Qt::Vertical)
        rect.moveTop(position);
      else
        rect.moveLeft(position);
      if (rect.contains(cursorPos))
        option.state |= QStyle::State_MouseOver;
    }

    if (!isEnabled())
      option.state.setFlag(QStyle::State_Enabled, false);

    if (orientation() == Qt::Horizontal)
      option.rect.moveLeft(option.rect.left() + rect.left());
    else
      option.rect.moveTop(rect.top() + sectionSize(logicalIndex) / 2 - option.rect.height() / 2);

    headerStyle->drawControl(QStyle::CE_CheckBox, &option, painter, this);

    opt.rect.moveTop(option.rect.top());
    opt.rect.moveLeft(rect.left() + CheckBoxLocalRect.right());

    headerStyle->drawControl(QStyle::CE_HeaderLabel, &opt, painter, this);
  }

  void DataCheckableHeaderView::paintSortIcon(QPainter* painter, const QRect& rect, int logicalIndex) const
  {
    if (!isSortIndicatorShown())
      return;

    const auto x = rect.x() + rect.width() - Style::Scale(Pad) - IconSize;
    const auto y = rect.y() + rect.height() / 2 - IconSize / 2 - IconSize / 4;

    if (logicalIndex == sortIndicatorSection())
    {
      const QIcon* indicator = (sortIndicatorOrder() == Qt::AscendingOrder)
        ? &IconSortAsc
        : &IconSortDes;
      indicator->paint(painter, x, y, IconSize, IconSize);
    }
  }

  void DataCheckableHeaderView::mousePressEvent(QMouseEvent* event)
  {
    auto rect = CheckBoxLocalRect;
    const int logicalIndex = logicalIndexAt(event->pos());
    const auto position = sectionViewportPosition(logicalIndex);
    if (orientation() == Qt::Vertical)
      rect.moveTop(position);
    else
      rect.moveLeft(position);
    if (rect.contains(event->pos()))
    {
      if (auto tableModel = model())
      {
        auto value = tableModel->headerData(logicalIndex, orientation(), Qt::CheckStateRole).value<Qt::CheckState>();
        switch(value)
        {
        case Qt::Unchecked:
          value = Qt::Checked;
          break;
        case Qt::PartiallyChecked:
        case Qt::Checked:
          value = Qt::Unchecked;
          break;
        }

        tableModel->setHeaderData(logicalIndex, orientation(), value, Qt::CheckStateRole);
        const auto row = orientation() == Qt::Vertical ? logicalIndex : -1;
        const auto column = orientation() == Qt::Vertical ? -1 : logicalIndex;
        tableModel->setData(QModelIndex(), QPoint{ row, column }, Qt::BackgroundRole);
        repaint();
      }
    }
    else
      QHeaderView::mousePressEvent(event);
  }

  void DataCheckableHeaderView::resizeEvent(QResizeEvent* event)
  {
    QHeaderView::resizeEvent(event);
    if (orientation() == Qt::Horizontal)
      CheckBoxLocalRect.moveTop((height() - CheckBoxLocalRect.height()) * .5);
    else
      CheckBoxLocalRect.moveLeft((width() - CheckBoxLocalRect.width()) * .5);
  }
}
