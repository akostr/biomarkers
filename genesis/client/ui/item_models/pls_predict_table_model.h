#pragma once
#ifndef PLS_PREDICT_TABLE_MODEL_H
#define PLS_PREDICT_TABLE_MODEL_H

#include <QAbstractTableModel>

#include "logic/structures/analysis_model_structures.h"

namespace Models
{
  class PlsPredictTableModel : public QAbstractTableModel
  {
    using PredictAnalysisMetaInfoList = Structures::PredictAnalysisMetaInfoList;

    Q_OBJECT

  public:
    explicit PlsPredictTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetPredictTable(const PredictAnalysisMetaInfoList& table);

  private:
    PredictAnalysisMetaInfoList PredictTable;
    QStringList HeaderNames;
  };
}
#endif