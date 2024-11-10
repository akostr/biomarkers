#include "analysis_model_settings_context.h"

#include "genesis_style/style.h"

namespace Widgets
{
  AnalysisModelSettingsContext::AnalysisModelSettingsContext(QWidget* parent)
    : QMenu(parent)
  {
    Setup();
  }

  void AnalysisModelSettingsContext::Setup()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());

    FirstAction = addAction(QIcon(":/resource/icons/icon_action_settings.png"), tr("Model parameters"));
    connect(FirstAction, &QAction::triggered, this, &AnalysisModelSettingsContext::OpenModelParameters);

    const auto openDataTableAction = addAction(QIcon(":/resource/icons/icon_repeat.png"), tr("Open data table"));
    connect(openDataTableAction, &QAction::triggered, this, &AnalysisModelSettingsContext::OpenDataTable);

    const auto openMarkupAction = addAction(QIcon(":/resource/icons/icon_union.png"), tr("Open markup"));
    openMarkupAction->setDisabled(true);
    connect(openMarkupAction, &QAction::triggered, this, &AnalysisModelSettingsContext::OpenMarkup);

    const auto exportAction = addAction(QIcon(":/resource/icons/icon_action_export.png"), tr("Export"));
    connect(exportAction, &QAction::triggered, this, &AnalysisModelSettingsContext::Export);
  }
}