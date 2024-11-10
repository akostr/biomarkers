#include "tree_model_dynamic_project_info.h"
#include "tree_model_item.h"

#include "context_root.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"

#include <QVariantMap>
#include <QDateTime>
#include "logic/known_context_tag_names.h"

using namespace Core;

////////////////////////////////////////////////////
//// Tree model dynamic / ProjectInfo
TreeModelDynamicProjectInfo::TreeModelDynamicProjectInfo(QObject* parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnId].Ids[Qt::EditRole]                           = "id";
  Columns[ColumnId].Ids[Qt::DisplayRole]                        = "id";
  Columns[ColumnId].HeaderData[Qt::DisplayRole]                 = tr("Id");

  Columns[ColumnName].Ids[Qt::EditRole]                         = "title";
  Columns[ColumnName].Ids[Qt::DisplayRole]                      = "title";
  Columns[ColumnName].HeaderData[Qt::DisplayRole]               = tr("Project name");

  Columns[ColumnParentId].Ids[Qt::EditRole]                     = "id_parent";
  Columns[ColumnParentId].Ids[Qt::DisplayRole]                  = "id_parent";
  Columns[ColumnParentId].HeaderData[Qt::DisplayRole]           = tr("Parent id");

  Columns[ColumnParentName].Ids[Qt::EditRole]                   = "parent_title";
  Columns[ColumnParentName].Ids[Qt::DisplayRole]                = "parent_title";
  Columns[ColumnParentName].HeaderData[Qt::DisplayRole]         = tr("Parent project");

  Columns[ColumnDateCreated].Ids[Qt::EditRole]                  = "created";
  Columns[ColumnDateCreated].Ids[Qt::DisplayRole]               = "created";
  Columns[ColumnDateCreated].HeaderData[Qt::DisplayRole]        = tr("Date created");

  Columns[ColumnStatus].Ids[Qt::EditRole]                       = "status";
  Columns[ColumnStatus].Ids[Qt::DisplayRole]                    = "status";
  Columns[ColumnStatus].HeaderData[Qt::DisplayRole]             = tr("Status");

  Columns[ColumnChromatogramCount].Ids[Qt::EditRole]            = "count";
  Columns[ColumnChromatogramCount].Ids[Qt::DisplayRole]         = "count";
  Columns[ColumnChromatogramCount].HeaderData[Qt::DisplayRole]  = tr("Count");

  Columns[ColumnGroup].Ids[Qt::EditRole]                        = "group_title";
  Columns[ColumnGroup].Ids[Qt::DisplayRole]                     = "group_title";
  Columns[ColumnGroup].HeaderData[Qt::DisplayRole]              = tr("Group");

  Columns[ColumnComment].Ids[Qt::EditRole]                      = "comment";
  Columns[ColumnComment].Ids[Qt::DisplayRole]                   = "comment";
  Columns[ColumnComment].HeaderData[Qt::DisplayRole]            = tr("Comment");

  Columns[ColumnCodeWord].Ids[Qt::EditRole]                     = "code_word";
  Columns[ColumnCodeWord].Ids[Qt::DisplayRole]                  = "code_word";
  Columns[ColumnCodeWord].HeaderData[Qt::DisplayRole]           = tr("Code word");

//  Columns[ColumnAuthorFullName].Ids[Qt::EditRole]               = "full_name";
//  Columns[ColumnAuthorFullName].Ids[Qt::DisplayRole]            = "full_name";
//  Columns[ColumnAuthorFullName].HeaderData[Qt::DisplayRole]     = tr("Author");

  Reset();
}

TreeModelDynamicProjectInfo::~TreeModelDynamicProjectInfo()
{
}

//// Handle
void TreeModelDynamicProjectInfo::ApplyContextProject(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if (dataId == Names::ContextTagNames::ProjectId)
  {
    if(data.toInt())
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicProjectInfo::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicProjectInfo::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicProjectInfo::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicProjectInfo::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetProjectInfo(
      GenesisContextRoot::Get()->ProjectId(),
      [item](QNetworkReply*, QVariantMap result)
      {
        if (!item)
          return;

        //// Transform
        if (result.contains("children"))
        {
          QVariantList children = result["children"].toList();
          for (auto& c : children)
          {
            auto row = c.toMap();
            row["full_name"] = row["first_name"].toString() + " "
                             + row["last_name"].toString();
            c = row;
          }
          result["children"] = children;
        }

        item->Load(result);
      },
      [item](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        if (!item)
          return;

        item->Clear();
        Notification::NotifyError(tr("Failed to get project information from database"), err);
      }
    );
  }
}
