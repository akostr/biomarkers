#include "bar_plot_table_model.h"

#include "logic/models/pca_analysis_response_model.h"
#include <logic/models/analysis_entity_model.h>
// #include "logic/structures/common_structures.h"

// using namespace Model;
// using namespace Structures;
using namespace AnalysisEntity;

namespace Models
{
  BarPlotTableModel::BarPlotTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , verticalHeaders{ tr("Primary component"), tr("% explained variance") }
  {
  }

  int BarPlotTableModel::rowCount(const QModelIndex& parent) const
  {
    return verticalHeaders.size();
  }

  int BarPlotTableModel::columnCount(const QModelIndex& parent) const
  {
    return static_cast<int>(horizontalHeaders.size());
  }

  QVariant BarPlotTableModel::data(const QModelIndex& index, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    const auto column = index.column();

    if (index.row() == 0)
      return horizontalHeaders[column];

    auto varianceMap = mEntityModel->modelData(ModelRoleComponentsExplVarianceMap).value<TComponentsExplVarianceMap>();

    return QString("%1 %").arg(varianceMap.value(column + 1, -1));
  }

  QVariant BarPlotTableModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole || orientation != Qt::Vertical)
      return QVariant();

    return verticalHeaders[section];
  }

  void BarPlotTableModel::setModel(QPointer<AnalysisEntityModel> model)
  {
    if(mEntityModel)
    {
      disconnect(mEntityModel, nullptr, this, nullptr);
    }
    mEntityModel = model;
    if(mEntityModel)
    {
      connect(mEntityModel, &AnalysisEntityModel::modelDataChanged, this, &BarPlotTableModel::onModelDataChanged);
      connect(mEntityModel, &AnalysisEntityModel::modelReset, this, &BarPlotTableModel::onModelReset);
      onModelReset();
    }
  }

  void BarPlotTableModel::onModelDataChanged(const QHash<int, QVariant> &roleDataMap)
  {
    onModelReset();
  }

  void BarPlotTableModel::onModelReset()
  {
    beginResetModel();
    horizontalHeaders.clear();
    if(!mEntityModel)
      return;
    auto varianceMap = mEntityModel->modelData(ModelRoleComponentsExplVarianceMap).value<TComponentsExplVarianceMap>();
    for(int i = 0; i < varianceMap.size(); i++)
      horizontalHeaders << tr("PC %1").arg(i+1);
    endResetModel();
  }
}
