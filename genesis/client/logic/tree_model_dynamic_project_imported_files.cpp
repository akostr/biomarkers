#include "tree_model_dynamic_project_imported_files.h"
#include "tree_model_item.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>

#include <logic/context_root.h>
#include <QVariantMap>
#include <QDateTime>

using namespace Core;

////////////////////////////////////////////////////
//// Tree model dynamic / ProjectImportedFiles
TreeModelDynamicProjectImportedFiles::TreeModelDynamicProjectImportedFiles(QObject* parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnFileName].Ids[Qt::EditRole]                     = "filename";
  Columns[ColumnFileName].Ids[Qt::DisplayRole]                  = "filename_short";
  Columns[ColumnFileName].HeaderData[Qt::DisplayRole]           = tr("File name");
  Columns[ColumnFileName].Filter                                = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnFileType].Ids[Qt::EditRole]                     = "file_type";
  Columns[ColumnFileType].Ids[Qt::DisplayRole]                  = "file_type";
  Columns[ColumnFileType].HeaderData[Qt::DisplayRole]           = tr("File type");
  Columns[ColumnFileType].Filter                                = ColumnInfo::FilterTypeTextValuesList;

  Columns[ColumnFileSize].Ids[Qt::EditRole]                     = "filesize";
  Columns[ColumnFileSize].Ids[Qt::DisplayRole]                  = "filesize";
  Columns[ColumnFileSize].HeaderData[Qt::DisplayRole]           = tr("File size");
  Columns[ColumnFileSize].Filter                                = ColumnInfo::FilterTypeIntegerRange;

//  Columns[ColumnAuthorFullName].Ids[Qt::EditRole]               = "author";
//  Columns[ColumnAuthorFullName].Ids[Qt::DisplayRole]            = "author";
//  Columns[ColumnAuthorFullName].HeaderData[Qt::DisplayRole]     = tr("Author");
//  Columns[ColumnAuthorFullName].Filter                          = ColumnInfo::FilterTypeTextValuesList;

  Columns[ColumnDateUploaded].Ids[Qt::EditRole]                 = "uploaded";
  Columns[ColumnDateUploaded].Ids[Qt::DisplayRole]              = "uploaded";
  Columns[ColumnDateUploaded].HeaderData[Qt::DisplayRole]       = tr("Upload date");
  Columns[ColumnDateUploaded].Filter                            = ColumnInfo::FilterTypeDateRange;

  //// Setup item actions
  ItemActions =
  {
    { "information", ":/resource/icons/icon_action_information.png", tr("Information") }
  };

  Reset();
}

TreeModelDynamicProjectImportedFiles::~TreeModelDynamicProjectImportedFiles()
{
}

//// Handle
void TreeModelDynamicProjectImportedFiles::ApplyContextUser(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    Reset();
  }
  else if(dataId == Names::UserContextTags::kUserState)
  {
    if(data.toInt() == Names::UserContextTags::authorized)
      Reset();
    else
      Clear();
  }
}

void TreeModelDynamicProjectImportedFiles::ApplyContextProject(const QString &dataId, const QVariant &data)
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

void TreeModelDynamicProjectImportedFiles::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicProjectImportedFiles::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicProjectImportedFiles::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicProjectImportedFiles::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    if (int projectId = GenesisContextRoot::Get()->ProjectId())
    {
      API::REST::Tables::GetProjectFiles(
        projectId,
        [item](QNetworkReply*, QVariantMap result)
        {
          if (!item)
            return;
          //// Transform
//          qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(result)).toJson();
          if (result.contains("children"))
          {
            QVariantList children = result["children"].toList();
            for (auto& c : children)
            {
              auto row = c.toMap();
              QStringList filePath = row["filename"].toString().split("/");
              if (!filePath.empty())
                row["filename_short"] = filePath.last();
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
          Notification::NotifyError(tr("Failed to get imported files from database"), err);
        }
        );
    }
  }
}
