#pragma once
#ifndef PLS_STATISTIC_WIDGET_H
#define PLS_STATISTIC_WIDGET_H

#include <QWidget>
#include "ui/item_models/statistic_table_model.h"

namespace Ui
{
  class PlsStatisticWidget;
}
namespace Style
{
QString statisticsTableStyle();
QString statisticsPlateStyle();

}

namespace Style
{
QString statisticsTableStyle();
QString statisticsPlateStyle();

}

namespace Widgets
{
  using namespace Model;

  class PlsStatisticWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit PlsStatisticWidget(QWidget* parent = nullptr);
    ~PlsStatisticWidget();

    void SetStatistic(const PlsStatistic* statisticData);

  protected:

    Ui::PlsStatisticWidget* WidgetUi = nullptr;
    StatisticTableModelPtr StatisticModel = nullptr;

    // QWidget interface
  protected:
    void paintEvent(QPaintEvent *event);
  };
}
#endif
