#pragma once
#ifndef DOUBLE_TABLE_MODEL_H
#define DOUBLE_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QPointer>

// #include "logic/models/identity_model.h"
class AnalysisEntityModel;
namespace Models
{
  class BarPlotTableModel final : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    explicit BarPlotTableModel(QObject* parent = nullptr);
    ~BarPlotTableModel() override = default;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setModel(QPointer<AnalysisEntityModel> model = nullptr);

  private:
    void onModelDataChanged(const QHash<int, QVariant>& roleDataMap);
    void onModelReset();

  private:
    QPointer<AnalysisEntityModel> mEntityModel = nullptr;

    QStringList horizontalHeaders;
    QStringList verticalHeaders;
  };
}
#endif
