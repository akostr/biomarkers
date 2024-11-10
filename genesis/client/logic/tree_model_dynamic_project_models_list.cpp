#include "tree_model_dynamic_project_models_list.h"
#include "tree_model_item.h"
#include "context_root.h"
#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>

using namespace Core;

namespace
{
  const QString parent_id = "_parent_id";
  const QString children_tag = "children";

  bool IsPredictPlsModel(const QVariant& item)
  {
    const auto itemMap = item.toMap();
    const auto parentVariant = itemMap[parent_id];
    if (parentVariant.isValid() && !parentVariant.isNull())
      return parentVariant.toInt() != 0;
    return false;
  }

  bool IsPlsModel(const QVariant& item)
  {
    const auto itemMap = item.toMap();
    const auto parentVariant = itemMap[parent_id];
    if (parentVariant.isValid() && !parentVariant.isNull())
      return parentVariant.toInt() == 0;
    return true;
  }
}
TreeModelDynamicProjectModelsList::TreeModelDynamicProjectModelsList(QObject* parent,
																	 Constants::AnalysisType modelType,
																	 Constants::AnalysisType filteringType)
  : TreeModelDynamic(parent),
  ModelType(modelType),
  FilteringType(filteringType)
{
  //// Setup columns
  Columns[ColumnTitle].Ids[Qt::EditRole] = "_analysis_title";
  Columns[ColumnTitle].Ids[Qt::DisplayRole] = "_analysis_title";
  Columns[ColumnTitle].HeaderData[Qt::DisplayRole] = tr("Name");
  Columns[ColumnTitle].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnTableTitle].Ids[Qt::EditRole] = "_table_title";
  Columns[ColumnTableTitle].Ids[Qt::DisplayRole] = "_table_title";
  Columns[ColumnTableTitle].HeaderData[Qt::DisplayRole] = tr("Table");
  Columns[ColumnTableTitle].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnComment].Ids[Qt::EditRole] = "_analysis_comment";
  Columns[ColumnComment].Ids[Qt::DisplayRole] = "_analysis_comment";
  Columns[ColumnComment].HeaderData[Qt::DisplayRole] = tr("Comment");
  Columns[ColumnComment].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnSamplesCount].Ids[Qt::EditRole] = "_samples_count";
  Columns[ColumnSamplesCount].Ids[Qt::DisplayRole] = "_samples_count";
  Columns[ColumnSamplesCount].HeaderData[Qt::DisplayRole] = tr("Samples Count");
  Columns[ColumnSamplesCount].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnPeakCount].Ids[Qt::EditRole] = "_peaks_count";
  Columns[ColumnPeakCount].Ids[Qt::DisplayRole] = "_peaks_count";
  Columns[ColumnPeakCount].HeaderData[Qt::DisplayRole] = tr("Peak Count");
  Columns[ColumnPeakCount].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnCreated].Ids[Qt::EditRole] = "_created";
  Columns[ColumnCreated].Ids[Qt::DisplayRole] = "_created";
  Columns[ColumnCreated].HeaderData[Qt::DisplayRole] = tr("Created");
  Columns[ColumnCreated].Filter = ColumnInfo::FilterTypeDateRange;

  //// Setup item actions
  ItemActions =
  {
//    { "open", QString(), tr("Open") },
    { "edit", QString(), tr("Edit title and comment") },
    { "remove", QString(), tr("Remove") }
  };

  Reset();
}

TreeModelDynamicProjectModelsList::~TreeModelDynamicProjectModelsList()
{

}

void TreeModelDynamicProjectModelsList::ApplyContextProject(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if (dataId ==  Names::ContextTagNames::ProjectId)
  {
    if(data.toInt())
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicProjectModelsList::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicProjectModelsList::canFetchMore(const QModelIndex& parent) const
{
  QPointer<TreeModelItem> parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = Root;
  else
    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

  return parentItem->CanFetchMore();
}

void TreeModelDynamicProjectModelsList::fetchMore(const QModelIndex& parent)
{
  QPointer<TreeModelItem> parentItem;
  if (parent.column() > 0)
    return;

  if (!parent.isValid())
    parentItem = Root;
  else
    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

  if (!parentItem->CanFetchMore())
    return;

  FetchMore(parentItem);
}

void TreeModelDynamicProjectModelsList::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetAnalysisListByType(
      GenesisContextRoot::Get()->ProjectId(),
      ModelType,
      [&, item](QNetworkReply*, QVariantMap result)
      {
        if (!item)
          return;
        preprocessFetch(result);
        beginResetModel();
        item->Load(result, false);
        endResetModel();
      },
      [item](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        if (!item)
          return;

        item->Clear();
        Notification::NotifyError(tr("Failed to get models list from database"), err);
      }
    );
  }
}

void TreeModelDynamicProjectModelsList::SetType(Constants::AnalysisType type)
{
  ModelType = type;
  Reset();
}

void TreeModelDynamicProjectModelsList::SetFilteringType(Constants::AnalysisType type)
{
  FilteringType = type;
  Reset();
}

void TreeModelDynamicProjectModelsList::preprocessFetch(QVariantMap& variantMap)
{
  if (variantMap.contains(children_tag))
  {
    QVariantList childrenList = variantMap[children_tag].toList();
    if (FilteringType == Constants::AnalysisType::PLS)
    {
      childrenList.removeIf(IsPredictPlsModel);
    }
    else if (FilteringType == Constants::AnalysisType::PLSPredict)
    {
      childrenList.removeIf(IsPlsModel);
    }
    variantMap[children_tag] = childrenList;
  }
}
