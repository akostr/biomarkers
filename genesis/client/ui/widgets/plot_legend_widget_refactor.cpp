#include "plot_legend_widget_refactor.h"
#include <ui/flow_layout.h>
#include <ui/widgets/legend/legend_shape_item.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QEvent>

namespace
{
  static const char* kUidProperty = "uid";
  const static QString collapseDown(":/resource/controls/collpse_dn.png");
  const static QString collapseUp(":/resource/controls/collpse_up.png");
}

PlotLegendWidgetRefactor::PlotLegendWidgetRefactor(QWidget* parent)
  : QWidget{ parent },
  mSpacing(5)
{
  setupUi();
}

PlotLegendWidgetRefactor::~PlotLegendWidgetRefactor()
{
}

void PlotLegendWidgetRefactor::setupUi()
{
  mRootLayout = new QVBoxLayout(this);
  setLayout(mRootLayout);
}

int PlotLegendWidgetRefactor::addLayout(const QList<LegendItem>& items)
{
  auto subLayout = makeSubLayout(items);
  mLayouts << subLayout;
  mRootLayout->addLayout(subLayout);
  return mLayouts.size() - 1;
}

int PlotLegendWidgetRefactor::insertLayout(int ind, const QList<LegendItem>& items)
{
  auto layout = makeSubLayout(items);
  auto iter = mLayouts.insert(ind, layout);
  mRootLayout->insertLayout(ind, layout);
  return std::distance(mLayouts.begin(), iter);
}

bool PlotLegendWidgetRefactor::removeLayout(int ind)
{
  if (ind < 0 || ind >= mLayouts.size())
    return false;
  auto layout = mLayouts.takeAt(ind);
  mRootLayout->takeAt(ind);
  clearLayout(layout);
  layout->deleteLater();
  return true;
}

bool PlotLegendWidgetRefactor::replaceLayout(int ind, const QList<LegendItem>& items)
{
  if (ind < 0 || ind >= mLayouts.size())
    return false;
  auto subLayout = mLayouts.at(ind);
  if (mFlowLayout)
  {
    clearLayout(mFlowLayout);
    fillFlowLayout(items);
    return true;
  }
  return false;
}

bool PlotLegendWidgetRefactor::addItem(int ind, const LegendItem& item, bool prepend)
{
  if (ind < 0 || ind >= mLayouts.size())
    return false;
  auto subLayout = mLayouts.at(ind);
  if (auto flowLayout = subLayout->findChild<FlowLayout*>())
  {
    fillFlowLayout({ item }, prepend);
    return true;
  }
  return false;
}

bool PlotLegendWidgetRefactor::removeItem(int ind, QUuid& uid)
{
  if (ind < 0 || ind >= mLayouts.size())
    return false;
  auto subLayout = mLayouts.at(ind);
  if (auto flowLayout = subLayout->findChild<FlowLayout*>())
  {
    QSet<QWidget*> widgetsToRemove;
    for (int i = 0; i < flowLayout->count(); i++)
    {
      auto item = flowLayout->itemAt(i);
      if (auto widget = item->widget())
      {
        if (widget->property(kUidProperty).toUuid() == uid)
          widgetsToRemove << widget;
      }
    }
    for (auto iter : mItemWidgets.asKeyValueRange())
    {
      for (auto& w : iter.second)
      {
        if (widgetsToRemove.contains(w))
          iter.second.removeAll(w);
      }
      if (iter.second.isEmpty())
        mItemWidgets.remove(iter.first);
    }
    for (auto& wgt : widgetsToRemove)
      wgt->deleteLater();
    return true;
  }
  return false;
}

void PlotLegendWidgetRefactor::setToolTip(QUuid &uid, const QString &tooltip)
{
  auto widgets = mItemWidgets.value(uid, {});
  for (auto& w : widgets)
    w->setToolTip(tooltip);
}

bool PlotLegendWidgetRefactor::setShape(const QUuid& uid, QPainterPath shape)
{
  auto widgets = mItemWidgets.value(uid, {});
  for (auto& w : widgets)
    w->setShape(shape);
  return true;
}

bool PlotLegendWidgetRefactor::setColor(const QUuid& uid, QColor color)
{
  auto widgets = mItemWidgets.value(uid, {});
  for (auto& w : widgets)
    w->setShapeBrush(color);
  return true;
}

bool PlotLegendWidgetRefactor::setTitle(const QUuid& uid, QString title)
{
  auto widgets = mItemWidgets.value(uid, {});
  for (auto& w : widgets)
    w->setText(title);
  return true;
}

void PlotLegendWidgetRefactor::clear()
{
  for (auto& layout : mLayouts)
  {
    clearLayout(layout);
    layout->deleteLater();
  }
  mItemWidgets.clear();
  mLayouts.clear();
}

void PlotLegendWidgetRefactor::collapse()
{
  if(mIsCollapsed)
    return;
  auto count = mFlowLayout->getItemCountForRow(mMinimumVisibleRows);
  int i = 0;
  for (; i < count; i++)
    mFlowLayout->itemAt(i)->widget()->setVisible(true);
  for (; i < mFlowLayout->count(); i++)
    mFlowLayout->itemAt(i)->widget()->setVisible(false);
  mIsCollapsed = true;
  if(mHideBtn)
  {
    mHideBtn->setText(tr("show"));
    mHideBtn->setIcon(QIcon(collapseDown));
  }
}

void PlotLegendWidgetRefactor::expand()
{
  if(!mIsCollapsed)
    return;
  for(int i = 0; i < mFlowLayout->count(); i++)
  {
    mFlowLayout->itemAt(i)->widget()->setVisible(true);
  }
  mIsCollapsed = false;
  if(mHideBtn)
  {
    mHideBtn->setText(tr("hide"));
    mHideBtn->setIcon(QIcon(collapseUp));
  }
}

void PlotLegendWidgetRefactor::clearLayout(QPointer<FlowLayout> layout)
{
  QSet<QWidget*> widgetsToRemove;
  for (int i = 0; i < layout->count(); i++)
  {
    auto item = layout->itemAt(i);
    if (auto widget = item->widget())
      widgetsToRemove << widget;
  }
  for (auto iter : mItemWidgets.asKeyValueRange())
  {
    for (auto& w : iter.second)
    {
      if (widgetsToRemove.contains(w))
        iter.second.removeAll(w);
    }
    if (iter.second.isEmpty())
      mItemWidgets.remove(iter.first);
  }
  for (auto& wgt : widgetsToRemove)
    wgt->deleteLater();
}

void PlotLegendWidgetRefactor::clearLayout(QPointer<QVBoxLayout> layout)
{
  std::function<void(QObject*, QSharedPointer<TreeItem> node)> scanRecursive = [&scanRecursive](QObject* obj, QSharedPointer<TreeItem> parentNode)
  {
    parentNode->className = obj->metaObject()->className();
    if(!obj)
      return;
    for(auto& child : obj->children())
    {
      QSharedPointer<TreeItem> node(new TreeItem());
      parentNode->nodes << node;
      scanRecursive(child, node);
    }
  };

  {
    QSharedPointer<TreeItem> rootItem(new TreeItem());
    scanRecursive(this, rootItem);
    qDebug() << "Hi folks";
  }
  auto hl = layout->findChild<QHBoxLayout*>();
  if (hl)
    clearLayout(hl);

  auto fl = layout->findChild<FlowLayout*>();
  if (fl)
    clearLayout(fl);
  layout.clear();
}

void PlotLegendWidgetRefactor::clearLayout(QPointer<QHBoxLayout> layout)
{
  for (int i = 0; i < layout->count(); i++)
  {
    auto item = layout->itemAt(i);
    if (auto widget = item->widget())
      widget->deleteLater();
  }
  layout.clear();
}

QPointer<QVBoxLayout> PlotLegendWidgetRefactor::makeSubLayout(const QList<LegendItem>& items)
{
  auto verticalLayout = new QVBoxLayout();
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  mFlowLayout = new FlowLayout(nullptr, 0, mSpacing, mSpacing);
  fillFlowLayout(items);
  auto footerLayout = new QHBoxLayout();
  footerLayout->setContentsMargins(0, 0, 0, 0);
  mHideBtn = QPointer<QPushButton>(new QPushButton(QIcon(collapseUp), tr("hide"), this));
  mHideBtn->setFlat(true);
  footerLayout->addWidget(mHideBtn);
  footerLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Maximum));
  verticalLayout->addLayout(mFlowLayout);
  verticalLayout->addLayout(footerLayout);
  connect(mHideBtn, &QPushButton::clicked, this, &PlotLegendWidgetRefactor::toggleCollapsing);
  connect(mFlowLayout, &FlowLayout::geometryUpdated, this,
    [this]()
    {
      if (!mHideBtn || !mFlowLayout)
        return;
      if (mFlowLayout->rows() > mMinimumVisibleRows || mIsCollapsed)
        mHideBtn->setVisible(true);
      else
        mHideBtn->setVisible(false);
    });
  return verticalLayout;
}

void PlotLegendWidgetRefactor::fillFlowLayout(const QList<LegendItem>& items, bool prepend)
{
  for (auto item : items)
  {
    auto wgt = new LegendShapeItem(this);
    wgt->setProperty(kUidProperty, item.itemId);
    wgt->setShape(item.shape);
    wgt->setShapeBrush(item.color);
    wgt->setText(item.title);
    wgt->setToolTip(item.tooltip);
    connect(wgt, &QObject::destroyed, this, [item]()
            {
      qDebug() << "destroy legend item" << item.title;
    });
    mItemWidgets[item.itemId] << wgt;
    if(prepend)
      mFlowLayout->insertWidget(0, wgt);
    else
      mFlowLayout->addWidget(wgt);
    connect(wgt, &LegendShapeItem::editClicked, this,
      [this]()
      {
        auto uid = sender()->property(kUidProperty).toUuid();
        emit itemEditingRequested(uid);
      });
  }
}

void PlotLegendWidgetRefactor::toggleCollapsing()
{
  if(!mIsCollapsed)
    collapse();
  else
    expand();
}

int PlotLegendWidgetRefactor::minimumVisibleRows() const
{
  return mMinimumVisibleRows;
}

void PlotLegendWidgetRefactor::setMinimumVisibleRows(int newMinimumVisibleRows)
{
  mMinimumVisibleRows = newMinimumVisibleRows;
}
