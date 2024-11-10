#include "tree_model_dynamic_field_wells.h"
#include "tree_model_item.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>

#include <QVariantMap>
#include <QDateTime>

using namespace Core;

////////////////////////////////////////////////////
//// Tree model dynamic / FieldWells
TreeModelDynamicFieldWells::TreeModelDynamicFieldWells(QObject* parent, int fieldId)
  : TreeModelDynamic(parent)
  , FieldId(fieldId)
{
  //// Setup columns
  Columns[ColumnId].Ids[Qt::EditRole]               = "id";
  Columns[ColumnId].Ids[Qt::DisplayRole]            = "id";
  Columns[ColumnId].HeaderData[Qt::DisplayRole]     = tr("Id");

  Columns[ColumnFieldWell].Ids[Qt::EditRole]            = "name";
  Columns[ColumnFieldWell].Ids[Qt::DisplayRole]         = "name";
  Columns[ColumnFieldWell].HeaderData[Qt::DisplayRole]  = tr("Well");

  Reset();
}

TreeModelDynamicFieldWells::~TreeModelDynamicFieldWells()
{
}

//// Handle
void TreeModelDynamicFieldWells::ApplyContextUser(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if (dataId == Names::UserContextTags::kUserState)
  {
    if(data.toInt() == Names::UserContextTags::authorized)
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicFieldWells::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicFieldWells::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicFieldWells::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicFieldWells::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetFieldWells(FieldId,
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
        Notification::NotifyError(tr("Failed to get field wells from database"), err);
      }
    );
  }
}
