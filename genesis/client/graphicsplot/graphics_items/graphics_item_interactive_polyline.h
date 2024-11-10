#ifndef GRAPHICS_ITEM_INTERACTIVE_POLYLINE_H
#define GRAPHICS_ITEM_INTERACTIVE_POLYLINE_H
#include <graphicsplot/graphicsplot.h>
#include "common_item_fuctional.h"

class GraphicsItemInteractivePolyline;
struct PolySegment
{
  PolySegment(){};
  PolySegment(QSharedPointer<QList<GPVector2D>> points, int startIndex, int endIndex,
              std::optional<GPVector2D> startDirection = std::optional<GPVector2D>(),
              std::optional<GPVector2D> endDirection = std::optional<GPVector2D>());

  QSharedPointer<QList<GPVector2D>> pointsArrayPtr;
  int startInd;
  int endInd;
  GPVector2D startDir;//RELATIVE point i.e. (0, 0) - is straight line
  GPVector2D endDir;
  enum SelectPart
  {
    PSSPStart,
    PSSPLine,
    PSSPEnd,
    PSSPNone
  };

  double selectTest(const QPointF& pixelPos, const GraphicsItemInteractivePolyline *item, double selectionTolerance, QVariant* details) const;
  bool isIndexesValid() const;
};

class GraphicsItemInteractivePolyline : public GPAbstractItem, public GPEXDraggingInterface
{
  Q_OBJECT

  Q_PROPERTY(QPen pen READ pen WRITE setPen RESET resetPen NOTIFY penChanged FINAL)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush RESET resetBrush NOTIFY brushChanged FINAL)
  Q_PROPERTY(GPLineEnding lineEnding READ lineEnding WRITE setLineEnding RESET resetLineEnding NOTIFY lineEndingChanged FINAL)

public:
  enum EditableAnchorIndex
  {
    aiTopLeft,
    aiTop,
    aiTopRight,
    aiRight,
    aiBottomRight,
    aiBottom,
    aiBottomLeft,
    aiLeft,
    aiCenter,
    aiRotate
  };
  enum DragMode
  {
    DmNone = 0,
    DmMove,
    DmPoint,
    DmLine,
    DmResizeTopLeft,
    DmResizeTop,
    DmResizeTopRight,
    DmResizeRight,
    DmResizeBottomRight,
    DmResizeBottom,
    DmResizeBottomLeft,
    DmResizeLeft,
    DmRotate
  };
  enum SelectPart
  {
    PLSPPoint,
    PLSPLine,
    PLSPPolygon,
    PLSPAnchor,
    PLSPNone
  };
  enum ActionMode
  {
    MoveMode,
    EditMode
  };
  struct SelectTestDetails
  {
    SelectPart part;
    int index;
  };

  GraphicsItemInteractivePolyline(GraphicsPlot *parentPlot);
  void FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions);
  void setPoints(const QList<GPVector2D>& points);
  void insertPoint(int beforeInd, const GPVector2D& point);
  void prependPoint(const GPVector2D& point);
  void appendPoint(const GPVector2D& point);
  void rebuildSegments();
  bool isClosed() const;
  void closeLine(bool closed = true);
  QSharedPointer<QList<GPVector2D>> points();
  QRectF boundingRect() const;

  GPItemAnchor * const center;
  GPItemAnchor * const rotateAnchor;
  GPItemAnchor * const topLeft;
  GPItemAnchor * const top;
  GPItemAnchor * const topRight;
  GPItemAnchor * const right;
  GPItemAnchor * const bottomRight;
  GPItemAnchor * const bottom;
  GPItemAnchor * const bottomLeft;
  GPItemAnchor * const left;

public slots:
  void movePos(QPointF delta);
  void scale(QPointF coordOriginPt, double scaleX, double scaleY);
  void scaleByDelta(GPItemAnchor* originAnchor, GPItemAnchor* draggingAnchor, QPointF pixelDelta, bool scaleX = true, bool scaleY = true);
  void rotateInCoord(QPointF coordOriginPt, double deg);
  void rotateInPixel(QPointF pixelOriginPt, double deg);
  virtual void setVisible(bool isVisible) override;

private:
  bool destination;
  QSharedPointer<QList<GPVector2D>> mPoints;
  QList<PolySegment> mSegments;
  QMap<int/*anchorIndex*/, int/*DragMode*/> mDragModeMap;
  ActionMode m_actionMode;
  QList<QPointF> m_savedPosition;
  QRect m_pixmapVisibleRect;
  QPixmap m_visiblePart;
  QPixmap m_visibleScalePart;
  QPointF mDragStartPosPixel;
  QPointF mDragCurrentPosPixel;
  QPointF mDragPrevPosPixel;
  QPointF mDragDeltaPixel;
  QPointF mDragRotateCenterCoord;
  int m_draggingInd = -1;
  int m_draggingSavedInd = -1;

  QPen mPen;
  QBrush mBrush;
  GPLineEnding mLineEnding;

protected:
  void DragStart(QMouseEvent *event, unsigned part) override;
  void DragMove(QMouseEvent *event) override;
  void DragAccept(QMouseEvent *event) override;
  void move(QPointF pixelDelta);
  void movePosition(QPointF pixelDelta);
  virtual QPointF anchorPixelPosition(int anchorId) const override;
  QPainterPath calcPath() const;

  // GPLayerable interface
public:
  double selectTest(const QPointF &pixelPos, bool onlySelectable, QVariant *details = nullptr) const override;
  double anchorsSelectTest(const QPointF &pixelPos, QVariant *details = nullptr) const;
  void draw(GPPainter *painter) override;
  void drawAnchors(GPPainter *painter);
  void drawBoundingRect(GPPainter *painter);
  Qt::CursorShape HitTest(QMouseEvent *event, unsigned int *part) override;

  template <typename T>
  T pixelToCoord(const T& some, GPAxis* axisX = nullptr, GPAxis* axisY = nullptr) const
  {
    return mParentPlot->pixelToCoord(some, axisX, axisY);
  }
  template <typename T>
  T coordToPixel(const T& some, GPAxis* axisX = nullptr, GPAxis* axisY = nullptr) const
  {
    return mParentPlot->coordToPixel(some, axisX, axisY);
  }

  QPen pen() const;
  void setPen(const QPen &newPen);
  void resetPen();

  QBrush brush() const;
  void setBrush(const QBrush &newBrush);
  void resetBrush();

  GPLineEnding lineEnding() const;
  void setLineEnding(const GPLineEnding &newLineEnding);
  void resetLineEnding();

signals:
  void replot();
  void penChanged();
  void brushChanged();
  void lineEndingChanged();
  void pointsChanged();
  void lineClosedStateChanged();
};

#endif // GRAPHICS_ITEM_INTERACTIVE_POLYLINE_H
