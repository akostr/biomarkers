#include "flow_layout.h"

#include <QWidget>

FlowLayout::FlowLayout(QWidget* parent, int margin, int hSpacing, int vSpacing)
  : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
  setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
  : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
  setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
  QLayoutItem* item;
  while ((item = takeAt(0)))
    delete item;
}

void FlowLayout::addItem(QLayoutItem* item)
{
  itemList.append(item);
}

void FlowLayout::insertWidget(qsizetype ind, QWidget *w)
{
  if(!w)
  {
    qDebug() << "FlowLayout error:" << tr("Cannot add null widget into layout" );
    return;
  }
  if(w == parentWidget())
  {
    qDebug() << "FlowLayout error:" << tr("Cannot add parent widget to its child layout" );
    return;
  }
  if(ind < 0)
    ind = 0;
  if(ind > itemList.size())
    ind = itemList.size();
  addChildWidget(w);
  auto newItem = new QWidgetItem(w);
  itemList.insert(ind, newItem);
  invalidate();
}

int FlowLayout::horizontalSpacing() const
{
  if (m_hSpace >= 0) {
    return m_hSpace;
  }
  else {
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
  }
}

int FlowLayout::verticalSpacing() const
{
  if (m_vSpace >= 0) {
    return m_vSpace;
  }
  else {
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
  }
}

int FlowLayout::count() const
{
  return itemList.size();
}

QLayoutItem* FlowLayout::itemAt(int index) const
{
  return itemList.value(index);
}

QLayoutItem* FlowLayout::takeAt(int index)
{
  if (index >= 0 && index < itemList.size())
    return itemList.takeAt(index);
  return nullptr;
}

void FlowLayout::setSpacing(int spacing)
{
  m_hSpace = spacing;
  update();
}

int FlowLayout::rows() const
{
  return mRowItemCounts.size();
}

int FlowLayout::getItemCountForRow(int row)
{
  int items = 0;
  auto endIt = mRowItemCounts.begin();
  if (row > mRowItemCounts.size())
    endIt = mRowItemCounts.end();
  else
    std::advance(endIt, row);
  for (auto it = mRowItemCounts.begin(); it != endIt; it++)
  {
    items += *it;
  }
  return items;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
  return { };
}

bool FlowLayout::hasHeightForWidth() const
{
  return true;
}

int FlowLayout::heightForWidth(int width) const
{
  int height = doLayout(QRect(0, 0, width, 0), true);
  return height;
}

void FlowLayout::setGeometry(const QRect& rect)
{
  QLayout::setGeometry(rect);
  doLayout(rect, false);
  if(!mRowItemCounts.empty())
    emit geometryUpdated();
}

QSize FlowLayout::sizeHint() const
{
  return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
  QSize size;
  for (const QLayoutItem* item : itemList)
    size = size.expandedTo(item->minimumSize());

  const QMargins margins = contentsMargins();
  size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
  return size;
}

int FlowLayout::doLayout(const QRect& rect, bool testOnly) const
{
  int left, top, right, bottom;
  getContentsMargins(&left, &top, &right, &bottom);
  QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
  int x = effectiveRect.x();
  int y = effectiveRect.y();
  int lineHeight = 0;
  mRowItemCounts.clear();
  for (QLayoutItem* item : std::as_const(itemList))
  {
    const QWidget* wid = item->widget();
    int spaceX = horizontalSpacing();
    if (spaceX == -1)
      spaceX = wid->style()->layoutSpacing(
        QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
    int spaceY = verticalSpacing();
    if (spaceY == -1)
      spaceY = wid->style()->layoutSpacing(
        QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
    int nextX = x + item->sizeHint().width() + spaceX;
    if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
    {
      x = effectiveRect.x();
      y = y + lineHeight + spaceY;
      nextX = x + item->sizeHint().width() + spaceX;
      lineHeight = 0;
    }
    mRowItemCounts[y]++;
    if (!testOnly)
      item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

    x = nextX;
    lineHeight = qMax(lineHeight, item->sizeHint().height());
  }
  return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
  QObject* parent = this->parent();
  if (!parent) {
    return -1;
  }
  else if (parent->isWidgetType()) {
    QWidget* pw = static_cast<QWidget*>(parent);
    return pw->style()->pixelMetric(pm, nullptr, pw);
  }
  else {
    return static_cast<QLayout*>(parent)->spacing();
  }
}

