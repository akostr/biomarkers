#ifndef COMPLETERMODEL_H
#define COMPLETERMODEL_H

#include <QAbstractListModel>
using ItemData = QMap<int, QVariant>;
class CompleterModel : public QAbstractListModel
{
  Q_OBJECT
public:
  explicit CompleterModel(QObject *parent = nullptr);

  // QAbstractItemModel interface
public:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;
  QList<ItemData> mData;

  void appendRow(QString displayData, const QVariant& userData = QVariant());
};

#endif // COMPLETERMODEL_H
