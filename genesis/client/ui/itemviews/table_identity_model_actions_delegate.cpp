#include "table_identity_model_actions_delegate.h"

#include <QMenu>

#include "logic/models/pls_table_item_model.h"

using namespace Model;

namespace Widgets
{
  TableIdentityModelActionsDelegate::TableIdentityModelActionsDelegate(QAbstractItemView* parent)
    : QStyledItemDelegate(parent)
  {
  }

  QWidget* TableIdentityModelActionsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
  {
    if (index.data().canConvert<QIcon>())
    {
      const auto menu = new QMenu(parent);
      menu->addActions(Actions->actions());
      return menu;
    }
    return nullptr;
  }

  void TableIdentityModelActionsDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
  {
    MenuPosition = editor->parentWidget()->mapToGlobal(QPoint(option.rect.x(), option.rect.y() + option.rect.height()));
  }

  void TableIdentityModelActionsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
  {
    if (QMenu* menu = qobject_cast<QMenu*>(editor))
    {
      menu->popup(MenuPosition);
    }
  }

  void TableIdentityModelActionsDelegate::SetMenu(QMenu* menu)
  {
    Actions = menu;
  }
}