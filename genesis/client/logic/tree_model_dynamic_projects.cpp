#include "tree_model_dynamic_projects.h"
#include "tree_model_item.h"

#include "../api/api_rest.h"
#include "../logic/notification.h"
#include <logic/known_context_tag_names.h>
#include "context_root.h"

#include <QVariantMap>
#include <QDateTime>
#include <QPainter>

using namespace Core;

////////////////////////////////////////////////////
//// Tree model dynamic / projects
TreeModelDynamicProjects::TreeModelDynamicProjects(QObject* parent)
  : TreeModelDynamic(parent)
  , mLockIcon(":/resource/icons/icon_lock@2x.png")
{
  //// Setup columns
  Columns[ColumnCodeWord].HeaderData[Qt::DisplayRole]           = tr("");
  Columns[ColumnLocked].HeaderData[Qt::DisplayRole]           = tr("");

  Columns[ColumnId].Ids[Qt::EditRole]                           = "id";
  Columns[ColumnId].Ids[Qt::DisplayRole]                        = "id";
  Columns[ColumnId].HeaderData[Qt::DisplayRole]                 = tr("Id");
  Columns[ColumnId].Filter                                      = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnName].Ids[Qt::EditRole]                         = "title";
  Columns[ColumnName].Ids[Qt::DisplayRole]                      = "title";
  Columns[ColumnName].HeaderData[Qt::DisplayRole]               = tr("Project name");
  Columns[ColumnName].Filter                                    = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnComment].Ids[Qt::EditRole]                      = "comment";
  Columns[ColumnComment].Ids[Qt::DisplayRole]                   = "comment";
  Columns[ColumnComment].HeaderData[Qt::DisplayRole]            = tr("Comment");
  Columns[ColumnComment].Filter                                 = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnChromatogramCount].Ids[Qt::EditRole]            = "count";
  Columns[ColumnChromatogramCount].Ids[Qt::DisplayRole]         = "count";
  Columns[ColumnChromatogramCount].HeaderData[Qt::DisplayRole]  = tr("Chromatogram\ncount");
  Columns[ColumnChromatogramCount].Filter                       = ColumnInfo::FilterTypeIntegerRange;

  Columns[ColumnStatus].HeaderData[Qt::DisplayRole]             = tr("Status");

  Columns[ColumnDateCreated].Ids[Qt::EditRole]                  = "created";
  Columns[ColumnDateCreated].Ids[Qt::DisplayRole]               = "created";
  Columns[ColumnDateCreated].HeaderData[Qt::DisplayRole]        = tr("Creation date");
  Columns[ColumnDateCreated].Filter                             = ColumnInfo::FilterTypeDateRange;

  //// Setup item actions
  ItemActions =
  {
    { "open", QString(), tr("Open") },
    { "rename", ":/resource/icons/icon_action_edit.png", tr("Rename")}
  };

//  Reset();
}

TreeModelDynamicProjects::~TreeModelDynamicProjects()
{
}

//// Handle
void TreeModelDynamicProjects::ApplyContextUser(const QString &dataId, const QVariant &data)
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

void TreeModelDynamicProjects::Reset()
{
  //костыль, поправили это в новой ветке, здесь пока так
  bool forceFetch = Root->IsEmpty();
  if(forceFetch)
  {
    Root->SetFetched(false);
    FetchMore(Root);
  }
  else
  {
    TreeModelDynamic::Reset();
  }

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicProjects::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicProjects::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicProjects::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetUserProjects(
      [this, item](QNetworkReply* /*reply*/, QVariantMap result)
      {
#ifdef DEBUG
//        qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(result)).toJson();
#endif
        if (!item)
          return;
        //// Transform
        if (result.contains("children"))
        {
          QVariantList children = result["children"].toList();
          for (int i = 0; i < children.size(); i++)
          {
            auto& c = children[i];
            auto row = c.toMap();
            //remove full_name from anywhere
            row["full_name"] = tr("UNIDENTIFIED");
            row["code_word_lock"] = !row["codeword"].toString().isEmpty();
            row["is_locked"] = row["is_locked"].toBool() ? 1 : 0;
            c = row;
          }
          result["children"] = children;
        }
//        qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(result)).toJson();
        beginResetModel();
        item->Load(result);
        endResetModel();
      },
      [item](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        if (!item)
          return;

        item->Clear();
        Notification::NotifyError(tr("Failed to get projects from database"), err);
      }
    );
  }
}

QIcon TreeModelDynamicProjects::getCircleIcon(QString colorName) const
{
  if(colorName.isEmpty())
    return QIcon();
  if(!mCirclesIconsCache.contains(colorName))
  {
    int actualPmapSize = 10;
    int dpi = 3;
    QPixmap pmap(dpi * actualPmapSize, dpi * actualPmapSize);
    pmap.fill(QColor(0,0,0,0));
    QPainter p(&pmap);
    p.setPen(Qt::NoPen);
    auto color = QColor(colorName);
    p.setBrush(color);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawEllipse(pmap.rect());
    pmap.setDevicePixelRatio(3);
    mCirclesIconsCache[colorName] = QIcon(pmap);
  }
  return mCirclesIconsCache[colorName];
}


QVariant TreeModelDynamicProjects::data(const QModelIndex &index, int role) const
{
  if(index.column() == ColumnStatus)
  {
    QPointer<TreeModelItem> item = static_cast<TreeModelItem*>(index.internalPointer());
    if (!item.isNull())
    {
      switch (role)
      {
      case Qt::DisplayRole:
      case Qt::EditRole:
      {
        auto statusArr = item->GetData("status").toJsonArray();
        if(statusArr.size() < 2)
          return QVariant();
        return statusArr[1].toString();
      }
      case Qt::DecorationRole:
      {
        auto statusArr = item->GetData("status").toJsonArray();
        if(statusArr.size() < 3)
          return QVariant();
        auto colorName = statusArr[2].toString();
        if(colorName.isEmpty())
          return QVariant();
        return getCircleIcon(colorName);
      }
      default:
        return TreeModelDynamic::data(index, role);
      }
    }
  }
  else if(index.column() == ColumnLocked)
  {
    QPointer<TreeModelItem> item = static_cast<TreeModelItem*>(index.internalPointer());
    if (!item.isNull() && item->GetData("code_word_lock").toBool())
    {
      switch (role)
      {
      case Qt::DecorationRole:
          return mLockIcon;
      case Qt::ToolTipRole:
          return tr("The project is protected by a code word");
      default:
        return TreeModelDynamic::data(index, role);
      }
    }
    return QVariant();
  }
  return TreeModelDynamic::data(index, role);
}


int TreeModelDynamicProjects::columnCount(const QModelIndex &parent) const
{
  return Columns.size();
}
