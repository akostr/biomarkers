#pragma once

#include "ui/dialogs/templates/dialog.h"

namespace Ui
{
  class WebDialogPLSBuildSettingsRefactor;
}

class WebDialogPLSBuildSettingsRefactor : public Dialogs::Templates::Dialog
{
  Q_OBJECT
public:
  WebDialogPLSBuildSettingsRefactor(QWidget* parent);
  ~WebDialogPLSBuildSettingsRefactor();

  size_t MaxCountOfPC() const;
  void SetMaxPCCount(size_t value);
  void SetPCValue(size_t value);

  size_t SamplesPerSegment() const;
  void SetMaxSamplesPerSegmentCount(size_t value);
  void SetSamplesPerSegmentCount(size_t value);

  bool Autoscale() const;
  void SetAutoscale(bool autoscale);

  bool Normalization() const;
  void SetNormalization(bool normalization);

  bool IsCross() const;
  void SetIsCross(bool isCross);

  bool IsFull() const;
  void SetIsFull(bool isFull);

  bool IsSegment() const;
  void SetIsSegment(bool isSegment);

  void SetEnableAdditionParameters(bool isEnable);
  void SetTestData(const std::map<int, QString>& list);

  void SetSelectedData(const std::vector<int>& selected);
  QVector<int> GetSelectedData();

private:
  void SetupUi();
  void SelectedDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
    const QList<int>& roles);

  Ui::WebDialogPLSBuildSettingsRefactor* ui;
  QWidget* mContent;
};
