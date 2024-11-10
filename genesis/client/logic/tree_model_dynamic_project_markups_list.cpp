#include "tree_model_dynamic_project_markups_list.h"
#include "tree_model_item.h"

#include "context_root.h"
#include <genesis_style/style.h>
#include <logic/known_context_tag_names.h>

#include "../api/api_rest.h"

#include "../logic/notification.h"

#include <QVariantMap>
#include <QDateTime>
#include <QPainter>

#define COLOR_BLUE  "#0071B2"
//#define COLOR_GREEN "#22C38E"


using namespace Core;

TreeModelDynamicProjectMarkupsList::TreeModelDynamicProjectMarkupsList(QObject* parent)
  : TreeModelDynamic(parent)
{
  //// Setup columns
  Columns[ColumnTitle].Ids[Qt::EditRole]                     = "title";
  Columns[ColumnTitle].Ids[Qt::DisplayRole]                  = "title";
  Columns[ColumnTitle].HeaderData[Qt::DisplayRole]           = tr("Name");
  Columns[ColumnTitle].Filter                                = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnStatus].Ids[Qt::EditRole]                     = "is_finished";
  Columns[ColumnStatus].Ids[Qt::DisplayRole]                  = "is_finished";
  Columns[ColumnStatus].HeaderData[Qt::DisplayRole]           = tr("Status");
  Columns[ColumnStatus].Filter                                = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnReference].Ids[Qt::EditRole]                     = "filename";
  Columns[ColumnReference].Ids[Qt::DisplayRole]                  = "filename";
  Columns[ColumnReference].HeaderData[Qt::DisplayRole]           = tr("Reference");
  Columns[ColumnReference].Filter                                = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnCount].Ids[Qt::EditRole]                     = "count";
  Columns[ColumnCount].Ids[Qt::DisplayRole]                  = "count";
  Columns[ColumnCount].HeaderData[Qt::DisplayRole]           = tr("Count");
  Columns[ColumnCount].Filter                                = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnComment].Ids[Qt::EditRole]                     = "comment";
  Columns[ColumnComment].Ids[Qt::DisplayRole]                  = "comment";
  Columns[ColumnComment].Ids[Qt::DisplayRole]                  = "comment";
  Columns[ColumnComment].HeaderData[Qt::DisplayRole]           = tr("Comment");
  Columns[ColumnComment].Filter                                = ColumnInfo::FilterTypeTextSubstring;

  Columns[ColumnChanged].Ids[Qt::EditRole]                   = "changed";
  Columns[ColumnChanged].Ids[Qt::DisplayRole]                = "changed";
  Columns[ColumnChanged].HeaderData[Qt::DisplayRole]         = tr("Changed");
  Columns[ColumnChanged].Filter                              = ColumnInfo::FilterTypeDateRange;

  //// Setup item actions
  ItemActions =
  {
    { "edit", QString(), tr("Edit title and comment") },
    { "remove", QString(), tr("Remove markup") },
  };

  Reset();
}

TreeModelDynamicProjectMarkupsList::~TreeModelDynamicProjectMarkupsList()
{
}

void TreeModelDynamicProjectMarkupsList::ApplyContextProject(const QString &dataId, const QVariant &data)
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

void TreeModelDynamicProjectMarkupsList::Reset()
{
  //// Clear
  TreeModelDynamic::Reset();

  //// Fetch
//  if (canFetchMore(QModelIndex()))
//  {
//    FetchMore(Root);
//  }
}

bool TreeModelDynamicProjectMarkupsList::canFetchMore(const QModelIndex& parent) const
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

void TreeModelDynamicProjectMarkupsList::fetchMore(const QModelIndex& parent)
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

void TreeModelDynamicProjectMarkupsList::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetProjectMarkups(
      GenesisContextRoot::Get()->ProjectId(),
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
        Notification::NotifyError(tr("Failed to get markups list from database"), err);
      }
    );
  }
}

QIcon TreeModelDynamicProjectMarkupsList::getCircleIcon(QString colorName) const
{
  if(colorName.isEmpty())
    return QIcon();
  if(!mCirclesIconsCache.contains(colorName))
  {
    QPixmap pmap(30,30);
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


QVariant TreeModelDynamicProjectMarkupsList::data(const QModelIndex &index, int role) const
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
        auto status = item->GetData("is_finished").toBool();
        if(!status)
        {
          return tr("In process");
        }
        else
        {
          return tr("Completed");
        }
      }
      case Qt::DecorationRole:
      {
        auto status = item->GetData("is_finished").toBool();
        QString colorName;
        if(!status)
        {
          colorName = COLOR_BLUE;
        }
        else
        {
          colorName = Style::GetSASSValue("successColor");
//          colorName = COLOR_GREEN;
        }
        return getCircleIcon(colorName);
      }
      default:
        return TreeModelDynamic::data(index, role);
      }
    }
  }
  return TreeModelDynamic::data(index, role);
}
