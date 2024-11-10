#pragma once
#ifndef ANALYSIS_MODEL_SETTINGS_CONTEXT_H
#define ANALYSIS_MODEL_SETTINGS_CONTEXT_H

#include <QMenu>

namespace Widgets
{
  class AnalysisModelSettingsContext : public QMenu
  {
    Q_OBJECT

  public:
    AnalysisModelSettingsContext(QWidget* parent = nullptr);
    ~AnalysisModelSettingsContext() = default;

  signals:
    void OpenModelParameters();
    void OpenDataTable();
    void OpenMarkup();
    void Export();

  protected:
    QAction* FirstAction = nullptr;

  private:
    void Setup();
  };
}
#endif