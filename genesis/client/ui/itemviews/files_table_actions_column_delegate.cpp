#include "files_table_actions_column_delegate.h"
#include "../../logic/tree_model.h"
#include "../../logic/tree_model_presentation.h"
#include "../../logic/tree_model_dynamic.h"

#include <QTableView>
#include <QMenu>
#include <QPainter>


FilesTableActionsColumnDelegate::FilesTableActionsColumnDelegate(QAbstractItemView *parent)
  : QStyledItemDelegate{parent}
  , View(parent)
{
  Setup();
}

void FilesTableActionsColumnDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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

QWidget *FilesTableActionsColumnDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if (auto model = View->model())
  {
    auto internalIndex = index;
    {
      auto proxyModel = qobject_cast<QSortFilterProxyModel*>(model);
      if(proxyModel)
      {
        model = proxyModel->sourceModel();
        internalIndex = proxyModel->mapToSource(internalIndex);
      }
    }
    QList<ActionInfo> actions;
    if (auto presentationModel = qobject_cast<TreeModelPresentation*>(model))
    {
      actions = presentationModel->GetItemActions(internalIndex);
    }
    else if (auto treeModel = qobject_cast<TreeModelPresentation*>(model))
    {
      actions = treeModel->GetItemActions(internalIndex);
    }
    else if (auto dynamicModel = qobject_cast<TreeModelDynamic*>(model))
    {
      actions = dynamicModel->GetItemActions(internalIndex);
    }
    if (!actions.empty())
    {
      if (Menu)
        delete Menu;
      Menu = new QMenu(parent);

      for (auto& a : actions)
      {
        if (a.Id == "separator")
        {
          Menu->addSeparator();
        }
        else
        {
          auto action = Menu->addAction(QIcon(a.Icon), a.Text);
          action->setProperty("id", a.Id);
        }
      }
      return Menu;
    }
  }
  return nullptr;
}

void FilesTableActionsColumnDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{

}

void FilesTableActionsColumnDelegate::updateEditorGeometry(QWidget */*editor*/, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
  MenuPosition = QCursor::pos();
}

void FilesTableActionsColumnDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  if (QMenu* menu = qobject_cast<QMenu*>(editor))
  {
    QModelIndex internalIndex = index;
    connect(menu, &QMenu::triggered, this, [this, internalIndex](QAction* action)
    {
      if (action)
      {
        if (auto model = View->model())
        {
          auto index = internalIndex;
          {
            if(auto proxy = qobject_cast<QSortFilterProxyModel*>(model))
            {
              model = proxy->sourceModel();
              index = proxy->mapToSource(internalIndex);
            }
          }
          if (auto presentationModel =
              qobject_cast<TreeModelPresentation*>(model))
          {
            presentationModel->TriggerAction(action->property("id").toString(), presentationModel->GetItem(index));
          }
          else if (auto treeModel = qobject_cast<TreeModelPresentation*>(model))
          {
            treeModel->TriggerAction(action->property("id").toString(), treeModel->GetItem(index));
          }
          else if (auto dynamicModel = qobject_cast<TreeModelDynamic*>(model))
          {
            dynamicModel->TriggerAction(action->property("id").toString(), dynamicModel->GetItem(index));
          }
        }
      }
    });
    menu->popup(MenuPosition);
  }
}

void FilesTableActionsColumnDelegate::Setup()
{
  if (View)
  {
    connect(View, &QAbstractItemView::clicked, [this](const QModelIndex& index)
      {
        if (View)
        {
          if (View->itemDelegateForColumn(index.column()) == this)
          {
            View->edit(index);
          }
        }
      });
  }
}
