#pragma once
#ifndef PLS_TABLE_ITEM_MODEL_H
#define PLS_TABLE_ITEM_MODEL_H

#include <QAbstractTableModel>

#include "logic/structures/pls_model_structures.h"

namespace Model
{
  using namespace Structures;

  class PlsTableItemModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    explicit PlsTableItemModel(QObject* parent);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetModelData(const std::vector<PlsModel>& new_data);
    void AppendItem(const PlsModel& item);

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void setIconPath(const QString& icon_path);

    PlsModel at(int index) const;

  private:
    std::vector<PlsModel> PlsData;
    QString IconPath;
    QStringList HeaderData;
  };
}
#endif