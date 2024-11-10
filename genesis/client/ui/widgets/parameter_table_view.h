#pragma once
#ifndef PARAMETER_TABLE_VIEW_H
#define PARAMETER_TABLE_VIEW_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "logic/markup/genesis_markup_forward_declare.h"
#include "logic/markup/abstract_data_model.h"

namespace Ui
{
  class ParameterTableView;
}

namespace Models
{
  class AnalysisDataTableModel;
}

namespace Widgets
{
  class ParameterTableView : public QWidget
  {
    Q_OBJECT

  public:
    explicit ParameterTableView(QWidget* parent = nullptr);
    ~ParameterTableView();

    void FillParameterComboBoxShort();
    void FillParameterComboBoxFull();
    void FillCustomParameterComboBox();

    void SetCheckBoxVisible(bool isVisible);
    void SetPeaksCheckBoxVisible(bool isVisible);
    void SetCheckBoxChecked();
    void SetHideYConcentration(bool isHide);
    void AllowEmptyConcentration(bool allow);
    void SetModel(Models::AnalysisDataTableModel* model);
    void SetSampleState(Qt::CheckState state);
    void SetPeakState(Qt::CheckState state);
    void UpdateSampleCheckBoxLabel();
    void UpdatePeakCheckBoxLabel();

    Models::AnalysisDataTableModel* GetModel() const;

    QString GetParameterType() const;

  private:
    Ui::ParameterTableView* WidgetUi = nullptr;
    QMap<QString, int> Parameters;
    Models::AnalysisDataTableModel* AnalysisModel = nullptr;
    QSortFilterProxyModel* ProxyModel = nullptr;

    void SetupUi();
    void ConnectSignalsToModel();
    void DisconnectSignalsToModel();

    void SetCurrentParameterToModel(const QString& text);
    void FillPeaksStateInModel(int state);
    void FillSamplesStateInModel(int state);
    void UpdateCheckBoxStateByHeaderModel(Qt::Orientation orientation, Qt::CheckState state);
  };
}
#endif