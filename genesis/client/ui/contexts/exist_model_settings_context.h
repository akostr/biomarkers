#pragma once
#ifndef EXIST_MODEL_SETTINGS_CONTEXT_H
#define EXIST_MODEL_SETTINGS_CONTEXT_H

#include "analysis_model_settings_context.h"

namespace Widgets
{
  class ExistModelSettingsContext : public AnalysisModelSettingsContext
  {
    Q_OBJECT

  public:
    explicit ExistModelSettingsContext(QWidget* parent = nullptr);
    ~ExistModelSettingsContext() = default;

    QAction* SaveChangesAction = nullptr;

  signals:
    void SaveChanges();
    void SaveAsNew();

  private:
    void Setup();
  };
}

#endif