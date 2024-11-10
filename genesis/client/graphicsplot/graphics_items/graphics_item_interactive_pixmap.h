#pragma once

#include "../graphicsplot.h"
#include "common_item_fuctional.h"

class GP_LIB_DECL GPItemInteractivePixmap : public GPItemPixmap, public CommonItemFuctional
{
  Q_OBJECT
  Q_PROPERTY(double opacity READ opacity WRITE setOpacity RESET resetOpacity NOTIFY opacityChanged FINAL)
public:
  GPItemInteractivePixmap(GraphicsPlot *parentPlot);

  enum DragMode
  {
    DmNone = 0,
    DmMove,
    DmResizeTopLeft,
    DmResizeTopRight,
    DmResizeBottomRight,
    DmResizeBottomLeft,
    DmResizeTop,
    DmResizeBottom,
    DmResizeLeft,
    DmResizeRight,
    DmRotate
  };

  enum EditableAnchorIndex
  {
    aiTop = GPItemPixmap::aiTop,
    aiTopRight = GPItemPixmap::aiTopRight,
    aiRight = GPItemPixmap::aiRight,
    aiBottom = GPItemPixmap::aiBottom,
    aiBottomLeft = GPItemPixmap::aiBottomLeft,
    aiLeft = GPItemPixmap::aiLeft,
    aiCenter,
    aiRotate,
    aiTopEditable,
    aiTopRightEditable,
    aiRightEditable,
    aiBottomEditable,
    aiBottomLeftEditable,
    aiLeftEditable,
  };

  enum ActionMode
  {
    MoveMode,
    EditMode
  };

  GPItemPosition * const topLeftEditable;
  GPItemPosition * const bottomRightEditable;

  GPItemAnchor * const center;
  GPItemAnchor * const rotateAnchor;
  GPItemAnchor * const topEditable;
  GPItemAnchor * const topRightEditable;
  GPItemAnchor * const rightEditable;
  GPItemAnchor * const bottomEditable;
  GPItemAnchor * const bottomLeftEditable;
  GPItemAnchor * const leftEditable;

  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  void FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions);

  QPointF topLeftCoordinate() { return topLeft->coords(); }
  QPointF bottomRightCoordinate() { return bottomRight->coords(); }

  double opacity() const;
  void setOpacity(double opacity);
  void resetOpacity();

public slots:
  void setEditMode();
  void saveEditModeChanges();
  void rejectEditModeChanges();

  void movePos(QPointF delta);
  void resizePos(QPointF newTopLeft, QPointF newBottomRight);
  void resizeTopLeftEditablePos(QPointF newTopLeft);
  void resizeBottomRightEditablePos(QPointF newBottomRight);

  void setTopLeftPos(QPointF);
  void setBottomRightPos(QPointF);
  void setTopLeftCoordinate(QPointF);
  void setBottomRightCoordinate(QPointF);
  void setTopLeftEditablePos(QPointF);
  void setBottomRightEditablePos(QPointF);
  void setTopLeftEditableCoordinate(QPointF);
  void setBottomRightEditableCoordinate(QPointF);
  virtual void setVisible(bool) override;
  virtual void updateTransform();
  virtual void updateTransformAfterRangeChanged();

  virtual void setRotate(double rotate) override { CommonItemFuctional::setRotate(rotate); }
signals:
  void topLeftChanged(QPointF);
  void bottomRightChanged(QPointF);
  void topLeftEditableChanged(QPointF);
  void bottomRightEditableChanged(QPointF);

  void opacityChanged(double);
  void rotateChanged(double) override;
  void changedPosition(QPointF deltaMoving, unsigned part) override;
  void DragStarted(QPointF, int) override;
  void DragMoved(QPointF) override;
  void DragAccepted(QPointF) override;
  void replot() override;


protected:
  void DragStart(QMouseEvent*, unsigned part) override;
  void move(QPointF deltaMoving);
  void movePosition(QPointF deltaMoving);
  void editPosition(QPointF deltaMoving);

protected:
  virtual void draw(GPPainter *painter) override;
  virtual QPointF anchorPixelPosition(int anchorId) const override;
  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part = nullptr) override;

protected:
  double m_opacity;

  QMap<GPItemAnchor *, int/*DragMode*/> DragModeMap;

  ActionMode m_actionMode;
  QList<QPointF> m_savedPosition;
  QRect m_pixmapVisibleRect;
  QPixmap m_visiblePart;
  QPixmap m_visibleScalePart;
};
