#pragma once
#ifndef CONCENTRATION_TABLE_MODEL_H
#define CONCENTRATION_TABLE_MODEL_H

#include <QAbstractTableModel>
#include "logic/structures/common_structures.h"

namespace Models
{
  class ConcentrationTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    explicit ConcentrationTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetConcentrationTable(Structures::ConcetrantionTable&& table);

  private:
    std::vector<QString> HorizontalHeaders;
    Structures::ConcetrantionTable TableData;
  };
}
#endif