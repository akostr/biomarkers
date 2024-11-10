#include "pls_predict_table_model.h"

#include <QDateTime>

using namespace Structures;

namespace Models
{
  PlsPredictTableModel::PlsPredictTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , HeaderNames
    {
      tr("Sample"),
      tr("Predict"),
      tr("Y Concentration"),
      tr("Field"),
      tr("Well"),
      tr("Well Cluster"),
      tr("Layer"),
      tr("Date"),
    }
  {
  }

  int PlsPredictTableModel::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(PredictTable.size());
  }

  int PlsPredictTableModel::columnCount(const QModelIndex& parent) const
  {
    return HeaderNames.count();
  }

  QVariant PlsPredictTableModel::data(const QModelIndex& index, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    switch (index.column())
    {
    case 0:
    {
      auto name = QString::fromStdString(PredictTable[index.row()].FileName);
      return name.mid(name.lastIndexOf("/") + 1);
    }
    case 1:
      return PredictTable[index.row()].YPredConcentration.has_value()
        ? PredictTable[index.row()].YPredConcentration.value()
        : QVariant();
    case 2:
      return PredictTable[index.row()].YConcentration.has_value()
        ? PredictTable[index.row()].YConcentration.value()
        : QVariant();
    case 3:
      return QString::fromStdString(PredictTable[index.row()].Field);
    case 4:
      return QString::fromStdString(PredictTable[index.row()].Well);
    case 5:
        return QString::fromStdString(PredictTable[index.row()].WellCluster);
    case 6:
      return QString::fromStdString(PredictTable[index.row()].Layer);
    case 7:
      return QString::fromStdString(PredictTable[index.row()].Date);
    default:
      break;
    }
    return QVariant();
  }

  QVariant PlsPredictTableModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
      return QVariant();

    if (section > HeaderNames.count())
      return QVariant();

    return HeaderNames[section];
  }

  void PlsPredictTableModel::SetPredictTable(const PredictAnalysisMetaInfoList& table)
  {
    beginResetModel();
    PredictTable = table;
    endResetModel();
  }
}
