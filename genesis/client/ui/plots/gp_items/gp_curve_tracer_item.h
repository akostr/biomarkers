#ifndef GPCurveTracerItem_H
#define GPCurveTracerItem_H

#include <graphicsplot/graphicsplot_extended.h>

class GPCurveTracerItem : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
  Q_PROPERTY(QBrush selectedBrush READ selectedBrush WRITE setSelectedBrush)
  Q_PROPERTY(double size READ size WRITE setSize)
  Q_PROPERTY(TracerStyle style READ style WRITE setStyle)
  Q_PROPERTY(GPCurve* curve READ curve WRITE setCurve)
  Q_PROPERTY(double curveKey READ curveKey WRITE setCurveKey)
  Q_PROPERTY(bool interpolating READ interpolating WRITE setInterpolating)
  /// \endcond
public:


public:
  /*!
    The different visual appearances a tracer item can have. Some styles size may be controlled with \ref setSize.

    \see setStyle
  */
  enum TracerStyle { tsNone        ///< The tracer is not visible
                     ,tsPlus       ///< A plus shaped crosshair with limited size
                     ,tsCrosshair  ///< A plus shaped crosshair which spans the complete axis rect
                     ,tsCircle     ///< A circle
                     ,tsSquare     ///< A square
                   };
  Q_ENUM(TracerStyle)

  explicit GPCurveTracerItem(GraphicsPlot *parentPlot);
  virtual ~GPCurveTracerItem();

  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  QBrush brush() const { return mBrush; }
  QBrush selectedBrush() const { return mSelectedBrush; }
  double size() const { return mSize; }
  TracerStyle style() const { return mStyle; }
  GPCurve *curve() const { return mCurve; }
  double curveKey() const { return mCurveKey; }
  bool interpolating() const { return mInterpolating; }

  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setSelectedBrush(const QBrush &brush);
  void setSize(double size);
  void setStyle(TracerStyle style);
  void setCurve(GPCurve *curve);
  void setCurveKey(double key);
  void setInterpolating(bool enabled);

  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;

  // non-virtual methods:
  void updatePosition();

  GPItemPosition * const position;

protected:
  // property members:
  QPen mPen, mSelectedPen;
  QBrush mBrush, mSelectedBrush;
  double mSize;
  TracerStyle mStyle;
  GPCurve *mCurve;
  double mCurveKey;
  bool mInterpolating;

  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;

  // non-virtual methods:
  QPen mainPen() const;
  QBrush mainBrush() const;
};
Q_DECLARE_METATYPE(GPCurveTracerItem::TracerStyle)

#endif // GPCurveTracerItem_H
