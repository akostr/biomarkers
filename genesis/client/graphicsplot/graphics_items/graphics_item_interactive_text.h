#pragma once

#include "../graphicsplot.h"
#include "common_item_fuctional.h"
#include <QObject>

class GP_LIB_DECL GPItemInteractiveText : public GPItemText, public CommonItemFuctional
{
  Q_OBJECT
public:
  enum DragMode
  {
    DmNone = 0,
    DmMove,
    DmRotate
  };

  enum EditableAnchorIndex
  {
    aiTopLeft = GPItemText::aiTopLeft,
    aiTop = GPItemText::aiTop,
    aiTopRight = GPItemText::aiTopRight,
    aiRight = GPItemText::aiRight,
    aiBottomRight = GPItemText::aiBottomRight,
    aiBottom = GPItemText::aiBottom,
    aiBottomLeft = GPItemText::aiBottomLeft,
    aiLeft = GPItemText::aiLeft,
    aiRotate,
  };

  GPItemAnchor * const rotateAnchor;

  explicit GPItemInteractiveText(GraphicsPlot *parentPlot);

  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;

public slots:
  void movePosition(QPointF delta);
  void setPosition(QPointF);
  void setPositionAsCoordinate(QPointF);
  virtual void setVisible(bool) override;

signals:
  void rotateChanged(double) override;
  void changedPosition(QPointF , unsigned part) override;
  void DragStarted(QPointF, int) override;
  void DragMoved(QPointF) override;
  void DragAccepted(QPointF) override;
  void replot() override;

  void positionChanged(QPointF);

protected:
  void move(QPointF deltaMoving);

  virtual void draw(GPPainter *painter) override;
  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part = nullptr) override;
  virtual void setTransform(QPointF center, double moveRotationAngle);
  virtual QPointF anchorPixelPosition(int anchorId) const override;
};

