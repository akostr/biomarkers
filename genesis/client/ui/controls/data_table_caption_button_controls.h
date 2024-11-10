#pragma once
#ifndef DATA_TABLE_CAPTION_BUTTON_CONTROLS_H
#define DATA_TABLE_CAPTION_BUTTON_CONTROLS_H

#include <QWidget>

#include "logic/enums.h"
#include "ui/contexts/data_table_context.h"
#include "ui/contexts/build_model_context.h"

namespace Ui
{
  class DataTableCaptionButtonControls;
}

namespace Controls
{
  class DataTableCaptionButtonControls : public QWidget
  {
    Q_OBJECT

  public:
    DataTableCaptionButtonControls(QString tableTitle = "", QWidget* parent = nullptr);
    ~DataTableCaptionButtonControls();

    void SetupBuildSettings(Constants::AnalysisType type, const QString& tableTitle = tr("Data table"));
    void SetAnalysisType(Constants::AnalysisType type);
    void ShowControlForNewModel(bool isNewModel);
    void ShowReturnToMarkupAction(bool show);
    void ShowRatioMatrixAction(bool show);
    void ShowCalculateCoefficientAction(bool show);
    void ShowBuildPlotAction(bool show);
    void ShowTableControl(bool show);
    void SetActiveModule(int module);

    void SetEnabledCalculateCoefficientAction(bool isEnabled);
    void SetEnabledBuildPlotAction(bool isEnabled);

  signals:
    void OpenMatrix();
    void CalculationCoefficientTable();
    void CreatePlot();
    void Export();
    void Edit();
    void Remove();
    void Cancel();
    void BuildModel(Constants::AnalysisType type);
    void ToMarkupData();
    void AnalysisIndexChanged(Constants::AnalysisType type);
    void ReturnTo();

  private:
    Ui::DataTableCaptionButtonControls* ControlUi = nullptr;
    Widgets::DataTableContext* Context = nullptr;
    Widgets::BuildModelContext* BuildContext = nullptr;
    Constants::AnalysisType CurrentType = Constants::AnalysisType::NONE;
    QString TableName;

  private:
    void SetupUi();
    void ConnectActions();
    void SetBuildCancelVisible(bool isVisible);
    QString GetTypeName() const;
  };
}
#endif
