#pragma once
#ifndef COEFFICIENT_TABLE_CALC_H
#define COEFFICIENT_TABLE_CALC_H

#include <QWidget>
#include <QPointer>

#include <functional>
#include <vector>

namespace Ui
{
  class CoefficientTableCalc;
}

namespace Control
{
  class DataCheckableHeaderView;
}

class TreeModelDynamicCalculationCoefficientTable;
class TreeModelPresentation;
class TreeModelItem;

class CoefficientTableCalc : public QWidget
{
  Q_OBJECT

public:
  explicit CoefficientTableCalc(QWidget* parent = nullptr);
  ~CoefficientTableCalc();

  int CurrentStep() const;
  void Next();
  void Undo();
  void LoadCoefficient(int tableId);

signals:
  void disableOkButton(bool disable);

private:
  Ui::CoefficientTableCalc* ui = nullptr;
  Control::DataCheckableHeaderView* Header = nullptr;
  QAction* NameSearchAction = nullptr;
  QAction* NameSearchClearAction = nullptr;
  QPointer<TreeModelDynamicCalculationCoefficientTable> CoefficientsTreeModel;
  QPointer<TreeModelPresentation> CoefficientsTreeProxyModel;
  std::vector<std::function<void()>> StepFunc;

  int currentStep = 0;

  void SetupUi();
  void ConnectSignals();
  void ApplyFilter();
  void HideClearAction(const QString& text);
  void RequestCalculationTable();
  void SaveTitleAndComment();
  void SelectStorage();
  void SaveCoefficient();

  bool IsValidTitleAndComment() const;
};
#endif
