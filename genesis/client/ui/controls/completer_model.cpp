#include "completer_model.h"

CompleterModel::CompleterModel(QObject *parent)
  : QAbstractListModel{parent}
{
}

int CompleterModel::rowCount(const QModelIndex &parent) const
{
  return mData.size();
}

QVariant CompleterModel::data(const QModelIndex &index, int role) const
{
  if(index.row() < 0 || index.row() > rowCount())
    return QVariant();
  if(!index.isValid())
    return QVariant();
  return mData[index.row()].value(role);
}

bool CompleterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(index.row() < 0 || index.row() > rowCount(QModelIndex()))
    return false;
  if(!index.isValid())
    return false;
  mData[index.row()][role] = value;
  emit dataChanged(index, index, {role});
  return true;
}

void CompleterModel::appendRow(QString displayData, const QVariant &userData)
{
  ItemData itemData;
  itemData[Qt::DisplayRole] = itemData[Qt::EditRole] = displayData;
  if(userData.isValid())
    itemData[Qt::UserRole] = userData;

  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  mData.append(itemData);
  endInsertRows();
}
