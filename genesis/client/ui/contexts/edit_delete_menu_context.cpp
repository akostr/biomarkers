#include "edit_delete_menu_context.h"

namespace Widgets
{
  EditDeleteMenuContext::EditDeleteMenuContext(QWidget* parent)
    : QMenu(parent)
  {
    Setup();
  }

  void EditDeleteMenuContext::Setup()
  {
    const auto editAction = addAction(tr("Edit name and comments"));
    connect(editAction, &QAction::triggered, this, &EditDeleteMenuContext::Edit);

    const auto deleteAction = addAction(tr("Delete"));
    connect(deleteAction, &QAction::triggered, this, &EditDeleteMenuContext::Delete);
  }
}