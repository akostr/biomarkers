#pragma once
#ifndef NEW_MODEL_SETTINGS_CONTEXT_H
#define NEW_MODEL_SETTINGS_CONTEXT_H

#include "analysis_model_settings_context.h"

namespace Widgets
{
  class NewModelSettingsContext : public AnalysisModelSettingsContext
  {
    Q_OBJECT

  public:
    NewModelSettingsContext(QWidget* parent = nullptr);
    ~NewModelSettingsContext() = default;

  signals:
    void SaveNewModel();

  private:
    void Setup();
  };
}
#endif