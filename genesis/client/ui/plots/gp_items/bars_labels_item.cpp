#include "bars_labels_item.h"

BarsLabelsItem::BarsLabelsItem(GPAxis *keyAxis, GPAxis *valueAxis, GPLayerable *parentLayerable)
  : GPAbstractPlottable1D<GPBarsData>(keyAxis, valueAxis, parentLayerable),
  // mPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin),
  // mBrush(Qt::NoBrush),
  mFont("Segoe UI", 9, 400, false),
  mShift(0,0)
{

}


void BarsLabelsItem::draw(GPPainter *painter)
{
  painter->save();
  painter->setPen(mPen);
  painter->setBrush(mBrush);
  painter->setFont(mFont);
  for(auto& data : *mDataContainer.data())
  {
    QPointF pt;
    pt.setX(mParentPlot->xAxis->coordToPixel(data.key));
    pt.setY(mParentPlot->yAxis->coordToPixel(data.value));
    pt += mShift;
    painter->drawText(pt, QString::number((int)data.key));
  }
  painter->restore();
}

void BarsLabelsItem::setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  mDataContainer->clear();
  if (keys.size() != values.size())
    qDebug() << Q_FUNC_INFO << "keys and values have different sizes:" << keys.size() << values.size();
  const int n = qMin(keys.size(), values.size());
  QVector<GPBarsData> tempData(n);
  QVector<GPBarsData>::iterator it = tempData.begin();
  const QVector<GPBarsData>::iterator itEnd = tempData.end();
  int i = 0;
  while (it != itEnd)
  {
    it->key = keys[i];
    it->value = values[i];
    ++it;
    ++i;
  }
  mDataContainer->add(tempData, alreadySorted); // don't modify tempData beyond this to prevent copy on write
}

GPRange BarsLabelsItem::getKeyRange(bool &foundRange, GP::SignDomain inSignDomain) const
{
  GPRange range;
  foundRange = false;
  for(auto& data : *mDataContainer.data())
  {
    switch(inSignDomain)
    {
    case GP::sdNegative:
      if(data.key < 0)
      {
        range.expand(data.key);
        foundRange = true;
      }
      break;
    case GP::sdBoth:
      if(data.key > 0)
      {
        range.expand(data.key);
        foundRange = true;
      }
      break;
    case GP::sdPositive:
      range.expand(data.key);
      foundRange = true;
      break;
    }
  }
  return range;
}

GPRange BarsLabelsItem::getValueRange(bool &foundRange, GP::SignDomain inSignDomain, const GPRange &inKeyRange) const
{
  GPRange range;
  foundRange = false;
  for(auto& data : *mDataContainer.data())
  {
    switch(inSignDomain)
    {
    case GP::sdNegative:
      if(data.value < 0)
      {
        range.expand(data.value);
        foundRange = true;
      }
      break;
    case GP::sdBoth:
      if(data.value > 0)
      {
        range.expand(data.value);
        foundRange = true;
      }
      break;
    case GP::sdPositive:
      range.expand(data.value);
      foundRange = true;
      break;
    }
  }
  return range;
}

GPDataRange BarsLabelsItem::getDataRange() const
{
  return mDataContainer->dataRange();
}

void BarsLabelsItem::drawLegendIcon(GPPainter *painter, const QRectF &rect) const
{
  painter->drawText(rect, "m/z");
}
