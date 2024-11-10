#include "load_plot.h"
#include "ui/plots/gp_items/gpshape_with_label_item.h"

LoadPlot::LoadPlot(QWidget* parentWidget)
  : AnalysisPlot(parentWidget)
  , HasExcludedItems(false)
{
}

void LoadPlot::setPoints(const QStringList& titles,
  const QMap<QString, int>& P_Ids,
  const Component& firstComp,
  const Component& secondComp,
  const QList<int>& excluded)
{
  QFont font;
  font.setBold(true);
  for (int i = 0; i < firstComp.P.size(); i++)
  {
    auto shape = new GPShapeWithLabelItem(this,
      { firstComp.P[i], secondComp.P[i] },
      GPShape::Circle,
      GPShapeItem::ToDefaultSize(GPShapeItem::Small),
      titles[i],
      font
    );
    shape->setProperty("order_num", i);
    if (P_Ids.contains(titles[i]))
      shape->setProperty("id", P_Ids[titles[i]]);
    shape->setSelectable(true);
    shape->setMargins(5, 5, 5, 5);

    {
      shape->setShapePen(Qt::NoPen);
      shape->setShapeSelectedPen(Qt::NoPen);
      auto brush = QBrush(GPShapeItem::ToColor(GPShapeItem::DefaultColor(0)));
      shape->setColor(GPShapeItem::DefaultColor(0));
      shape->setShapeSelectedBrush(brush);
    }
    {
      shape->setPlatePen(Qt::NoPen);
      shape->setPlateSelectedPen(Qt::NoPen);
      shape->setPlateBrush(Qt::NoBrush);
      shape->setPlateSelectedBrush(QColor(86, 185, 242, 51)); //color from figma
    }

    shape->setLabelText(titles[i]);

    P_Points.append(shape);
    if (excluded.contains(i))
      excludeItem(shape);
  }
}

void LoadPlot::clearData()
{
  for (auto& item : P_Points)
    removeItem(item);
  P_Points.clear();
  resetExcludedItems();
}

const QList<GPShapeWithLabelItem*>& LoadPlot::GetExcludedItems()
{
  return ExcludedItems;
}

int LoadPlot::GetPointsCount()
{
  return P_Points.size();
}

void LoadPlot::ExcludePeaks(const QStringList& peaks)
{
  for (const auto& name : peaks)
  {
    const auto it =
      std::find_if(P_Points.begin(), P_Points.end(),
        [&](const GPShapeWithLabelItem* item)
        {
          return name == item->getLabelText();
        });
    if (it != P_Points.end())
    {
      excludeItem(*it);
    }
  }
}

void LoadPlot::CancelExclude()
{
  resetExcludedItems();
}

void LoadPlot::showNames(bool isShown)
{
  for (auto& item : P_Points)
    item->setLabelHidden(!isShown);
  queuedReplot();
}

void LoadPlot::FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void()> >& actions)
{
  auto selected = selectedItems();
  auto item = dynamic_cast<GPShapeWithLabelItem*>(itemAt(event->pos(), true));
  if (!item && selected.empty())
  {
    //default plot context menu
    menu.setHidden(true);
    auto nameAct = menu.addAction(tr("Count plot"));
    nameAct->setEnabled(false);

  }
  else
  {
    menu.setHidden(false);
    if ((item && !selected.contains(item)) ||
      (!item && selected.size() == 1) ||
      (selected.contains(item) && selected.size() == 1)
      )
    {
      if (!item && selected.size() == 1)
        item = dynamic_cast<GPShapeWithLabelItem*>(selected.first());
      //item menu
      auto nameAct = menu.addAction(QTextDocumentFragment::fromHtml(item->getLabelText()).toPlainText());
      nameAct->setEnabled(false);
      menu.addSeparator();
      if (ExcludedItems.empty())
      {
        QAction* pickForModel = menu.addAction(tr("Pick for new model"));
        actions[pickForModel] = [&, item]()
          {
            QList<GPAbstractItem*> excluded;
            for (const auto& point : P_Points)
            {
              if (item != point)
                excluded.append(point);
            }
            excludeItems(excluded);
          };
      }
      bool bItemIsNonExcluded = !ExcludedItems.contains(item);

      if (bItemIsNonExcluded)
      {
        QAction* excludeFromModel = menu.addAction(tr("Pick for excluding from model"));
        actions[excludeFromModel] = std::bind(&LoadPlot::excludeItems, this, QList<GPAbstractItem*>{ item });
        excludeFromModel->setEnabled(true);
      }
      else
      {
        QAction* reincludeToModel = menu.addAction(tr("Reset excluding from model"));
        actions[reincludeToModel] = std::bind(qOverload<GPAbstractItem*>(&LoadPlot::resetExcludedItem), this, item);
        reincludeToModel->setEnabled(true);
      }

    }
    else
    {
      //whole selected items context menu

      auto nameAct = menu.addAction(tr("PICKED %n PEAKS", "", selected.size()));
      nameAct->setEnabled(false);
      menu.addSeparator();
      bool bHasNonExcludedItem = false;
      {
        int selectedShapes = 0;
        for (auto& item : selected)
          if (auto shape = dynamic_cast<GPShapeWithLabelItem*>(item))
            if (!ExcludedItems.contains(shape))
            {
              bHasNonExcludedItem = true;
              break;
            }
      }
      if (bHasNonExcludedItem)
      {
        QAction* excludeFromModel = menu.addAction(tr("Pick for excluding from model"));
        actions[excludeFromModel] = std::bind(qOverload<QList<GPAbstractItem*>>(&LoadPlot::excludeItems), this, selected);
        excludeFromModel->setEnabled(true);
      }
      if (ExcludedItems.empty())
      {
        QAction* pickForModel = menu.addAction(tr("Pick for new model"));
        actions[pickForModel] = [&, selected]()
          {
            QList<GPAbstractItem*> excluded;
            for (const auto& point : P_Points)
            {
              if (!selected.contains(point))
                excluded.append(point);
            }
            excludeItems(excluded);
          };
      }
      bool bHasExcludedItems = false;
      {
        for (auto& item : selected)
          if (ExcludedItems.contains(item))
          {
            bHasExcludedItems = true;
            break;
          }
      }
      if (bHasExcludedItems)
      {
        QAction* reincludeToModel = menu.addAction(tr("Reset excluding from model"));
        actions[reincludeToModel] = std::bind(qOverload<QList<GPAbstractItem*>>(&LoadPlot::resetExcludedItems), this, selected);
        reincludeToModel->setEnabled(true);
      }
    }
  }
}

void LoadPlot::excludeItem(GPShapeWithLabelItem* item)
{
  if (!ExcludedItems.contains(item))
  {
    ExcludedItems.append(item);
    item->setInactive(true);
    updateExcludedState();
  }
}

void LoadPlot::excludeItems(QList<GPAbstractItem*> items)
{
  for (auto& item : items)
    if (auto shape = dynamic_cast<GPShapeWithLabelItem*>(item))
      excludeItem(shape);

  emit AllItemsExcluded();
}

void LoadPlot::resetExcludedItems(bool update)
{
  for (auto& item : ExcludedItems)
    if (hasItem(item))
      item->setInactive(false);
  ExcludedItems.clear();
  HasExcludedItems = false;
  if (update)
    updateExcludedState();
}

void LoadPlot::resetExcludedItems(QList<GPAbstractItem*> items)
{
  for (auto& item : ExcludedItems)
    if (items.contains(item))
      item->setInactive(false);
  ExcludedItems.erase(std::remove_if(ExcludedItems.begin(), ExcludedItems.end(),
    [items](GPShapeWithLabelItem*& v) { return items.contains(v); }),
    ExcludedItems.end()
  );
  updateExcludedState();
}

void LoadPlot::resetExcludedItem(GPAbstractItem* item)
{
  resetExcludedItems(QList{ item });
}

void LoadPlot::updateExcludedState()
{
  HasExcludedItems = !ExcludedItems.empty();
  emit excludedStateChanged(HasExcludedItems);
}

bool LoadPlot::getHasExcludedItems() const
{
  return HasExcludedItems;
}
