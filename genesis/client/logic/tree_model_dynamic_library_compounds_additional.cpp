#include "tree_model_dynamic_library_compounds_additional.h"
#include "logic/tree_model_item.h"
#include "logic/notification.h"
#include "api/api_rest.h"

TreeModelDynamicLibraryCompoundsAdditional::TreeModelDynamicLibraryCompoundsAdditional(QObject *parent, ElementType type)
  : TreeModelDynamicLibraryCompounds{type, parent}
{
//  Columns[static_cast<int>(Column::GROUP)].Ids[Qt::EditRole] = "groups_names_list";
//  Columns[static_cast<int>(Column::GROUP)].Ids[Qt::DisplayRole] = "groups_names_list";
//  Columns[static_cast<int>(Column::GROUP)].HeaderData[Qt::DisplayRole] = tr("Groups");
  Columns.remove(Column::CHECKED);
}

void TreeModelDynamicLibraryCompoundsAdditional::FindRelated(const QString &shortTitle, const QString &fullTitle)
{
  ClearRelated();
  API::REST::Tables::PostCheckRelatedLibraryItems((int)Type,
                                                  shortTitle,
                                                  fullTitle,
    [this](QNetworkReply*, QVariantMap result)
    {
//      qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(result)).toJson(QJsonDocument::Compact);
      LoadRelated(result);
    },
    [this](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      ClearRelated();
      QMap <ElementType, QString> typeName = {{ElementType::COMPOUND, tr("compounds")},
                                              {ElementType::COEFFICIENT, tr("coefficients")},
                                              {ElementType::PLOT_TEMPLATE, tr("template")}};
      Notification::NotifyError(tr("Failed to check related %1 from database").
                                arg(typeName[Type]),
                                err);
    }
    );
}


void TreeModelDynamicLibraryCompoundsAdditional::FetchMore(QPointer<TreeModelItem> item)
{
  FetchMore(item,
            MainItem()->GetData("short_title").toString(),
            MainItem()->GetData("full_title").toString());
}

void TreeModelDynamicLibraryCompoundsAdditional::FetchMore(QPointer<TreeModelItem> item, const QString &shortTitle, const QString &fullTitle)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);
    ClearRelated();
  }
}


void TreeModelDynamicLibraryCompoundsAdditional::Clear()
{
  beginResetModel();
  MainItem()->SetData(QVariantMap());
  ClearRelated();
  endResetModel();
}

void TreeModelDynamicLibraryCompoundsAdditional::ClearRelated()
{
  for(auto& item : Root->GetChildren())
  {
    if(item == MainItem())
      continue;
    delete item;
  }
}

void TreeModelDynamicLibraryCompoundsAdditional::LoadRelated(const QVariantMap &data)
{
  beginResetModel();

  MainItem();
  //// Children
  QVariantList childrenArray = data["children"].toList();
  int row = 1;
  if(!childrenArray.isEmpty())
  {
    auto subHeaderItem = new TreeModelItem(Root, row++);
//    subHeaderItem->SetData("subheader", tr("found %n match(es)", "", childrenArray.size()));
    mSubHeaderIndex = subHeaderItem->GetIndex(1);// not 0, cause 0 we filtering
    emit subHeaderIndexChanged(mSubHeaderIndex);
  }
  else
  {
    mSubHeaderIndex = QModelIndex();
    emit subHeaderIndexChanged(mSubHeaderIndex);
  }
  QMap<int, TreeModelItem*> uniqueCompounds;
  for (auto& a : childrenArray)
  {
    QVariantMap object = a.toMap();
    TreeModelItem* child = new TreeModelItem(Root, row);
    child->Load(object);
    child->SetData("groups_names_list", QStringList({child->GetData("group").toString()}));
    int libElemId = child->GetData("library_element_id").toInt();
    if(uniqueCompounds.contains(libElemId))
    {
      auto groupsList = uniqueCompounds[libElemId]->GetData("groups_names_list").toStringList();
      groupsList.append(child->GetData("groups_names_list").toStringList());
      uniqueCompounds[libElemId]->SetData("groups_names_list", groupsList);
      delete child;
    }
    else
    {
      uniqueCompounds.insert(libElemId, child);
    }
    ++row;
  }
  if(mSubHeaderIndex.isValid())
    GetItem(mSubHeaderIndex)->SetData("subheader", tr("found %n match(es)", "", uniqueCompounds.size()));
  endResetModel();
}

QModelIndex TreeModelDynamicLibraryCompoundsAdditional::SubHeaderIndex() const
{
  return mSubHeaderIndex;
}

TreeModelItem *TreeModelDynamicLibraryCompoundsAdditional::MainItem()
{
  if(!mMainItem)
  {
    mMainItem = new TreeModelItem(Root, 0);
    connect(mMainItem, &QObject::destroyed, this,
            [this](){mMainItem = nullptr;});
  }
  return mMainItem;
}


QVariant TreeModelDynamicLibraryCompoundsAdditional::data(const QModelIndex &index, int role) const
{
  if(mSubHeaderIndex.isValid() && index == mSubHeaderIndex)
  {
    qDebug() << index << role;
    switch(role)
    {
    case Qt::DecorationRole:
      return QIcon(":/resource/icons/icon_warning.png");
    case Qt::DisplayRole:
      return GetItem(index)->GetData("subheader").toString();
    default:
      break;
    }
  }
  return TreeModelDynamicLibraryCompounds::data(index, role);
}
