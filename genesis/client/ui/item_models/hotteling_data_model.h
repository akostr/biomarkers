#pragma once
#ifndef HOTTELING_DATA_MODEL_H
#define HOTTELING_DATA_MODEL_H

#include <QAbstractTableModel>

#include "logic/structures/common_structures.h"

namespace Models
{
  class HottelingDataModel final : public QAbstractTableModel
  {
    Q_OBJECT
  public:
    HottelingDataModel(QObject* parent = nullptr);
    ~HottelingDataModel() override = default;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetHeaderData(const Structures::StringList& headers);
    void SetColors(const Structures::StringList2D& colors);
    void SetTableData(const Structures::StringList2D& tableData);

  private:
    Structures::StringList Headers;
    Structures::StringList2D Colors;
    Structures::StringList2D TableData;
  };
}

#endif
