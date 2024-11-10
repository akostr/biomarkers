#pragma once
#ifndef BUILD_MODEL_CONTEXT_H
#define BUILD_MODEL_CONTEXT_H

#include <QMenu>

namespace Widgets
{
  class BuildModelContext : public QMenu
  {
    Q_OBJECT

  public:
    BuildModelContext(QWidget* parent = nullptr);
    ~BuildModelContext() override = default;
    void SetActiveModule(int module);

    QAction* BuildPCAAction = nullptr;
    QAction* BuildMCRAction = nullptr;
    QAction* BuildPLSAction = nullptr;
    QAction* BuildPLSPredictAction = nullptr;
    QAction* BuildPLSPredictSubMenuAction = nullptr;

  private:
    void SetupUi();
  };
}
#endif
