#include "statistic_table_model.h"

#include <QString>
#include <QColor>
// #include <graphicsplot/graphicsplot_extended.h>
#include <ui/plots/gp_items/gpshape_item.h>

namespace Model
{
  const static std::vector<QColor> ForegroundColors{GPShapeItem::ToColor(GPShapeItem::Blue), GPShapeItem::ToColor(GPShapeItem::Red)};

  StatisticTableModel::StatisticTableModel(QObject* parent)
    :QAbstractTableModel(parent)
    , HorizontalHeaders
    {
      QObject::tr("Parameters"),
      QObject::tr("Slope"),
      QObject::tr("Offset"),
      QObject::tr("RMSE"),
      QObject::tr("R.Squared"),
    }
    , VerticalHeaders
    {
      QObject::tr("Graduirovka"),
      QObject::tr("Proverka"),
    }
  {
  }

  int StatisticTableModel::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(VerticalHeaders.size());
  }

  int StatisticTableModel::columnCount(const QModelIndex& parent) const
  {
    return static_cast<int>(HorizontalHeaders.size());
  }

  QVariant StatisticTableModel::data(const QModelIndex& index, int role) const
  {
    const auto rowIndex = index.row();
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      // represent 2d array in 1d array
      const auto indexArray = rowIndex * columnCount() + index.column();
      if (indexArray > static_cast<int>(StatisticData.size()))
        return QVariant();

      return StatisticData[indexArray];
    }

    if (role == Qt::ForegroundRole
      && rowIndex > -1
      && rowIndex < static_cast<int>(ForegroundColors.size()))
    {
      return ForegroundColors[rowIndex];
    }

    if (role == Qt::TextAlignmentRole)
    {
      return Qt::AlignLeft;
    }

    return QVariant();
  }

  QVariant StatisticTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const
  {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      switch (orientation)
      {
      case Qt::Horizontal:
        return HorizontalHeaders[section];
      default:
        return QVariant();
      }
    }

    if (role == Qt::ForegroundRole
      && orientation == Qt::Vertical
      && section > -1
      && section < static_cast<int>(ForegroundColors.size()))
    {
      return QColor(ForegroundColors[section]);
    }

    if (role == Qt::TextAlignmentRole)
    {
      return Qt::AlignLeft;
    }

    return QVariant();
  }

  void StatisticTableModel::SetStatistic(const PlsStatistic* statisticData)
  {
    // append row by columns
    // row 0
    StatisticData[0] = VerticalHeaders[0];
    StatisticData[1] = statisticData->Slope.Graduirovka;
    StatisticData[2] = statisticData->Offset.Graduirovka;
    StatisticData[3] = statisticData->RMSE.Graduirovka;
    StatisticData[4] = statisticData->R2.Graduirovka;

    // row 1
    StatisticData[5] = VerticalHeaders[1];
    StatisticData[6] = statisticData->Slope.Proverka;
    StatisticData[7] = statisticData->Offset.Proverka;
    StatisticData[8] = statisticData->RMSE.Proverka;
    StatisticData[9] = statisticData->R2.Proverka;

    emit dataChanged(QModelIndex(), QModelIndex());
  }
}
