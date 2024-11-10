#pragma once
#ifndef CHECKABLE_STRING_LIST_MODEL_H
#define CHECKABLE_STRING_LIST_MODEL_H

#include <QAbstractTableModel>

#include "logic/structures/analysis_model_structures.h"

namespace Models
{
  class CheckableStringListModel final : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    CheckableStringListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QString SelectedNames() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    std::vector<int> GetCheckedData() const;
    void SetCheckedItems(const std::vector<int>& items);
    void SetTestData(const std::map<int, QString>& items);

    int SelectedCount() const;

  private:
    QSet<QPersistentModelIndex> checkedItems;
    std::map<int, QString> TestData;
  };
}
#endif