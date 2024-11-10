#ifndef CHROMATOGRAM_KEY_SELECTOR_H
#define CHROMATOGRAM_KEY_SELECTOR_H

#include "graphicsplot/graphicsplot.h"
#include <logic/markup/genesis_markup_forward_declare.h>
#include <logic/markup/genesis_markup_enums.h>

class ChromatogramPlot;

class ChromatogramKeySelector: public GPLayerable
{
  Q_OBJECT
public:
  enum HitPart
  {
    HitPartNone,
    HitPartCenter,
    HitPartPlot
  };
  ChromatogramKeySelector(ChromatogramPlot* plot, GenesisMarkup::CurveDataModelPtr curve, QUuid uid);
  ~ChromatogramKeySelector();
  void setXCoord(double x);

signals:
  void xCoordChangedByUser(double xCoord);

private:
  double mXCoord = 0;
  unsigned mDragPart = HitPartNone;
  GenesisMarkup::CurveDataModelPtr mCurve;

private:
  double closestCurvePoint(double xCoord);

  // GPLayerable interface
public:
  void applyDefaultAntialiasingHint(GPPainter *painter) const override{};
  void draw(GPPainter *painter) override;
  void mousePressEvent(QMouseEvent *event, const QVariant &details) override;
  void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos) override;
  void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos) override;
  Qt::CursorShape HitTest(QMouseEvent *event, unsigned int *part) override;
  double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const override;
};

#endif // CHROMATOGRAM_KEY_SELECTOR_H
