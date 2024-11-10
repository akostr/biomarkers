#include "tree_model_dynamic_markup_versions.h"
#include "tree_model_item.h"

#include "context_root.h"

#include "../api/api_rest.h"

#include <QVariantMap>
#include "logic/known_context_tag_names.h"

using namespace Core;

TreeModelDynamicMarkupVersions::TreeModelDynamicMarkupVersions(QObject* parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnTitle].Ids[Qt::EditRole]                            = "version_title";
  Columns[ColumnTitle].Ids[Qt::DisplayRole]                         = "version_title";
  Columns[ColumnTitle].HeaderData[Qt::DisplayRole]                  = tr("Title");
  Columns[ColumnTitle].Filter                                       = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnCreated].Ids[Qt::EditRole]                          = "created";
  Columns[ColumnCreated].Ids[Qt::DisplayRole]                       = "created";
  Columns[ColumnCreated].HeaderData[Qt::DisplayRole]                = tr("Created");
  Columns[ColumnCreated].Filter                                     = ColumnInfo::FilterTypeDateRange;

  Columns[ColumnUserName].Ids[Qt::EditRole]                         = "name";
  Columns[ColumnUserName].Ids[Qt::DisplayRole]                      = "name";
  Columns[ColumnUserName].HeaderData[Qt::DisplayRole]               = tr("User name");
  Columns[ColumnUserName].Filter                                    = ColumnInfo::FilterTypeTextValuesList;

  Columns[ColumnParentVersionTitle].Ids[Qt::EditRole]               = "parent_version_title";
  Columns[ColumnParentVersionTitle].Ids[Qt::DisplayRole]            = "parent_version_title";
  Columns[ColumnParentVersionTitle].HeaderData[Qt::DisplayRole]     = tr("Parent version title");
  Columns[ColumnParentVersionTitle].Filter                          = ColumnInfo::FilterTypeTextValuesList;

  //// Setup item actions
  ItemActions =
  {
    { "remove", QString(), tr("Remove") }
  };

  Reset();
}

TreeModelDynamicMarkupVersions::~TreeModelDynamicMarkupVersions()
{
}

void TreeModelDynamicMarkupVersions::ApplyContextMarkup(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if (dataId == Names::MarkupContextTags::kMarkupId)
  {
    if(data.toInt())
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicMarkupVersions::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicMarkupVersions::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicMarkupVersions::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicMarkupVersions::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetMarkupVersions(GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kMarkupId).toInt(),
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
