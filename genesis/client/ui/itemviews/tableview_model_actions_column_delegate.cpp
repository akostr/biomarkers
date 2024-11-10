#include "tableview_model_actions_column_delegate.h"

#include "../../logic/tree_model.h"
#include "../../logic/tree_model_presentation.h"
#include "../../logic/tree_model_dynamic.h"

#include <QMenu>
#include <QStyleOptionViewItem>

////////////////////////////////////////////////////
//// Actions menu for tree model
TableViewModelActionsColumnDelegate::TableViewModelActionsColumnDelegate(QAbstractItemView* parent)
  : QStyledItemDelegate(parent)
  , View(parent)
{
  Setup();
}

TableViewModelActionsColumnDelegate::~TableViewModelActionsColumnDelegate()
{
}

//// Setup
void TableViewModelActionsColumnDelegate::Setup()
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

//// Impl
QWidget* TableViewModelActionsColumnDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
  if (auto model = View->model())
  {
    QList<ActionInfo> actions;
    if (auto presentationModel = qobject_cast<TreeModelPresentation*>(model))
    {
      actions = presentationModel->GetItemActions(index);
    }
    else if (auto dynamicModel = qobject_cast<TreeModelDynamic*>(model))
    {
      actions = dynamicModel->GetItemActions(index);
    }
    else if (auto treeModel = qobject_cast<TreeModel*>(model))
    {
      actions = treeModel->GetItemActions(index);
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

void TableViewModelActionsColumnDelegate::destroyEditor(QWidget* /*editor*/, const QModelIndex& /*index*/) const
{
  //// shall NOT destroy "editor" menu, otherwise likely to lead to crash in WASM
  //// return QStyledItemDelegate::destroyEditor(editor, index);
}

void TableViewModelActionsColumnDelegate::updateEditorGeometry(QWidget* /*editor*/, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
  MenuPosition = QCursor::pos();
}

void TableViewModelActionsColumnDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (QMenu* menu = qobject_cast<QMenu*>(editor))
  {
    connect(menu, &QMenu::triggered, [this, index](QAction* action)
    {
      if (action)
      {
        if (auto model = View->model())
        {
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
