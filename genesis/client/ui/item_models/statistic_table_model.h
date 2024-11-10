#pragma once
#ifndef STATISTIC_TABLE_MODEL_H
#define STATISTIC_TABLE_MODEL_H

#include <QAbstractTableModel>
//#include <QObject>
//#include <QVariant>

#include "logic/structures/analysis_model_structures.h"
namespace Structures
{
  class PlsStatistic;
}

namespace Model
{
  using namespace Structures;

  class StatisticTableModel : public QAbstractTableModel
  {
    Q_OBJECT

  public:
    explicit StatisticTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetStatistic(const PlsStatistic* statisticData);

  private:
    std::vector<QString> HorizontalHeaders;
    std::vector<QString> VerticalHeaders;
    // check count value according vertical headers * horizontal headers
    std::array<QVariant, 10> StatisticData = { };
  };
  using StatisticTableModelPtr = std::shared_ptr<StatisticTableModel>;
}
#endif
