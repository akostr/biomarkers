#include "readonly_simple_tree_model.h"

ReadOnlySimpleTreeItem::ReadOnlySimpleTreeItem(const QString &itemString, int id)
  : QStandardItem()
{
  setText(itemString);
  setCheckable(true);
  setEditable(false);
  setData(id, Qt::UserRole);
//  connect(this, &QStandardItem::emitDataChanged)

}
