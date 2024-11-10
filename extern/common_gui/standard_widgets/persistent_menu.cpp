#include "persistent_menu.h"

PersistentMenu::PersistentMenu(QWidget* parent)
  : QMenu(parent)
{
}

PersistentMenu::PersistentMenu(const QString& title, QWidget* parent)
  : QMenu(title, parent)
{
}

void PersistentMenu::mouseReleaseEvent(QMouseEvent *e)
{
  QAction *action = activeAction();
  if (action && action->isEnabled()) 
  {
    action->setEnabled(false);
    QMenu::mouseReleaseEvent(e);
    action->setEnabled(true);
    action->trigger();
  }
  else
  { 
    QMenu::mouseReleaseEvent(e);
  }
}
