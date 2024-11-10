#include "tableview_headerview.h"

#include "tableview_headerview_filter_list.h"
#include "tableview_headerview_filter_popup.h"
#include "ui/flow_layout.h"

#include <genesis_style/style.h>

#include <QMouseEvent>
#include <QWidgetAction>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QResizeEvent>
#include <QApplication>

#include <set>

namespace Details
{
  const int IconSize = 16;
  const int Pad = 10;
}

////////////////////////////////////////////////////
//// Table view header
TableViewHeaderView::TableViewHeaderView(Qt::Orientation orientation, QWidget* parent)
  : QHeaderView(orientation, parent)
  , IconFilter(":/resource/styles/genesis/qheaderview/filter@2x.png")
  , IconSortAsc(":/resource/styles/genesis/qheaderview/sort_asc@2x.png")
  , IconSortDes(":/resource/styles/genesis/qheaderview/sort_desc@2x.png")
{
  setSectionsClickable(true);
  setHighlightSections(true);
  setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  FilterList = new TableViewHeaderViewFilterList(viewport());
  FilterList->SetHeaderSectionPositionAccessFunction([&](int logicalIndex)->int
    { return sectionViewportPosition(logicalIndex); });
  FilterList->SetHeaderSectionWidthAccessFunction([&](int logicalIndex)->int
    { return sectionSize(logicalIndex); });

  connect(this, &TableViewHeaderView::sectionCountChanged, FilterList, &TableViewHeaderViewFilterList::OnSectionCountChanged);
  connect(this, &TableViewHeaderView::sectionResized, FilterList, &TableViewHeaderViewFilterList::OnSectionSizeChanged);
  connect(this, &TableViewHeaderView::geometriesChanged, FilterList, &TableViewHeaderViewFilterList::OnHeaderResize);

  FilterList->OnSectionCountChanged(0, this->count());
  FilterList->OnHeaderResize();

  Layout();
}

//// Impls
void TableViewHeaderView::mousePressEvent(QMouseEvent* event)
{
  const int position = orientation() == Qt::Horizontal ? event->x() : event->y();
  const int logicalIndex = logicalIndexAt(position);
  //// Ignore last section clicks (@todo - depending on item delegate?)
  if (sectionsClickable())
  {
    if (logicalIndex == count() - 1)
    {
      event->accept();
      return;
    }
  }

  //// Handle click in extras
  int pad = Style::Scale(Details::Pad);
  if (logicalIndex != -1)
  {
    int extrasWidth = GetExtrasWidthFilter(logicalIndex);
    if (extrasWidth)
    {
      int sectionPosition = sectionViewportPosition(logicalIndex);
      int sectionSize = QHeaderView::sectionSize(logicalIndex);
      if (position > sectionPosition + sectionSize - extrasWidth - pad
        && position < sectionPosition + sectionSize - pad)
      {
        if (auto m = model())
        {
          if (auto p = qobject_cast<TreeModelPresentation*>(m))
          {
            if (auto t = qobject_cast<TreeModel*>(p->sourceModel()))
            {
              //// Filter
              if (MenuFilter)
                delete MenuFilter;

              //// Create
              MenuFilter = new QMenu(this);

              //// Fill
              {
                QWidgetAction* widgetAction = new QWidgetAction(MenuFilter);
                FilterPopup* fp = new FilterPopup(MenuFilter, logicalIndex, p, t);
                widgetAction->setDefaultWidget(fp);
                MenuFilter->addAction(widgetAction);
              }

              //// Run
              QPoint global = mapToGlobal(QPoint(sectionPosition + sectionSize - extrasWidth - pad, rect().y() + rect().height()));
              MenuFilter->popup(global);
            }
          }
        }
      }
    }
  }

  return QHeaderView::mousePressEvent(event);
}

void TableViewHeaderView::paintSection(QPainter* painter, const QRect& rectArg, int logicalIndex) const
{
  QHeaderView::paintSection(painter, rectArg, logicalIndex);

  painter->setClipRect(rectArg);
  PaintExtras(painter, rectArg, logicalIndex);
}

QSize TableViewHeaderView::sectionSizeFromContents(int logicalIndex) const
{
  QSize sz = QHeaderView::sectionSizeFromContents(logicalIndex);
  sz.setWidth(sz.width() + Details::IconSize);
  return sz;
}

void TableViewHeaderView::setModel(QAbstractItemModel* model)
{
  QHeaderView::setModel(model);
  FilterList->SetModel(model);
}

void TableViewHeaderView::updateGeometries()
{
  QHeaderView::updateGeometries();
  Layout();
}

void TableViewHeaderView::resizeEvent(QResizeEvent* event)
{
  QHeaderView::resizeEvent(event);
  Layout();
}

void TableViewHeaderView::Layout()
{
  if (FilterList->isVisible())
  {
    int fh = GetFilterListHeight();
    FilterList->setGeometry(0, height() - fh, width(), fh);
  }
}

int TableViewHeaderView::GetExtrasWidth(int logicalIndex) const
{
  return GetExtrasWidthSortIndicator(logicalIndex)
    + GetExtrasWidthFilter(logicalIndex);
}

int TableViewHeaderView::GetExtrasWidthSortIndicator(int logicalIndex) const
{
  return Details::IconSize;
}

int TableViewHeaderView::GetExtrasWidthFilter(int logicalIndex) const
{
  if (auto m = model())
  {
    if (auto p = qobject_cast<TreeModelPresentation*>(m))
    {
      if (auto tm = qobject_cast<TreeModel*>(p->sourceModel()))
      {
        auto columnInfo = tm->GetColumnInfo(logicalIndex);
        if (columnInfo.Filter != ColumnInfo::FilterTypeNone)
        {
          return Details::IconSize;
        }
      }
    }
  }
  return 0;
}

void TableViewHeaderView::PaintExtras(QPainter* painter, const QRect& rect, int logicalIndex) const
{
  const auto x = rect.x() + rect.width() - Style::Scale(Details::Pad) - Details::IconSize;
  const auto y = rect.y() + rect.height() / 2 - Details::IconSize / 2 - Details::IconSize / 4;

  if (logicalIndex == sortIndicatorSection())
  {
    const auto offset = (GetExtrasWidthFilter(logicalIndex) != 0) ? Details::IconSize : 0;
    const QIcon* indicator = (sortIndicatorOrder() == Qt::AscendingOrder)
      ? &IconSortAsc
      : &IconSortDes;
    indicator->paint(painter, x - offset, y, Details::IconSize, Details::IconSize);
  }

  if (GetExtrasWidthFilter(logicalIndex))
  {
    IconFilter.paint(painter, x, y, Details::IconSize, Details::IconSize);
  }
}

int TableViewHeaderView::GetFilterListHeight() const
{
  if (FilterList->isVisible())
    return FilterList->GetHeight();
  return 0;
}
