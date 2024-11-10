#ifndef CHROMATOGRAMPEAK_H
#define CHROMATOGRAMPEAK_H

#include <QPair>
#include "graphicsplot/graphicsplot.h"
#include <logic/markup/genesis_markup_forward_declare.h>
#include <logic/markup/genesis_markup_enums.h>

class IPeak;
class BaselineCustom;
class ChromatogramPlot;

//  check PeakType and color, add Line with symbol in the middle of peak
//  later do not draw negative fill
//  add select test, draw contour peak (mb)

namespace GenesisMarkup{
class ChromatogramBaseline;

class ChromatogramPeak: public GPLayerable
{
  Q_OBJECT
public:
  enum HitPart
{
  HitPartNone,
  HitPartStart,
  HitPartEnd
};

  ChromatogramPeak(ChromatogramPlot* plot, const GPGraph &graph, double left, double right, QUuid uid);
  void setBaseLine(const ChromatogramBaseline *baseLine);

  void updateGeometry();
  void updateInteractivityFlags(ChromatogramPlot* plot);
  void updateInteractivityFlags(ChromatogramPlot* plot, GenesisMarkup::PeakTypes newPeakType);
  void draw(GPPainter* painter) override;

  Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part) override;
  void applyDefaultAntialiasingHint(GPPainter*) const override {}

  double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details = 0) const override;
  void selectEvent(QMouseEvent* event, bool additive, const QVariant& details, bool* selectionStateChanged) override;
  void deselectEvent(bool* selectionStateChanged) override;

  double leftBorder() const;
  double rightBorder() const;
  int peakId() const;

  void setLeftBorder(double newLeftBorder);
  void setRightBorder(double newRightBorder);
  void setPen(const QPen &newPen);
  void setSelectedPen(const QPen &newSelectedPen);
  void setBrush(const QBrush &newBrush);
  void setSelectedBrush(const QBrush &newSelectedBrush);
  void setBorderLinesPen(const QPen &newBorderLinesPen);
  void setLabelPen(const QPen &newLabelPen);
  void setLabel(const QString &newLabel);

  ChromatogramPeak *getLeftPeak() const;
  ChromatogramPeak *getRightPeak() const;
  void setLeftPeak(ChromatogramPeak *newLeftPeak);
  void setRightPeak(ChromatogramPeak *newRightPeak);
  void setPeakId(int newPeakId);

protected:
//  void mousePressEvent(QMouseEvent *event, const QVariant &details) override;
  const GPGraph* mCurve;
  const ChromatogramBaseline* mBaseLine;
  QVector<QPointF> mCurveCut;
  QVector<QPointF> mBaseLineCut;
  QPolygonF mPolygon;
  QPointF mPeakCenter;
  ChromatogramPlot* mParentPlot;
  double mLeftBorder;
  double mRightBorder;
  QPen mPen;
  QPen mSelectedPen;
  QBrush mBrush;
  QBrush mSelectedBrush;
  QPen mBorderLinesPen;
  QPen mLabelPen;
  QStaticText mLabel;

  bool isSelected = false;

  ChromatogramPeak* leftPeak;
  ChromatogramPeak* rightPeak;
  unsigned mDragPart = HitPartNone;

  //for dependent things, not used yet, doubles the model data for something in future
  int mPeakId = -1;
  QUuid mUid;
  double mMovedBorderOriginalValue;

  bool mSetUnsetMarkerAllowed;
  bool mSetUnsetInterMarkerAllowed;
  bool mBorderMoveAllowed;
  GenesisMarkup::PeakTypes mPeakType;

protected:
  QPair<double, double> interpolateBaseLine(GPCurve *baselineCurve, double leftXPoint, double rightPeakXPoint) const;
  void createBaseLineCut(const ChromatogramBaseline *baseLine);
  void createCurveCut(const GPGraph *Curve);

  static double lerp(const double &x1, const double &y1, const double &x2, const double &y2, const double &x);
  static double lerp(const QPointF& p1, const QPointF& p2, const double &x);
  //will move input val to border of data range if it needed
  static double lerp(double val, const QList<GPGraphData>::const_iterator &iter, QSharedPointer<GPDataContainer<GPGraphData>>data, double eps);
  static double lerp(double val, const QList<GPCurveData>::const_iterator &iter, QSharedPointer<GPDataContainer<GPCurveData>>data, double eps);

  // GPLayerable interface
public:
  QRect clipRect() const override;
  bool getIsSelected() const;
  void setIsSelected(bool newIsSelected);
  void resetIsSelected();
signals:
  void isSelectedChanged();
  void newCommand(QUndoCommand* cmd);
private:
  Q_PROPERTY(bool isSelected READ getIsSelected WRITE setIsSelected RESET resetIsSelected NOTIFY isSelectedChanged)

  // GPLayerable interface
public:
  void mousePressEvent(QMouseEvent *event, const QVariant &details) override;
  void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos) override;
  void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos) override;
  GenesisMarkup::PeakTypes peakType() const;
  void setPeakType(GenesisMarkup::PeakTypes newPeakType);

  QPointF peakCenter() const;
  const QUuid &uid() const;
};


}//GenesisMarkup
#endif // ChromatogrammPeak_H
