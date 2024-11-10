#include "mcr_table_model.h"

#include <cassert>
#include <QDateTime>

namespace Models
{
  MCRTableModel::MCRTableModel(QObject* parent)
    : QAbstractTableModel(parent)
  {
  }

  int MCRTableModel::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(ConcentrationsTable.size());
  }

  int MCRTableModel::columnCount(const QModelIndex& parent) const
  {
    return static_cast<int>(HorizontalHeaders.size());
  }

  QVariant MCRTableModel::data(const QModelIndex& index, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    const auto row = index.row();
    if (row > rowCount())
      return QVariant();

    const auto tableId = ConcentrationsTable[row];
    const auto componentCount = static_cast<int>(ConcentrationsTable.front().ColumnValues.size());
    const auto column = index.column();
    if (column > 0 && column <= componentCount)
      return QString("%1%").arg(tableId.ColumnValues[column - 1].Value);

    const auto& title = TTitle[row];
    if (column == 0)
      return QString::fromStdString(title.FileName);

    if (column == componentCount + 1)
      return QString::fromStdString(title.Field);

    if (column == componentCount + 2)
      return QString::fromStdString(title.Well);

    if (column == componentCount + 3)
      return QString::fromStdString(title.WellCluster);

    if (column == componentCount + 4)
      return QString::fromStdString(title.Layer);

    if (column == componentCount + 5)
        return QString::fromStdString(title.Depth);

    if (column == componentCount + 6)
      return QDateTime::fromString(QString::fromStdString(title.Date), Qt::ISODate);
  }

  QVariant MCRTableModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
      return QVariant();

    if (section > static_cast<int>(HorizontalHeaders.size()))
      return QVariant();

    return HorizontalHeaders[section];
  }

  void MCRTableModel::SetData(const MCRConcentrationTable& table, const AnalysisMetaInfoList& ttitle)
  {
    if (table.empty() || ttitle.empty())
      return;
    beginResetModel();
    assert(table.size() == ttitle.size());
    HorizontalHeaders.clear();
    HorizontalHeaders.push_back(tr("Sample"));
    const auto& components = table.front();
    for (const auto& column : components.ColumnValues)
    {
      HorizontalHeaders.push_back(QString::fromStdString(column.TableHeader));
    }
    HorizontalHeaders.push_back(tr("Field"));
    HorizontalHeaders.push_back(tr("Well"));
    HorizontalHeaders.push_back(tr("Well cluster"));
    HorizontalHeaders.push_back(tr("Layer"));
    HorizontalHeaders.push_back(tr("Depth"));
    HorizontalHeaders.push_back(tr("Date"));
    ConcentrationsTable = table;
    TTitle = ttitle;
    endResetModel();
  }
}
