#pragma once
#ifndef NUMERIC_DATA_TABLE_MODEL_H
#define NUMERIC_DATA_TABLE_MODEL_H

#include <QAbstractTableModel>

#include "logic/structures/analysis_model_structures.h"
#include "logic/markup/genesis_markup_forward_declare.h"
#include "logic/markup/abstract_data_model.h"

using Structures::AnalysisMetaInfoList;

namespace Models
{
  class NumericDataTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    enum NumericDataRoles {ChromaIdRole = Qt::UserRole + 1};
    NumericDataTableModel(QObject* parent = nullptr);
    ~NumericDataTableModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void Clear();
    void ClearHorizontalHeaders();
    void SetModel(GenesisMarkup::MarkupModelPtr model);
    void SetPassportData(const QList<AbstractEntityDataModel::ConstDataPtr>& data);
    void UpdateData();

    void SetCurrentParameter(int param);

  private:
    std::vector<QString> HorizontalHeaders;
    GenesisMarkup::MarkupModelPtr MarkupModel = nullptr;
    int Parameter = -1;
    QStringList Names;
    QList<int> Ids;
    QMap<QString, QMap<QString, AbstractEntityDataModel::ConstDataPtr>> Rows;
    QList<AbstractEntityDataModel::ConstDataPtr> PassportData;

    void Append(int id, const QList<AbstractEntityDataModel::ConstDataPtr>& values);
  };
}
#endif
