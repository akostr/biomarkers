#include "identification_tree_model.h"
#include "identification_presentation_model.h"
#include <logic/tree_model_item.h>
#include <QJsonObject>
#include <api/network_manager.h>
#include <QToolTip>

IdentificationTreeModel::IdentificationTreeModel(QObject *parent)
  : TreeModel{parent},
  currentKey(0)
{
  Columns[CHECKED].HeaderData[Qt::DisplayRole] = tr("Show on plot");
  Columns[CHECKED].Filter = ColumnInfo::FilterTypeNone;

  Columns[METRICS].HeaderData[Qt::DisplayRole] = tr("Metrics");
  Columns[METRICS].Filter = ColumnInfo::FilterTypeNone;

  Columns[SHORT_TITLE].Ids[Qt::EditRole] = "short_title";
  Columns[SHORT_TITLE].Ids[Qt::DisplayRole] = "short_title";
  Columns[SHORT_TITLE].HeaderData[Qt::DisplayRole] = tr("Short title");
  Columns[SHORT_TITLE].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[FULL_TITLE].Ids[Qt::EditRole] = "full_title";
  Columns[FULL_TITLE].Ids[Qt::DisplayRole] = "full_title";
  Columns[FULL_TITLE].HeaderData[Qt::DisplayRole] = tr("Full title");
  Columns[FULL_TITLE].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[CLASSIFIER_TITLE].Ids[Qt::EditRole] = "classifier_title";
  Columns[CLASSIFIER_TITLE].Ids[Qt::DisplayRole] = "classifier_title";
  Columns[CLASSIFIER_TITLE].HeaderData[Qt::DisplayRole] = tr("Classifier title");
  Columns[CLASSIFIER_TITLE].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[COMPOUND_CLASS_TITLE].Ids[Qt::EditRole] = "compound_class_title";
  Columns[COMPOUND_CLASS_TITLE].Ids[Qt::DisplayRole] = "compound_class_title";
  Columns[COMPOUND_CLASS_TITLE].HeaderData[Qt::DisplayRole] = tr("Compound class title");
  Columns[COMPOUND_CLASS_TITLE].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[TIC_SIM].Ids[Qt::EditRole] = "tic_sim";
  Columns[TIC_SIM].Ids[Qt::DisplayRole] = "tic_sim";
  Columns[TIC_SIM].HeaderData[Qt::DisplayRole] = tr("Tic/sim");
  Columns[TIC_SIM].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[MZ].Ids[Qt::EditRole] = "mz";
  Columns[MZ].Ids[Qt::DisplayRole] = "mz";
  Columns[MZ].HeaderData[Qt::DisplayRole] = tr("Mz");
  Columns[MZ].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[CHEMICAL_FORMULA].Ids[Qt::EditRole] = "chemical_formula";
  Columns[CHEMICAL_FORMULA].Ids[Qt::DisplayRole] = "chemical_formula";
  Columns[CHEMICAL_FORMULA].HeaderData[Qt::DisplayRole] = tr("Chemical formula");
  Columns[CHEMICAL_FORMULA].Filter = ColumnInfo::FilterTypeTextSubstring;

  Columns[DIMENSION].Ids[Qt::EditRole] = "dimension";
  Columns[DIMENSION].Ids[Qt::DisplayRole] = "dimension";
  Columns[DIMENSION].HeaderData[Qt::DisplayRole] = tr("Unit of measurement");
  Columns[DIMENSION].Filter = ColumnInfo::FilterTypeTextSubstring;

  PresentationModel = new IdentificationPresentationModel(this);
}

void IdentificationTreeModel::Load(const QJsonObject &json)
{
  auto compoundsSearchResult = json["compound_search_result"].toObject();
  for(auto matchIter = compoundsSearchResult.begin(); matchIter != compoundsSearchResult.end(); matchIter++)
  {
    KeyData data;
    auto jobj = matchIter->toObject();
    auto jmatchArr = jobj["compound_match_results"].toArray();
    auto jintencity = jobj["intensity_values"].toArray();
    auto jmass = jobj["mass_values"].toArray();

    for(auto it = jmatchArr.begin(); it != jmatchArr.end(); it++)
    {
      auto jobj = it->toObject();
      data.matchData[jobj["library_group_id"].toInt()] = {
          jobj["compound_title"].toString(),
          jobj["metrics"].toDouble()};
    }

    for(auto it = jintencity.begin(); it != jintencity.end(); it++)
      data.intensity << 100 * it->toDouble();

    for(auto it = jmass.begin(); it != jmass.end(); it++)
      data.mass << it->toInt();

    mKeysData[matchIter.key().toDouble()] = data;
  }
  mCheckedRows.clear();
  mUsedCompoundsIds.clear();
  emit spectersCleared();

  auto vm = json.toVariantMap();
  Root->Load(NetworkManagerDetails::TransformTable(vm));
  for(auto& item : Root->GetChildren())
  {
    auto data = item->GetData("mass_spectrum");
    if(data.isValid())
    {
      QVector<double> massSpec;
      for(auto v : data.toList())
        massSpec << v.toInt();
      item->SetData("mass_spectrum", QVariant::fromValue(massSpec));
    }
    data = item->GetData("intensity_spectrum");
    if(data.isValid())
    {
      QVector<double> intensSpec;
      for(auto v : data.toList())
        intensSpec << 100 * v.toDouble();
      item->SetData("intensity_spectrum", QVariant::fromValue(intensSpec));
    }
  }
  setCurrentKey(0);
}

void IdentificationTreeModel::setCurrentKey(double key)
{
  currentKey = key;
  QList<int> whiteListedIds;
  KeyData data = mKeysData.value(key, KeyData());
  whiteListedIds = data.matchData.keys();

  auto pmodel = qobject_cast<IdentificationPresentationModel*>(PresentationModel);
  pmodel->setWhiteListIds(whiteListedIds);
  emit currentSpectreChanged(data.mass, data.intensity);
}


QVariant IdentificationTreeModel::data(const QModelIndex &index, int role) const
{
  if(index.isValid())
  {
    switch(index.column())
    {
    case METRICS:
      if(role == Qt::DisplayRole || role == Qt::EditRole)
      {
        if(!mKeysData.contains(currentKey))
          return QVariant();
        int id = GetItem(index)->GetData("library_group_id").toInt();
        if(!mKeysData[currentKey].matchData.contains(id))
          return QVariant();
        return mKeysData[currentKey].matchData[id].metrics;
      }
      break;
    case CHECKED:
      if(role == Qt::CheckStateRole)
      {
        if(!GetItem(index)->GetData("mass_spectrum").value<QVector<double>>().empty())
          return mCheckedRows.contains(index.row()) ? Qt::Checked : Qt::Unchecked;
        else
          return QVariant();
      }
      else if(role == Qt::DecorationRole)
      {
        if(mUsedCompoundsIds.contains(GetItem(index)->GetData("library_group_id").toInt()))
        {
          return QIcon(":/resource/icons/icon_action_label.png");
        }
        return QVariant();
      }
      else
      {
        if(role == Qt::ToolTipRole)
        {
          if(mUsedCompoundsIds.contains(GetItem(index)->GetData("library_group_id").toInt()))
          {
            return tr("Compaund has already been assigned");
          }
        }
      }
      break;
    }
  }
  return TreeModel::data(index, role);
}

bool IdentificationTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(index.isValid())
  {
    if(index.column() == CHECKED)
    {
      if(role == Qt::CheckStateRole)
      {
        if(value.toBool())
        {
          mCheckedRows.insert(index.row());
          auto item = GetItem(index);
          emit additionalSpectreAdded(item->GetData("mass_spectrum").value<QVector<double>>(),
                                      item->GetData("intensity_spectrum").value<QVector<double>>(),
                                      index.row());
        }
        else
        {
          mCheckedRows.remove(index.row());
          emit additionalSpectreRemoved(index.row());
        }
        return true;
      }
    }
  }
  return TreeModel::setData(index, value, role);
}

void IdentificationTreeModel::setAlredyUsedCompoundsIds(const QSet<int> &usedIds)
{
  QSet<int> changed;
  {
    QSet<int> oldSet = mUsedCompoundsIds;
    QSet<int> newSet = usedIds;
    changed = oldSet.subtract(newSet).unite(newSet.subtract(mUsedCompoundsIds));
  }

  mUsedCompoundsIds = usedIds;
  for(auto& item : Root->GetChildren())
  {
    auto libraryGroupId = item->GetData("library_group_id").toInt();
    if(changed.contains(libraryGroupId))
      emit dataChanged(item->GetIndex(CHECKED), item->GetIndex(CHECKED), {Qt::DecorationRole});
  }
}

Qt::ItemFlags IdentificationTreeModel::flags(const QModelIndex &index) const
{
  auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if(index.column() == CHECKED && !GetItem(index)->GetData("mass_spectrum").value<QVector<double>>().empty())
  {
    flags |= Qt::ItemIsUserCheckable;
  }
  return flags;
}
