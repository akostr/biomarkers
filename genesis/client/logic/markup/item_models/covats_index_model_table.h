#pragma once
#ifndef COVATS_INDEX_MODEL_TABLE_H
#define COVATS_INDEX_MODEL_TABLE_H

#include <QAbstractTableModel>

#include "logic/markup/chromatogram_data_model.h"
#include "logic/markup/abstract_data_model.h"

class QUndoCommand;
namespace Models
{
  class CovatsIndexModelTable : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    CovatsIndexModelTable(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void SetModel(GenesisMarkup::MarkupModelPtr model);
    void onEntityChanged(EntityType type,
                         TEntityUid eId,
                         DataRoleType role,
                         const QVariant& value);
    void onEntityResetted(EntityType type,
                        TEntityUid eId,
                        AbstractEntityDataModel::ConstDataPtr peakData);

    void onEntityAdded(EntityType type,
                     TEntityUid eId,
                     AbstractEntityDataModel::ConstDataPtr peakData);
    void onEntityAboutToRemove(EntityType type,
                       TEntityUid eId);
    void onMarkupDataChanged(uint role, const QVariant& data);
    void onModelAboutToReset();
    void onModelReset();

    void AddData(const TEntityUid uid);
    void RemoveData(const TEntityUid uid);

    void Clear();
    void CalculateIndices();

  signals:
    void newCommand(QUndoCommand* cmd);

  private:
    void connectModelSignals(GenesisMarkup::ChromatogrammModelPtr model);

    GenesisMarkup::ChromatogrammModelPtr mChromatogramModel;
    GenesisMarkup::MarkupModelPtr mMarkupModel;
    std::vector<QString> HorizontalHeaders;
    std::vector<AbstractEntityDataModel::ConstDataPtr> TableData;
//    std::map<double, AbstractEntityDataModel::EntityUId> UidByRetTime;
  };
}
#endif
