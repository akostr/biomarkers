#ifndef CHROMATOGRAMBASELINE_H
#define CHROMATOGRAMBASELINE_H

#include "graphicsplot/graphicsplot_extended.h"
#include "logic/markup/genesis_markup_enums.h"
class ChromatogramPlot;
namespace GenesisMarkup{

class ChromatogramBaseline: public GPInteractiveCurve
{
  Q_OBJECT
public:
  ChromatogramBaseline(ChromatogramPlot * Plot, QUuid uid);
  ~ChromatogramBaseline();
  void setBaseLineModel(BaseLineDataModelPtr blineModel);
  void addPointToBaseLine(double key, double value);
  void removePointFromBaseLine(double key);
  void removePointsFromBaseLine(QList<double> keys);
  void removeSelectedPointsFromBaseLine();
  void RotateLine(double deltaAngle); // Поворот линии
  QPointF calculateLineCenter();  // Функция для вычисления центра линии
  QPair<double, GPCurveDataContainer::const_iterator> closestPoint(QPoint pixelPos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;  // Начало поворота
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;   // Поворот при движении мыши
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override; // Завершение поворота

signals:
  void newCommand(QUndoCommand* cmd);


  // GPEXDraggingInterface interface
public:
  void DragAccept(QMouseEvent *e) override;
  void DragStart(QMouseEvent *e, unsigned part) override;
  void DragStart(QMouseEvent *event, GPCurveData *dataPoint) override;
  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part) override;
  virtual void DragUpdateData(QMouseEvent* event) override;

private:
  Q_PROPERTY(bool isSelected READ selected)
  QPair<QVector<double>, QVector<double>> mBaseLineBeforeDragging;
  QPair<QVector<double>, QVector<double>> mBaseLineAfterDragging;
  BaseLineDataModelPtr mBaseLineModel;
  QUuid mUid;
  ChromatogramPlot* mParentPlot;
};

}//GenesisMarkup

#endif // BASELINECUSTOM_H
