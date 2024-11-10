#include "counts_plot_manual_hotteling.h"
#include "graphicsplot/graphicsplot.h"
#include <ui/plots/gp_items/sample_info_plate.h>
#include <ui/plots/gp_items/gpshape_with_label_item.h>
#include <ui/dialogs/web_dialog_create_group_manual_hotteling.h>
#include "logic/notification.h"

CountsPlotManualHotteling::CountsPlotManualHotteling(QWidget* parent)
  : CountsPlot(parent)
{
}

void CountsPlotManualHotteling::FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void()> >& actions)
{
  if (!IsGroupContext)
  {
    CountsPlot::FillContextMenu(event, menu, actions);
    return;
  }

  auto selItems = selectedItems();
  if (!selItems.size()) {
    return;
  }

  QAction* cMenuTitleAction = menu.addAction("");
  cMenuTitleAction->setEnabled(false);
  menu.addSeparator();

  if (selItems.size() == 1) {
    auto item = qobject_cast<GPShapeWithLabelItem*>(selItems.first());
    if (!item) {
      return;
    }

    cMenuTitleAction->setText(tr("Add in group"));
    for (const auto& [id, group] : Groups)
    {
      QIcon groupIcon = QIcon::fromTheme("folder");
      QAction* addToGroupAction = menu.addAction(groupIcon, group.nameGroup);
      actions[addToGroupAction] = [this, gId = id, item]()
      {
        addToGroup(gId, item);
      };
    }

    QAction* CreateGroupAction = menu.addAction(tr("Create group"));
    actions[CreateGroupAction] = [this, item]()
    {
      WebDialogCreateGroupManualHotteling* dialog = 0;
      auto f = [this, item](WebDialogCreateGroupManualHotteling* dialog) {
        auto id = createGroup(dialog->getNameGroup());
        if (id == -1) {
          return;
        }
        addToGroup(id, item);
        dialog->Done(QDialog::Accepted);
      };

      QSet<QString> occupiedNames;
      for (const auto& [i, m] : Groups)
        occupiedNames.insert(m.nameGroup);

      dialog = new WebDialogCreateGroupManualHotteling(this, occupiedNames);
      dialog->setGroupAddingFunc(f);
      dialog->Open();
    };

    QAction* DeleteGroupAction = menu.addAction(tr("Remove from group"));
    auto idFoundGroup = findItemGroup(item);
    if (idFoundGroup != -1) {
      DeleteGroupAction->setEnabled(true);
    }

    actions[DeleteGroupAction] = [this, item]() {
      deleteFromGroup(item);
    };
  }
  if (selItems.size() && selItems.size() != 1)
  { //multiple selection

    cMenuTitleAction->setText(tr("SELECTED %n SAMPLES", "", selItems.size()));

    for (const auto& [id, group] : Groups)
    {
      QIcon groupIcon = QIcon::fromTheme("folder"); //issue
      QAction* addToGroupAction = menu.addAction(groupIcon, group.nameGroup);

      actions[addToGroupAction] = [this, groupId = id, selItems]()
      {
        for (auto& item : selItems)
          if (auto gpItem = qobject_cast<GPShapeWithLabelItem*>(item))
            addToGroup(groupId, gpItem);
      };
    }

    QAction* CreateGroupAction = menu.addAction(tr("Create group"));
    actions[CreateGroupAction] = [this, selItems]()
    {
      auto f = [this, selItems](WebDialogCreateGroupManualHotteling* dialog)
      {
        //qDebug() << dialog;
        auto id = createGroup(dialog->getNameGroup());
        if (id == -1)
          return;

        for (auto& item : selItems)
          if (auto gpItem = qobject_cast<GPShapeWithLabelItem*>(item))
            addToGroup(id, gpItem);

        dialog->Done(QDialog::Accepted);
      };

      QSet<QString> occupiedNames;
      for (const auto& [i, m] : Groups)
        occupiedNames.insert(m.nameGroup);

      const auto dialog = new WebDialogCreateGroupManualHotteling(this, occupiedNames);
      dialog->setGroupAddingFunc(f);
      dialog->Open();
    };

    QAction* DeleteGroupAction = menu.addAction(tr("Remove from group"));
    DeleteGroupAction->setEnabled(false);

    for (auto& item : selItems)
    {
      if (auto gpItem = qobject_cast<GPShapeWithLabelItem*>(item);
        findItemGroup(gpItem) != -1)
        DeleteGroupAction->setEnabled(true);
    }

    actions[DeleteGroupAction] = [this, selItems, DeleteGroupAction]()
    {
      for (auto& item : selItems)
        if (auto gpItem = qobject_cast<GPShapeWithLabelItem*>(item))
          deleteFromGroup(gpItem);
    };
  }
}

void CountsPlotManualHotteling::clearData()
{
  CountsPlot::clearData();
  Groups.clear();
}

void CountsPlotManualHotteling::clearGroups()
{
  Groups.clear();
}

void CountsPlotManualHotteling::SetGroupContext(bool isGroup)
{
  IsGroupContext = isGroup;
  if (IsGroupContext)
    greyAll();
  else
  {
    SetupDefaultStyle();
    UpdateItemsSizes();
    UpdateItemsColorsBy(ColorSortParam, false);
    UpdateItemsShapesBy(ShapeSortParam);
  }
}

std::map<int, ManualGroup> CountsPlotManualHotteling::GetGroups()
{
  return Groups;
}

int CountsPlotManualHotteling::findItemGroup(GPShapeWithLabelItem* findItem)
{
  const auto it = std::find_if(Groups.begin(), Groups.end(),
    [&](const std::pair<int, ManualGroup>& item)
    {
      return item.second.groupItems.contains(findItem);
    });

  if (it == Groups.end())
    return -1;

  return it->first;
}

int CountsPlotManualHotteling::createGroup(const QString& nameGroup)
{
  QString newName = nameGroup;
  int lastIdGroup = static_cast<int>(Groups.size());
  auto newColor = RandomColor();
  auto it = std::find_if(Groups.begin(), Groups.end(),
    [&](const std::pair<int, ManualGroup>& item)
    {
      return item.second.colorGroup == newColor;
    });

  while (it != Groups.end())
  {
    newColor = RandomColor();
    it = std::find_if(Groups.begin(), Groups.end(),
      [&](const std::pair<int, ManualGroup>& item)
      {
        return item.second.colorGroup == newColor;
      });
  }

  Groups.emplace(lastIdGroup, ManualGroup{ newName, newColor, {} });
  return lastIdGroup;
}

void CountsPlotManualHotteling::addToGroup(int newGroupId, GPShapeWithLabelItem* item)
{
  if (!item)
    return;

  if (newGroupId > Groups.size() - 1)
  {
    //qDebug() << Q_FUNC_INFO << ": groupId > Groups->keys().size() - 1";
    return;
  }

  const auto it = std::find_if(Groups.begin(), Groups.end(),
    [&](const std::pair<int, ManualGroup>& groupItem)
    {
      return groupItem.second.groupItems.contains(item);
    });

  if (it != Groups.end())
    Groups[it->first].groupItems.removeAll(item);

  Groups[newGroupId].groupItems.append(qobject_cast<GPShapeWithLabelItem*>(item));
  const auto color = Groups.find(newGroupId)->second.colorGroup;
  item->setColor(color);
}

void CountsPlotManualHotteling::deleteFromGroup(GPShapeWithLabelItem* item)
{
  int alreadyAddedGroupId = -1;
  const auto it = std::find_if(Groups.begin(), Groups.end(),
    [&](const std::pair<int, ManualGroup>& groupItem)
    {
      return groupItem.second.groupItems.contains(item);
    });

  if (alreadyAddedGroupId != -1)  // remove from previous added
    Groups[alreadyAddedGroupId].groupItems.removeAll(item);

  if (item)
    item->setColor(GPShapeItem::Gray);
}

void CountsPlotManualHotteling::greyAll()
{
  for (auto& item : T_Points)
  {
    item.first->setColor(GPShapeItem::Gray);
  }
}

GPShapeItem::DefaultColor CountsPlotManualHotteling::RandomColor() const
{
  QVector<GPShapeItem::DefaultColor> exceptColors =
  { GPShapeItem::Gray,
    GPShapeItem::DarkGray
  };

  auto grandom = QRandomGenerator::global();
  GPShapeItem::DefaultColor color;
  do
  {
    color = static_cast<GPShapeItem::DefaultColor>(grandom->bounded(GPShapeItem::LastColor - 1));
  } while (exceptColors.contains(color));
  return color;
}
