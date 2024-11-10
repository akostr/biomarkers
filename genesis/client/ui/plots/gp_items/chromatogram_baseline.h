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
  QPair<double, GPCurveDataContainer::const_iterator> closestPoint(QPoint pixelPos);

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
