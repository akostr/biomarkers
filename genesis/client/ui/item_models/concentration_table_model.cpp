#include "concentration_table_model.h"

using namespace Structures;

namespace Models
{
  ConcentrationTableModel::ConcentrationTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , HorizontalHeaders
      {
        tr("Sample name"),
        tr("Concentration"),
        tr("Graduirovka"),
        tr("Proverka")
      }
  {
  }

  int ConcentrationTableModel::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(TableData.size());
  }

  int ConcentrationTableModel::columnCount(const QModelIndex& parent) const
  {
    return static_cast<int>(HorizontalHeaders.size());
  }

  QVariant ConcentrationTableModel::data(const QModelIndex& index, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    const auto row = index.row();
    if (row > rowCount())
      return QVariant();

    switch (index.column())
    {
    case 0:
      return QString::fromStdString(TableData[row].SampleName);
    case 1:
      return TableData[row].YConcentration.has_value()
        ? TableData[row].YConcentration.value()
        : QVariant();
    case 2:
      return std::fabs(TableData[row].YxGrad) < std::numeric_limits<double>::epsilon()
        ? QVariant()
        : TableData[row].YxGrad;
    case 3:
      return std::fabs(TableData[row].YxProv) < std::numeric_limits<double>::epsilon()
        ? QVariant()
        : TableData[row].YxProv;
    default:
      return QVariant();
    }
  }

  QVariant ConcentrationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
      return QVariant();

    if (section > static_cast<int>(HorizontalHeaders.size()))
      return QVariant();

    return HorizontalHeaders[section];
  }

  void ConcentrationTableModel::SetConcentrationTable(ConcetrantionTable&& table)
  {
    beginResetModel();
    TableData = std::forward<ConcetrantionTable>(table);
    endResetModel();
  }
}
