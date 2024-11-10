#ifndef BARS_LABELS_ITEM_H
#define BARS_LABELS_ITEM_H

#include <graphicsplot/graphicsplot.h>

class BarsLabelsItem : public GPAbstractPlottable1D<GPBarsData>
{
  Q_OBJECT
public:
  BarsLabelsItem(GPAxis *keyAxis, GPAxis *valueAxis, GPLayerable *parentLayerable=0);


  // GPLayerable interface
public:
  void draw(GPPainter *painter) override;
  void setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);

  // GPAbstractPlottable interface
public:
  GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain) const override;
  GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain, const GPRange &inKeyRange) const override;
  GPDataRange getDataRange() const override;

protected:
  void drawLegendIcon(GPPainter *painter, const QRectF &rect) const override;

  // QPen mPen;
  // QBrush mBrush;
  QFont mFont;
  QPointF mShift;
};

#endif // BARS_LABELS_ITEM_H
