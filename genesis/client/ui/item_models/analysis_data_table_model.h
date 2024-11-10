#pragma once
#ifndef ANALYSIS_DATA_TABLE_MODEL_H
#define ANALYSIS_DATA_TABLE_MODEL_H

#include <QColor>
#include <QAbstractTableModel>

#include "logic/models/analysis_identity_model.h"
#include "logic/structures/common_structures.h"


namespace Models
{
  class AnalysisDataTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    AnalysisDataTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value,
      int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void SetModel(Model::IdentityModelPtr model);
    void SetCurrentParameter(int param);
    void SetShowCheckboxes(bool isShow);
    void SetShowPeakCheckboxes(bool isShow);
    void FillPeaksChecked(bool checked);
    void FillSamplesChecked(bool checked);
    void SetCheckedPeaks(const std::vector<bool>& peaks);
    void SetCheckedSamples(const std::vector<bool>& samples);
    void SetShowYConcentration(bool isShown);
    void SetAllowEmptyConcentrations(bool allow);

    Model::IdentityModelPtr GetModel() const;
    std::vector<bool> GetCheckedPeaks() const;
    std::vector<bool> GetCheckedSamples() const;
    std::vector<std::optional<double>> GetConcentrations() const;
    void SetConcentration(const std::vector<std::optional<double>>& yConcentrations);
    int GetYConcentrationColumn() const;
    int GetCurrentParameter();
    Structures::Double2DVector GetCurrentData();
    Structures::IntVector GetLibraryElementIds();

  signals:
    void HeaderCheckStateChanged(Qt::Orientation orientation, Qt::CheckState state);

  private:
    Model::IdentityModelPtr DataTableModel = nullptr;
    std::unordered_map<int, std::string> ParameterAccessName;

    std::vector<QString> HorizontalHeaders;
    std::vector<std::optional<double>> YConcentrations;
    int PeakCount = 0;
    int PeakZeroIndex = 0; // for cut checked peaks
    int Parameter = -1;

    std::vector<bool> HorizontalCheckboxDecoration;
    mutable std::vector<bool> VerticalCheckboxDecoration;
    std::vector<bool> CheckedPeaks;
    std::vector<bool> CheckedSamples;

    bool ShowPeakCheckboxes = false;
    bool ShowSampleCheckboxes = false;
    bool ShowYConcentration = false;
    bool AllowEmptyConcentration = false;
    int YConcentrationColumn = 1;
    const QColor BackgroundColor = { 236, 241, 244 };
    void FillHorizontalHeaders();
  };
}

#endif
