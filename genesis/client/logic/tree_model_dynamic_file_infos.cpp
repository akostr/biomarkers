#include "tree_model_dynamic_file_infos.h"
#include "tree_model_item.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>

#include <QVariantMap>
#include <QDateTime>
#include <QDebug>

using namespace Core;

////////////////////////////////////////////////////
//// Tree model dynamic / FileInfos
TreeModelDynamicFileInfos::TreeModelDynamicFileInfos(QObject* parent, int fileId)
  : TreeModelDynamic(parent)
  , FileId(fileId)
{
  //// Setup columns
  Columns[ColumnId].Ids[Qt::EditRole]                     = "id";
  Columns[ColumnId].Ids[Qt::DisplayRole]                  = "id";
  Columns[ColumnId].HeaderData[Qt::DisplayRole]           = tr("Id");

  Columns[ColumnTitle].Ids[Qt::EditRole]                     = "title";
  Columns[ColumnTitle].Ids[Qt::DisplayRole]                  = "title";
  Columns[ColumnTitle].HeaderData[Qt::DisplayRole]           = tr("Title");

  Columns[ColumnDateTime].Ids[Qt::EditRole]               = "date_time";
  Columns[ColumnDateTime].Ids[Qt::DisplayRole]            = "date_time";
  Columns[ColumnDateTime].HeaderData[Qt::DisplayRole]     = tr("Date");

  Columns[ColumnWellTitle].Ids[Qt::EditRole]              = "well_title";
  Columns[ColumnWellTitle].Ids[Qt::DisplayRole]           = "well_title";
  Columns[ColumnWellTitle].HeaderData[Qt::DisplayRole]    = tr("Well");

  Columns[ColumnFieldTitle].Ids[Qt::EditRole]              = "field_title";
  Columns[ColumnFieldTitle].Ids[Qt::DisplayRole]           = "field_title";
  Columns[ColumnFieldTitle].HeaderData[Qt::DisplayRole]    = tr("Field");

  Columns[ColumnLayerTitle].Ids[Qt::EditRole]              = "layer_title";
  Columns[ColumnLayerTitle].Ids[Qt::DisplayRole]           = "layer_title";
  Columns[ColumnLayerTitle].HeaderData[Qt::DisplayRole]    = tr("Layer");

  Reset();
}

TreeModelDynamicFileInfos::~TreeModelDynamicFileInfos()
{
}

//// Handle
void TreeModelDynamicFileInfos::ApplyContextUser(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if(dataId == "(Names::UserContextTags::kUserState")
  {
    if (data.toInt() == Names::UserContextTags::authorized)
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicFileInfos::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicFileInfos::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicFileInfos::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicFileInfos::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetFileInfo(
      FileId,
      [item](QNetworkReply*, QVariantMap result)
      {
        if (!item)
          return;

        item->Load(result);
      },
      [item](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        if (!item)
          return;

        item->Clear();
        Notification::NotifyError(tr("Failed to get file information from database"), err);
      }
    );
  }
}
