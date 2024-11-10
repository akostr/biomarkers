#ifndef GPITEMINTERACTIVELINE_H
#define GPITEMINTERACTIVELINE_H

#include <QObject>
#include "../graphicsplot.h"
#include "common_item_fuctional.h"

class GPItemInteractiveLine : public GPItemLine, public CommonItemFuctional
{
  Q_OBJECT
public:
  enum DragMode
  {
    DmNone = 0,
    DmMove,
    DmMoveStart,
    DmMoveEnd
  };

  explicit GPItemInteractiveLine(GraphicsPlot *parentPlot);

  void setStartCoord(QPointF coord);
  void setStartPixel(QPointF pixel);
  void setEndCoord(QPointF coord);
  void setEndPixel(QPointF pixel);

signals:
  virtual void rotateChanged(double);
  virtual void changedPosition(QPointF , unsigned part);
  virtual void DragStarted(QPointF, int);
  virtual void DragMoved(QPointF);
  virtual void DragAccepted(QPointF);

  virtual void replot();
  void startCoordinateChanged(QPointF coord);
  void endCoordinateChanged(QPointF coord);

protected:
  void move(QPointF deltaMoving);

  virtual void draw(GPPainter *painter) override;
  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part = nullptr) override;
};

#endif // GPITEMINTERACTIVELINE_H
