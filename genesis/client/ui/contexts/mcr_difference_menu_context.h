#pragma once
#ifndef MCR_DIFFERENT_MENU_CONTEXT_H
#define MCR_DIFFERENT_MENU_CONTEXT_H

#include <QMenu>

class McrDifferentMenuContext final : public QMenu
{
  Q_OBJECT
public:
  explicit McrDifferentMenuContext(QWidget* parent);
  ~McrDifferentMenuContext() override = default;

  void SetTitleCount(int count);
  QAction* BuildOriginRestoredAction = nullptr;
  QAction* ExcludeFromModelAction = nullptr;
  QAction* CancelExcludeAction = nullptr;
  QAction* BuildForNewModel = nullptr;

private:
  QAction* TitleAction = nullptr;
  void SetupUi();
};
#endif