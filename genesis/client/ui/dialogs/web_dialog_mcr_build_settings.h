#pragma once

#include "ui/dialogs/templates/dialog.h"

#include "logic/structures/common_structures.h"

namespace Ui
{
  class WebDialogMCRBuildSettings;
}

namespace Widgets
{
  class MCRComponentSettings;
}

namespace Dialogs
{
  class WebDialogMCRBuildSettings : public Templates::Dialog
  {
    using ReferenceSettingsList = Structures::ReferenceSettingsList;

    Q_OBJECT

  public:
    explicit WebDialogMCRBuildSettings(QWidget* parent = nullptr);
    ~WebDialogMCRBuildSettings();

    void SetSampleIds(const Structures::IntVector& samples);
    Structures::IntVector GetSampleIds() const;

    void SetMarkersIds(const Structures::IntVector& markers);
    Structures::IntVector GetMarkerIds() const;

    void SetTableId(const int tableId);
    int GetTableId() const;

    void SetMaximumComponentCount(int max);
    void SetSampleNames(const QStringList& names);
    void SetParameterType(const QString& param);
    QString GetParameterType() const;

    ReferenceSettingsList GetReferenceSettings() const;
    void SetReferenceSettings(const ReferenceSettingsList& settings);

    size_t GetComponentCurrentValue() const;
    void SetComponetCurrentValue(size_t count);

    bool IsClosedSystem() const;
    void SetClosedSystem(bool isClosed) const;

    bool IsNormData() const;
    void SetNormData(bool isNorm) const;

    size_t GetMaxIterations() const;
    void SetMaxIterations(size_t maxIters) const;

    // Tolerance error change
    double GetEvaluatedValue() const; // tol_error_change
    void SetEvaluatedValue(double tolErrChange);

    std::string GetMethod() const;
    void SetMethod(const std::string& method);

  private:
    Ui::WebDialogMCRBuildSettings* WidgetUi = nullptr;
    QString ParameterType;
    std::vector<double> ErrorChange;
    QStringList SampleNames;
    Structures::IntVector SampleIds;
    Structures::IntVector MarkersIds;
    int TableId = 0;

    void SetupUi();
    void ConnectActions();
    void InitDefaultSettings();
    void EvaluatedRadioButtonToggled(bool checked);
    void ComponentValueChanged(int currentValue);
    void CalculateEFA();
    void CheckBuildEnabled();

    Widgets::MCRComponentSettings* AddComponent(const QStringList& list);
  };
}
