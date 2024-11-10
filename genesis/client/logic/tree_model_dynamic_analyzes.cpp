#include "tree_model_dynamic_analyzes.h"
#include "tree_model_item.h"
#include "logic/known_context_tag_names.h"
#include "../api/api_rest.h"

#include "context_root.h"

using namespace Core;

TreeModelDynamicAnalyzes::TreeModelDynamicAnalyzes(QObject *parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnTitle].Ids[Qt::EditRole]                            = "_analysis_title";
  Columns[ColumnTitle].Ids[Qt::DisplayRole]                         = "_analysis_title";
  Columns[ColumnTitle].HeaderData[Qt::DisplayRole]                  = tr("Title");
  Columns[ColumnTitle].Filter                                       = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnCreated].Ids[Qt::EditRole]                          = "_created";
  Columns[ColumnCreated].Ids[Qt::DisplayRole]                       = "_created";
  Columns[ColumnCreated].HeaderData[Qt::DisplayRole]                = tr("Created");
  Columns[ColumnCreated].Filter                                     = ColumnInfo::FilterTypeDateRange;

  Columns[ColumnUserName].Ids[Qt::EditRole]                         = "_user";
  Columns[ColumnUserName].Ids[Qt::DisplayRole]                      = "_user";
  Columns[ColumnUserName].HeaderData[Qt::DisplayRole]               = tr("User name");
  Columns[ColumnUserName].Filter                                    = ColumnInfo::FilterTypeTextValuesList;

//  Columns[ColumnParentVersionTitle].Ids[Qt::EditRole]               = "parent_version_title";
//  Columns[ColumnParentVersionTitle].Ids[Qt::DisplayRole]            = "parent_version_title";
//  Columns[ColumnParentVersionTitle].HeaderData[Qt::DisplayRole]     = tr("Parent version title");
//  Columns[ColumnParentVersionTitle].Filter                          = ColumnInfo::FilterTypeTextValuesList;

  //// Setup item actions
  ItemActions =
  {
    { "remove", QString(), tr("Remove") }
  };

  Reset();
}

TreeModelDynamicAnalyzes::~TreeModelDynamicAnalyzes()
{
}

void TreeModelDynamicAnalyzes::ApplyContextMarkup(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if(dataId == Names::MarkupContextTags::kMarkupId)
  {
    if (data.toInt())
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicAnalyzes::ApplyContextMarkupVersion(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if(dataId == "version_id")
  {
    if (data.toInt())
      Reset();
    else
      Clear();
  }
}
void TreeModelDynamicAnalyzes::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicAnalyzes::canFetchMore(const QModelIndex &parent) const
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

void TreeModelDynamicAnalyzes::fetchMore(const QModelIndex &parent)
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

void TreeModelDynamicAnalyzes::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetAnalysisListByMarkupVersion(
          GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kMarkupId).toInt(),
          GenesisContextRoot::Get()->GetContextMarkupVersion()->GetData("version_id").toInt(),
    [item](QNetworkReply*, QVariantMap data)
    {
      if (!item)
        return;

      item->Load(data);
    },
    [item](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      if (!item)
        return;

      item->Clear();
      qDebug() << err;
    });
  }
}


