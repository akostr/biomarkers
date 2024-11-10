#pragma once
#ifndef MULTI_SELECT_COMBOBOX_H
#define MULTI_SELECT_COMBOBOX_H

#include <QComboBox>

namespace Controls
{
  class MultiSelectComboBox : public QComboBox
  {
    Q_OBJECT

  public:
    explicit MultiSelectComboBox(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model) override;
    void AddTestData(const std::map<int, QString>& testData);
    void SetSelectedItems(const std::vector<int>& selectedItems);

    std::vector<int> SelectedItems() const;

  private:
    void CheckStateChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
      const QList<int>& roles = QList<int>());
  };
}

#endif