#ifndef CHROMATOGRAMINTERVAL_H
#define CHROMATOGRAMINTERVAL_H

#include "graphicsplot/graphicsplot.h"
#include <logic/markup/genesis_markup_forward_declare.h>
#include <logic/markup/genesis_markup_enums.h>
class ChromatogramPlot;

using namespace GenesisMarkup;

namespace GenesisMarkup{
class ChromatogramBaseline;

class ChromatogramInterval: public GPLayerable
{
  Q_OBJECT
public:
  enum HitPart
  {
    HitPartNone,
    HitPartStart,
    HitPartEnd
  };
  ChromatogramInterval(ChromatogramPlot* plot, double left, double right, QUuid uid);
  ~ChromatogramInterval(){};

  void updateGeometry();

  void updateInteractivityFlags(ChromatogramPlot* plot);
  void updateInteractivityFlags(ChromatogramPlot* plot, GenesisMarkup::IntervalTypes newIntervalType);
  void draw(GPPainter* painter) override;

  Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part) override;
  void applyDefaultAntialiasingHint(GPPainter*) const override {}

  double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details = 0) const override;
  void selectEvent(QMouseEvent* event, bool additive, const QVariant& details, bool* selectionStateChanged) override;
  void deselectEvent(bool* selectionStateChanged) override;

  double leftBorder() const;
  double rightBorder() const;
  void setLeftBorder(double newLeftBorder);
  void setRightBorder(double newRightBorder);


public:
//  QRect clipRect() const override;//необязательно, вроде
  bool getIsSelected() const;
  void setIsSelected(bool newIsSelected);
  void resetIsSelected();

  void mousePressEvent(QMouseEvent *event, const QVariant &details) override;
  void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos) override;
  void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos) override;
  GenesisMarkup::IntervalTypes intervalType() const;
  void setIntervalType(GenesisMarkup::IntervalTypes newIntervalType);

  ChromatogramInterval *getLeftInterval() const;
  void setLeftInterval(ChromatogramInterval *newLeftInterval);
  ChromatogramInterval *getRightInterval() const;
  void setRightInterval(ChromatogramInterval *newRightInterval);

signals:
  void isSelectedChanged();
  void newCommand(QUndoCommand* cmd);

protected:
  double mMovedBorderOriginalValue;
  bool mBorderMoveAllowed;
  GenesisMarkup::IntervalTypes mIntervalType;

private:
  bool isSelected = false;
  unsigned mDragPart = HitPartNone;
  Q_PROPERTY(bool isSelected READ getIsSelected WRITE setIsSelected RESET resetIsSelected NOTIFY isSelectedChanged)

  const GPGraph* mCurve;
  const ChromatogramBaseline* mBaseLine;
  QVector<QPointF> mCurveCut;
  QVector<QPointF> mBaseLineCut;
  QPolygonF mPolygon;
  QPointF mIntervalCenter;
  ChromatogramPlot* mParentPlot;
  double mLeftBorder;
  double mRightBorder;


  ChromatogramInterval* leftInterval;
  ChromatogramInterval* rightInterval;
  QPen mCustomPen;
  QPen mLockedPen;
  QBrush mCustomBrush;
  QBrush mLockedBrush;
  QBrush mSelectedCustomBrush;
  QBrush mSelectedLockedBrush;
  QPen mLabelPen;
  QStaticText mLabel;

  int mIntervalId = -1;
  QUuid mUid;


  // GPLayerable interface
public:
  QRect clipRect() const override;
};
}
#endif // CHROMATOGRAMINTERVAL_H
