#pragma once
#ifndef PLOT_WIDGET_H
#define PLOT_WIDGET_H

#include <QWidget>

#include "logic/structures/pca_data_structures.h"
#include "logic/structures/analysis_model_structures.h"
#include <ui/plots/gp_items/gpshape_with_label_item.h>

#include "ui/item_models/object_with_model.h"

namespace Widgets
{
  using namespace Objects;
  using namespace Structures;

  class PlotWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit PlotWidget(QWidget* parent = nullptr);
    virtual ~PlotWidget() = default;

    void SetModel(IdentityModelPtr model);

  protected:
    ObjectWithModelPtr ObjectModel = nullptr;

    std::optional<Component> FindComponentByIndex(int index) const;
    std::optional<PlsComponent> FindPlsComponentByIndex(int index) const;
    GPRange CreateRangeFromVector(const QVector<double>& values, double eps = 1);
    QString GenerateAxisLabel(int number, double explPCsVariance) const;

    GPShapeWithLabelItem* CreateCircle(GraphicsPlot* plot,
      const QPointF& point, const QString& title,
      GPShapeItem::DefaultColor color, QSize size = {7, 7});

    GPItemStraightLine* CreateLine(GraphicsPlot* plot, int width,
      const QPointF& p1, const QPointF& p2, GPShapeItem::DefaultColor color);

    void AppendCurve(GraphicsPlot* plot,
      const QVector<double>& xCoords, const QVector<double>& yCoords,
      GPShapeItem::DefaultColor color,
      int width = 5, GPCurve::LineStyle style = GPCurve::lsLine,
      const QString& curveName = "");

    void AppendCurve(GraphicsPlot *plot,
                     const QVector<double>& xCoords, const QVector<double>& yCoords,
                     QColor color,
                     int width = 5, GPCurve::LineStyle style = GPCurve::lsLine,
                     const QString& curveName = "");

  private:
    void SetupUi();

  protected:
    // bool eventFilter(QObject* watched, QEvent* event) override;

    QObjectList newChildren;

    // void childEvent(QChildEvent* event) override;
  };
  using PlotWidgetPtr = QSharedPointer<PlotWidget>;
}

#endif
