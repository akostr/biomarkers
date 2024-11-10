#include "tree_model_dynamic_fields.h"
#include "tree_model_item.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>

#include <QVariantMap>
#include <QDateTime>

using namespace Core;

////////////////////////////////////////////////////
//// Tree model dynamic / Fields
TreeModelDynamicFields::TreeModelDynamicFields(QObject* parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnId].Ids[Qt::EditRole]               = "id";
  Columns[ColumnId].Ids[Qt::DisplayRole]            = "id";
  Columns[ColumnId].HeaderData[Qt::DisplayRole]     = tr("Id");

  Columns[ColumnField].Ids[Qt::EditRole]            = "title";
  Columns[ColumnField].Ids[Qt::DisplayRole]         = "title";
  Columns[ColumnField].HeaderData[Qt::DisplayRole]  = tr("Field");

  Reset();
}

TreeModelDynamicFields::~TreeModelDynamicFields()
{
}

//// Handle
void TreeModelDynamicFields::ApplyContextUser(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if(dataId == Names::UserContextTags::kUserState)
  {
    if (data.toInt() == Names::UserContextTags::authorized)
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicFields::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicFields::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicFields::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicFields::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetFields(
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
        Notification::NotifyError(tr("Failed to get fields from database"), err);
      }
    );
  }
}
