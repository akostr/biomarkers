#pragma once
#ifndef MCR_ORIGIN_RESTORED_MENU_CONTEXT_H
#define MCR_ORIGIN_RESTORED_MENU_CONTEXT_H

#include <QMenu>

class McrOriginRestoredMenuContext final: public QMenu
{
  Q_OBJECT

public:
    explicit McrOriginRestoredMenuContext(QWidget* parent = nullptr);
    ~McrOriginRestoredMenuContext() override = default;

    QAction* ExcludeAction = nullptr;
    QAction* CancelExcludeAction = nullptr;
    QAction* BuildForNewModel = nullptr;

private:
  void SetupUi();
};
#endif