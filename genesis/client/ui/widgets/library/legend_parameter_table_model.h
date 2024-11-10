#pragma once
#ifndef LEGEND_PARAMETER_TABLE_MODEL_H
#define LEGEND_PARAMETER_TABLE_MODEL_H

#include "logic/models/analysis_entity_model.h"
#include "ui/plots/gp_items/gpshape_item.h"

#include <QAbstractTableModel>

class LegendParameterTableModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit LegendParameterTableModel(QObject* parent = nullptr);
  ~LegendParameterTableModel() = default;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  void setLegendData(QList<AnalysisEntity::LegendGroup>& data);
  QList<AnalysisEntity::LegendGroup> getLegendData() const;

  void hideIndexDecorations(const QModelIndex& index);

private:
  QList<AnalysisEntity::LegendGroup> mData;
  QModelIndex mHideDecorationIndex = QModelIndex();
};

#endif