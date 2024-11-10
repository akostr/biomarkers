#pragma once
#ifndef MCR_TABLE_MODEL_H
#define MCR_TABLE_MODEL_H

#include <QAbstractTableModel>
#include "logic/structures/common_structures.h"
#include "logic/structures/analysis_model_structures.h"

using Structures::MCRConcentrationTable;
using Structures::AnalysisMetaInfoList;

namespace Models
{
  class MCRTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    MCRTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetData(const MCRConcentrationTable& table, const AnalysisMetaInfoList& ttitle);

  private:
    std::vector<QString> HorizontalHeaders;
    MCRConcentrationTable ConcentrationsTable;
    AnalysisMetaInfoList TTitle;
  };
}
#endif