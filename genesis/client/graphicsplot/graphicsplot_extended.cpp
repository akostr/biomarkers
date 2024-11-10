#include "graphicsplot_extended.h"

#if defined(USE_XLNT)
#include <xlnt/include/xlnt/xlnt.hpp>
#endif

#if defined(USE_SETTINGS)
#include <settings/settings.h>
#endif

#if defined(USE_STANDARD_VIEWS)
#include <settings_widgets/caption_edit_dialog.h>
#endif

#if defined(USE_CUSTOM_WINDOWS)
#include <custom_windows/custom_windows.h>
#endif

#ifndef DISABLE_STANDARD_MODELS
#include "extern/common_core/standard_models/uom_settings.h"
#endif // ~~DISABLE_STANDARD_MODELS

#if !defined(DISABLE_CONCURRENT)
#include <QtConcurrent/QtConcurrent>
#endif

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFormLayout>
#include <QPointer>
#include <QRect>
#include <QDateTimeEdit>
#include <QDebug>
#include <QPolygonF>
#include <QTextDocumentFragment>

#include "settings_widgets/plot_settings_dialog.h"
#include "settings_widgets/zones_edit_dialog.h"
#include "settings_widgets/zones_amount_dialog.h"

#if defined(USE_STANDARD_VIEWS)
#include <standard_views/e_number_editor.h>
#endif

#include <algorithm>
#include <float.h>
#ifdef __APPLE__
  #include <OpenGL/gl.h>
#else
  #include <gl/GL.h>
#endif

using namespace GPObjectsKeys;
using namespace GPPropertiesKeys;
using namespace GPJsonKeys;
using namespace GPDefaultColors;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define VARIANT_TYPE_ID typeId
#else
#define VARIANT_TYPE_ID type
#endif

namespace GraphicsPlotExtendedUtils
{
  using namespace std;

  ////////////////////////////////////////////////////////
  //// Interpolation functor
  Interpolation::Interpolation(const vector<double> &_x, const vector<double> &_y)
    : x(_x)
    , y(_y)
  {
    if (_x.empty() || _y.empty())
      return;

    if (_x.front() > _x.back())
    {
      x.resize(_x.size());
      y.resize(_y.size());
      for (int i = 0; i < _x.size(); ++i)
      {
        x[i] = _x[x.size() - 1 - i];
        y[i] = _y[x.size() - 1 - i];
      }
    }
    else
    {
      x = _x;
      y = _y;
    }

    vector<double>::size_type newSize = x.size() - 1;
    koeff0.resize(newSize);
    koeff1.resize(newSize);

    for (int i = 0; i < koeff0.size(); ++i)
    {
      koeff0[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
      koeff1[i] = y[i + 1] - koeff0[i] * x[i + 1];
    }
  }

  Interpolation::Interpolation(const QVector<double>& x, QVector<double>& y)
    : Interpolation(std::vector<double>(x.begin(), x.end()),
                    std::vector<double>(y.begin(), y.end()))
  {
  }

  Interpolation::~Interpolation()
  {
  }

  bool Interpolation::IsEmpty() const
  {
    return x.empty() || y.empty();
  }

  int Interpolation::searchId(double _x) const
  {
    if (_x < x[0])
    {
      return -1;
    }

    if (_x > x[x.size() - 1])
    {
      return -2;
    }
    return std::upper_bound(x.begin(), x.end(), _x) - x.begin() - 1;
  }

  double Interpolation::interp1d(double _x) const
  {
    if (IsEmpty())
      return qQNaN();

    int i = searchId(_x);
    if (i == -1)
    {
      return y.front();
    }
    if (i == -2 || i >= koeff0.size())
    {
      return y.back();
    }
    else
    {
      return _x * koeff0[i] + koeff1[i];
    }
  }

  double Interpolation::interp1dWithNaN(double _x) const
  {
    if (IsEmpty())
      return qQNaN();

    int i = searchId(_x);
    if (i == -1 || i == -2 || i >= koeff0.size())
    {
      return qQNaN();
    }
    return _x * koeff0[i] + koeff1[i];
  }

  double Interpolation::derivative(double _x, double dx) const
  {
    double y1 = interp1d(_x - dx);
    double y2 = interp1d(_x + dx);
    return (y2 - y1) / (2.0 * dx);
  }

  std::vector<double> operator*(const std::vector<double> &v1, const std::vector<double> &v2)
  {
    size_t s1 = v1.size();
    size_t s2 = v2.size();

    if (s1 != s2)
      throw std::runtime_error("size_1 != size_2");

    std::vector<double> res(s1);
    for (size_t i = 0; i < s1; ++i)
      res[i] = v1[i] * v2[i];

    return std::move(res);
  }

  std::vector<double> accumulateVector(const std::vector<double>& vect)
  {
    std::vector<double> res(vect.size());

    res[0] = vect[0];

    for (size_t i = 1; i < vect.size(); ++i)
      res[i] = res[i-1] + vect[i];

    return std::move(res);
  }

  std::vector<double> diffVector(const std::vector<double>& vect)
  {
    std::vector<double> res(vect.size());

    res[0] = 0.0;

    for (size_t i = 1; i < vect.size(); ++i)
      res[i] = vect[i] - vect[i - 1];

    return std::move(res);
  }

  Interpolation getAccumulatedDataInterpolation(const std::vector<double>& data, const std::vector<double>& time)
  {
    return std::move(Interpolation(time, accumulateVector(data * diffVector(time))));
  }

  double getRangeAmount(const Interpolation& dataInterp, double lhs, double rhs)
  {
    return dataInterp.interp1d(rhs) - dataInterp.interp1d(lhs);
  }

  double getAverageProperty(const Interpolation &dataInterp, double lhs, double rhs)
  {
    return (dataInterp.interp1d(rhs) - dataInterp.interp1d(lhs)) / (rhs - lhs);
  }

  QVector<double> getCurveKeyData(GPCurve *curve)
  {
    auto data = curve->data();
    QVector<double> result;
    result.reserve(data->size());
    for (int i = 0; i < data->size(); ++i)
      result.push_back(data->at(i)->key);
    return std::move(result);
  }

  std::vector<double> getCurveKeyDataStd(GPCurve *curve)
  {
    auto data = curve->data();
    std::vector<double> result;
    result.reserve(data->size());
    for (int i = 0; i < data->size(); ++i)
      result.push_back(data->at(i)->key);
    return std::move(result);
  }

  void setContainerData(const QSharedPointer<GPDataContainer<GPCurveData>> &c, const std::vector<double> &t, const std::vector<double> &v, int start, int end)
  {
    c->clear();
    if (t.empty() || t.size() != v.size())
      return;

    if (end == -1)
      end = t.size() - 1;
    if (start == -1)
      start = 0;

    if (start >= end)
      return;

    const int n = end - start + 1;

    QVector<GPCurveData> tempData(n);
    QVector<GPCurveData>::iterator it = tempData.begin();
    const QVector<GPCurveData>::iterator itEnd = tempData.end();
    int i = 0;
    while (it != itEnd && i + start <= end)
    {
      it->t = i;
      it->key = t[i + start];
      it->value = v[i + start];
      ++it;
      ++i;
    }
    c->set(tempData, true);
  }

  double interpolate(const QSharedPointer<GPDataContainer<GPCurveData> >& c, const double& t, bool withNaN)
  {
    return interpolate<GPDataContainer<GPCurveData>>(c, t, withNaN);
  }

  double interpolate(const QSharedPointer<GPDataContainer<GPGraphData> >& c, const double& t, bool withNaN)
  {
    return interpolate<GPDataContainer<GPGraphData>>(c, t, withNaN);
  }

  double interpolate(const std::vector<double> &time, const std::vector<double> &value, const double &t, bool withNaN)
  {
    if (!time.empty() && time.size() == value.size())
    {
      auto right = std::lower_bound(time.begin(), time.end(), t, [](const double& el, const double& v) {
        return el < v || qIsNaN(el);
      });

      if (right == time.begin())
      {
        if (qIsNaN(*time.begin()) || fabs(t - *(time.begin())) < std::numeric_limits<double>::epsilon())
          return *value.begin();
        return withNaN ? qQNaN() : *value.begin();
      }

      if (right != time.end())
      {
        auto vRight = value.begin() + (right - time.begin());
        if (fabs(*right - t) < std::numeric_limits<double>::epsilon())
          return *vRight;

        auto left = right - 1;
        auto vLeft = value.begin() + (left - time.begin());
        if (qIsNaN(*left) || qIsNaN(*vLeft))
          return qQNaN();

        return *vLeft + (t - *left) * (*vRight - *vLeft) / (*right - *left);
      }

      if (qIsNaN(time.back()) || fabs(t - time.back()) < std::numeric_limits<double>::epsilon())
        return value.back();
      return withNaN ? qQNaN() : value.back();
    }
    return withNaN ? qQNaN() : 0.0;
  }
}

double getAxisPixelRange(GPAxis* axis)
{
  if (auto axisRect = axis->axisRect())
  {
    const QRect& rect = axisRect->axisDrawRect(axis->axisType());
    if (axis->orientation() == Qt::Horizontal)
      return rect.right() - rect.left();
    return rect.bottom() - rect.top();
  }
  return 0;
}

namespace GPSettings {
  QVariantMap getSettings(QObject *obj)
  {
    if (auto axis = qobject_cast<GPAxis*>(obj))
      return getSettings(axis);
    if (auto curve = qobject_cast<GPCurve*>(obj))
      return getSettings(curve);
    if (auto legend = qobject_cast<GPLegend*>(obj))
      return getSettings(legend);
    if (auto elem = qobject_cast<GPTextElement*>(obj))
      return getSettings(elem);
    return QVariantMap();
  }

  void setSettings(QObject* obj, QVariantMap &settings)
  {
    if (auto axis = qobject_cast<GPAxis*>(obj))
      setSettings(axis, settings);
    else if (auto curve = qobject_cast<GPCurve*>(obj))
      setSettings(curve, settings);
    else if (auto legend = qobject_cast<GPLegend*>(obj))
      setSettings(legend, settings);
    else if (auto elem = qobject_cast<GPTextElement*>(obj))
      setSettings(elem, settings);
  }

  QVariantMap getSettings(GPAxis *axis)
  {
    QVariantMap settings;
    settings[JKAxisType] = (int)axis->axisType();
    settings[JKAxisScaleType] = (int)axis->scaleType();
    settings[JKVisibility] = axis->visible();
    settings[JKAxisRangeLower] = QString("%1").arg(axis->range().lower, 0, 'f', 10);
    settings[JKAxisRangeUpper] = QString("%1").arg(axis->range().upper, 0, 'f', 10);
    settings[JKFont] = axis->labelFont().toString();
    settings[JKFontColor] = axis->labelColor().name(QColor::HexArgb);

    //// Save uom
    {
      QVariantMap juom;
      GPUomDescriptor uom = axis->getUomDescriptor();
      uom.save(juom);
      settings[JKUom] = juom;
    }

    if (auto tickerAxisAware = qobject_cast<GPAxisAwareTicker*>(axis->ticker().data()))
    {
      if (tickerAxisAware && axis->scaleType() != GPAxis::stLogarithmic)
      {
        double reasonableStep = tickerAxisAware->getPreferredStep();
        settings[JKAxisTickerStep] = reasonableStep;
      }
    }

    return std::move(settings);
  }

  void setSettings(GPAxis *axis, QVariantMap &settings)
  {
    if (settings.contains(JKAxisType))
      axis->parentPlot()->moveAxis(axis->axisRect(), axis, (GPAxis::AxisType)settings[JKAxisType].toInt());
    if (settings.contains(JKAxisScaleType))
      axis->setScaleType((GPAxis::ScaleType)settings[JKAxisScaleType].toInt());

    axis->setVisible(settings.value(JKVisibility, true).toBool());

    QString low = settings[JKAxisRangeLower].toString();
    QString upp = settings[JKAxisRangeUpper].toString();
    if (!low.isEmpty() && !upp.isEmpty())
    {
      //// Load uom dependent data
      double l = low.toDouble();
      double u = upp.toDouble();

      //// Load stored uom
      GPUomDescriptor iuom;
      iuom.load(settings[JKUom].toMap());

      //// Get current uom
      GPUomDescriptor currentUom = axis->getUomDescriptor();

      //// Support legavcy as is
      if (iuom != currentUom && iuom.isNull())
      {
        iuom = currentUom;
      }

      //// Transform if necessary
      if (iuom != currentUom)
      {
        GPUomDataTransformation transformation = axis->getUomTransformation(iuom, currentUom);
        l = transformation(l, iuom, currentUom);
        u = transformation(u, iuom, currentUom);
      }

      //// Set
      axis->setRange(l, u);
    }

    if (settings.contains(JKFontColor))
    {
      QColor color(settings[JKFontColor].toString());
      axis->setLabelColor(color);
      axis->setTickLabelColor(color);
    }
    if (settings.contains(JKFont))
    {
      QFont font;
      font.fromString(settings[JKFont].toString());
      axis->setLabelFont(font);
      axis->setTickLabelFont(font);
    }

    {
      double reasonableStep = settings.value(JKAxisTickerStep, 0.0).toDouble();

      GPAxisTicker* ticker = axis->ticker().data();
      GPAxisAwareTicker*     tickerAxisAware = qobject_cast<GPAxisAwareTicker*>(ticker);
      if (tickerAxisAware && axis->scaleType() != GPAxis::stLogarithmic)
        tickerAxisAware->setPreferredStep(reasonableStep);
    }
  }

  QVariantMap getSettings(GPCurve *curve)
  {
    QVariantMap settings;
    QPen pen = curve->pen();
    settings[JKPenColor] = pen.color().name(QColor::HexArgb);
    settings[JKPenWidth] = pen.widthF();
    settings[JKPenStyle] = (int)pen.style();

    settings[JKBrushColor] = curve->brush().color().name(QColor::HexArgb);
    settings[JKBrushStyle] = (int)curve->brush().style();

    GPScatterStyle scatter = curve->scatterStyle();
    settings[JKScatterPenColor] = scatter.pen().color().name(QColor::HexArgb);
    settings[JKScatterPointSize] = scatter.size();
    settings[JKScatterPointStyle] = (int)scatter.shape();

    settings[JKScatterBrushColor] = curve->brush().color().name(QColor::HexArgb);
    settings[JKScatterBrushStyle] = (int)curve->brush().style();

    settings[JKVisibility] = curve->visible();

    return std::move(settings);
  }

  void setSettings(GPCurve *curve, QVariantMap &settings)
  {
    QPen pen = curve->pen();
    GPScatterStyle scatter = curve->scatterStyle();
    QBrush brush = curve->brush();

    if (settings.contains(JKPenColor))
      pen.setColor(QColor(settings[JKPenColor].toString()));
    else
    {
      auto colormap = GraphicsPlotExtendedStyle::GetDefaultColorMap();

      QString key = curve->property(OKCurveId).toString();
      if (!key.isEmpty())
      {
        if (curve->property(PKDefaultColor).isNull())
          pen.setColor(colormap.value(abs(key.toInt()) %
                                      colormap.size()));
      }
      else if (curve->parentPlot())
      {
        int index = -1;
        for (int i = 0; i < curve->parentPlot()->curveCount(); ++i)
        {
          if (curve->parentPlot()->curve(i) == curve)
          {
            index = i;
            break;
          }
        }
        if (index != -1)
          pen.setColor(colormap.value(abs(index) % colormap.size()));
      }
    }

    if (settings.contains(JKPenWidth))
      pen.setWidthF(settings[JKPenWidth].toDouble());
    if (settings.contains(JKPenStyle))
      pen.setStyle((Qt::PenStyle)settings[JKPenStyle].toInt());

    if (settings.contains(JKBrushColor))
      curve->setBrush(QBrush(QColor(settings[JKBrushColor].toString()), Qt::NoBrush));
    if (settings.contains(JKBrushStyle))
      curve->setBrush(QBrush(curve->brush().color(), (Qt::BrushStyle)settings[JKBrushStyle].toInt()));

    if (settings.contains(JKScatterPenColor))
      scatter.setPen(QPen(QColor(settings[JKScatterPenColor].toString())));
    else
      scatter.setPen(QPen(pen.color()));

    if (settings.contains(JKScatterPointSize))
      scatter.setSize(settings[JKScatterPointSize].toDouble());
    if (settings.contains(JKScatterPointStyle))
      scatter.setShape((GPScatterStyle::ScatterShape)settings[JKScatterPointStyle].toInt());

    if (settings.contains(JKScatterBrushColor))
      brush.setColor(QColor(settings[JKScatterBrushColor].toString()));
    else
      brush.setColor(pen.color());

    if (settings.contains(JKScatterBrushStyle))
      brush.setStyle((Qt::BrushStyle)settings[JKScatterBrushStyle].toInt());
    scatter.setBrush(brush);

    if (settings.contains(JKVisibility))
      curve->setVisible(settings[JKVisibility].toBool());

    curve->setPen(pen);
    if (auto decorator = curve->selectionDecorator())
    {
      QPen selectionPen = pen;
      selectionPen.setColor(pen.color().lighter(120));
      selectionPen.setWidthF(pen.widthF() + 1.5);
      decorator->setPen(selectionPen);
    }
    curve->setScatterStyle(scatter);
  }

  QVariantMap getSettings(GPLegend *legend)
  {
    QVariantMap settings;
    settings[JKFont] = legend->font().toString();
    settings[JKFontColor] = legend->textColor().name(QColor::HexArgb);
    settings[JKVisibility] = legend->visible();
    auto inset = legend->parentPlot()->axisRect()->insetLayout();
    settings[JKPositionIndex] = (int)(inset->insetAlignment(inset->indexOf(legend)));

    return std::move(settings);
  }

  void setSettings(GPLegend *legend, QVariantMap &settings)
  {
    if (settings.contains(JKFont))
    {
      QFont font;
      font.fromString(settings[JKFont].toString());
      legend->setFont(font);
    }

    if (settings.contains(JKFontColor))
      legend->setTextColor(QColor(settings[JKFontColor].toString()));

    if (settings.contains(JKVisibility))
      legend->setVisible(settings[JKVisibility].toBool());
    if (settings.contains(JKPositionIndex))
    {
      auto inset = legend->parentPlot()->axisRect()->insetLayout();
      inset->setInsetAlignment(inset->indexOf(legend), (Qt::Alignment)settings[JKPositionIndex].toInt());
    }
  }

  QVariantMap getSettings(GPTextElement *textElem)
  {
    QVariantMap settings;

    settings[JKFont] = textElem->font().toString();
    settings[JKFontColor] = textElem->textColor().name(QColor::HexArgb);
    settings[JKText] = textElem->text();

    return std::move(settings);
  }

  void setSettings(GPTextElement *textElem, QVariantMap &settings)
  {
    if (settings.contains(JKFont))
    {
      QFont font;
      font.fromString(settings[JKFont].toString());
      textElem->setFont(font);
    }

    if (settings.contains(JKFontColor))
      textElem->setTextColor(QColor(settings[JKFontColor].toString()));
    if (settings.contains(JKText))
      textElem->setText(settings[JKText].toString());
    textElem->setVisible(!textElem->text().isEmpty());
  }

  QVariantMap getSettings(GPCaptionItem* caption)
  {
    QVariantMap settings;

    settings[JKId] = caption->getId();
    settings[JKText] = caption->getText();
    settings[JKCaptionHasLine] = caption->getHasLine();
    QPointF pos = caption->getTextPos();
    settings[JKCaptionPositionX] = pos.x();
    settings[JKCaptionPositionY] = pos.y();
    pos = caption->getPointPos();
    settings[JKCaptionPointPositionX] = pos.x();
    settings[JKCaptionPointPositionY] = pos.y();
    settings[JKCaptionParentType] = caption->getParentType();
    settings[JKCaptionParentId] = caption->getParentId();

    //// Save uom
    {
      QVariantMap kjuom;
      caption->listenerKeys.uomDescriptor.save(kjuom);

      QVariantMap vjuom;
      caption->listenerValues.uomDescriptor.save(vjuom);

      QVariantMap juom;
      juom[JKUomKeys]   = kjuom;
      juom[JKUomValues] = vjuom;

      settings[JKUom] = juom;
    }
    return std::move(settings);
  }

  void setSettings(GPCaptionItem* caption, QVariantMap &settings)
  {
    if (settings.contains(JKId))
      caption->setId(settings[JKId].toUuid());

    if (settings.contains(JKCaptionParentType)
        && settings.contains(JKCaptionParentId))
      caption->setParent(settings[JKCaptionParentType].toString()
                       , settings[JKCaptionParentId].toString());

    if (settings.contains(JKText))
      caption->setText(settings[JKText].toString());

    if (settings.contains(JKCaptionHasLine))
      caption->setHasLine(settings[JKCaptionHasLine].toBool());

    //// Load uom keys
    GPUomDescriptor kiuom;
    kiuom.load(settings[JKUom].toMap()[JKUomKeys].toMap());
    if (kiuom != caption->listenerKeys.uomDescriptor && kiuom.isNull())
      kiuom = caption->listenerKeys.uomDescriptor;

    GPUomDataTransformation transformationKeys;
    if (caption->listenerKeys.uomProvider
     && caption->listenerKeys.uomDescriptor != kiuom)
    {
      transformationKeys = caption->listenerKeys.uomProvider->getUomTransformation(kiuom);
    }

    //// Load uom values
    GPUomDescriptor viuom;
    viuom.load(settings[JKUom].toMap()[JKUomValues].toMap());
    if (viuom != caption->listenerValues.uomDescriptor && viuom.isNull())
      viuom = caption->listenerValues.uomDescriptor;

    GPUomDataTransformation transformationValues;
    if (caption->listenerValues.uomProvider
     && caption->listenerValues.uomDescriptor != viuom)
    {
      transformationValues = caption->listenerValues.uomProvider->getUomTransformation(viuom);
    }

    //// Load uom dependent data - position
    if (settings.contains(JKCaptionPositionX)
     && settings.contains(JKCaptionPositionY))
    {
      QPointF p(settings[JKCaptionPositionX].toDouble(),
                settings[JKCaptionPositionY].toDouble());

      if (transformationKeys)
        p.setX(transformationKeys(p.x(), kiuom, caption->listenerKeys.uomDescriptor));
      if (transformationValues)
        p.setY(transformationKeys(p.y(), viuom, caption->listenerValues.uomDescriptor));

      caption->setTextPos(p);
    }

    //// Load uom dependent data - point position
    if (settings.contains(JKCaptionPointPositionX)
     && settings.contains(JKCaptionPointPositionY))
    {
      QPointF p(settings[JKCaptionPointPositionX].toDouble(),
                settings[JKCaptionPointPositionY].toDouble());

      if (transformationKeys)
        p.setX(transformationKeys(p.x(), kiuom, caption->listenerKeys.uomDescriptor));
      if (transformationValues)
        p.setY(transformationKeys(p.y(), viuom, caption->listenerValues.uomDescriptor));

      caption->setPointPos(p);
    }
  }

  QVariantMap getSettings(GPZonesRect *zones)
  {
    return std::move(zones->toVariantMap());
  }

  void setSettings(GPZonesRect *zones, QVariantMap &settings)
  {
    zones->fromVariantMap(settings);
  }
}

///////////////////////////////////////////////////////
// Range spin box
RangeSpinBox::RangeSpinBox(QWidget* parent)
  : QDoubleSpinBox(parent)
{
  setMinimum(-DBL_MAX);
  setMaximum(DBL_MAX);
  setMinimumWidth(70);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

///////////////////////////////////////////////////////
// zones rect

GPZonesData::GPZonesData()
  : QObject()
  , mRange(-DBL_MAX, DBL_MAX)
{
}

const GPZonesElement &GPZonesData::at(int i)
{
  return mData.at(i);
}

GPZonesElement &GPZonesData::first()
{
  return mData.first();
}

GPZonesElement &GPZonesData::last()
{
  return mData.last();
}

void GPZonesData::append(const GPZonesElement &e)
{
  mData.append(e);
  if (!rangeIsValid())
    fixInvalidatedRange();
}

int GPZonesData::size() const
{
  return mData.size();
}

void GPZonesData::resize(int size)
{
  mData.resize(size);
  if (!rangeIsValid())
    fixInvalidatedRange();
  emit dataChanged();
}

void GPZonesData::clear()
{
  mData.clear();
}

QVector<GPZonesElement>::iterator GPZonesData::begin()
{
  return mData.begin();
}

QVector<GPZonesElement>::iterator GPZonesData::end()
{
  return mData.end();
}

void GPZonesData::setLine(int index, double value)
{
  if (mData.size() > index && index >= 0 && !mData[index].mLocked)
  {
    if (uomProvider)
    {
      if (mInputUom != uomDescriptor)
      {
        if (mInputUom.canConvert(uomDescriptor))
        {
          GPUomDataTransformation transformation = uomProvider->getUomTransformation(mInputUom);
          value = transformation(value, mInputUom, uomDescriptor);
        }
        else
        {
          Q_ASSERT(!"Source uom descriptor differs from target and conversion cannot be performed, consider using setInputUomForKeys(), setInputUomForValues() prior to setLine()");
        }
      }
    }

    value = qBound(mRange.first, value, mRange.second);
    if (index > 0 && value < mData[index - 1].mValue)
      value = mData[index - 1].mValue;
    if (index < mData.size() - 1 && value > mData[index + 1].mValue)
      value = mData[index + 1].mValue;

    if (mData[index].mValue != value)
    {
      mData[index].mValue = value;
      if (!rangeIsValid())
        fixInvalidatedRange();
      emit dataChanged();
    }
  }
}

void GPZonesData::setName(int index, const QString &value)
{
  if (mData.size() > index && index >= 0)
    mData[index].mName = value;
}

void GPZonesData::setLocked(int index, const bool &value)
{
  if (mData.size() > index && index >= 0)
    mData[index].mLocked = value;
}

void GPZonesData::setVisible(int index, const bool &value)
{
  if (mData.size() > index && index >= 0)
    mData[index].mVisible = value;
}

void GPZonesData::setRange(double min, double max)
{
  mRange.first = min;
  mRange.second = max;
  if (!rangeIsValid())
  {
    fixInvalidatedRange();
    emit dataChanged();
  }
}

QPair<double, double> GPZonesData::getRange() const
{
  return mRange;
}

void GPZonesData::setLines(const QVector<double>& zonesArg)
{
  QVector<double> zones = zonesArg;
  if (!zones.empty())
  {
    if (uomProvider)
    {
      if (mInputUom != uomDescriptor)
      {
        if (mInputUom.canConvert(uomDescriptor))
        {
          GPUomDataTransformation transformation = uomProvider->getUomTransformation(mInputUom);
          for (double& z : zones)
          {
            z = transformation(z, mInputUom, uomDescriptor);
          }
        }
        else
        {
          Q_ASSERT(!"Source uom descriptor differs from target and conversion cannot be performed, consider using setInputUomForKeys(), setInputUomForValues() prior to setLines()");
        }
      }
    }
  }

  mData.resize(zones.size());
  for (int i = 0; i < mData.size(); ++i)
    mData[i].mValue = zones[i];

  if (!rangeIsValid())
    fixInvalidatedRange();
  emit dataChanged();
}

QVector<double> GPZonesData::getLines() const
{
  QVector<double> data(mData.size());
  for (int i = 0; i < mData.size(); ++i)
    data[i] = mData[i].mValue;
  return data;
}

void GPZonesData::setNames(const QVector<QString>& names)
{
  mData.resize(qMax(names.size() + 1, mData.size()));
  for (int i = 0; i < names.size(); ++i)
    mData[i].mName = names[i];
}

QVector<QString> GPZonesData::getNames() const
{
  QVector<QString> data(mData.size() - 1);
  for (int i = 0; i < mData.size() - 1; ++i)
    data[i] = mData[i].mName;
  return data;
}

void GPZonesData::setLocked(const QVector<bool>& locked)
{
  mData.resize(qMax(locked.size() + 1, mData.size()));
  for (int i = 0; i < locked.size(); ++i)
    mData[i].mLocked = locked[i];
}

QVector<bool> GPZonesData::getLocked() const
{
  QVector<bool> data(mData.size() - 1);
  for (int i = 0; i < mData.size() - 1; ++i)
    data[i] = mData[i].mLocked;
  return data;
}

void GPZonesData::setVisibility(const QVector<bool>& visibility)
{
  mData.resize(qMax(visibility.size() + 1, mData.size()));
  for (int i = 0; i < visibility.size(); ++i)
    mData[i].mVisible = visibility[i];
}

QVector<bool> GPZonesData::getVisibility() const
{
  QVector<bool> data(mData.size() - 1);
  for (int i = 0; i < mData.size() - 1; ++i)
    data[i] = mData[i].mVisible;
  return data;
}

bool GPZonesData::rangeIsValid()
{
  if (!mData.isEmpty())
  {
    for (int i = 0; i < mData.size() - 1; ++i)
    {
      if (mData[i].mValue < mRange.first || mData[i].mValue > mRange.second)
        return false;
      if (mData[i].mValue > mData[i + 1].mValue)
        return false;
    }
    if (mData.first().mValue < mRange.first || mData.last().mValue > mRange.second)
      return false;
  }
  return true;
}

void GPZonesData::fixInvalidatedRange()
{
  if (mData.size() >= 2)
  {
    bool needForceUpdate = true;
    for (int i = 1; i < mData.size(); ++i)
    {
      if (fabs(mData[i].mValue - mData[i - 1].mValue) > std::numeric_limits<double>::epsilon())
      {
        needForceUpdate = false;
        break;
      }
    }
    if (needForceUpdate)
      if (mData.first().mValue >= mRange.first && mData.last().mValue <= mRange.second)
        needForceUpdate = false;

    if (!needForceUpdate)
    {
      mData.first().mValue = qBound(mRange.first, mData.first().mValue, mRange.second);
      mData.last().mValue = qBound(mRange.first, mData.last().mValue, mRange.second);

      QVector<bool> elementIsValid(mData.size(), true);
      int lastValid = 0;
      for (int i = 1; i < mData.size() - 1; ++i)
      {
        if (mData[i].mValue < mData[lastValid].mValue || mData[i].mValue > mRange.second)
          elementIsValid[i] = false;
        else
          lastValid = i - 1;
      }

      int leftInvalid = -1;
      int rightInvalid = -1;
      for (int i = 0; i < mData.size(); ++i)
      {
        if (leftInvalid == -1)
        {
          if (!elementIsValid[i])
          {
            leftInvalid = i;
            rightInvalid = i;
          }
        }
        else
        {
          if (!elementIsValid[i])
          {
            rightInvalid = i;
          }
          else
          {
            double left = mData[leftInvalid - 1].mValue;
            double right = mData[rightInvalid + 1].mValue;
            double size = rightInvalid - leftInvalid + 1;
            double dv = (right - left) / (size + 1);

            double v = left;
            for (int i = leftInvalid; i <= rightInvalid; ++i)
            {
              v += dv;
              mData[i].mValue = v;
            }

            leftInvalid = -1;
            rightInvalid = -1;
          }
        }
      }
    }
    else
    {
      mData.first().mValue = mRange.first;
      mData.last().mValue = mRange.second;

      double dv = (mRange.second - mRange.first) / (size() - 1);
      double v = mRange.first;
      for (int i = 1; i < size() - 1; ++i)
      {
        v += dv;
        mData[i].mValue = v;
      }
    }
  }
}

void GPZonesData::setInputUom(const GPUomDescriptor& descriptor)
{
  mInputUom = descriptor;
}

void GPZonesData::setInputUom(int quantity, int units)
{
  GPUomDescriptor inputUom;
  inputUom.quantity = quantity;
  inputUom.units    = units;
  setInputUom(inputUom);
}

void GPZonesData::applyUom(const GPUomDescriptor& descriptor)
{
  applyUom(uomDescriptor, descriptor, begin(), end());
}

void GPZonesData::applyUom(const GPUomDescriptor& source, const GPUomDescriptor& target,
                            QVector<GPZonesElement>::iterator from,
                            QVector<GPZonesElement>::iterator to)
{
  if (size() != 0)
  {
    if (source != target)
    {
      //// Data
      GPUomDataTransformation transformation = uomProvider->getUomTransformation(source);
      for (auto i = from; i != to; ++i)
      {
        i->mValue = transformation(i->mValue, source, target);
      }
    }
  }
}

QVariantMap GPZonesRect::GPZonesRectStyle::toVariantMap()
{
  QVariantMap map;

  QVariantList colors;
  for (const auto& brush : zonesBrush)
    colors.append(brush.color().name());
  map[JKZonesColors] = colors;
  map[JKFont] = textFont.toString();
  map[JKFontColor] = textColor.name();
  map[JKVAlign] = (int)textAlignment;

  return map;
}

void GPZonesRect::GPZonesRectStyle::fromVariantMap(QVariantMap map)
{
  textFont.fromString(map[JKFont].toString());
  textColor.setNamedColor(map[JKFontColor].toString());
  textAlignment = (Qt::AlignmentFlag)map[JKVAlign].toInt();
  zonesBrush.clear();
  QVariantList colors = map[JKZonesColors].toList();
  for (const auto& color : colors)
  {
    QColor c(color.toString());
    c.setAlpha(20);
    zonesBrush.append(QBrush(c));
  }
}


GPZonesRect::GPZonesRect(GraphicsPlot *parentPlot)
  : GPSelectionRect(parentPlot)
  , mClipRect(true)
  , mDraggingIndexLeft(-1)
  , mDraggingIndexRight(-1)
  , mOrientation(Qt::Horizontal)
{
  amountFunc = nullptr;
  amountTestFunc = nullptr;

  if (layer())
    moveToLayer(layer(), true);

  setData(QSharedPointer<GPZonesData>(new GPZonesData));
}

GPZonesRect::~GPZonesRect()
{
  if (mLegend)
    mParentPlot->axisRect()->insetLayout()->remove(mLegend.data());
}

void GPZonesRect::DragStart(QMouseEvent *event, unsigned)
{
  startSelection(event);
}

void GPZonesRect::DragMove(QMouseEvent *event)
{
  moveSelection(event);
}

void GPZonesRect::DragAccept(QMouseEvent *event)
{
  endSelection(event);
}

bool GPZonesRect::IsDragging() const
{
  return mInteractive && mDraggingIndexLeft != -1;
}

QVariantMap GPZonesRect::toVariantMap()
{
  QVariantMap map;

  QVariantList lines;
  QVariantList vis;
  QVariantList names;

  for (auto it = mData->begin(); it != mData->end(); ++it)
  {
    lines.append(it->mValue);
    vis.append(it->mVisible);
    names.append(it->mName);
  }

  map[JKZonesLines] = lines;
  map[JKZonesVisibility] = vis;
  map[JKZonesNames] = names;

  map[JKZonesStyle] = mStyle.toVariantMap();
  map[JKVisibility] = visible();
  map[JKZonesInteractive] = mInteractive;

  map[JKZonesAmountSettings] = amountSettings;

  //// Save uom
  {
    QVariantMap juom;
    GPUomDescriptor uom = mData->uomDescriptor;
    uom.save(juom);
    map[JKUom] = juom;
  }

  return map;
}

void GPZonesRect::fromVariantMap(QVariantMap map)
{
  mData->clear();

  QVariantList lines = map[JKZonesLines].toList();
  QVariantList vis = map[JKZonesVisibility].toList();
  QVariantList names = map[JKZonesNames].toList();

  //// Load uom dependent data
  {
    //// Save input uom
    GPUomDescriptor inputUomSafe = mData->mInputUom;

    //// Load input uom
    GPUomDescriptor iuom;
    iuom.load(map[JKUom].toMap());

    //// Sert input uom (if any, otherwise support legacy as possible)
    if (!iuom.isNull())
      mData->setInputUom(iuom);

    //// Set data
    for (int i = 0; i < lines.size(); ++i)
    {
      mData->append(GPZonesElement(lines[i].toDouble(), vis.value(i).toBool(), false, names.value(i).toString()));
    }

    //// Restore input uom
    mData->mInputUom = inputUomSafe;
  }

  mStyle.fromVariantMap(map[JKZonesStyle].toMap());
  setVisible(map[JKVisibility].toBool());

  mInteractive = map[JKZonesInteractive].toBool();

  amountSettings = map[JKZonesAmountSettings].toMap();

  amountDesc      = ZonesAmountDialog::GetLabelDescriptonFromSettings(amountSettings);
  amountDim       = ZonesAmountDialog::GetValueDimensionFromSettings(amountSettings);
  amountFunc      = ZonesAmountDialog::GetAmountFunctionFromSettings(parentPlot(), amountSettings);
  amountTestFunc  = ZonesAmountDialog::GetTestFunctionFromSettings(parentPlot(), amountSettings);

  mDraggingIndexLeft = -1;
  mDraggingIndexRight = -1;
}

void GPZonesRect::setupDefaultLines()
{
  if (mData && mKeyAxis)
  {
    mData->append(mKeyAxis->range().lower);
    mData->append((mKeyAxis->range().upper + mKeyAxis->range().lower) / 2);
    mData->append(mKeyAxis->range().upper);
  }
}

void GPZonesRect::setData(const QSharedPointer<GPZonesData>& data)
{
  if (mData)
    disconnect(mData.data(), 0, this, 0);

  mData = data;

  if (mData)
  {
    connect(mData.data(), SIGNAL(dataChanged()), this, SIGNAL(zonesChanged()));
    connect(mData.data(), SIGNAL(dataChanged()), this, SLOT(updateZonesAmount()));
  }
}

QSharedPointer<GPZonesData> GPZonesRect::data()
{
  return mData;
}

void GPZonesRect::setStyle(const GPZonesRect::GPZonesRectStyle& style)
{
  mStyle = style;
}

GPZonesRect::GPZonesRectStyle GPZonesRect::getStyle() const
{
  return mStyle;
}

void GPZonesRect::setKeyAxis(GPAxis *keyAxis)
{
  mKeyAxis = keyAxis;

  mData->setUomProvider(mKeyAxis);

  if (mKeyAxis)
  {
    if (keyAxis->axisType() & GPAxis::atRight || keyAxis->axisType() & GPAxis::atLeft)
      mOrientation = Qt::Vertical;
    else
      mOrientation = Qt::Horizontal;
  }
  parentPlot()->replot();
}

GPAxis *GPZonesRect::getKeyAxis() const
{
  return mKeyAxis;
}

void GPZonesRect::setClipRect(bool clip)
{
  mClipRect = clip;
}

bool GPZonesRect::getClipRect() const
{
  return mClipRect;
}

void GPZonesRect::onTargetUpdated()
{
  if (amountFunc && amountTestFunc && !amountTestFunc())
  {
    amountFunc = nullptr;
    amountTestFunc = nullptr;
  }
  else
  {
    amountFunc = ZonesAmountDialog::GetAmountFunctionFromSettings(parentPlot(), amountSettings);
    amountTestFunc = ZonesAmountDialog::GetTestFunctionFromSettings(parentPlot(), amountSettings);
    amountDesc = ZonesAmountDialog::GetLabelDescriptonFromSettings(amountSettings);
    amountDim = ZonesAmountDialog::GetValueDimensionFromSettings(amountSettings);
  }
  updateZonesAmount();
}

void GPZonesRect::updateZonesAmount()
{
  if (!mData)
    return;

  if (mLegend)
  {
    if (!isLegendVisible())
    {
      mParentPlot->axisRect()->insetLayout()->remove(mLegend.data());
      return;
    }
    mLegend->clear();
  }
  else
  {
    if (isLegendVisible())
    {
      mLegend = new GPLegend;
      mParentPlot->axisRect()->insetLayout()->addElement(mLegend, Qt::AlignTop | Qt::AlignLeft);
      mLegend->setIconSize(0, 0);
      ((GraphicsPlotExtended*)mParentPlot)->getStyle().SetDefaultLegendStyle(mLegend);
      mLegend->setVisible(true);
      mLegend->setProperty(OKLegendId, "ZonesLabel");
      mLegend->setProperty(PKName, tr("Zones Label"));
      mLegend->setProperty(PKDefaultColor, QColor(Qt::black).name());
      ((GraphicsPlotExtended*)mParentPlot)->LoadLegend(mLegend.data());
    }
    else
      return;
  }

  if (mData->size() > 1)
  {
    auto textItem = new GPTextLegendItem(mLegend);
    mLegend->addElement(textItem);

    QString text;
    if (!amountDesc.isEmpty())
      text += amountDesc + ", ";
    text += amountDim + ":\n";
    for (int i = 1; i < mData->size(); ++i)
    {
      if (mData->at(i - 1).mVisible)
      {
        if (!text.isEmpty())
          text += + "\n";
        double amount = amountFunc(mData->at(i - 1).mValue, mData->at(i).mValue);
        QString name = mData->at(i - 1).mName;
        if (!name.isEmpty())
          name += " = ";
        text += name + QString::number(amount, 'f', 5);
      }
    }
    textItem->setText(text);
  }
}

bool GPZonesRect::isLegendVisible()
{
  return amountFunc && amountTestFunc && amountTestFunc();
}

GPLegend *GPZonesRect::getLegend()
{
  return mLegend;
}

void GPZonesRect::setAmountSettings(const QVariantMap& settings)
{
  amountSettings = settings;
  onTargetUpdated();
}

QVariantMap GPZonesRect::getAmountSettings() const
{
  return amountSettings;
}

bool GPZonesRect::startSelection(QMouseEvent *event)
{
  if (!mData || event->buttons() & Qt::RightButton || !mInteractive || !mKeyAxis || !visible())
    return false;

  double cursorPos = (mOrientation == Qt::Horizontal) ? event->pos().x() : event->pos().y();

  mDraggingIndexLeft = -1;
  mDraggingIndexRight = -1;
  for (int i = 0; i < mData->size(); ++i)
  {
    if (!mData->at(i).mVisible && !mData->at(i - 1).mVisible)
      continue;

    double delta = fabs(mKeyAxis->coordToPixel(mData->at(i).mValue) - cursorPos);
    if (delta < 10)
    {
      if (mDraggingIndexLeft == -1)
      {
        mDraggingIndexLeft = i;
        mDraggingIndexRight = i;
        mDragStart = event->pos();
      }
      else
      {
        mDraggingIndexRight = i;
      }
    }
    else if (mDraggingIndexLeft != -1)
      return true;
  }
  return false;
}

void GPZonesRect::moveSelection(QMouseEvent *event)
{
  if (!mData || !mInteractive || !mKeyAxis || mDraggingIndexLeft == -1 || !visible())
    return;

  double newPixPos = (mOrientation == Qt::Horizontal) ? event->pos().x() : event->pos().y();
  double oldPixPos = (mOrientation == Qt::Horizontal) ? mDragStart.x() : mDragStart.y();
  double oldPos = mKeyAxis->pixelToCoord(oldPixPos);
  double newPos = mKeyAxis->pixelToCoord(newPixPos);

  int mDraggingIndex = mDraggingIndexLeft;
  if (mDraggingIndexRight != -1)
  {
    if (newPos - oldPos >= 0)
    {
      mDraggingIndex = mDraggingIndexRight;
      mDraggingIndexLeft = mDraggingIndexRight;
    }
    mDraggingIndexRight = -1;
  }

  if (mDraggingIndex >= 0 && mDraggingIndex < mData->size())
  {
    //// Save input uom
    GPUomDescriptor inoputUomSafe = mData->mInputUom;

    //// Set input uom from axis bc data comes from axis
    mData->mInputUom = mKeyAxis->getUomDescriptor();

    //// Set data
    mData->setLine(mDraggingIndex, newPos);

    //// Restore input uom
    mData->mInputUom = inoputUomSafe;

    mParentPlot->replot();
  }
}

void GPZonesRect::endSelection(QMouseEvent *)
{
  if (!mInteractive || !visible())
    return;

  mDraggingIndexLeft = -1;
  mDraggingIndexRight = -1;
  mDragStart = QPoint();

  mParentPlot->replot();
}

void GPZonesRect::draw(GPPainter *painter)
{
  if (mData && mKeyAxis && mData->size() > 1 && visible())
  {
    auto rect = clipRect();

    painter->save();
    painter->setAntialiasing(false);
    painter->setClipRect(rect);

    QRect zoneRect;
    if (mOrientation == Qt::Horizontal)
    {
      zoneRect.setTop(rect.top());
      zoneRect.setBottom(rect.bottom());
    }
    else
    {
      zoneRect.setLeft(rect.left());
      zoneRect.setRight(rect.right());
    }
    painter->setFont(mStyle.textFont);

    for (int i = 0; i < mData->size() - 1; ++i)
    {
      if (mOrientation == Qt::Horizontal)
      {
        zoneRect.setLeft(mKeyAxis->coordToPixel(mData->at(i).mValue));
        zoneRect.setRight(mKeyAxis->coordToPixel(mData->at(i + 1).mValue));
      }
      else
      {
        zoneRect.setTop(mKeyAxis->coordToPixel(mData->at(i).mValue));
        zoneRect.setBottom(mKeyAxis->coordToPixel(mData->at(i + 1).mValue));
      }
      if (mData->at(i).mVisible)
      {
        painter->fillRect(zoneRect, mStyle.zonesBrush.value(i % mStyle.zonesBrush.size()));

        if (!mData->at(i).mName.isEmpty())
        {
          painter->setPen(mStyle.textColor);
          auto oldTr = painter->transform();

          painter->translate(zoneRect.center());
          painter->rotate(mStyle.textDrawAngle);

          int margin = 4;
          double rads = mStyle.textDrawAngle * M_PI / 180.0;
          double w = fabs(cos(rads)) * zoneRect.width() + fabs(sin(rads)) * zoneRect.height() - margin*2;
          double h = fabs(cos(rads)) * zoneRect.height() - 10 + fabs(sin(rads)) * zoneRect.width() - margin*2;
          QRectF newR(-w / 2.0, -h / 2.0, w, h);

          painter->drawText(newR, Qt::AlignHCenter | mStyle.textAlignment, mData->at(i).mName);

          painter->setTransform(oldTr);
        }
      }

      if (mData->at(i).mVisible || mData->at(i - 1).mVisible)
      {
        painter->setPen(mStyle.linePen);
        if (mOrientation == Qt::Horizontal)
          painter->drawLine(zoneRect.topLeft(), zoneRect.bottomLeft());
        else
          painter->drawLine(zoneRect.topLeft(), zoneRect.topRight());
      }
    }
    if (mData->at(mData->size() - 2).mVisible)
    {
      painter->setPen(mStyle.linePen);
      if (mOrientation == Qt::Horizontal)
        painter->drawLine(zoneRect.topRight(), zoneRect.bottomRight());
      else
        painter->drawLine(zoneRect.bottomLeft(), zoneRect.bottomRight());
    }

    painter->restore();
  }
}

double GPZonesRect::selectTest(const QPointF &pos, bool, QVariant*) const
{
  if (mData && mInteractive && mKeyAxis && mData->size() > 1 && visible())
  {
    if (mDraggingIndexLeft != -1)
      return 0.0;

    if (clipRect().contains(pos.toPoint()))
    {
      double cursorPos = (mOrientation == Qt::Horizontal) ? pos.x() : pos.y();
      for (int i = 0; i < mData->size(); ++i)
      {
        if (mData->at(i).mVisible && !mData->at(i).mLocked)
          if (fabs(mKeyAxis->coordToPixel(mData->at(i).mValue) - cursorPos) < 10)
            return 0.0;
      }
    }
  }
  return -1.0;
}

Qt::CursorShape GPZonesRect::HitTest(QMouseEvent* event, unsigned *)
{
  if (selectTest(event->pos(), false) == 0)
    return (mOrientation == Qt::Horizontal) ? Qt::SizeHorCursor : Qt::SizeVerCursor;
  return Qt::ArrowCursor;
}

QRect GPZonesRect::clipRect() const
{
  if (mClipRect && mKeyAxis)
  {
    if (auto axisRect = mKeyAxis->axisRect())
      return axisRect->rect();
  }
  return parentPlot()->viewport();
}

void GPZonesRect::setVisible(bool on)
{
  if (mLegend)
    mLegend->setVisible(isLegendVisible() && on);
  GPLayerable::setVisible(on);
}


///////////////////////////////////////////////////////
// Color sections rect
GPSectionsRect::GPSectionsRect(GraphicsPlot *parentPlot, GPAxis *keyAxis, GPAxisRect* rect)
  : GPSelectionRect(parentPlot)
  , mKeyAxis(keyAxis)
  , mAxisRect((rect == nullptr) ? mKeyAxis->axisRect() : rect)
{
  setUomProvider(mKeyAxis);
}

GPSectionsRect::~GPSectionsRect()
{
}

void GPSectionsRect::setIcons(const QMap<QString, QString>& icons)
{
  mIcons.clear();
  for (auto it = icons.begin(); it != icons.end(); ++it)
  {
    QPixmap pm(it.value());
    pm.setDevicePixelRatio(2);
    mIcons.insert(it.key(), pm);
  }
}

void GPSectionsRect::setSections(const QVector<double>& sections, const QVector<QString>& icons)
{
  mSections = sections;

  applyUom(mInputUom, uomDescriptor, mSections.begin(), mSections.end());

  mSectionIcons = icons;
}

void GPSectionsRect::setKeyAxis(GPAxis *axis)
{
  mKeyAxis = axis;
  setUomProvider(mKeyAxis);
}

GPAxis *GPSectionsRect::keyAxis()
{
  return mKeyAxis.data();
}

void GPSectionsRect::draw(GPPainter *painter)
{
  if (mKeyAxis && mAxisRect && !mSectionIcons.isEmpty() && (mSections.size() == mSectionIcons.size() + 1))
  {
    painter->save();
    QRect axisRect = mAxisRect->rect();
    axisRect.adjust(0, -2, 0, -2);
    painter->setClipRect(axisRect);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < mSectionIcons.size(); ++i)
    {
      if (mIcons.contains(mSectionIcons[i]))
      {
        auto sectionRect = axisRect;

        double top = mKeyAxis->coordToPixel(mSections[i]);
        double bot = mKeyAxis->coordToPixel(mSections[i + 1]);
        for (int j = i + 1; j < mSectionIcons.size(); ++j)
        {
          if (mSectionIcons[i] == mSectionIcons[j])
            bot = mKeyAxis->coordToPixel(mSections[j + 1]);
          else
            break;
          i = j;
        }
        if (top > bot)
          std::swap(top, bot);

        sectionRect.setTop(top - 1);
        sectionRect.setBottom(bot);

        auto oldTr = painter->transform();
        painter->translate(painter->transform().dx(), sectionRect.top());
        sectionRect.moveTop(0.0);
        QBrush iconBrush(mIcons[mSectionIcons[i]].scaled(32, 32, Qt::KeepAspectRatio, Qt::FastTransformation));
        painter->fillRect(sectionRect, iconBrush);
        painter->setTransform(oldTr);
      }
    }
    painter->restore();
  }
}

void GPSectionsRect::setInputUom(const GPUomDescriptor& descriptor)
{
  mInputUom = descriptor;
}

void GPSectionsRect::setInputUom(int quantity, int units)
{
  GPUomDescriptor inputUom;
  inputUom.quantity = quantity;
  inputUom.units = units;
  setInputUom(inputUom);
}

void GPSectionsRect::applyUom(const GPUomDescriptor& descriptor)
{
  applyUom(uomDescriptor, descriptor, mSections.begin(), mSections.end());
}

void GPSectionsRect::applyUom(const GPUomDescriptor& source, const GPUomDescriptor& target,
  QVector<double>::iterator from,
  QVector<double>::iterator to)
{
  if (mSections.size() != 0)
  {
    if (source != target)
    {
      //// Data
      GPUomDataTransformation transformation = uomProvider->getUomTransformation(source);
      for (auto i = from; i != to; ++i)
      {
        *i = transformation(*i, source, target);
      }
    }
  }
}


///////////////////////////////////////////////////////
// GraphicsPlotExtended impls

QMap<int, QColor> GraphicsPlotExtendedStyle::DefaultColorMap {
  {0, QColor(31,119,180)},
  {1, QColor(255,127,14)},
  {2, QColor(44,160,44)},
  {3, QColor(214,39,40)},
  {4, QColor(148,103,189)},
  {5, QColor(140,86,75)},
  {6, QColor(227,119,194)},
  {7, QColor(127,127,127)},
  {8, QColor(188,189,34)},
  {9, QColor(23,190,207)},
  {10, QColor(58,1,130)},
  {11, QColor(172,137,68)},
  {12, QColor(255,110,117)},
  {13, QColor(121,140,255)},
  {14, QColor(255,191,96)},
  {15, QColor(28,214,70)},
  {16, QColor(255,1,140)},
  {17, QColor(1,130,103)},
  {18, QColor(142,174,205)},
  {19, QColor(28,91,255)}
};

GraphicsPlotExtendedStyle::GraphicsPlotExtendedStyle()
  : DefaultPenWidth(0.7)
  , CaptionsFont(QFont("Segoe UI Light", 11))
  , CaptionsColor(Qt::black)
{

}

QMap<int, QColor> GraphicsPlotExtendedStyle::GetDefaultColorMap()
{
  return DefaultColorMap;
}

QColor GraphicsPlotExtendedStyle::GetDefaultColor(int index)
{
  return DefaultColorMap.value(index % DefaultColorMap.size());
}

QString GraphicsPlotExtendedStyle::GetDefaultColorName(int index)
{
  return GetDefaultColor(index).name();
}

void GraphicsPlotExtendedStyle::SetDefaultLegendStyle(GPLegend *legend)
{
  legend->setLayer(QLatin1String("legend"));
  legend->setAntialiased(true);
  legend->setMargins(QMargins(8, 6, 4, 6));
  legend->setBorderPen(QPen(QColor("#773b4756"), 0.7));
  legend->setBrush(QBrush(QColor("#ddf7f8f9")));
  legend->setTextColor(QColor("#ff666666"));
  legend->setSelectedTextColor(QColor("#ff000000"));
  legend->setSelectedIconBorderPen(QPen(QColor("#00000000")));
  legend->setSelectableParts(GPLegend::spItems);
}

void GraphicsPlotExtendedStyle::SetDefaultAxisStyle(GPAxis *axis)
{
  QPen lightGreyPen(QColor("#10000000"), 1.0);
  QPen midGreyPen(QColor("#2c000000"), 1.0);
  QPen darkGreyPen(QColor("#6a000000"), 1.0);

  axis->setShowCoords(true);
  axis->setAntialiased(false);
  axis->grid()->setAntialiased(false);

  axis->setBasePen(darkGreyPen);
  axis->grid()->setPen(midGreyPen);
  axis->grid()->setSubGridPen(lightGreyPen);

  axis->setTickPen(darkGreyPen);
  axis->setTickLength(0, 5);
  axis->setTicks(true);

  axis->setSubTickPen(darkGreyPen);
  axis->setSubTickLength(0, 2);
  axis->setSubTicks(false);

  axis->setLowerEnding(GPLineEnding(GPLineEnding::esNone));
  axis->setUpperEnding(GPLineEnding(GPLineEnding::esNone));

  QSharedPointer<GPAxisTicker> ticker(new GPAxisAwareTicker(axis));
  axis->setTicker(ticker);

  QFont axisFont("MS Shell Dlg 2", 8, QFont::Normal);
  axis->setLabelFont(axisFont);
  axis->setSelectedLabelFont(axisFont);
  axis->setTickLabelFont(axisFont);
  axis->setSelectedTickLabelFont(axisFont);
  axis->setLabelPadding(4);
}

//// Static data
QMap<GPAxis*, QSet<GPAxis*>> GraphicsPlotExtended::BoundAxis = QMap<GPAxis*, QSet<GPAxis*>>();
QMap<GPAxis*, QList<QMetaObject::Connection>> GraphicsPlotExtended::BoundAxisConnections = QMap<GPAxis*, QList<QMetaObject::Connection>>();

QMap<GraphicsPlotExtended*, QMap<GraphicsPlotExtended*, GP::BoundProperties>> GraphicsPlotExtended::BoundPlots = QMap<GraphicsPlotExtended*, QMap<GraphicsPlotExtended*, GP::BoundProperties>>();
QMap<GraphicsPlotExtended*, QList<QMetaObject::Connection>> GraphicsPlotExtended::BoundPlotsConnections = QMap<GraphicsPlotExtended*, QList<QMetaObject::Connection>>();

GraphicsPlotExtended::GraphicsPlotExtended(QWidget* parent, unsigned flags)
  : GraphicsPlot(parent)
  , KeyAxisOrientation(Qt::Horizontal)
  , Flags(flags)
  , PlotSettingsFlags(EnableAllTabs)
  , ViewAllDefaultFlags(GP::ViewAllDefault)
  , ZoomingAxisMoved(false)
  , DraggingObject(nullptr)
  , ZonesRect(nullptr)
  , PolygonLayer(nullptr)
  , LockAxes(false)
  , LockAxesInteraction(true)
  , PendingViewAll(false)
  , DisplayCursorCoords(false)
  , RulerShowAxesLabels(true)
  , PendingViewAllFlags(0)
  , InDragAndDrop(false)
  , InDrawingMeasuringPolygon(false)
  , InDrawingMagnifier(false)
{
  // OpenGL by default
#ifdef _MSC_VER
  setOpenGl(true, 8);
  setAntialiasedElements((GP::AntialiasedElements)
                         (GP::aeLegend
                          | GP::aeLegendItems
                          | GP::aePlottables
                          | GP::aeItems
                          | GP::aeScatters
                          | GP::aeFills));
#endif

  setAttribute(Qt::WA_Hover, true);
  grabGesture(Qt::PinchGesture);
  setFocusPolicy(Qt::WheelFocus);

  Title = new GPTextElement(this);
  plotLayout()->setRowSpacing(0);
  plotLayout()->setRowStretchFactor(0, 0.00001);
  Title->setVisible(false);
  Title->setFont(this->font());
  Title->setTextColor(QColor(0,0,0));
  Title->setMargins(QMargins(0, 0, 0, 0));

  // Set up legend
  if (legend)
  {
    delete legend;
    legend = new GPEXLegend(this);
    Style.SetDefaultLegendStyle(legend);
    legend->setVisible(false);
  }
  setAutoAddPlottableToLegend(false);

  // Can drag and zoom
  setLockAxesInteraction(false);
  setInteraction(GP::iSelectPlottables, false);
  setInteraction(GP::iSelectLegend, false);
  setInteraction(GP::iMultiSelect, true);
  setInteraction(GP::iSelectItems, true);

  setSelectionRectMode(GP::srmSelect);

  // Set up default axes
  Axes << xAxis << xAxis2 << yAxis << yAxis2;

  // Set explicitly zoomable
  QList<GPAxis*> rzaH = axisRect()->rangeZoomAxes(Qt::Horizontal);
  if (!rzaH.contains(xAxis))
    rzaH.append(xAxis);
  if (!rzaH.contains(xAxis2))
    rzaH.append(xAxis2);
  QList<GPAxis*> rzaV = axisRect()->rangeZoomAxes(Qt::Vertical);
  if (!rzaV.contains(yAxis))
    rzaV.append(yAxis);
  if (!rzaV.contains(yAxis2))
    rzaV.append(yAxis2);
  axisRect()->setRangeZoomAxes(rzaH, rzaV);

  // Set explicitly draggable
  QList<GPAxis*> rdaH = axisRect()->rangeDragAxes(Qt::Horizontal);
  if (!rdaH.contains(xAxis))
    rdaH.append(xAxis);
  if (!rdaH.contains(xAxis2))
    rdaH.append(xAxis2);
  QList<GPAxis*> rdaV = axisRect()->rangeDragAxes(Qt::Vertical);
  if (!rdaV.contains(yAxis))
    rdaV.append(yAxis);
  if (!rdaV.contains(yAxis2))
    rdaV.append(yAxis2);
  axisRect()->setRangeDragAxes(rdaH, rdaV);

  auto defaultInteraction = GP::iRangeDrag|GP::iRangeZoom;
  for (const auto& axis : Axes)
  {
    Style.SetDefaultAxisStyle(axis);
    AxisInteraction[axis] = defaultInteraction;
    axis->setShowCoords(true);
  }

  // Custom selection rect
  if (mSelectionRect)
  {
    delete mSelectionRect;
    mSelectionRect = 0;
  }

  // No labels cache
  setPlottingHint(GP::phCacheLabels, false);

  AddAssociatedPlot(this);

  // Create shortcuts
  {
    QPointer<GraphicsPlotExtended> instance(this);
    QTimer::singleShot(0, [instance](){ if (instance) instance->CreateShortcuts(); });
  }
}

GraphicsPlotExtended::~GraphicsPlotExtended()
{
  fastClearPlottables();
}

void GraphicsPlotExtended::CreateShortcuts()
{
  // ViewAll
  if (Flags & GP::AAViewActions)
  {
    auto shortcut = new QShortcut(QKeySequence(GPShortcutKeys::SKViewAll), this);
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, this, [this](){ viewAll(ViewAllDefaultFlags | GP::ViewAllForce | GP::ViewAllUser); });
  }

  // Copy plot image to clipboard
  {
    auto shortcut = new QShortcut(QKeySequence(GPShortcutKeys::SKCopyImage), this);
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, this, &GraphicsPlotExtended::CopyImage);

    shortcut = new QShortcut(QKeySequence(GPShortcutKeys::SKCopyImageWithCoords), this);
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, this, &GraphicsPlotExtended::CopyImage);
  }

#if defined(USE_XLNT)
  // Export as excel
  if(Flags & GP::AAExcelExport)
  {
    auto shortcut = new QShortcut(QKeySequence(GPShortcutKeys::SKExportExcel), this);
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, this, &GraphicsPlotExtended::ExportToExcel);
  }
#endif

  // Settings
  if (Flags & GP::AASettingsEdit)
  {
    auto shortcut = new QShortcut(QKeySequence(GPShortcutKeys::SKOpenSettings), this);
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, this, &GraphicsPlotExtended::ShowSettingsDialog);
  }
}

void GraphicsPlotExtended::setViewAllDefaultFlags(unsigned flags)
{
  ViewAllDefaultFlags = flags;
}

unsigned GraphicsPlotExtended::getViewAllDefaultFlags() const
{
  return ViewAllDefaultFlags;
}

void GraphicsPlotExtended::setFlags(const unsigned &flags)
{
  Flags = flags;
}

void GraphicsPlotExtended::setFlag(const unsigned &flag, bool active)
{
  active ? Flags = Flags | flag : Flags = Flags & ~flag;
}

bool GraphicsPlotExtended::testFlag(const unsigned &flag) const
{
  return Flags & flag;
}

void GraphicsPlotExtended::setPlotSettingsFlags(const unsigned &flags)
{
  PlotSettingsFlags = flags;
}

unsigned GraphicsPlotExtended::plotSettingsFlag()
{
  return PlotSettingsFlags;
}

GraphicsPlotExtendedStyle &GraphicsPlotExtended::getStyle() const
{
  return const_cast<GraphicsPlotExtendedStyle&>(Style);
}

unsigned GraphicsPlotExtended::getFlags() const
{
  return Flags;
}

void GraphicsPlotExtended::SetAxisGridVisible(GPAxis* axis, bool visible)
{
  axis->grid()->setVisible(visible);
  queuedReplot();
}

void GraphicsPlotExtended::SetAxisListGridVisible(const QList<GPAxis*>& axes, bool visible)
{
  for (const auto& a : axes)
    a->grid()->setVisible(visible);
  queuedReplot();
}

void GraphicsPlotExtended::SetAxisScaleType(GPAxis* axis, GPAxis::ScaleType scaleType)
{
  axis->setScaleType(scaleType);
  queuedReplot();
}

void GraphicsPlotExtended::SetMeasuringPolygonDraw(bool active)
{
  if (InDrawingMeasuringPolygon != active)
  {
    InDrawingMeasuringPolygon = active;

    if (InDrawingMeasuringPolygon)
    {
      if (!MeasuringPolygonLabel)
      {
        MeasuringPolygonLabel = new GPItemText(this);
        MeasuringPolygonLabel->setPositionAlignment(Qt::AlignRight | Qt::AlignTop);
        MeasuringPolygonLabel->position->setAxes(getDefaultAxisX(), getDefaultAxisY());
        MeasuringPolygonLabel->position->setType(GPItemPosition::ptPlotCoords);

        MeasuringPolygonLabel->setPen(QPen(Qt::black));
        MeasuringPolygonLabel->setBrush(QBrush("#ffffffff", Qt::SolidPattern));
        MeasuringPolygonLabel->setPadding(QMargins(5, 5, 5, 5));
        auto font = MeasuringPolygonLabel->font();
        font.setPixelSize(12);
        font.setBold(true);
        MeasuringPolygonLabel->setFont(font);
      }
      MeasuringPolygonLabel->setVisible(false);

      if (!MeasuringPolygon)
      {
        MeasuringPolygon = new GPCurve(getDefaultAxisX(), getDefaultAxisY());
        MeasuringPolygon->setSelectable(GP::stNone);
        GPScatterStyle ss;
        ss.setPen(QPen(QColor("#dd3535"), 1.0));
        ss.setBrush(QBrush(QColor("#dd3535")));
        ss.setShape(GPScatterStyle::ssCircle);
        ss.setSize(5.0);
        MeasuringPolygon->setScatterStyle(ss);
        MeasuringPolygon->setPen(QPen(QColor("#dd3535"), 2));
      }
      MeasuringPolygon->data()->clear();
    }
    else if (MeasuringPolygon && MeasuringPolygonLabel)
    {
      int size = MeasuringPolygon->data()->size();
      if (size > 2)
      {
        MeasuringPolygon->addData(size,
                                  MeasuringPolygon->data()->at(0)->key,
                                  MeasuringPolygon->data()->at(0)->value);

        double minX = MeasuringPolygon->data()->at(0)->key;
        double minY = MeasuringPolygon->data()->at(0)->value;
        for (int i = 1; i < size - 1; ++i)
        {
          double x = MeasuringPolygon->data()->at(i)->key;
          if (x < minX)
            minX = x;
          double y = MeasuringPolygon->data()->at(i)->value;
          if (y < minY)
            minY = y;
        }
        double area = 0.0;
        for (int i = 0; i < size - 1; ++i)
        {
          double x = MeasuringPolygon->data()->at(i)->key;
          double y1 = 0.0;
          double y2 = MeasuringPolygon->data()->at(i + 1)->value;
          if (i == 0)
            y1 = MeasuringPolygon->data()->at(size - 2)->value;
          else
            y1 = MeasuringPolygon->data()->at(i - 1)->value;
          area += (x - minX) * ((y1 - minY) - (y2 - minY));
        }
        area = fabs(area/2.0);

        MeasuringPolygonLabel->setText(QString::number(area));
        MeasuringPolygonLabel->position->setCoords(MeasuringPolygon->data()->at(0)->key, MeasuringPolygon->data()->at(0)->value);
        MeasuringPolygonLabel->setVisible(true);
      }
    }
    replot();
  }
}

void GraphicsPlotExtended::AddMeasuringPolygonPoint(QPoint mousePos)
{
  if (MeasuringPolygon && MeasuringPolygon->keyAxis() && MeasuringPolygon->valueAxis() && MeasuringPolygon->data())
  {
    double key = MeasuringPolygon->keyAxis()->pixelToCoord(mousePos.x());
    double value = MeasuringPolygon->valueAxis()->pixelToCoord(mousePos.y());
    MeasuringPolygon->addData(MeasuringPolygon->data()->size(), key, value);
  }
  replot();
}

void GraphicsPlotExtended::SetMagnifierDraw(bool active)
{
  if (InDrawingMagnifier != active)
  {
    InDrawingMagnifier = active;
    if (InDrawingMagnifier)
    {
      if (!Magnifier)
      {
        Magnifier = new GPMagnifier(this);
      }
    }
    else
    {
      if (Magnifier)
      {
        Magnifier->ApplyZoom();
        delete Magnifier.data();
        Magnifier.clear();
      }
    }
    replot();
  }
}

Qt::Orientation GraphicsPlotExtended::getKeyAxisOrientation() const
{
  return KeyAxisOrientation;
}

void GraphicsPlotExtended::setKeyAxisOrientation(const Qt::Orientation &value)
{
  KeyAxisOrientation = value;
}

void GraphicsPlotExtended::AxisReseted()
{
  for (const auto& ruler : LineRulers)
  {
    ruler->OnAxisReseted();
  }
}

void GraphicsPlotExtended::CurvesUpdated()
{
  for (const auto& ruler : LineRulers)
    ruler->OnCurveUpdated();

  if (ZonesRect)
    ZonesRect->onTargetUpdated();
}

void GraphicsPlotExtended::queuedReplot()
{
  replot(GraphicsPlot::rpQueuedReplot);
}

void GraphicsPlotExtended::OnAssociatedPlotChanged()
{
  auto changedPlot = qobject_cast<GraphicsPlotExtended*>(sender());
  if (changedPlot)
  {
    bool lock = changedPlot->getLockAxes();
    bool inter = changedPlot->getLockAxesInteraction();
    for (const auto& plot : AssociatedPlots)
    {
      if (plot != changedPlot)
      {
        QSignalBlocker l(plot);
        plot->setLockAxes(lock);
        plot->setLockAxesInteraction(inter);
      }
    }
  }
}

void GraphicsPlotExtended::AddRuler(QPoint pos)
{
  if (Flags & GP::AARulers)
  {
    GPRulerLine* ruler = new GPRulerLine(this, (KeyAxisOrientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal, pos);
    LineRulers.append(QPointer<GPRulerLine>(ruler));
    queuedReplot();
  }
  else if (Flags & GP::AADistanceRuler)
  {
    delete DistanceRuler;
    DistanceRuler = new GPRulerDistance(xAxis, yAxis, pos);
    queuedReplot();
  }
}

void GraphicsPlotExtended::RemoveRuler()
{
  if (Flags & GP::AARulers)
  {
    for (int i = 0; i < LineRulers.size(); ++i)
    {
      if (LineRulers[i])
      {
        if (LineRulers[i]->selected())
        {
          removeItem(LineRulers[i].data());
          LineRulers.removeAt(i--);
        }
      }
      else
      {
        LineRulers.removeAt(i--);
      }
    }
    queuedReplot();
  }
}

void GraphicsPlotExtended::RemoveRulers()
{
  if (Flags & GP::AARulers)
  {
    for (int i = 0; i < LineRulers.size(); ++i)
    {
      if (LineRulers[i])
      {
        removeItem(LineRulers[i].data());
        LineRulers.removeAt(i--);
      }
    }
    LineRulers.clear();
    queuedReplot();
  }
  else if (Flags & GP::AADistanceRuler)
  {
    if (DistanceRuler)
      removePlottable(DistanceRuler);
    queuedReplot();
  }
}

void GraphicsPlotExtended::setLockAxes(bool lock)
{
  if (LockAxes != lock)
  {
    LockAxes = lock;
    emit axesLockChanged(lock);
  }
}

bool GraphicsPlotExtended::getLockAxes() const
{
  return LockAxes;
}

void GraphicsPlotExtended::setLockAxesInteraction(bool lock)
{
  if (LockAxesInteraction != lock)
  {
    LockAxesInteraction = lock;

    setInteraction(GP::iRangeDrag, !lock);
    setInteraction(GP::iRangeZoom, !lock);
    emit axesInteractionChanged(lock);
  }
}

bool GraphicsPlotExtended::getLockAxesInteraction() const
{
  return LockAxesInteraction;
}

void GraphicsPlotExtended::setTitleText(const QString& text)
{
  if (plotLayout()->rowCount() > 0 && plotLayout()->columnCount() > 0)
  {
    if (text.isEmpty() && plotLayout()->element(0, 0) == Title)
    {
      if (plotLayout()->take(Title))
        plotLayout()->removeRow(0);
    }
    else if (!text.isEmpty() && plotLayout()->element(0, 0) != Title)
    {
      if (plotLayout()->hasElement(0,0))
        plotLayout()->insertRow(0);
      plotLayout()->addElement(0, 0, Title);
    }

    Title->setText(text);
    Title->setVisible(!text.isEmpty());
  }
  else
  {
    Title->setVisible(false);
  }
}

QString GraphicsPlotExtended::getTitleText() const
{
  return Title->text();
}

GPTextElement *GraphicsPlotExtended::getTitle()
{
  return Title;
}

GPEXLegend *GraphicsPlotExtended::legendEx() const
{
  return qobject_cast<GPEXLegend*>(legend);
}

void GraphicsPlotExtended::setLegendVisible(GPLegend *legend, bool visible)
{
  if (legend)
  {
    legend->setVisible(visible);
    queuedReplot();
  }
}

bool GraphicsPlotExtended::getLegendVisible(GPLegend *legend) const
{
  if (legend)
    return legend->visible();
  return false;
}

void GraphicsPlotExtended::setLegendPosition(GPLegend *legend, Qt::Alignment align)
{
  if (legend)
  {
    axisRect()->insetLayout()->setInsetAlignment(axisRect()->insetLayout()->indexOf(legend), align);
    queuedReplot();
  }
}

Qt::Alignment GraphicsPlotExtended::getLegendPosition(GPLegend *legend) const
{
  return axisRect()->insetLayout()->insetAlignment(axisRect()->insetLayout()->indexOf(legend));
}

void GraphicsPlotExtended::AddAssociatedPlot(GraphicsPlotExtended *plot)
{
  AssociatedPlots.append(QPointer<GraphicsPlotExtended>(plot));
  connect(plot, &GraphicsPlotExtended::axesInteractionChanged, this, &GraphicsPlotExtended::OnAssociatedPlotChanged);
  connect(plot, &GraphicsPlotExtended::axesLockChanged, this, &GraphicsPlotExtended::OnAssociatedPlotChanged);
}

void GraphicsPlotExtended::ViewAllAxis(GPAxis* ax, GPRange &prefferedRange, unsigned flags)
{
  if (!ax)
    return;

  // calc epsilon
  GPRange range = ax->range();

  // flags
  unsigned lowerExpandMask    = 0;
  unsigned lowerContractMask  = 0;
  unsigned lowerZeroMask      = 0;

  unsigned upperExpandMask    = 0;
  unsigned upperContractMask  = 0;
  unsigned upperZeroMask      = 0;

  if (ax->orientation() == Qt::Vertical)
  {
    lowerExpandMask   = GP::ViewAllVerticalExpandLower;
    lowerContractMask = GP::ViewAllVerticalContractLower;
    lowerZeroMask     = GP::ViewAllVerticalZeroLower;

    upperExpandMask   = GP::ViewAllVerticalExpandUpper;
    upperContractMask = GP::ViewAllVerticalContractUpper;
    upperZeroMask     = GP::ViewAllVerticalZeroUpper;
  }
  else
  {
    lowerExpandMask   = GP::ViewAllHorizontalExpandLower;
    lowerContractMask = GP::ViewAllHorizontalContractLower;
    lowerZeroMask     = GP::ViewAllHorizontalZeroLower;

    upperExpandMask   = GP::ViewAllHorizontalExpandUpper;
    upperContractMask = GP::ViewAllHorizontalContractUpper;
    upperZeroMask     = GP::ViewAllHorizontalZeroUpper;
  }

  // Zeros first
  if (flags & lowerZeroMask)
    range.lower = 0;
  if (flags & upperZeroMask)
    range.upper = 0;

  // Lower
  if (prefferedRange.lower < range.lower)
  {
    if (flags & lowerExpandMask)
      range.lower = prefferedRange.lower;
  }
  else
  {
    if (flags & lowerContractMask)
      range.lower = prefferedRange.lower;
  }

  // Upper
  if (prefferedRange.upper > range.upper)
  {
    if (flags & upperExpandMask)
      range.upper = prefferedRange.upper;
  }
  else
  {
    if (flags & upperContractMask)
      range.upper = prefferedRange.upper;
  }

  range.normalize();
  double apr = getAxisPixelRange(ax);
  double epsilon = 0.0;
  double coordPerPixel = range.size() / apr;

  if (range.size() > std::numeric_limits<double>::epsilon() &&
      coordPerPixel > std::numeric_limits<double>::epsilon())
  {
    epsilon = coordPerPixel * 2.0;
  }

  // Zeros first
  if (flags & lowerZeroMask)
    range.lower = -epsilon;
  if (flags & upperZeroMask)
    range.upper = epsilon;

  ax->setRange(range);
  prefferedRange = range;
}

void GraphicsPlotExtended::viewAll(const QSet<GPAxis*>& affectedAxes, unsigned flags)
{
  const bool forceViewAll = flags & GP::ViewAllForce;

  if (!isVisible() && !(flags & GP::ViewAllImmediately))
  {
    PendingViewAll = true;
    PendingViewAllFlags |= flags;
    PendingViewAllAxes |= affectedAxes;
    return;
  }

  bool oldblockReplots = getBlockReplots();
  blockReplots(true);

  PendingViewAll = false;
  flags |= PendingViewAllFlags;
  PendingViewAllAxes |= affectedAxes;

  if (flags & GP::ViewAllImmediately)
    updateLayout();

  if (forceViewAll || !LockAxes)
  {
    QMap<GPAxis*, double> ceils;
    QMap<GPAxis*, double> floors;

    auto calculateAxisRange = [&ceils, &floors](GPAxis* axis, GPRange &keyRange)
    {
      auto axes = getBoundAxes(axis);
      axes.insert(axis);

      {
        double max = 0.0;
        if (!ceils.contains(axis))
        {
          if (!qIsInf(keyRange.upper) && !qIsNaN(keyRange.upper))
            max = keyRange.upper;
        }
        else
        {
          if (!axes.isEmpty() && ceils.contains(*(axes.begin())))
            max = std::max(ceils[axis], ceils[*(axes.begin())]);
          if (!qIsInf(keyRange.upper) && !qIsNaN(keyRange.upper))
            max = std::max(max, keyRange.upper);
        }
        ceils[axis] = max;
        for (const auto& ax : axes)
          ceils[ax] = max;
      }
      {
        double min = 0.0;
        if (!floors.contains(axis))
        {
          if (!qIsInf(keyRange.lower) && !qIsNaN(keyRange.lower))
            min = keyRange.lower;
        }
        else
        {
          if (!axes.isEmpty() && floors.contains(*(axes.begin())))
            min = std::min(floors[axis], floors[*(axes.begin())]);
          if (!qIsInf(keyRange.lower) && !qIsNaN(keyRange.lower))
            min = std::min(min, keyRange.lower);
        }
        floors[axis] = min;
        for (auto ax : axes)
          floors[ax] = min;
      }
    };

    auto calculatePlottableRanges = [&calculateAxisRange, this](GPAbstractPlottable* pl)
    {
      auto calculateRange = [&calculateAxisRange](GPAxis* axis, GPRange &range, double penWidth) {
        if (axis->scaleType() != GPAxis::stLogarithmic)
        {
          if (fabs(range.size()) > std::numeric_limits<double>::epsilon())
          {
            double apr = getAxisPixelRange(axis);
            if (apr > 0)
            {
              double coordPerPixel = range.size() / apr;
              double penWidthInCoords = penWidth * coordPerPixel;
              if (axis->rangeReversed())
              {
                range.upper += penWidthInCoords;
                range.lower -= penWidthInCoords;
              }
              else
              {
                range.upper -= penWidthInCoords;
                range.lower += penWidthInCoords;
              }
            }
          }
        }
        calculateAxisRange(axis, range);
      };

      if (PendingViewAllAxes.contains(pl->keyAxis()))
      {
        bool foundRange = false;
        GPRange range = pl->getKeyRange(foundRange, GP::sdBoth);
        if (foundRange)
          calculateRange(pl->keyAxis(), range, pl->pen().widthF());
      }
      if (PendingViewAllAxes.contains(pl->valueAxis()))
      {
        bool foundRange = false;
        GPRange range = pl->getValueRange(foundRange, GP::sdBoth);
        if (foundRange)
          calculateRange(pl->valueAxis(), range, pl->pen().widthF());
      }
    };

    double scatterMaxSize = 0.0;

    for (const auto pl : mGraphs)
    {
      if (pl->realVisibility())
      {
        scatterMaxSize = qMax(pl->scatterStyle().size(), scatterMaxSize);
        calculatePlottableRanges(pl);
      }
    }
    for (const auto& pl : mCurves)
    {
      if (pl->realVisibility())
      {
        scatterMaxSize = qMax(pl->scatterStyle().size(), scatterMaxSize);
        calculatePlottableRanges(pl);
      }
    }

    if (fabs(scatterMaxSize) > std::numeric_limits<double>::epsilon())
    {
      const double scatterPad = 3.0;
      scatterMaxSize += scatterPad;
    }

    //////////////////////////////////////////////////
    // Apply new ranges
    double maxCoordPerPixel = 0;
    if (flags & GP::ViewAllKeepOriginalRatio)
    {
      for (auto ax = PendingViewAllAxes.begin(); ax != PendingViewAllAxes.end(); ++ax)
      {
        if (!floors.contains(*ax) || !ceils.contains(*ax))
          continue;

        GPRange range(floors.value(*ax), ceils.value(*ax));
        range.normalize();
        double apr = getAxisPixelRange(*ax);
        if (apr > 0)
        {
          double d = range.size() / apr;
          if (d > maxCoordPerPixel)
            maxCoordPerPixel = d;
        }
      }
    }

    for (auto ax = PendingViewAllAxes.begin(); ax != PendingViewAllAxes.end(); ++ax)
    {
      if (!floors.contains(*ax) || !ceils.contains(*ax))
        continue;

      unsigned lowerMargin = 0;
      unsigned upperMargin = 0;
      if ((*ax)->orientation() == Qt::Vertical)
      {
        lowerMargin = GP::ViewAllVerticalMarginLower;
        upperMargin = GP::ViewAllVerticalMarginUpper;
      }
      else
      {
        lowerMargin = GP::ViewAllHorizontalMarginLower;
        upperMargin = GP::ViewAllHorizontalMarginUpper;
      }

      GPRange range(floors.value(*ax), ceils.value(*ax));
      range.normalize();
      if (fabs(range.size()) < std::numeric_limits<double>::epsilon())
      {
        double eps = 0.5;
        range.upper = range.upper + eps;
        range.lower = range.lower - eps;
      }
      else
      {
        if ((*ax)->scaleType() == GPAxis::stLogarithmic)
        {
          /// 0.912 = sqrt(1/1.2)
          /// 1.095 = 1/sqrt(1/1.2)
          range = range.sanitizedForLogScale();
          if (flags & lowerMargin)
            range.lower = 0.912 * range.lower;
          if (flags & upperMargin)
            range.upper = 1.095 * range.upper;
        }
        else if (fabs(scatterMaxSize) > std::numeric_limits<double>::epsilon())
        {
          double apr = getAxisPixelRange(*ax);

          if (flags & GP::ViewAllKeepOriginalRatio)
          {
            double center = range.center();
            double scaledRangeDelta = maxCoordPerPixel * apr;
            range = GPRange(center - scaledRangeDelta / 2.0, center + scaledRangeDelta / 2.0);
          }

          if (apr > 0)
          {
            double pxRangeNew = apr - scatterMaxSize * 2;
            double newRatioVperPx = range.size() / pxRangeNew;
            double eps = scatterMaxSize * newRatioVperPx;
            if (pxRangeNew > 0)
            {
              if (flags & lowerMargin)
                range.lower -= eps;
              if (flags & upperMargin)
                range.upper += eps;
            }
          }
        }
      }
      ViewAllAxis(*ax, range, flags);
    }
  }

  PendingViewAllFlags = GP::ViewAllNone;
  PendingViewAllAxes.clear();

  blockReplots(oldblockReplots);
  queuedReplot();
}

void GraphicsPlotExtended::viewAll(unsigned flags)
{
  viewAll(QSet<GPAxis *>(Axes.begin(), Axes.end()), flags);
}

void GraphicsPlotExtended::viewAll(bool forceViewAll)
{
  unsigned flags = ViewAllDefaultFlags;
  if (forceViewAll)
    flags |= GP::ViewAllForce;
  viewAll(flags);
}

void GraphicsPlotExtended::viewAll(const QSet<GPAxis*>& affectedAxes, bool forceViewAll)
{
  unsigned flags = ViewAllDefaultFlags;
  if (forceViewAll)
    flags |= GP::ViewAllForce;
  viewAll(affectedAxes, flags);
}

void GraphicsPlotExtended::bindAxes(GPAxis* axis1, GPAxis* axis2)
{
  if (!BoundAxis.contains(axis1))
    connect(axis1, &GPAxis::destroyed, OnBoundAxisRemoved);
  if (!BoundAxis.contains(axis2))
    connect(axis2, &GPAxis::destroyed, OnBoundAxisRemoved);

  BoundAxis[axis1].insert(axis2);
  BoundAxis[axis2].insert(axis1);

  axis2->setRange(axis1->range());

  auto connectFor1 = connect(axis2, QOverload<const GPRange&>::of(&GPAxis::rangeChanged), [axis1, axis2](const GPRange &range) {
    if (axis1->range() != range
     && axis1->getUomDescriptor() == axis2->getUomDescriptor())
    {
      axis1->setRange(range);
      if (axis1->parentPlot() && axis1->parentPlot() != axis2->parentPlot())
        axis1->parentPlot()->replot(rpQueuedReplot);
    }
  });

  auto connectFor2 = connect(axis1, QOverload<const GPRange&>::of(&GPAxis::rangeChanged), [axis1, axis2](const GPRange &range) {
    if (axis2->range() != range
     && axis1->getUomDescriptor() == axis2->getUomDescriptor())
    {
      axis2->setRange(range);
      if (axis2->parentPlot() && axis1->parentPlot() != axis2->parentPlot())
        axis2->parentPlot()->replot(rpQueuedReplot);
    }
  });

  BoundAxisConnections[axis1].append(connectFor1);
  BoundAxisConnections[axis2].append(connectFor2);
}

void GraphicsPlotExtended::OnBoundAxisRemoved(QObject *obj)
{
  for (auto it = BoundAxis.begin(); it != BoundAxis.end(); ++it)
    (*it).remove((GPAxis*)obj);
  BoundAxis.remove((GPAxis*)obj);
  auto connections = BoundAxisConnections.value((GPAxis*)obj);
  for (const auto& connection : connections)
    disconnect(connection);
  BoundAxisConnections.remove((GPAxis*)obj);
}

void GraphicsPlotExtended::bindPlots(GraphicsPlotExtended* plot1, GraphicsPlotExtended* plot2, GP::BoundProperties bp)
{
  if (!BoundPlots.contains(plot1))
    connect(plot1, &GraphicsPlotExtended::destroyed, OnBoundPlotsRemoved);
  if (!BoundPlots.contains(plot2))
    connect(plot2, &GraphicsPlotExtended::destroyed, OnBoundPlotsRemoved);

  BoundPlots[plot1][plot2] = bp;
  BoundPlots[plot2][plot1] = bp;

  auto handler = [](GraphicsPlotExtended* source, GraphicsPlotExtended* target) {
    GP::BoundProperties bp = BoundPlots[source].value(target, GP::BoundPropertiesNone);
    if (bp & GP::BoundPropertiesLockAxes)
      target->setLockAxes(source->getLockAxes());
    if (bp & GP::BoundPropertiesBlockAxesInteraction)
      target->setLockAxesInteraction(source->getLockAxesInteraction());
  };

  // Sync now
  handler(plot1, plot2);

  // Callbacks
  auto handlerFor1 = [handler, plot1, plot2]() {
    handler(plot2, plot1);
  };

  auto handlerFor2 = [handler, plot1, plot2]() {
    handler(plot1, plot2);
  };

  BoundPlotsConnections[plot1].append(connect(plot2, &GraphicsPlotExtended::axesLockChanged,         handlerFor1));
  BoundPlotsConnections[plot1].append(connect(plot2, &GraphicsPlotExtended::axesInteractionChanged,  handlerFor1));

  BoundPlotsConnections[plot2].append(connect(plot1, &GraphicsPlotExtended::axesLockChanged,         handlerFor2));
  BoundPlotsConnections[plot2].append(connect(plot1, &GraphicsPlotExtended::axesInteractionChanged,  handlerFor2));
}

QMap<GraphicsPlotExtended*, GP::BoundProperties> GraphicsPlotExtended::getBoundPlotsMap(GraphicsPlotExtended* plot)
{
  auto bound = BoundPlots.value(plot);
  for (auto it = bound.begin(); it != bound.end(); ++it)
  {
    auto plots = BoundPlots.value(it.key());
    for (auto p = plots.begin(); p != plots.end(); p++)
      bound.insert(p.key(), p.value());
  }
  return bound;
}

QSet<GraphicsPlotExtended*> GraphicsPlotExtended::getBoundPlots(GraphicsPlotExtended* plot)
{
  auto ks = getBoundPlotsMap(plot).keys();
  return QSet<GraphicsPlotExtended*>(ks.begin(), ks.end());
}

void GraphicsPlotExtended::OnBoundPlotsRemoved(QObject* obj)
{
  for (auto it = BoundPlots.begin(); it != BoundPlots.end(); ++it)
    (*it).remove((GraphicsPlotExtended*)obj);
  BoundPlots.remove((GraphicsPlotExtended*)obj);
  auto connections = BoundPlotsConnections.value((GraphicsPlotExtended*)obj);
  for (const auto& connection : connections)
    disconnect(connection);
  BoundPlotsConnections.remove((GraphicsPlotExtended*)obj);
}

void GraphicsPlotExtended::processPointSelection(QMouseEvent* event)
{
  bool selectionStateChanged = false;
  blockSignals(true);

  QList<QVariant> detailsList;
  auto list = layerableListAt(event->pos(), true, &detailsList);
  if (!list.isEmpty())
  {
    GPLayerable *clickedLayerable = list.first();

    if (auto legendItem = qobject_cast<GPPlottableLegendItem*>(list.first()))
    {
      if (!mInteractions.testFlag(GP::iSelectLegend))
      {
        selectionStateChanged = true;
        legendItem->plottable()->setVisible(!legendItem->plottable()->visible());
        if (qobject_cast<GPCurve*>(legendItem->plottable()))
          CurvesUpdated();
      }
      else if (mInteractions.testFlag(GP::iMultiSelect))
      {
        if (auto legend = qobject_cast<GPEXLegend*>(legendItem->parentLegend()))
        {
          legend->selectItem(legendItem, event->modifiers());
        }
      }
    }
    else
    {
      QSet<GPLayerable *> clicked(list.begin(), list.end());
      bool additive = mInteractions.testFlag(GP::iMultiSelect) && event->modifiers().testFlag(mMultiSelectModifier);
      // deselect all other layerables if not additive selection:
      if (!additive)
      {
        for (const auto& layer : mLayers)
        {
          for (const auto& layerable : layer->children())
          {
            if (!clicked.contains(layerable) && mInteractions.testFlag(layerable->selectionCategory()))
            {
              bool selChanged = false;
              layerable->deselectEvent(&selChanged);
              selectionStateChanged |= selChanged;
            }
          }
        }
      }
      if (clickedLayerable && mInteractions.testFlag(clickedLayerable->selectionCategory()))
      {
        // a layerable was actually clicked, call its selectEvent:
        bool selChanged = false;
        clickedLayerable->selectEvent(event, additive, detailsList.first(), &selChanged);
        selectionStateChanged |= selChanged;
      }
    }
  }
  else
  {
    if (!event->modifiers().testFlag(mMultiSelectModifier))
    {
      bool selChanged = false;
      for (const auto& layerables : mLayers)
      {
        for (const auto& layerable : layerables->children())
        {
          layerable->deselectEvent(&selChanged);
          selectionStateChanged |= selChanged;
        }
      }
    }
  }
  blockSignals(false);

  if (selectionStateChanged)
  {
    emit selectionChangedByUser();
    replot(rpQueuedReplot);
  }
}

GPCurve *GraphicsPlotExtended::addCurve(int index, GPAxis *keyAxis, GPAxis *valueAxis)
{
  GPCurve* curve = GraphicsPlot::addCurve(keyAxis, valueAxis);
  if (curve)
  {
    if (index != -1)
      curve->setProperty(OKCurveId, index);
    LoadCurve(curve);
  }
  return curve;
}

GPCurve* GraphicsPlotExtended::addCurve(GPAxis *keyAxis, GPAxis *valueAxis)
{
  return addCurve(-1, keyAxis, valueAxis);
}

GPCurve* GraphicsPlotExtended::addCurve(int index)
{
  return addCurve(index, nullptr, nullptr);
}

void GraphicsPlotExtended::resizeEvent(QResizeEvent *event)
{
  GraphicsPlot::resizeEvent(event);
  emit viewportResized();
}

QSet<GPAxis*> GraphicsPlotExtended::getBoundAxes(GPAxis* axis)
{
  auto bound = BoundAxis.value(axis);
  for (auto it = bound.begin(); it != bound.end(); ++it)
    bound.unite(BoundAxis.value(*it));
  return bound;
}

GPAxis* GraphicsPlotExtended::addAxis(Qt::Orientation orientation, const QString& label, const QString& id)
{
  if (orientation == Qt::Vertical)
    return addAxis(GPAxis::atLeft, label, id);
  return addAxis(GPAxis::atBottom, label, id);
}

GPAxis *GraphicsPlotExtended::addAxis(GPAxis::AxisType axisType, const QString& label, const QString& id)
{
  return addAxis(axisRect(), axisType, nullptr, label, id);
}

GPAxis* GraphicsPlotExtended::addAxis(GPAxis::AxisType axisType, GPAxis *axis, const QString& label, const QString& id)
{
  return addAxis(axisRect(), axisType, axis, label, id);
}

GPAxis *GraphicsPlotExtended::addAxis(GPAxisRect* rect, GPAxis::AxisType axisType, GPAxis* axis, const QString& label, const QString& id)
{
  GPAxis* result = rect->addAxis(axisType, axis);
  if (result)
  {
    result->setLabel(label);
    result->setProperty(OKAxisId, id);
    result->setLayer(QLatin1String("axes"));
    SetAxisInteraction(result, rect, (GP::Interaction)(int)(GP::iRangeDrag|GP::iRangeZoom));
    Style.SetDefaultAxisStyle(result);
    Axes.append(result);
  }
  return std::move(result);
}

QList<GPAxis*> GraphicsPlotExtended::GetAllAxes()
{
  return Axes;
}

QList<QPointer<GPAxis>> GraphicsPlotExtended::GetAllZoomableAxes(GPAxisRect* parent)
{
  QList<QPointer<GPAxis>> zoomables;
  GP::Interaction ia = (GP::Interaction)(int)interactions();
  if (ia & GP::iRangeZoom)
  {
    QList<GPAxis*> axes;
    if (parent)
    {
      axes << parent->rangeZoomAxes(Qt::Vertical);
      axes << parent->rangeZoomAxes(Qt::Horizontal);
    }
    for (const auto& axis : axes)
      if ((GetAxisInteraction(axis) & GP::iRangeZoom))
        zoomables.append(axis);
  }
  return std::move(zoomables);
}

QList<QPointer<GPAxis>> GraphicsPlotExtended::GetAllDraggableAxes()
{
  QList<QPointer<GPAxis>> draggables;
  GP::Interaction ia = (GP::Interaction)(int)interactions();
  if (ia & GP::iRangeDrag)
  {
    for (const auto& axis : Axes)
      if (GetAxisInteraction(axis) & GP::iRangeDrag)
        draggables << axis;
  }
  return std::move(draggables);
}

QList<GPAxis*> GraphicsPlotExtended::GetAllHorizontalAxes()
{
  QList<GPAxis*> horizontals;
  for (const auto& axis : Axes)
    if (axis->orientation() == Qt::Horizontal)
      horizontals.append(axis);
  return horizontals;
}

QList<GPAxis*> GraphicsPlotExtended::GetAllVerticalAxes()
{
  QList<GPAxis*> verticals;

  for (const auto& axis : Axes)
  {
    if (axis->orientation() == Qt::Vertical)
      verticals.append(axis);
  }
  return std::move(verticals);
}

QList<GPAxis*> GraphicsPlotExtended::GetHorizontalAxesWithGrid()
{
  QList<GPAxis*> result;
  for (const auto& axis : Axes)
  {
    if (axis->orientation() == Qt::Horizontal)
    {
      if (axis->property("Grid").toBool())
        result.append(axis);
    }
  }
  if (!result.contains(xAxis))
    result.append(xAxis);
  return std::move(result);
}

QList<GPAxis*> GraphicsPlotExtended::GetVerticalAxesWithGrid()
{
  QList<GPAxis*> result;
  for (const auto& axis : Axes)
  {
    if (axis->orientation() == Qt::Vertical)
    {
      if (axis->property("Grid").toBool())
        result.append(axis);
    }
  }
  if (!result.contains(yAxis))
    result.append(yAxis);
  return std::move(result);
}

bool GraphicsPlotExtended::IsZoomable(GPAxis* axis)
{
  return GetAxisInteraction(axis) & GP::iRangeZoom;
}

bool GraphicsPlotExtended::IsDraggable(GPAxis* axis)
{
  return GetAxisInteraction(axis) & GP::iRangeDrag;
}

void GraphicsPlotExtended::moveAxis(GPAxisRect* rect, GPAxis* axis, GPAxis::AxisType to)
{
  if (axis->axisType() == to)
    return;

  GraphicsPlot::moveAxis(rect, axis, to);
}

void GraphicsPlotExtended::SaveSettings()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
  QString key = GetObjectKey(this);

  SetSettings(projectId, key + "/lock_axis", LockAxes);
  SetSettings(projectId, key + "/block_axis", getLockAxesInteraction());
  SetSettings(projectId, key + "/rulers_show_axis_labels", RulerShowAxesLabels);

  SetSettings(projectId, key + "/captions_color", Style.CaptionsColor.name(QColor::HexArgb));
  SetSettings(projectId, key + "/captions_font", Style.CaptionsFont.toString());

  SaveAxes();
  SaveCurves();
  SaveLegends();
  SaveTitle();
  SaveObjects();
}

void GraphicsPlotExtended::LoadSettings()
{
  auto load = [this](){
    LoadAxes();
    LoadCurves();
    LoadLegends();
    LoadTitle();
    LoadObjects();
  };

  if (!property(OKProjectId).isNull())
  {
    QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
    QString key = GetObjectKey(this);

    QVariant color = GetSettings(projectId, key + "/captions_color");
    if (color.isNull())
    {
      Style.CaptionsColor.setNamedColor(color.toString());
      Style.CaptionsFont.fromString(GetSettings(projectId, key + "/captions_font").toString());
    }
    else
    {
      Style.CaptionsColor = Qt::black;
      Style.CaptionsFont = QFont("Segoe UI Semilight", 11);
    }

    load();

    LockAxes = GetSettings(projectId, key + "/lock_axis", LockAxes).toBool();
    setLockAxesInteraction(GetSettings(projectId, key + "/block_axis", LockAxesInteraction).toBool());
    RulerShowAxesLabels = GetSettings(projectId, key + "/rulers_show_axis_labels", RulerShowAxesLabels).toBool();
  }
  else
  {
    load();
  }
}

void GraphicsPlotExtended::ResetSettings()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());

  QString key = property(OKPlotId).toString();
  if (key.isEmpty())
    return;
  if (!property(OKDataId).isNull())
    key += "/" + property(OKDataId).toString();

  QString curveId = key + "/curves";
  SetSettings(projectId, curveId, QVariant());

  QString axisKey = key + "/axes";
  SetSettings(projectId, axisKey, QVariant());

  QString legendKey = key + "/legends";
  SetSettings(projectId, legendKey, QVariant());

  QString textKey = key + "/text_items";
  SetSettings(projectId, textKey, QVariant());
}

void GraphicsPlotExtended::SetSettings(const QUuid &projectId, const QString &key, const QVariant &settings)
{
  Q_UNUSED(projectId);
  Q_UNUSED(key);
  Q_UNUSED(settings);

#if defined(USE_SETTINGS)
  if (!projectId.isNull())
    Common::Settings::Get().SetDynamicValue(projectId, key, settings);
  else
  {
    QVariantMap map;
    map["value"] = settings;
    Common::Settings::Get().SetSection(key, map);
  }
#endif
}

QVariant GraphicsPlotExtended::GetSettings(const QUuid &projectId, const QString &key, const QVariant &defaultValue)
{
  Q_UNUSED(projectId);
  Q_UNUSED(key);
  Q_UNUSED(defaultValue);

#if defined(USE_SETTINGS)
  if (!projectId.isNull())
    return Common::Settings::Get().GetDynamicValue(projectId, key, defaultValue);
  return Common::Settings::Get().GetSection(key).value("value", defaultValue);
#endif
  return defaultValue;
}

void GraphicsPlotExtended::LoadSettings(GPCurve *curve)
{
  LoadCurve(curve);
}

void GraphicsPlotExtended::LoadSettings(GPAxis *axis)
{
  LoadAxis(axis);
}

void GraphicsPlotExtended::LoadSettings(GPLegend *legend)
{
  LoadLegend(legend);
}

void GraphicsPlotExtended::SaveAxes()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());

  for (const auto& axis : Axes)
    SaveObjectSettings(projectId, axis);
}

void GraphicsPlotExtended::LoadAxes()
{
  for (const auto& axis : Axes)
    LoadAxis(axis);
}

void GraphicsPlotExtended::LoadAxis(GPAxis* axis)
{
  if (!property(OKProjectId).isNull())
  {
    QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
    if (LoadObjectSettings(projectId, axis))
      return;
  }
  LoadAxisDefault(axis);
}

void GraphicsPlotExtended::LoadAxisDefault(GPAxis *axis)
{
  if (!axis->property(PKDefaultColor).isNull())
  {
    QColor color(axis->property(PKDefaultColor).toString());
    axis->setLabelColor(color);
    axis->setTickLabelColor(color);
  }
  if (!axis->property(PKDefaultFont).isNull())
  {
    QFont font = this->font();
    font.fromString(axis->property(PKDefaultFont).toString());
    axis->setLabelFont(font);
    axis->setTickLabelFont(font);
  }
}

void GraphicsPlotExtended::SaveCurves()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());

  for (int i = 0; i < curveCount(); ++i)
  {
    auto ptr = curve(i);
    if (qobject_cast<GPInteractiveCurve*>(ptr))
      return;
    SaveObjectSettings(projectId, ptr);
  }
}

void GraphicsPlotExtended::LoadCurves()
{
  for (int i = 0; i < curveCount(); ++i)
    LoadCurve(curve(i));
}

void GraphicsPlotExtended::LoadCurve(GPCurve *curve)
{
  if (qobject_cast<GPInteractiveCurve*>(curve))
    return;
  if (!property(OKProjectId).isNull())
  {
    QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
    if (LoadObjectSettings(projectId, curve))
      return;
  }
  LoadCurveDefault(curve);
}

void GraphicsPlotExtended::LoadCurveDefault(GPCurve *curve)
{
  GPScatterStyle scatter;
  scatter.setShape(GPScatterStyle::ssNone);
  scatter.setSize(5);
  if (!curve->property(PKDefaultScatterShape).isNull())
    scatter.setShape((GPScatterStyle::ScatterShape)curve->property(PKDefaultScatterShape).toInt());
  if (!curve->property(PKDefaultScatterSize).isNull())
    scatter.setSize(curve->property(PKDefaultScatterSize).toDouble());
  if (!curve->property(PKDefaultScatterColor).isNull()
   && !curve->property(PKDefaultScatterPenWidth).isNull())
    scatter.setPen(QPen(QColor(curve->property(PKDefaultScatterColor).toString()), curve->property(PKDefaultScatterPenWidth).toDouble()));
  else if (!curve->property(PKDefaultScatterColor).isNull())
    scatter.setPen(QColor(curve->property(PKDefaultScatterColor).toString()));
  if (!curve->property(PKDefaultScatterBrush).isNull())
    scatter.setBrush(QBrush(QColor(curve->property(PKDefaultScatterBrush).toString())));

  QPen pen(curve->pen().color(), Style.DefaultPenWidth, Qt::SolidLine);
  if (!curve->property(PKDefaultPenStyle).isNull())
    pen.setStyle((Qt::PenStyle)curve->property(PKDefaultPenStyle).toInt());
  if (!curve->property(PKDefaultPenWidth).isNull())
    pen.setWidthF(curve->property(PKDefaultPenWidth).toDouble());

  QBrush brush;
  if (!curve->property(PKDefaultBrushStyle).isNull())
    brush.setStyle((Qt::BrushStyle)curve->property(PKDefaultBrushStyle).toInt());
  if (!curve->property(PKDefaultBrushColor).isNull())
    brush.setColor(QColor(curve->property(PKDefaultBrushColor).toString()));

  QString key = curve->property(OKCurveId).toString();
  if (!key.isEmpty())
  {
    if (curve->property(PKDefaultColor).isNull())
      pen.setColor(Style.GetDefaultColorMap().value(abs(key.toInt()) % Style.GetDefaultColorMap().size()));
    else if (!curve->property(PKDefaultColor).isNull())
      pen.setColor(QColor(curve->property(PKDefaultColor).toString()));
  }
  else
  {
    if (!curve->property(PKDefaultColor).isNull())
      pen.setColor(QColor(curve->property(PKDefaultColor).toString()));
    else
    {
      int index = mCurves.indexOf(curve);
      if (index != -1)
        pen.setColor(Style.GetDefaultColorMap().value(abs(index) % Style.GetDefaultColorMap().size()));
    }
  }
  if (!curve->property(PKDefaultVisibility).isNull())
    curve->setVisible(curve->property(PKDefaultVisibility).toBool());

  curve->setPen(pen);
  if (auto decorator = curve->selectionDecorator())
  {
    QPen selectionPen = pen;
    selectionPen.setColor(pen.color().lighter(120));
    selectionPen.setWidthF(pen.widthF() + 1.5);
    decorator->setPen(selectionPen);
  }
  curve->setBrush(brush);
  curve->setScatterStyle(scatter);
}

void GraphicsPlotExtended::SaveLegends()
{
  if (property(OKProjectId).isNull() || axisRects().isEmpty())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());

  auto inset = axisRect()->insetLayout();
  for (int i = 0; i < inset->elementCount(); ++i)
    if (auto legend = qobject_cast<GPLegend*>(inset->elementAt(i)))
      SaveObjectSettings(projectId, legend);
}

void GraphicsPlotExtended::LoadLegends()
{
  if (!axisRects().isEmpty())
  {
    if (auto inset = axisRect()->insetLayout())
    {
      for (int i = 0; i < inset->elementCount(); ++i)
      {
        if (auto legend = qobject_cast<GPLegend*>(inset->elementAt(i)))
        {
          LoadLegend(legend);
        }
      }
    }
  }
}

void GraphicsPlotExtended::LoadLegend(GPLegend *legend)
{
  if (!property(OKProjectId).isNull())
  {
    QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
    if (LoadObjectSettings(projectId, legend))
      return;
  }
  LoadLegendDefault(legend);
}

void GraphicsPlotExtended::LoadLegendDefault(GPLegend *legend)
{
  QFont font = this->font();
  if (!legend->property(PKDefaultFont).isNull())
    font.fromString(legend->property(PKDefaultFont).toString());

  legend->setFont(font);
  legend->setTextColor(QColor(legend->property(PKDefaultColor).toString()));
}

void GraphicsPlotExtended::SaveTitle()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());

  SaveObjectSettings(projectId, Title);
}

void GraphicsPlotExtended::LoadTitle()
{
  if (!property(OKProjectId).isNull())
  {
    QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
    if (LoadObjectSettings(projectId, Title))
      return;
  }
  LoadTitleDefault();
}

void GraphicsPlotExtended::LoadTitleDefault()
{
  QFont font = this->font();
  if (!Title->property(PKDefaultFont).isNull())
    font.fromString(Title->property(PKDefaultFont).toString());

  if (Title->property(PKDefaultText).isNull())
    setTitleText(Title->property(PKDefaultText).toString());

  Title->setFont(font);
  Title->setTextColor(QColor(Title->property(PKDefaultColor).toString()));
}

void GraphicsPlotExtended::SaveRulers()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
  QString key = GetObjectKey(this);

  //// Save positions
  QVariantList rulersPos;
  for (const auto& ruler : LineRulers)
  {
    if (ruler)
    {
      rulersPos.append(ruler->getValue());
    }
  }
  SetSettings(projectId, key + "/objects/rulers", rulersPos);

  //// Save uom
  QVariantList rulersUom;
  for (const auto& ruler : LineRulers)
  {
    if (ruler)
    {
      QVariantMap kruom;
      ruler->start->listenerKeys.uomDescriptor.save(kruom);

      QVariantMap vruom;
      ruler->start->listenerValues.uomDescriptor.save(vruom);

      QVariantMap ruom;
      ruom[JKUomKeys]   = kruom;
      ruom[JKUomValues] = vruom;

      rulersUom << ruom;
    }
  }
  SetSettings(projectId, key + "/objects/rulers_uom", rulersUom);

  if (DistanceRuler && DistanceRuler->data()->size() == 2)
  {
    //// Save positions
    {
      QVariantMap rulerSettings;
      rulerSettings["x1"] = DistanceRuler->data()->at(0)->key;
      rulerSettings["x2"] = DistanceRuler->data()->at(1)->key;
      rulerSettings["y1"] = DistanceRuler->data()->at(0)->value;
      rulerSettings["y2"] = DistanceRuler->data()->at(1)->value;

      SetSettings(projectId, key + "/objects/distance_ruler", rulersPos);
    }

    //// Save uom
    {
      QVariantMap kruom;
      DistanceRuler->data()->listenerKeys.uomDescriptor.save(kruom);

      QVariantMap vruom;
      DistanceRuler->data()->listenerValues.uomDescriptor.save(vruom);

      QVariantMap ruom;
      ruom[JKUomKeys]   = kruom;
      ruom[JKUomValues] = vruom;

      SetSettings(projectId, key + "/objects/distance_ruler_uom", rulersPos);
    }
  }
}

void GraphicsPlotExtended::LoadRulers()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
  QString key = GetObjectKey(this);

  for (const auto& ruler : LineRulers)
    removeItem(ruler);
  LineRulers.clear();

  auto settings    = GetSettings(projectId, key + "/objects/rulers",     QVariant()).toList();
  auto settingsUom = GetSettings(projectId, key + "/objects/rulers_uom", QVariant()).toList();

  while (settingsUom.size() < settings.size())
  {
    settingsUom << QVariantMap();
  }

  if (!settings.isEmpty())
  {
    for (auto pos = settings.begin(); pos != settings.end(); ++pos)
    {
      //// Create
      GPRulerLine* ruler = new GPRulerLine(this,
                                             (KeyAxisOrientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal,
                                             0);

      //// Save input uom
      GPUomDescriptor kuom = ruler->start->mInputUomForKeys;
      GPUomDescriptor vuom = ruler->start->mInputUomForValues;

      //// Load stored uom
      GPUomDescriptor kiuom;
      kiuom.load(settingsUom.at(std::distance(settings.begin(), pos)).toMap()[JKUomKeys].toMap());
      if (!kiuom.isNull())
        ruler->start->setInputUomForKeys(kiuom);

      GPUomDescriptor viuom;
      viuom.load(settingsUom.at(std::distance(settings.begin(), pos)).toMap()[JKUomValues].toMap());
      if (!viuom.isNull())
        ruler->start->setInputUomForValues(viuom);

      //// Set
      double k = pos->toDouble();
      ruler->setValue(k);

      LineRulers.append(QPointer<GPRulerLine>(ruler));
    }
    CurvesUpdated();
  }

  auto distanceSettings = GetSettings(projectId, key + "/objects/distance_ruler", QVariant()).toMap();
  if (!distanceSettings.isEmpty())
  {
    if (!DistanceRuler)
    {
      DistanceRuler = new GPRulerDistance(xAxis, yAxis, QPoint(0, 0));
    }

    //// Clear
    DistanceRuler->data()->clear();

    //// Save input uom
    GPUomDescriptor kuom = DistanceRuler->data()->mInputUomForKeys;
    GPUomDescriptor vuom = DistanceRuler->data()->mInputUomForValues;

    //// Load stored uom
    auto distanceSettingsUom = GetSettings(projectId, key + "/objects/distance_ruler_uom", QVariant()).toMap();

    GPUomDescriptor kiuom;
    kiuom.load(distanceSettingsUom[JKUomKeys].toMap());
    if (!kiuom.isNull())
      DistanceRuler->data()->setInputUomForKeys(kiuom);

    GPUomDescriptor viuom;
    viuom.load(distanceSettingsUom[JKUomValues].toMap());
    if (!viuom.isNull())
      DistanceRuler->data()->setInputUomForValues(viuom);

    //// Load uom dependent data
    double x1 = distanceSettings["x1"].toDouble();
    double y1 = distanceSettings["y1"].toDouble();
    double x2 = distanceSettings["x2"].toDouble();
    double y2 = distanceSettings["y2"].toDouble();

    //// Set
    DistanceRuler->addData(x1, y1);
    DistanceRuler->addData(x2, y2);

    //// Restore input uom
    DistanceRuler->data()->mInputUomForKeys   = kuom;
    DistanceRuler->data()->mInputUomForValues = vuom;
  }
}

void GraphicsPlotExtended::SaveCaptions()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
  QString key = GetObjectKey(this);

  QVariantMap captions = GetSettings(projectId, key + "/objects/captions", QVariant()).toMap();
  QVariantMap newCaptions;
  for (auto it = Captions.begin(); it != Captions.end(); ++it)
  {
    QString id = it.key().toString();
    QVariantMap settings;
    if (!it.value().isNull())
      settings = GPSettings::getSettings(it.value());
    else
      settings = captions[id].toMap();
    newCaptions[id] = settings;
  }
  SetSettings(projectId, key + "/objects/captions", newCaptions);
}

void GraphicsPlotExtended::LoadCaptions()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
  QString key = GetObjectKey(this);

  qDeleteAll(Captions);
  Captions.clear();

  auto settings = GetSettings(projectId, key + "/objects/captions", QVariant()).toMap();
  if (!settings.isEmpty())
  {
    for (const auto& k : settings.keys())
    {
      QVariantMap settingsMap = settings[k].toMap();
      auto caption = new GPCaptionItem(this);
      caption->setFont(Style.CaptionsFont);
      caption->setTextColor(Style.CaptionsColor);
      GPSettings::setSettings(caption, settingsMap);
      Captions.insert(caption->getId(), caption);
    }
  }
}

void GraphicsPlotExtended::SaveZones()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
  QString key = GetObjectKey(this);

  QVariantMap settings;
  if (ZonesRect)
  {
    settings = ZonesRect->toVariantMap();
  }
  else
  {
    settings = GetSettings(projectId, key + "/objects/zones", QVariant()).toMap();
    if (!settings.isEmpty())
      settings["visible"] = false;
  }

  SetSettings(projectId, key + "/objects/zones", settings);
}

void GraphicsPlotExtended::LoadZones()
{
  if (property(OKProjectId).isNull())
    return;
  QUuid projectId = QUuid::fromString(property(OKProjectId).toString());
  QString key = GetObjectKey(this);

  auto settings = GetSettings(projectId, key + "/objects/zones", QVariant()).toMap();
  if (!settings.isEmpty())
  {
    if (!ZonesRect)
      addZonesRect();
    ZonesRect->fromVariantMap(settings);
  }
}

void GraphicsPlotExtended::LoadObjects()
{
  LoadRulers();
  LoadCaptions();
  LoadZones();
}

void GraphicsPlotExtended::SaveObjects()
{
  SaveRulers();
  SaveCaptions();
  SaveZones();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsPlotExtended::setDefaultSettings(QObject *obj, const QColor &color, const QFont &font)
{
  setDefaultSettings(obj, color);
  setDefaultSettings(obj, font);
}

void GraphicsPlotExtended::setDefaultSettings(QObject *obj, const QColor &color)
{
  obj->setProperty(PKDefaultColor, color.name());
}

void GraphicsPlotExtended::setDefaultSettings(QObject *obj, const QFont &font)
{
  obj->setProperty(PKDefaultFont, font.toString());
}

void GraphicsPlotExtended::setDefaultSettings(GPCurve *curve, const QPen &pen, const QBrush &brush, const GPScatterStyle &scatter)
{
  setDefaultSettings(curve, pen);
  setDefaultSettings(curve, brush);
  setDefaultSettings(curve, scatter);
}

void GraphicsPlotExtended::setDefaultSettings(GPCurve *curve, const QPen &pen)
{
  curve->setProperty(PKDefaultPenStyle, (int)pen.style());
  curve->setProperty(PKDefaultColor, pen.color().name(QColor::HexArgb));
  curve->setProperty(PKDefaultPenWidth, pen.widthF());
}

void GraphicsPlotExtended::setDefaultSettings(GPCurve *curve, const QBrush &brush)
{
  curve->setProperty(PKDefaultBrushColor, brush.color());
  curve->setProperty(PKDefaultBrushStyle, (int)brush.style());
}

void GraphicsPlotExtended::setDefaultSettings(GPCurve *curve, const GPScatterStyle &scatter)
{
  curve->setProperty(PKDefaultScatterShape, (int)scatter.shape());
  curve->setProperty(PKDefaultScatterSize, scatter.size());
  curve->setProperty(PKDefaultScatterColor, scatter.pen().color());
  curve->setProperty(PKDefaultScatterBrush, scatter.pen().color());
}

void GraphicsPlotExtended::setDefaultSettings(GPTextElement *textElem, const QString &text)
{
  textElem->setProperty(PKDefaultText, text);
}

QString GraphicsPlotExtended::GetObjectKey(QObject *obj)
{
  if (property(OKPlotId).isNull() || !obj)
    return QString();
  QString key = property(OKPlotId).toString();

  if (!property(OKDataId).isNull())
    key += "/" + property(OKDataId).toString();

  if (!obj->property(OKCurveId).isNull())
    return key + "/curves/" + obj->property(OKCurveId).toString();
  if (!obj->property(OKAxisId).isNull())
    return key + "/axes/" + obj->property(OKAxisId).toString();
  if (!obj->property(OKLegendId).isNull())
    return key + "/legends/" + obj->property(OKLegendId).toString();
  if (legend == obj)
    return key + "/legends/default";
  if (Title == obj)
    return key + "/text_items/title";
  if (this == obj)
    return key;

  return QString();
}

bool GraphicsPlotExtended::LoadObjectSettings(const QUuid& projectId, QObject *obj)
{
  QString key = GetObjectKey(obj);
  if (!key.isEmpty())
  {
    auto settings = GetSettings(projectId, key, QVariant()).toMap();
    if (!settings.isEmpty())
    {
      GPSettings::setSettings(obj, settings);
      setTitleText(Title->text());

      return true;
    }
  }
  return false;
}

void GraphicsPlotExtended::SaveObjectSettings(const QUuid& projectId, QObject *obj)
{
  QString key = GetObjectKey(obj);
  if (!key.isEmpty())
    SetSettings(projectId, key, GPSettings::getSettings(obj));
}

#if defined(USE_XLNT)
void GraphicsPlotExtended::ExportToExcel()
{
  // Path
  QString path = QFileDialog::getSaveFileName(this, tr("Export data to excel file"), "", "Excel files (*.xlsx)");
  if (path.isEmpty())
    return;

  try
  {
    xlnt::workbook workbook;
    xlnt::worksheet sheet = workbook.active_sheet();
    // workbook.remove_sheet(sheet);

    // Collect data
    std::map<std::pair<GPAxis* /*key_axis*/, std::pair<double /*data_container.key.k*/, double /*data_container.key.t*/>>,
             std::map<std::pair<std::pair<int /*order*/, GPAbstractPlottable*>, GPAxis* /*value_axis*/>,
                      double /*data_container.value*/>> exportedData;

    std::map<std::pair<GPAxis* /*key_axis*/, std::pair<double /*data_container.key.k*/, double /*data_container.key.t*/>>,
             std::map<GPAxis* /*key_axis*/, double /*data_container.key.k*/>> exportedKeys;

    std::set<std::pair<GPAxis*, GPAbstractPlottable*>> keyAxis;
    std::set<std::pair<std::pair<int /*order*/, GPAbstractPlottable*>, GPAxis*>> plottableWithAxisValuesSet;
    std::set<GPAbstractPlottable*> plottableSet;

    /////////////////////////////////////////////////////////////////////////
    // Create plottables global key values
    std::set<std::pair<double/*t*/, double/*k*/>> keyValuesGlobal;
    for (auto graph = mGraphs.begin(); graph != mGraphs.end(); ++graph)
    {
      if ((*graph)->property("NoExport").toBool())
      {
        continue;
      }
      auto data = (*graph)->data();
      if (data->isEmpty() && (*graph)->name().isEmpty())
      {
        continue;
      }
      for (auto d = data->begin(); d != data->end(); ++d)
      {
        keyValuesGlobal.insert(std::make_pair(d->key, d->key));
      }
    }
    for (auto curve = mCurves.begin(); curve != mCurves.end(); ++curve)
    {
      if ((*curve)->property("NoExport").toBool())
      {
        continue;
      }
      auto data = (*curve)->data();
      if (data->isEmpty() && (*curve)->name().isEmpty())
      {
        continue;
      }
      for (auto d = data->begin(); d != data->end(); ++d)
      {
        keyValuesGlobal.insert(std::make_pair(d->t, d->key));
      }
    }


    /////////////////////////////////////////////////////////////////////////
    // Graphs
    int plottableOrder = 0;
    for (auto graph = mGraphs.begin(); graph != mGraphs.end(); ++graph)
    {
      if ((*graph)->property("NoExport").toBool())
      {
        continue;
      }

      auto data = (*graph)->data();
      if (data->isEmpty() && (*graph)->name().isEmpty())
      {
        continue;
      }

      auto ka = (*graph)->keyAxis();
      auto va = (*graph)->valueAxis();

      keyAxis.insert(std::make_pair(ka, (*graph)->property("ExportUngroupped").toBool() ? *graph : (GPAbstractPlottable*)0/**graph*/)); // 1d plottables are groupped

      for (auto d = keyValuesGlobal.begin(); d != keyValuesGlobal.end(); ++d)
      {
        double val = GraphicsPlotExtendedUtils::interpolate(data, d->second, true);

        std::pair<double, double> tk = std::make_pair(d->first, d->second);
        std::pair<GPAxis*, std::pair<double, double>> t = std::make_pair(ka, tk);

        exportedKeys[t][ka] = d->second;

        if (plottableSet.find(*graph) == plottableSet.end())
        {
          ++plottableOrder;
          plottableSet.insert(*graph);
        }

        std::pair<std::pair<int, GPAbstractPlottable*>, GPAxis*> plottableWithAxisValues = std::make_pair(std::make_pair(plottableOrder, *graph), va);
        plottableWithAxisValuesSet.insert(plottableWithAxisValues);
        exportedData[t][plottableWithAxisValues] = val;
      }
    }

    /////////////////////////////////////////////////////////////////////////
    // Curves
    for (auto curve = mCurves.begin(); curve != mCurves.end(); ++curve)
    {
      if ((*curve)->property("NoExport").toBool())
      {
        continue;
      }

      auto data = (*curve)->data();
      if (data->isEmpty() && (*curve)->name().isEmpty())
      {
        continue;
      }

      auto ka = (*curve)->keyAxis();
      auto va = (*curve)->valueAxis();

      keyAxis.insert(std::make_pair(ka, (*curve)->property("ExportUngroupped").toBool() ? *curve : (GPAbstractPlottable*)0/**curve*/)); // 1d plottables are groupped

      //// Sorted curve data
      std::map<std::pair<double, double>, double> sortedData;
      {
        for (auto d = data->begin(); d != data->end(); ++d)
        {
          std::pair<double, double> tk = std::make_pair(d->t, d->key);
          sortedData[tk] = d->value;
        }
      }

      for (auto d = keyValuesGlobal.begin(); d != keyValuesGlobal.end(); ++d)
      {
        double val = qQNaN();
        auto v = sortedData.find(std::make_pair(d->first, d->second));
        if (v != sortedData.end())
        {
          val = v->second;
        }
        else
        {
          val = GraphicsPlotExtendedUtils::interpolate(data, d->second, true);
        }

        std::pair<double, double> tk = std::make_pair(d->first, d->second);
        std::pair<GPAxis*, std::pair<double, double>> t = std::make_pair(ka, tk);

        exportedKeys[t][ka] = d->second;

        if (plottableSet.find(*curve) == plottableSet.end())
        {
          ++plottableOrder;
          plottableSet.insert(*curve);
        }

        std::pair<std::pair<int, GPAbstractPlottable*>, GPAxis*> plottableWithAxisValues = std::make_pair(std::make_pair(plottableOrder, *curve), va);
        plottableWithAxisValuesSet.insert(plottableWithAxisValues);
        exportedData[t][plottableWithAxisValues] = val;
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Color maps
    {
      QList<GPColorMap*> colorMaps;
      for (const auto& p : mPlottables)
      {
        if (GPColorMap* cm = qobject_cast<GPColorMap*>(p))
        {
          colorMaps << cm;
        }
      }

      for (const auto& colorMap : colorMaps)
      {
        if (colorMap->property("NoExport").toBool())
        {
          continue;
        }

        auto data = colorMap->data();

        auto ka   = colorMap->keyAxis();
        auto va   = colorMap->valueAxis();

        auto keyRange = data->keyRange();
        auto valRange = data->valueRange();

        int keySize = data->keySize();
        int valSize = data->valueSize();

        const bool transposed = colorMap->property("ExportTransposed").toBool();
        if (transposed)
        {
          std::swap(ka,       va);
          std::swap(keyRange, valRange);
          std::swap(keySize,  valSize);
        }

        if (keySize && valSize)
        {
          keyAxis.insert(std::make_pair(ka, colorMap));

          double cellKeySize = keyRange.size() / keySize;
          double cellValSize = valRange.size() / valSize;

          for (int ki = 0; ki < keySize; ki++)
          {
            for (int vi = 0; vi < valSize; vi++)
            {
              double k = keyRange.lower + keyRange.size() * (double)ki / (double)keySize + cellKeySize / 2.0;
              double v = valRange.lower + valRange.size() * (double)vi / (double)valSize + cellValSize / 2.0;
              double c = transposed ? data->cell(vi, ki) : data->cell(ki, vi);

              std::pair<double, double> tk = std::make_pair(k, v);
              std::pair<GPAxis*, std::pair<double, double>> t = std::make_pair(ka, tk);

              exportedKeys[t][ka] = k;

              if (plottableSet.find(colorMap) == plottableSet.end())
              {
                ++plottableOrder;
                plottableSet.insert(colorMap);
              }

              std::pair<std::pair<int, GPAbstractPlottable*>, GPAxis*> plottableWithAxisValues = std::make_pair(std::make_pair(plottableOrder, colorMap), va);
              plottableWithAxisValuesSet.insert(plottableWithAxisValues);
              exportedData[t][plottableWithAxisValues] = c;
            }
          }
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Go through key axis
    int columnIndex = 1;
    for (auto ka = keyAxis.begin(); ka != keyAxis.end(); ++ka)
    {
      // New worksheet
      // xlnt::worksheet sheet = workbook.create_sheet();

      auto ticker = (*ka).first->ticker();
      GPAxisAwareTicker* awareTicker = qobject_cast<GPAxisAwareTicker*>(ticker.data());

      // Collect objects & keys
      std::set<std::pair<std::pair<int, GPAbstractPlottable*>, GPAxis*>> plottableWithAxisValuesSetForKeyAxis;
      std::set<std::pair<double, double>> ts;
      {
        for (auto exported = exportedData.begin(); exported != exportedData.end(); ++exported)
        {
          if (exported->first.first == (*ka).first)
          {
            // Key
            std::pair<double, double> keyT = exported->first.second;
            ts.insert(keyT);

            // Objects
            for (auto k = exported->second.begin(); k != exported->second.end(); ++k)
            {
              int                   keyPlottableOrder = k->first.first.first;
              GPAbstractPlottable* keyPlottable      = k->first.first.second;
              GPAxis*              keyPlottableAx    = k->first.second;

              std::pair<std::pair<int, GPAbstractPlottable*>, GPAxis*> keyPlottableWithAxis = std::make_pair(std::make_pair(keyPlottableOrder, keyPlottable), keyPlottableAx);

              plottableWithAxisValuesSetForKeyAxis.insert(keyPlottableWithAxis);
            }
          }
        }
      }

      // Bold
      auto boldFont = xlnt::font().bold(true);
      int headerColumnIndex = columnIndex;

      // Header (key)
      {
        xlnt::cell_reference cellReferenceCaptionAxis(xlnt::column_t::column_string_from_index(headerColumnIndex), xlnt::row_t(1));
        xlnt::cell cellCaptionAxis = sheet.cell(cellReferenceCaptionAxis);
        cellCaptionAxis.value(tr("Axis").toStdString());
        cellCaptionAxis.font(boldFont);

        xlnt::cell_reference cellReferenceCaption(xlnt::column_t::column_string_from_index(headerColumnIndex), xlnt::row_t(2));
        xlnt::cell cellCaption = sheet.cell(cellReferenceCaption);
        cellCaption.value((*ka).first->label().toStdString());
        cellCaption.font(boldFont);

        ++headerColumnIndex;
      }

      ////////////////////////////////////////////////////////////////////
      // Header (labels)
      {
        int ci = headerColumnIndex;
        for (auto exportedColumnKey = plottableWithAxisValuesSetForKeyAxis.begin(); exportedColumnKey != plottableWithAxisValuesSetForKeyAxis.end(); ++exportedColumnKey)
        {
          xlnt::cell_reference cellReferencePlottableCaption(xlnt::column_t::column_string_from_index(ci), xlnt::row_t(1));
          xlnt::cell cellPlottableCaption = sheet.cell(cellReferencePlottableCaption);

          QTextDocumentFragment html = QTextDocumentFragment::fromHtml(exportedColumnKey->first.second->name());
          QString plottableCaption = html.toPlainText();
          cellPlottableCaption.value(plottableCaption.toStdString());
          cellPlottableCaption.font(boldFont);

          xlnt::cell_reference cellReferencePlottableAxisCaption(xlnt::column_t::column_string_from_index(ci), xlnt::row_t(2));
          xlnt::cell cellPlottableAxisCaption = sheet.cell(cellReferencePlottableAxisCaption);

          QTextDocumentFragment htmlLabel = QTextDocumentFragment::fromHtml(exportedColumnKey->second->label());
          cellPlottableAxisCaption.value(htmlLabel.toPlainText().toStdString());
          cellPlottableAxisCaption.font(boldFont);

          ++ci;
        }
      }

      ////////////////////////////////////////////////////////////////////
      // Keys (values)
      int row = 4;
      {
        int keyColumnIndex = columnIndex;
        int r = row;

        ////////////////////////////////////////////////////////////////////
        // If plottable is distinguished
        // - assume it's 2d plottable
        // - treat data container T subkey as dimension
        // - shrink keys
        decltype(ts) tsKeys = ts;
        if (ka->second)
        {
          std::map<double, double> tsFirsts;
          for (const auto& t : ts)
          {
            auto f = tsFirsts.find(t.first);
            if (f == tsFirsts.end())
            {
              tsFirsts[t.first] = t.second;
            }
          }

          tsKeys.clear();
          for (const auto& t : ts)
          {
            tsKeys.insert(std::make_pair(t.first, tsFirsts[t.first]));
          }
        }

        for (auto t = tsKeys.begin(); t != tsKeys.end(); ++t)
        {
          std::pair<GPAxis*, std::pair<double, double>> key = std::make_pair((*ka).first, *t);

          auto keyValue = exportedKeys.find(key);
          if (keyValue != exportedKeys.end())
          {
            xlnt::cell_reference keyValueCellReference(xlnt::column_t::column_string_from_index(keyColumnIndex), xlnt::row_t(r));
            xlnt::cell keyValueCell = sheet.cell(keyValueCellReference);

            auto kv = keyValue->second.find((*ka).first);
            if (kv != keyValue->second.end())
            {
              if (awareTicker && awareTicker->isFormatedTime())
              {
                QDateTime dt = QDateTime::fromMSecsSinceEpoch(kv->second);
                QDate date = dt.date();
                QTime time = dt.time();
                xlnt::datetime xldt(date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second(), time.msec() * 1000);
                keyValueCell.value(xldt);
              }
              else
              {
                if (awareTicker)
                  keyValueCell.value(awareTicker->toFormatedKey(kv->second).toDouble());
                else
                  keyValueCell.value(kv->second);
              }
            }
            ++r;
          }
        }
        ++columnIndex;
      }

      ////////////////////////////////////////////////////////////////////
      // Values
      {
        for (auto valueKey = plottableWithAxisValuesSetForKeyAxis.begin(); valueKey != plottableWithAxisValuesSetForKeyAxis.end(); ++valueKey)
        {
          int                     keyPlottableOrder = valueKey->first.first;
          GPAbstractPlottable*   keyPlottable      = valueKey->first.second;
          GPAxis*                keyPlottableAx    = valueKey->second;

          //// 2d data has all another layout
          bool multiColumn = false;
          if (GPColorMap* map = qobject_cast<GPColorMap*>(keyPlottable))
            multiColumn = true;

          //// Fill
          {
            int r = row;
            int c = columnIndex;

            QMap<double, int> tsSecondaryToColumnIndex;

            for (auto t = ts.begin(); t != ts.end(); ++t)
            {
              if (multiColumn)
              {
                double secondary = t->second;
                if (tsSecondaryToColumnIndex.empty())
                {
                  tsSecondaryToColumnIndex[secondary] = c;
                }
                else if (tsSecondaryToColumnIndex.contains(secondary))
                {
                  c = tsSecondaryToColumnIndex.value(secondary);
                  if (c == columnIndex)
                  {
                    ++r;
                  }
                }
                else
                {
                  ++c;
                  tsSecondaryToColumnIndex[secondary] = c;
                }
              }
              else //// single column
              {
                if (t != ts.begin())
                {
                  ++r;
                }
              }

              //// Also, prepend subheader
              if (multiColumn && r == row)
              {
                xlnt::cell_reference valueCellReference(xlnt::column_t::column_string_from_index(c), xlnt::row_t(r - 1));
                xlnt::cell valueCell = sheet.cell(valueCellReference);
                valueCell.value(t->second);
              }

              //// Fill the value
              std::pair<GPAxis*, std::pair<double, double>> tk = std::make_pair((*ka).first, *t);
              auto exporttedMap = exportedData.find(tk);
              if (exporttedMap != exportedData.end())
              {
                std::pair<std::pair<int, GPAbstractPlottable*>, GPAxis*> keyPlottableWithAxis = std::make_pair(std::make_pair(keyPlottableOrder, keyPlottable), keyPlottableAx);
                auto value = exporttedMap->second.find(keyPlottableWithAxis);
                if (value != exporttedMap->second.end())
                {
                  if (!qIsNaN(value->second))
                  {
                    xlnt::cell_reference valueCellReference(xlnt::column_t::column_string_from_index(c), xlnt::row_t(r));
                    xlnt::cell valueCell = sheet.cell(valueCellReference);

                    valueCell.value(value->second);
                  }
                }
              }
            }

            columnIndex = c + 1;
          }
        }

        // Skip column for another set of keys
        ++columnIndex;
      }
    }
    workbook.save(path.toStdWString());
  }
  catch (const std::runtime_error& exception)
  {
    Q_UNUSED(exception)
  }
}
#endif

void GraphicsPlotExtended::SaveImage()
{
#ifdef Q_OS_WASM
  QBuffer buffer;
  savePng(&buffer);
  QFileDialog::saveFileContent(buffer.data(), "plot.png");
#else
  QString path = QFileDialog::getSaveFileName(this, tr("Set save path"), "", "png (*.png);;pdf (*.pdf)");
  if (!path.isEmpty())
  {
    path = QDir::toNativeSeparators(path);
    if(path.endsWith(".png", Qt::CaseInsensitive))
      savePng(path);
    else if(path.endsWith(".pdf", Qt::CaseInsensitive))
      savePdf(path);
    else
    {
      path += ".png";
      savePng(path);
    }
  }
#endif
}

void GraphicsPlotExtended::CopyImage()
{
  QImage buffer = toPixmap(0, 0, 1.0).toImage();

  int resolution = 96;
  int dotsPerMeter = resolution/0.0254;
  buffer.setDotsPerMeterX(dotsPerMeter);
  buffer.setDotsPerMeterY(dotsPerMeter);
  QApplication::clipboard()->setImage(buffer, QClipboard::Clipboard);
}

void GraphicsPlotExtended::ShowSettingsDialog()
{
  QWidget* w = window();
  PlotSettingsDialog* dialog = new PlotSettingsDialog(this, w, PlotSettingsFlags);
#if defined(USE_CUSTOM_WINDOWS)
  CustomWindows::ShowNewDialog(w, dialog);
#else
  dialog->exec();
  delete dialog;
#endif
  SaveSettings();
}

void GraphicsPlotExtended::SetAxisInteraction(GPAxis* axis, GPAxisRect *rect, QFlags<GP::Interaction> iteraction)
{
  if (rect)
  {
    AxisInteraction[axis] = iteraction;

    bool z = (iteraction & GP::iRangeZoom);
    bool d = (iteraction & GP::iRangeDrag);
    int id = (int)axis->orientation() - 1;

    // Set explicitly zoomable
    QList<GPAxis*> rzas[2];
    rzas[0] = rect->rangeZoomAxes(Qt::Horizontal);
    rzas[1] = rect->rangeZoomAxes(Qt::Vertical);
    if (z)
    {
      if (!rzas[id].contains(axis))
      {
        rzas[id].append(axis);
      }
    }
    else
    {
      rzas[id].removeAll(axis);
    }
    rect->setRangeZoomAxes(rzas[0], rzas[1]);

    // Set explicitly draggable
    QList<GPAxis*> rdas[2];
    rdas[0] = rect->rangeDragAxes(Qt::Horizontal);
    rdas[1] = rect->rangeDragAxes(Qt::Vertical);
    if (d)
    {
      if (!rdas[id].contains(axis))
      {
        rdas[id].append(axis);
      }
    }
    else
    {
      rdas[id].removeAll(axis);
    }
    rect->setRangeDragAxes(rdas[0], rdas[1]);
  }
}

QFlags<GP::Interaction> GraphicsPlotExtended::GetAxisInteraction(GPAxis* axis)
{
  return AxisInteraction.value(axis, (GP::Interaction)(int)interactions());
}

void GraphicsPlotExtended::fastClearPlottables()
{
  decltype(mPlottables)::iterator plottable = mPlottables.begin();
  while (plottable != mPlottables.end())
  {
    delete *plottable;
    ++plottable;
  }
  mPlottables.clear();
  mGraphs.clear();
  mCurves.clear();
}

GPPolygonGroup* GraphicsPlotExtended::addPolygonGroup(GPAxis* keyAxis, GPAxis* valueAxis)
{
  if (!PolygonLayer)
  {
    PolygonLayer = new GPPolygonLayer(this, QLatin1String("polygons"));
    mLayers.insert(mLayers.size() - 1, PolygonLayer);
  }

  return PolygonLayer->addPolygonGroup(keyAxis, valueAxis);
}

void GraphicsPlotExtended::removePolygonGroup(GPPolygonGroup *group)
{
  if (PolygonLayer)
    PolygonLayer->removePolygonGroup(group);
}

GPZonesRect* GraphicsPlotExtended::addZonesRect(GPAxis* keyAxis)
{
  if (!ZonesRect)
    ZonesRect = new GPZonesRect(this);
  if (keyAxis == nullptr)
  {
    if (KeyAxisOrientation == Qt::Horizontal)
      keyAxis = getDefaultAxisX();
    else
      keyAxis = getDefaultAxisY();
  }
  ZonesRect->setKeyAxis(keyAxis);

  return ZonesRect.data();
}

GPCaptionItem *GraphicsPlotExtended::addCaptionItem(GPLayerable* target, const QString& text, bool hasLine, QPoint pos)
{
  QPointer<GPCaptionItem> item(new GPCaptionItem(target, text, hasLine, pos));
  item->setFont(Style.CaptionsFont);
  item->setTextColor(Style.CaptionsColor);
  Captions.insert(item->getId(), item);
  return item.data();
}

void GraphicsPlotExtended::SetRulerShowAxesLabels(bool show)
{
  RulerShowAxesLabels = show;
}

bool GraphicsPlotExtended::GetRulerShowAxesLabels() const
{
  return RulerShowAxesLabels;
}

void GraphicsPlotExtended::mousePressEvent(QMouseEvent* event)
{
  if (legend && legend->visible() && legend->rect().contains(event->pos()))
    return GraphicsPlot::mousePressEvent(event);

  if ((Flags & GP::AAMagnifier) && !InDrawingMagnifier && (QApplication::keyboardModifiers() == Qt::ShiftModifier))
  {
    mMouseHasMoved = false;
    mMousePressPos = event->pos();
    event->accept();
    SetMagnifierDraw(true);
    return;
  }

  ZoomingAxisMoved = false;

  // Right button zoom
  if (event->buttons() & Qt::RightButton)
  {
    if (InDrawingMeasuringPolygon)
    {
      AddMeasuringPolygonPoint(event->pos());
      event->accept();
      return;
    }

    // Can cange range
    if (interactions().testFlag(GP::iRangeZoom))
    {
      // Clear zooming list
      ZoomingAxis.clear();

      // Collect by hit within axis area
      QList<GPLayerable*> candidates = layerableListAt(event->pos(), false);
      for (const auto& candidate : candidates)
      {
        if (GPAxis* axis = qobject_cast<GPAxis*>(candidate))
        {
          if (IsZoomable(axis))
          {
            ZoomingAxis.append(axis);
          }
        }
      }

      // More than one collected by hit within axis area, pick one
      while (ZoomingAxis.size() > 1)
        ZoomingAxis.removeLast();

      // Plotting area hit, zoom everything
      if (ZoomingAxis.empty())
      {
        auto axisRect = axisRectAt(event->pos());
        ZoomingAxis = GetAllZoomableAxes(axisRect);
      }

      // Save initial properties for all zooming axis
      decltype(ZoomingAxis)::iterator axis = ZoomingAxis.begin();
      while (axis != ZoomingAxis.end())
      {
        if ((*axis)->orientation() == Qt::Horizontal)
        {
          ZoomingAxisStartCenterX[*axis] = (*axis)->pixelToCoord(event->pos().x());
          ZoomingAxisStartRangeX[*axis] = (*axis)->range();
        }
        else
        {
          ZoomingAxisStartCenterY[*axis] = (*axis)->pixelToCoord(event->pos().y());
          ZoomingAxisStartRangeY[*axis] = (*axis)->range();
        }
        ++axis;
      }
      ZoomingAxisStartPoint = event->pos();

    }
  } // ~(event->buttons() & Qt::RightButton)
  else
  {
    // See where hit
    for (const auto& candidate : layerableListAt(event->pos(), false))
    {
      unsigned part;
      auto cursor = candidate->HitTest(event, &part);
      setCursor(cursor);
      if (cursor != Qt::ArrowCursor)
      {
        if (auto di = dynamic_cast<GPEXDraggingInterface*>(candidate))
        {
          DraggingObject = di;
          di->DragStart(event, part);
          break;
        }
      }
    }
  }
  return GraphicsPlot::mousePressEvent(event);
}

void GraphicsPlotExtended::mouseDoubleClickEvent(QMouseEvent *event)
{
  QList<GPLayerable*> candidates = layerableListAt(event->pos(), false);
  for (const auto& candidate : candidates)
  {
    if (GPAxis* axis = qobject_cast<GPAxis*>(candidate))
    {
      if (ContextMenu)
      {
        delete ContextMenu;
      }
      ContextMenu = new QMenu(this);
      ContextMenu->setProperty("Persistent", true);

      auto axisWidget = new QWidget(ContextMenu);
      axisWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
      axisWidget->setContentsMargins(0,0,0,0);

      auto layout = new QVBoxLayout(axisWidget);
      layout->setContentsMargins(0,0,0,0);

      if (auto rangeWidget = CreateAxisRangeActions(axisWidget, axis))
        layout->addWidget(rangeWidget);
      if (auto tickerWidget = CreateAxisTickerActions(axisWidget, axis))
        layout->addWidget(tickerWidget);

      QWidgetAction* wa = new QWidgetAction(GraphicsPlotExtended::ContextMenu);
      wa->setDefaultWidget(axisWidget);
      ContextMenu->addAction(wa);

      ContextMenu->popup(event->globalPos());
      return;
    }
  }

#ifdef Q_OS_WASM // <- looks like wasm cant receive right button clicks, provide menu by double click
  {
    // Global actions
    if (ContextMenu)
    {
      delete ContextMenu;
    }
    ContextMenu = new QMenu(this);
    std::map<QAction*, std::function<void(void)>> actions;

    QContextMenuEvent cme(QContextMenuEvent::Mouse, event->pos(), event->globalPos());
    FillContextMenu(&cme, *ContextMenu, actions);

    if (!actions.empty())
    {
      connect(ContextMenu, &QMenu::triggered, [actions](QAction* action)
        {
          auto a = actions.find(action);
          if (a != actions.end())
          {
            a->second();
          }
        });

      // Cancel zooming axis
      ZoomingAxis.clear();
      ZoomingAxisMoved = false;

      // Show
      ContextMenu->popup(event->globalPos());
      event->accept();

      // Done
      return;
    }
  }
#endif // ~Q_OS_WASM

  return GraphicsPlot::mouseDoubleClickEvent(event);
}

void GraphicsPlotExtended::mouseMoveEvent(QMouseEvent* event)
{
  if (!mMouseHasMoved && (mMousePressPos-event->pos()).manhattanLength() > 3)
    mMouseHasMoved = true;

  // move dragging object
  {
    bool needUpdate = false;
    if (InDrawingMagnifier)
    {
      needUpdate = true;
    }
    else if (DraggingObject)
    {
      DraggingObject->DragMove(event);
      needUpdate = true;
    }

    if (needUpdate)
    {
      replot();
      return;
    }
  }

  if (legend && legend->visible() && legend->rect().contains(event->pos()) && ZoomingAxis.empty())
  {
    setCursor(Qt::ArrowCursor);
    return GraphicsPlot::mouseMoveEvent(event);
  }

  // moving while zooming / dragging axis
  {
    if (!LockAxesInteraction)
    {
      if (!ZoomingAxis.empty())
      {
        ZoomingAxisMoved = true;
        for (auto axis = ZoomingAxis.begin(); axis != ZoomingAxis.end(); ++axis)
        {
          QPoint delta = event->pos() - ZoomingAxisStartPoint;
          double pixelDelta = (*axis)->orientation() == Qt::Horizontal ? delta.x() : -delta.y();
          double pixelRange = (*axis)->orientation() == Qt::Horizontal ? width() : height();
          GPAxisRect* axisRect = (*axis)->axisRect();
          if (axisRect)
          {
            double relativePixelDelta = pixelDelta / pixelRange;
            double factor = (*axis)->scaleType() == GPAxis::stLinear ? pixelRange * qPow(relativePixelDelta, 2) + 1
                                                                      : pixelRange * qPow(relativePixelDelta, 2) / 10 + 1;
            if (pixelDelta > 0)
              factor = 1.0 / factor;

            (*axis)->blockSignals(true);
            if ((*axis)->orientation() == Qt::Horizontal)
            {
              (*axis)->setRange(ZoomingAxisStartRangeX[*axis]);
              (*axis)->blockSignals(false);
              (*axis)->scaleRange(factor, ZoomingAxisStartCenterX[*axis]);
            }
            else
            {
              (*axis)->setRange(ZoomingAxisStartRangeY[*axis]);
              (*axis)->blockSignals(false);
              (*axis)->scaleRange(factor, ZoomingAxisStartCenterY[*axis]);
            }
          }
        }
        replot();
        return;
      }
      if (mMouseEventLayerable)
      {
        // Process move event if captured by axis
        if (GPAxis* axis = qobject_cast<GPAxis*>(mMouseEventLayerable.data()))
        {
          axis->mouseMoveEvent(event, mMousePressPos);
          replot();
          return;
        }
      }
    }
  }

  // set new cursor
  if (event->button() == Qt::NoButton)
  {
    setCursor(GetCursorAt(event->pos()));
  }

  return GraphicsPlot::mouseMoveEvent(event);
}

void GraphicsPlotExtended::mouseReleaseEvent(QMouseEvent* event)
{
  if ((Flags & GP::AAMagnifier) && InDrawingMagnifier)
  {
    SetMagnifierDraw(false);
    event->accept();
    return;
  }

  // end dragging object
  if (DraggingObject)
  {
    DraggingObject->DragAccept(event);
    DraggingObject = nullptr;
  }

  // end zoom axis
  ZoomingAxis.clear();
  ZoomingAxisMoved = false;
  return GraphicsPlot::mouseReleaseEvent(event);
}

// Context menu
void GraphicsPlotExtended::FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions)
{
  menu.installEventFilter(this);

  auto createActionWithShortcut = [](QMenu* menu, QString actionName, int keySequence){
    auto action = new QAction(actionName, menu);
    action->setShortcut(keySequence);
    action->setShortcutContext(Qt::WidgetShortcut);
    action->setShortcutVisibleInContextMenu(true);
    menu->addAction(action);
    return action;
  };

  // Collect by hit within axis area
  GPAxis* axis = nullptr;
  QList<GPLayerable*> candidates = layerableListAt(event->pos(), false);
  for (const auto& candidate : candidates)
  {
    if (GPAxis* a = qobject_cast<GPAxis*>(candidate))
    {
      axis = a;
    }
  }

  if (Flags & GP::AAViewActions)
  {
    // Fill actions
    if (graphCount() + curveCount() > 0)
    {
      QString funcName = ViewAllDefaultFlags & GP::ViewAllKeepOriginalRatio ? tr("Scale to 1:1") : tr("View All");
      auto viewAllAction = createActionWithShortcut(&menu, funcName, GPShortcutKeys::SKViewAll);
      actions[viewAllAction] = [this](){ viewAll(ViewAllDefaultFlags | GP::ViewAllForce | GP::ViewAllUser); };
    }

    if (axis && !(ViewAllDefaultFlags & GP::ViewAllKeepOriginalRatio))
    {
      QString name = axis->label().split(',').value(0);
      if (!name.isEmpty())
      {
        QAction* viewAllAxis = menu.addAction(tr("View All (%1)").arg(name));
        actions[viewAllAxis] = [this, axis](){ viewAll(QSet<GPAxis*>({ axis }), ViewAllDefaultFlags | GP::ViewAllForce | GP::ViewAllUser); };
      }
    }

    {
      QAction* toggleLockAxesAction = menu.addAction(tr("Auto axes"));
      toggleLockAxesAction->setCheckable(true);
      toggleLockAxesAction->setChecked(!LockAxes);
      actions[toggleLockAxesAction] = [this](){ setLockAxes(!LockAxes); };
    }

    {
      QAction* toggleBlockAxesAction = menu.addAction(tr("Interactive axes"));
      toggleBlockAxesAction->setCheckable(true);
      toggleBlockAxesAction->setChecked(!LockAxesInteraction);
      actions[toggleBlockAxesAction] = [this](){ setLockAxesInteraction(!LockAxesInteraction); };
    }
  }

#if defined(USE_XLNT)
  if(Flags & GP::AAExcelExport)
  {
    menu.addSeparator();
    auto exportAction = createActionWithShortcut(&menu, tr("Save to Excel file"), GPShortcutKeys::SKExportExcel);
    actions[exportAction] = std::bind(&GraphicsPlotExtended::ExportToExcel, this);
  }
#endif

  {
    menu.addSeparator();
    QMenu* imageActions = menu.addMenu(tr("Image"));
    actions[imageActions->addAction(tr("Save"))] = std::bind(&GraphicsPlotExtended::SaveImage, this);

    auto copyAction = createActionWithShortcut(imageActions, tr("Copy"), GPShortcutKeys::SKCopyImage);
    actions[copyAction] = std::bind(&GraphicsPlotExtended::CopyImage, this);
  }

  if (Flags & GP::AASettingsEdit)
  {
    menu.addSeparator();
    auto settingsAction = createActionWithShortcut(&menu, tr("Plot settings"), GPShortcutKeys::SKOpenSettings);
    actions[settingsAction] = std::bind(&GraphicsPlotExtended::ShowSettingsDialog, this);
  }

  menu.addSeparator();
  if (Flags & GP::AARulers)
  {
    if (curveCount() > 0)
    {
      QMenu* rulerActions = menu.addMenu(tr("Rulers"));

      actions[rulerActions->addAction(tr("Add"))] = std::bind(&GraphicsPlotExtended::AddRuler, this, event->pos());

      auto remove = rulerActions->addAction(tr("Remove"));
      remove->setEnabled(false);
      for (const auto& ruler : LineRulers)
      {
        if (ruler->selected())
        {
          remove->setEnabled(true);
          break;
        }
      }
      actions[remove] = std::bind(&GraphicsPlotExtended::RemoveRuler, this);

      auto removeAll = rulerActions->addAction(tr("Remove all"));
      removeAll->setEnabled(!LineRulers.isEmpty());
      actions[removeAll] = std::bind(&GraphicsPlotExtended::RemoveRulers, this);

      auto toggleLabel = rulerActions->addAction(tr("Show axes names"));
      toggleLabel->setEnabled(!LineRulers.isEmpty());
      toggleLabel->setCheckable(true);
      toggleLabel->setChecked(RulerShowAxesLabels);
      actions[toggleLabel] = [this](){
        RulerShowAxesLabels = !RulerShowAxesLabels;
        for (const auto& ruler : LineRulers)
          ruler->UpdatePoints();
      };
    }
  }
  else if (Flags & GP::AADistanceRuler)
  {
    QMenu* rulerActions = menu.addMenu(tr("Measure distance"));

    auto addAction = rulerActions->addAction(tr("Show"));
    addAction->setEnabled(DistanceRuler.isNull());
    actions[addAction] = std::bind(&GraphicsPlotExtended::AddRuler, this, event->pos());

    auto removeAll = rulerActions->addAction(tr("Hide"));
    removeAll->setEnabled(!DistanceRuler.isNull());
    actions[removeAll] = std::bind(&GraphicsPlotExtended::RemoveRulers, this);
  }

  CreateGridActions(event, menu, actions);

  if ((Flags & GP::AALegend) && legend && legend->itemCount())
  {
    QMenu* legendActions = menu.addMenu(tr("Legend"));
    {
      QAction* leggendVisible = legendActions->addAction(tr("Visible"));
      leggendVisible->setCheckable(true);
      leggendVisible->setChecked(legend->visible());
      actions[leggendVisible] = std::bind(&GraphicsPlotExtended::setLegendVisible, this, legend, !legend->visible());

      QMenu* legendPos = legendActions->addMenu(tr("Position"));

      Qt::Alignment horPos = getLegendPosition(legend) & ~Qt::AlignTop & ~Qt::AlignVCenter & ~Qt::AlignBottom;
      Qt::Alignment vertPos = getLegendPosition(legend) & ~Qt::AlignLeft & ~Qt::AlignHCenter & ~Qt::AlignRight;

      actions[legendPos->addAction(tr("Top"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, horPos | Qt::AlignTop);
      actions[legendPos->addAction(tr("Center"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, horPos | Qt::AlignVCenter);
      actions[legendPos->addAction(tr("Bottom"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, horPos | Qt::AlignBottom);

      legendPos->addSeparator();

      actions[legendPos->addAction(tr("Left"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, vertPos | Qt::AlignLeft);
      actions[legendPos->addAction(tr("Center"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, vertPos | Qt::AlignHCenter);
      actions[legendPos->addAction(tr("Right"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, vertPos | Qt::AlignRight);
    }
  }

#ifndef DISABLE_STANDARD_VIEWS
#if defined(USE_STANDARD_VIEWS)
  if (Flags & GP::AACaptions)
  {
    QMenu* captionActions = menu.addMenu(tr("Captions"));
    {
      QAction* captionAdd = captionActions->addAction(tr("Add caption"));

      auto add = [this, event]() {
        CaptionEditDialog* dialog = new CaptionEditDialog(this, tr("Caption text:"), "");
        dialog->SetCurvesList(mPlottables);
        if (auto rect = axisRectAt(event->pos()))
          dialog->SetAxesList(rect->axes(GPAxis::atLeft | GPAxis::atRight));

#if defined(USE_CUSTOM_WINDOWS)
        CustomWindows::CustomNativeDialog window(this, dialog, CustomNativeWindow::CWDefaultDialogFlags);
        if (window.exec() == QDialog::Accepted)
#else
        if (dialog->exec() == QDialog::Accepted)
#endif
        {
          addCaptionItem(dialog->GetTarget(), dialog->GetValue(), dialog->GetHasLine(), event->pos());
          queuedReplot();
        }

#if !defined(USE_CUSTOM_WINDOWS)
        delete dialog;
#endif
      };
      actions[captionAdd] = add;
    }

    {
      QAction* captionRemove = captionActions->addAction(tr("Remove caption"));
      captionRemove->setEnabled(false);
      auto item = layerableAt(event->pos(), false);
      auto caption = qobject_cast<GPCaptionItem*>(item);
      if (caption)
        captionRemove->setEnabled(true);

      auto remove = [this, event, caption]() {
        if (caption)
        {
          Captions.remove(caption->getId());
          delete caption;
          queuedReplot();
        }
      };
      actions[captionRemove] = remove;
    }

    {
      QAction* captionRemoveAll = captionActions->addAction(tr("Remove all captions"));
      captionRemoveAll->setEnabled(!Captions.isEmpty());

      auto removeAll = [this, event](){
        qDeleteAll(Captions);
        Captions.clear();
        queuedReplot();
      };
      actions[captionRemoveAll] = removeAll;
    }
  }
#endif
#endif // ~!

  // Zones
  if (Flags & GP::AAZones)
  {
    QMenu* zonesActions = menu.addMenu(tr("Zones"));
    {
      {
        QAction* zonesShow = zonesActions->addAction(tr("Show zones"));
        auto show = [this]() {
          addZonesRect();
          LoadZones();
          ZonesRect->setVisible(true);

          if (auto data = ZonesRect->data())
          {
            if (data->size() == 0)
              ZonesRect->setupDefaultLines();
          }

          queuedReplot();
        };

        if (ZonesRect && ZonesRect->visible())
          zonesShow->setEnabled(false);
        actions[zonesShow] = show;
      }

      {
        QAction* zonesHide = zonesActions->addAction(tr("Hide zones"));
        auto hide = [this]() {
          if (ZonesRect)
          {
            ZonesRect->setVisible(false);
            queuedReplot();
          }
        };

        if (!ZonesRect || !ZonesRect->visible())
          zonesHide->setEnabled(false);
        actions[zonesHide] = hide;
      }

      if (Flags & GP::AAZonesAmount)
      {
        zonesActions->addSeparator();
        QMenu* legendPos = zonesActions->addMenu(tr("Label position"));
        legendPos->setEnabled(ZonesRect && ZonesRect->getLegend());
        if (ZonesRect)
        {
          if (auto legend = ZonesRect->getLegend())
          {
            Qt::Alignment horPos = getLegendPosition(legend) & ~Qt::AlignTop & ~Qt::AlignVCenter & ~Qt::AlignBottom;
            Qt::Alignment vertPos = getLegendPosition(legend) & ~Qt::AlignLeft & ~Qt::AlignHCenter & ~Qt::AlignRight;

            actions[legendPos->addAction(tr("Top"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, horPos | Qt::AlignTop);
            actions[legendPos->addAction(tr("Center"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, horPos | Qt::AlignVCenter);
            actions[legendPos->addAction(tr("Bottom"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, horPos | Qt::AlignBottom);

            legendPos->addSeparator();

            actions[legendPos->addAction(tr("Left"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, vertPos | Qt::AlignLeft);
            actions[legendPos->addAction(tr("Center"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, vertPos | Qt::AlignHCenter);
            actions[legendPos->addAction(tr("Right"))] = std::bind(&GraphicsPlotExtended::setLegendPosition, this, legend, vertPos | Qt::AlignRight);
          }
        }
      }

#ifndef DISABLE_STANDARD_VIEWS
      if (Flags & GP::AAZonesManagement)
      {
        zonesActions->addSeparator();
        QAction* zonesSettings = zonesActions->addAction(tr("Zones management"));
        auto settings = [this]() {
          ZonesEditDialog* dialog = new ZonesEditDialog(this, ZonesRect);

#if defined(USE_CUSTOM_WINDOWS)
          CustomWindows::ShowNewDialog(this, dialog, CustomNativeWindow::CWDefaultDialogFlags);
#else
          dialog->exec();
          delete dialog;
#endif
        };

        if (!ZonesRect || !ZonesRect->visible())
          zonesSettings->setEnabled(false);
        actions[zonesSettings] = settings;
      }
#endif // ~!DISABLE_STANDARD_VIEWS

      if (Flags & GP::AAZonesAmount)
      {
        QAction* zonesSettings = zonesActions->addAction(tr("Amount settings"));
        auto settings = [this](){
          if (ZonesRect && ZonesRect->visible())
          {
            ZonesAmountDialog* dialog = new ZonesAmountDialog(this);

#if defined(USE_CUSTOM_WINDOWS)
            CustomWindows::CustomNativeDialog window(this, dialog, CustomNativeWindow::CWDefaultDialogFlags);
            if (window.exec() == QDialog::Accepted)
#else
            if (dialog->exec() == QDialog::Accepted)
#endif
            {
              ZonesRect->setAmountSettings(dialog->GetAmountSettings());
              SaveSettings();
            }

#if !defined(USE_CUSTOM_WINDOWS)
            delete dialog;
#endif

            queuedReplot();
          }
        };
        if (!ZonesRect || !ZonesRect->visible())
          zonesSettings->setEnabled(false);
        actions[zonesSettings] = settings;
      }
    }
  }

  // Axis actions
  if (axis)
  {
    menu.addSeparator();
    QString label = axis->label();
    if (label.isEmpty())
    {
      label = tr("Axis");
    }
    QMenu* axisActions = menu.addMenu(label);

    // Position
    QActionGroup* positionGroup = new QActionGroup(axisActions);
    if (axis->orientation() == Qt::Vertical)
    {
      QAction* positionLeft = axisActions->addAction(tr("Left"));
      positionLeft->setActionGroup(positionGroup);
      positionLeft->setCheckable(true);
      positionLeft->setChecked(axis->axisType() == GPAxis::atLeft);
      actions[positionLeft] = std::bind(&GraphicsPlotExtended::moveAxis, this, axis->axisRect(), axis, GPAxis::atLeft);

      QAction* positionRight = axisActions->addAction(tr("Right"));
      positionRight->setActionGroup(positionGroup);
      positionRight->setCheckable(true);
      positionRight->setChecked(axis->axisType() == GPAxis::atRight);
      actions[positionRight] = std::bind(&GraphicsPlotExtended::moveAxis, this, axis->axisRect(), axis, GPAxis::atRight);
    }
    else
    {
      QAction* positionTop = axisActions->addAction(tr("Top"));
      positionTop->setActionGroup(positionGroup);
      positionTop->setCheckable(true);
      positionTop->setChecked(axis->axisType() == GPAxis::atTop);
      actions[positionTop] = std::bind(&GraphicsPlotExtended::moveAxis, this, axis->axisRect(), axis, GPAxis::atTop);

      QAction* positionBottom = axisActions->addAction(tr("Bottom"));
      positionBottom->setActionGroup(positionGroup);
      positionBottom->setCheckable(true);
      positionBottom->setChecked(axis->axisType() == GPAxis::atBottom);
      actions[positionBottom] = std::bind(&GraphicsPlotExtended::moveAxis, this, axis->axisRect(), axis, GPAxis::atBottom);
    }

    // Transform
    QActionGroup* transformGroup = new QActionGroup(axisActions);
    if (axis->property(PKAxisDisableLogView).toBool() == false)
    {
      if (!axisActions->isEmpty())
        axisActions->addSeparator();

      QAction* linear = axisActions->addAction(tr("Linear"));
      linear->setActionGroup(transformGroup);
      linear->setCheckable(true);
      linear->setChecked(axis->scaleType() == GPAxis::stLinear);
      actions[linear] = std::bind(&GraphicsPlotExtended::SetAxisScaleType, this, axis, GPAxis::stLinear);

      QAction* logarithmic = axisActions->addAction(tr("Logarithmic"));
      logarithmic->setActionGroup(transformGroup);
      logarithmic->setCheckable(true);
      logarithmic->setChecked(axis->scaleType() == GPAxis::stLogarithmic);
      actions[logarithmic] = std::bind(&GraphicsPlotExtended::SetAxisScaleType, this, axis, GPAxis::stLogarithmic);
    }

    if (!axisActions->isEmpty())
      axisActions->addSeparator();

    axisActions->installEventFilter(this);

    // Step
    if (qobject_cast<GPTrackAxis*>(axis) == nullptr)
    {
      GPAxisTicker* ticker = axis->ticker().data();
      GPAxisAwareTicker* tickerAxisAware = qobject_cast<GPAxisAwareTicker*>(ticker);
      if (tickerAxisAware && axis->scaleType() != GPAxis::stLogarithmic)
      {
        QMenu* axisStepActionMenu = axisActions->addMenu(tr("Step"));
        axisStepActionMenu->setProperty("Persistent", true);

        QWidgetAction* wa = new QWidgetAction(axisStepActionMenu);
        wa->setDefaultWidget(CreateAxisTickerActions(axisStepActionMenu, axis));
        axisStepActionMenu->addAction(wa);
      }
    }

    // Range
    {
      QMenu* axisRangeActions = axisActions->addMenu(tr("Range"));
      axisRangeActions->setProperty("Persistent", true);

      QWidgetAction* wa = new QWidgetAction(axisRangeActions);
      wa->setDefaultWidget(CreateAxisRangeActions(axisRangeActions, axis));
      axisRangeActions->addAction(wa);
    }

    // Uom
    {
#ifdef USE_STANDARD_MODELS
      QString uomSemantics = axis->getUomSemantics();
      if (!uomSemantics.isEmpty() && axis->canChangeUom())
      {
        UOM::PhysicalQuantity quantity = UOMSettings::Get().GetQuantityForSemantics(uomSemantics);
        int currentUnits = UOMSettings::Get().GetUiUnitsForSemantics(uomSemantics);
        if (quantity != UOM::PhysicalQuantityNone)
        {
          QSet<int> units = UOMSettings::Get().GetAcceptableUiUnitsForSemantics(uomSemantics);
          if (!units.empty())
          {
            axisActions->addSeparator();
            QMenu* uomActions = axisActions->addMenu(tr("Units"));
            QActionGroup* uomGroup = new QActionGroup(uomActions);

            for (auto u : units)
            {
              QString uomUnitsName  = UOM::GetUnitsDisplayNameSimplifiedAlternative(quantity, u);
              QAction* unitAction   = uomActions->addAction(uomUnitsName);
              unitAction->setActionGroup(uomGroup);
              unitAction->setCheckable(true);
              unitAction->setChecked(u == currentUnits);
              actions[unitAction]   = [uomSemantics, u]() { UOMSettings::Get().SetUiUnitsForSemantics(uomSemantics, u); };
            }
          }
        }
      }
#endif // ~USE_STANDARD_MODELS
    }
  }
}

void GraphicsPlotExtended::CreateGridActions(QContextMenuEvent *, QMenu &menu, std::map<QAction *, std::function<void ()>> &actions)
{
  if (Flags & GP::AAGrid && ((xAxis && xAxis->grid()) || (yAxis && yAxis->grid())))
  {
    QMenu* gridActions = menu.addMenu(tr("Grid"));

    if (xAxis && xAxis->grid())
    {
      QAction* actionToggleXAxisGrid = gridActions->addAction(tr("Show X Grid"));
      actionToggleXAxisGrid->setCheckable(true);
      actionToggleXAxisGrid->setChecked(xAxis->grid()->visible());
      actions[actionToggleXAxisGrid] = std::bind(&GraphicsPlotExtended::SetAxisListGridVisible, this, GetHorizontalAxesWithGrid(), !xAxis->grid()->visible());
    }

    if (yAxis && yAxis->grid())
    {
      QAction* actionToggleYAxisGrid = gridActions->addAction(tr("Show Y Grid"));
      actionToggleYAxisGrid->setCheckable(true);
      actionToggleYAxisGrid->setChecked(yAxis->grid() && yAxis->grid()->visible());
      actions[actionToggleYAxisGrid] = std::bind(&GraphicsPlotExtended::SetAxisListGridVisible, this, GetVerticalAxesWithGrid(), !yAxis->grid()->visible());
    }
  }
}

QWidget *GraphicsPlotExtended::CreateAxisRangeActions(QWidget *parent, GPAxis *axis)
{
  if (!axis || !axis->ticks() || !axis->tickLabels())
    return nullptr;

#ifdef USE_STANDARD_VIEWS
  auto isENUm = [](const double& v) {
    Q_UNUSED(v);
    return fabs(v) > std::numeric_limits<double>::epsilon() && (fabs(v) < 1e-2 || fabs(v) > 1e6);
    return false;
  };
#endif // USE_STANDARD_VIEWS

  QWidget* menu = new QWidget(parent);
  menu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  QGridLayout* grid = new QGridLayout(menu);
  grid->setContentsMargins(4, 4, 4, 4);


  auto dateTimeFormat = GPAxisAwareTicker::getQDateTimeFormat(axis);
  QVariant rangeLower = GPAxisAwareTicker::toFormatedKey(axis, axis->range().lower);
  QVariant rangeUpper = GPAxisAwareTicker::toFormatedKey(axis, axis->range().upper);

  // Limits
  double limitMinimum = std::numeric_limits<double>::quiet_NaN();
  double limitMaximum = std::numeric_limits<double>::quiet_NaN();
  {
    bool ok = false;
    if (axis->property("Minimum").isValid())
      limitMinimum = axis->property("Minimum").toDouble(&ok);
    if (!ok)
      limitMinimum = std::numeric_limits<double>::quiet_NaN();

    if (axis->property("Maximum").isValid())
      limitMaximum = axis->property("Maximum").toDouble(&ok);
    if (!ok)
      limitMaximum = std::numeric_limits<double>::quiet_NaN();
  }


  QWidget* rangeLowerEditor = nullptr;
  QDoubleSpinBox* sbMin = nullptr;
#ifdef USE_STANDARD_VIEWS
  ENumberEditor* eMin = nullptr;
#endif
  QDateTimeEdit* dateTimeMin = nullptr;
  if (rangeLower.VARIANT_TYPE_ID() == QMetaType::QDateTime)
  {
    menu->setMinimumWidth(255);

    dateTimeMin = new QDateTimeEdit(menu);
    dateTimeMin->setCalendarPopup(true);
    dateTimeMin->setDisplayFormat(dateTimeFormat);
    dateTimeMin->setDateTime(rangeLower.toDateTime());
    dateTimeMin->setMaximumDateTime(rangeUpper.toDateTime());
    dateTimeMin->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    rangeLowerEditor = dateTimeMin;
  }
  else
  {
#ifdef USE_STANDARD_VIEWS
    if (isENUm(rangeLower.toDouble()) || isENUm(rangeUpper.toDouble()))
    {
      menu->setMaximumWidth(220);

      eMin = new ENumberEditor(menu);
      eMin->setDecimals(3);
      eMin->setMaximum(rangeUpper.toDouble());
      eMin->setValue(rangeLower.toDouble());
      if (!std::isnan(limitMinimum))
        eMin->setMinimum(limitMinimum);

      rangeLowerEditor = eMin;
    }
    else
#endif
    {
      menu->setMaximumWidth(190);

      sbMin = new RangeSpinBox(menu);
      sbMin->setValue(rangeLower.toDouble());
      sbMin->setMaximum(rangeUpper.toDouble());
      if (!std::isnan(limitMinimum))
        sbMin->setMinimum(limitMinimum);

      rangeLowerEditor = sbMin;
    }
  }

  QWidget* rangeUpperEditor = nullptr;
  QDoubleSpinBox* sbMax = nullptr;
#ifdef USE_STANDARD_VIEWS
  ENumberEditor* eMax = nullptr;
#endif // #ifdef USE_STANDARD_VIEWS
  QDateTimeEdit* dateTimeMax = nullptr;
  if (rangeLower.VARIANT_TYPE_ID() == QMetaType::QDateTime)
  {
    dateTimeMax = new QDateTimeEdit(menu);
    dateTimeMax->setCalendarPopup(true);
    dateTimeMax->setDisplayFormat(dateTimeFormat);
    dateTimeMax->setDateTime(rangeUpper.toDateTime());
    dateTimeMax->setMinimumDateTime(rangeLower.toDateTime());
    dateTimeMax->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    rangeUpperEditor = dateTimeMax;
  }
  else
  {
#ifdef USE_STANDARD_VIEWS
    if (isENUm(rangeLower.toDouble()) || isENUm(rangeUpper.toDouble()))
    {
      eMax = new ENumberEditor(menu);
      eMax->setDecimals(3);
      eMax->setMinimum(rangeLower.toDouble());
      eMax->setValue(rangeUpper.toDouble());
      if (!std::isnan(limitMaximum))
        eMax->setMaximum(limitMaximum);

      rangeUpperEditor = eMax;
    }
    else
#endif
    {
      sbMax = new RangeSpinBox(menu);
      sbMax->setValue(rangeUpper.toDouble());
      sbMax->setMinimum(rangeLower.toDouble());
      if (!std::isnan(limitMaximum))
        sbMax->setMaximum(limitMaximum);

      rangeUpperEditor = sbMax;
    }
  }


  QString mdash(QChar(0x2014));
  grid->addWidget(rangeLowerEditor, 0, 0);
  grid->addWidget(new QLabel(mdash, menu), 0, 1);
  grid->addWidget(rangeUpperEditor, 0, 2);


  auto handler = [this, axis, sbMin, sbMax,
#ifdef USE_STANDARD_VIEWS
                  eMin, eMax,
#endif // #ifdef USE_STANDARD_VIEWS
                  dateTimeMin, dateTimeMax]()
  {
    double min = qQNaN();
    double max = qQNaN();
    if (sbMin && sbMax)
    {
      min = sbMin->value();
      max = sbMax->value();

      sbMin->setMaximum(max);
      sbMax->setMinimum(min);
    }
#ifdef USE_STANDARD_VIEWS
    else if (eMin && eMax)
    {
      min = eMin->getValue();
      max = eMax->getValue();

      eMin->setMaximum(max);
      eMax->setMinimum(min);
    }
#endif
    else if (dateTimeMin && dateTimeMax)
    {
      min = dateTimeMin->dateTime().toMSecsSinceEpoch();
      max = dateTimeMax->dateTime().toMSecsSinceEpoch();

      dateTimeMin->setMaximumDateTime(dateTimeMax->dateTime());
      dateTimeMax->setMinimumDateTime(dateTimeMin->dateTime());
    }

    QObject* s = sender();
    if (s)
    {
      bool symmetric = !qIsNaN(axis->rangeLockedMiddle());
      if (symmetric)
      {
        if (s == sbMin
#ifdef USE_STANDARD_VIEWS
          || s == eMin
#endif
          || s == dateTimeMin)
        {
          axis->setRangeLower(GPAxisAwareTicker::fromFormatedKey(axis, min));
          if (s == sbMin)
          {
            if (sbMax)
              sbMax->setValue(axis->range().upper);
          }
#ifdef USE_STANDARD_VIEWS
          else if (s == eMin)
          {
            if (eMax)
              eMax->setValue(axis->range().upper);
          }
#endif
          else if (s == dateTimeMin)
          {
            if (dateTimeMax)
            {
              QVariant rangeUpper = GPAxisAwareTicker::toFormatedKey(axis, axis->range().upper);
              dateTimeMax->setDateTime(rangeUpper.toDateTime());
            }
          }
        }
        else if (s == sbMax
#ifdef USE_STANDARD_VIEWS
              || s == eMax
#endif
              || s == dateTimeMax)
        {
          axis->setRangeUpper(GPAxisAwareTicker::fromFormatedKey(axis, max));
          if (s == sbMax)
          {
            if (sbMin)
              sbMin->setValue(axis->range().lower);
          }
#ifdef USE_STANDARD_VIEWS
          else if (s == eMax)
          {
            if (eMin)
              eMin->setValue(axis->range().lower);
          }
#endif
          else if (s == dateTimeMax)
          {
            if (dateTimeMin)
            {
              QVariant rangeLower = GPAxisAwareTicker::toFormatedKey(axis, axis->range().lower);
              dateTimeMin->setDateTime(rangeLower.toDateTime());
            }
          }
        }
      }
      else
      {
        axis->setRangeLower(GPAxisAwareTicker::fromFormatedKey(axis, min));
        axis->setRangeUpper(GPAxisAwareTicker::fromFormatedKey(axis, max));
      }
    }
    queuedReplot();
  };


  if (sbMin && sbMax)
  {
    connect(sbMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, handler);
    connect(sbMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, handler);
  }
#ifdef USE_STANDARD_VIEWS
  else if (eMin && eMax)
  {
    connect(eMin, &ENumberEditor::valueChanged, this, handler);
    connect(eMax, &ENumberEditor::valueChanged, this, handler);
  }
#endif
  else if (dateTimeMin && dateTimeMax)
  {
    connect(dateTimeMin, &QDateTimeEdit::dateTimeChanged, this, handler);
    connect(dateTimeMax, &QDateTimeEdit::dateTimeChanged, this, handler);
  }

  return menu;
}

QWidget *GraphicsPlotExtended::CreateAxisTickerActions(QWidget *parent, GPAxis *axis)
{
  if (!axis || !axis->ticks() || !axis->tickLabels())
    return nullptr;

  if (qobject_cast<GPTrackAxis*>(axis))
    return nullptr;

  GPAxisTicker* ticker = axis->ticker().data();
  GPAxisAwareTicker* tickerAxisAware = qobject_cast<GPAxisAwareTicker*>(ticker);
  if (!tickerAxisAware)
    return nullptr;

  if (axis->scaleType() == GPAxis::stLogarithmic)
    return nullptr;

  QWidget* axisStepAction = new QWidget(parent);

  QGridLayout* grid = new QGridLayout(axisStepAction);
  grid->setContentsMargins(4, 4, 4, 4);

  double rangeAbs = qAbs(axis->range().upper - axis->range().lower);
  double reasonableMinimum = rangeAbs / 100;
  double reasonableMaximum = rangeAbs / 2;
  double reasonableDefault = ticker->getTickStep(axis->range());
  double reasonableStepSet = tickerAxisAware->getPreferredStep();
  bool automatic = reasonableStepSet == 0;

  RangeSpinBox* spinDouble = nullptr;
  QSpinBox* spinDays = nullptr;
  QSpinBox* spinHours = nullptr;
  QSpinBox* spinMins = nullptr;
  QSpinBox* spinSecs = nullptr;
  QSpinBox* spinMSecs = nullptr;

  auto getDateData = [](int defMs, int& days, int& hours, int& mins, int& secs, int& ms) {
    days = defMs / (1000 * 60 * 60 * 24);
    defMs = defMs % (1000 * 60 * 60 * 24);
    hours = defMs / (1000 * 60 * 60);
    defMs = defMs % (1000 * 60 * 60);
    mins = defMs / (1000 * 60);
    defMs = defMs % (1000 * 60);
    secs = defMs / 1000;
    defMs = defMs % 1000;
    ms = defMs;
  };

  if (tickerAxisAware->isFormatedTime())
  {
    QWidget* dateEditor = new QWidget(axisStepAction);
    QHBoxLayout* dateEditorLayout = new QHBoxLayout(dateEditor);
    dateEditorLayout->setContentsMargins(0, 0, 0, 0);

    int minMs = reasonableMinimum;
    int maxMs = reasonableMaximum;
    int defMs = reasonableDefault;
    int setMs = reasonableStepSet;

    //////////////////////////////////////////////////////////////
    // Mins & maxs
    int minDays, minHours, minMins, minSecs, minMsecs;
    getDateData(minMs, minDays, minHours, minMins, minSecs, minMsecs);

    int maxDays, maxHours, maxMins, maxSecs, maxMsecs;
    getDateData(maxMs, maxDays, maxHours, maxMins, maxSecs, maxMsecs);

    //////////////////////////////////////////////////////////////
    // Defs & sets
    int defDays, defHours, defMins, defSecs, defMsecs;
    getDateData(defMs, defDays, defHours, defMins, defSecs, defMsecs);

    int setDays, setHours, setMins, setSecs, setMsecs;
    getDateData(setMs, setDays, setHours, setMins, setSecs, setMsecs);

    auto addSpinBox = [dateEditor, dateEditorLayout, automatic](int defVal, int setVal, QString text)
    {
      auto sb = new QSpinBox(dateEditor);
      sb->setValue(automatic ? defVal : setVal);
      sb->setEnabled(!automatic);
      dateEditorLayout->addWidget(sb);
      dateEditorLayout->addWidget(new QLabel(text, dateEditor));
      return sb;
    };

    // Days
    if (maxDays)
      spinDays = addSpinBox(defDays, setDays, tr("d"));

    // Hours
    if (maxDays || maxHours)
      spinHours = addSpinBox(defHours, setHours, tr("h"));

    // Minutes
    if (maxDays || maxHours || maxMins)
      spinMins = addSpinBox(defMins, setMins, tr("m"));

    // Seconds
    if (maxDays || maxHours || maxMins || maxSecs)
      spinSecs = addSpinBox(defSecs, setSecs, tr("s"));

    // Milliseconds
    spinMSecs = addSpinBox(defMsecs, setMsecs, tr("ms"));

    grid->addWidget(dateEditor, 0, 0);
  }
  else
  {
    spinDouble = new RangeSpinBox(axisStepAction);
    spinDouble->setValue(tickerAxisAware->toFormatedKey(automatic ? reasonableDefault : reasonableStepSet).toDouble());
    spinDouble->setEnabled(!automatic);
    grid->addWidget(spinDouble, 0, 0);
  }

  QCheckBox* checkbox = new QCheckBox(tr("Automatic"), axisStepAction);
  checkbox->setChecked(automatic);
  grid->addWidget(checkbox, 1, 0);

  auto setTickerData = [this, spinDouble, spinDays, spinHours, spinMins, spinSecs, spinMSecs, checkbox]
      (GPAxisAwareTicker* tickerAxisAware)
  {
    if (spinDouble)
    {
      spinDouble->setEnabled(!checkbox->isChecked());
      tickerAxisAware->setPreferredStep(checkbox->isChecked() ? 0.0 : tickerAxisAware->fromFormatedKey(spinDouble->value()));
    }
    else
    {
      if (spinMSecs)
        spinMSecs->setEnabled(!checkbox->isChecked());
      if (spinSecs)
        spinSecs->setEnabled(!checkbox->isChecked());
      if (spinMins)
        spinMins->setEnabled(!checkbox->isChecked());
      if (spinHours)
        spinHours->setEnabled(!checkbox->isChecked());
      if (spinDays)
        spinDays->setEnabled(!checkbox->isChecked());

      double newValue = 0;
      if (!checkbox->isChecked())
      {
        if (spinMSecs)
          newValue += (double)spinMSecs->value();
        if (spinSecs)
          newValue += (double)spinSecs->value() * 1000;
        if (spinMins)
          newValue += (double)spinMins->value() * 1000 * 60;
        if (spinHours)
          newValue += (double)spinHours->value() * 1000 * 60 * 60;
        if (spinDays)
          newValue += (double)spinDays->value() * 1000 * 60 * 60 * 24;
      }
      tickerAxisAware->setPreferredStep(checkbox->isChecked() ? 0.0 : newValue);
    }
    queuedReplot();
  };

  auto updateSpinValue = [axis, tickerAxisAware, checkbox, spinDouble, spinMSecs, spinSecs, spinMins, spinHours, spinDays, getDateData]()
  {
    if (checkbox->isChecked())
    {
      double defMs = tickerAxisAware->getTickStep(axis->range());
      if (spinDouble)
      {
        spinDouble->setValue(tickerAxisAware->toFormatedKey(defMs).toDouble());
      }
      else
      {
        int defDays, defHours, defMins, defSecs, defMsecs;
        getDateData(defMs, defDays, defHours, defMins, defSecs, defMsecs);

        if (spinMSecs)
          spinMSecs->setValue(defMsecs);
        if (spinSecs)
          spinSecs->setValue(defSecs);
        if (spinMins)
          spinMins->setValue(defMins);
        if (spinHours)
          spinHours->setValue(defHours);
        if (spinDays)
          spinDays->setValue(defDays);
      }
    }
  };

  auto updateAxisTicker = [axis, setTickerData, updateSpinValue]()
  {
    auto bound = getBoundAxes(axis);
    bound.insert(axis);
    for (const auto& a : bound)
      setTickerData(qobject_cast<GPAxisAwareTicker*>(a->ticker().data()));
    updateSpinValue();
  };

  auto updateSpinRanges = [axis, spinDouble, spinDays, spinHours, spinMins, spinSecs, spinMSecs, tickerAxisAware, getDateData, updateSpinValue]()
  {
    double rangeAbs = qAbs(axis->range().upper - axis->range().lower);
    double reasonableMinimum = rangeAbs / 100;
    double reasonableMaximum = rangeAbs / 2;
    double reasonableSpinStp = (reasonableMaximum) / 10;

    if (tickerAxisAware->isFormatedTime())
    {
      int minMs = reasonableMinimum;
      int maxMs = reasonableMaximum;

      //////////////////////////////////////////////////////////////
      // Mins & maxs
      int minDays, minHours, minMins, minSecs, minMsecs;
      getDateData(minMs, minDays, minHours, minMins, minSecs, minMsecs);

      int maxDays, maxHours, maxMins, maxSecs, maxMsecs;
      getDateData(maxMs, maxDays, maxHours, maxMins, maxSecs, maxMsecs);

      if (spinDays)
      {
        QSignalBlocker l(spinDays);
        spinDays->setRange(minDays, maxDays);
      }
      if (spinHours)
      {
        QSignalBlocker l(spinHours);
        spinHours->setRange(0, 23);
      }
      if (spinMins)
      {
        QSignalBlocker l(spinMins);
        spinMins->setRange(0, 59);
      }
      if (spinSecs)
      {
        QSignalBlocker l(spinSecs);
        spinSecs->setRange(0, 59);
      }
      if (spinMSecs)
      {
        QSignalBlocker l(spinMSecs);
        spinMSecs->setRange(0, 999);
      }
    }
    else
    {
      if (spinDouble)
      {
        QSignalBlocker l(spinDouble);
        spinDouble->setRange(tickerAxisAware->toFormatedKey(reasonableMinimum).toDouble(),
                             tickerAxisAware->toFormatedKey(reasonableMaximum).toDouble());
        spinDouble->setSingleStep(tickerAxisAware->toFormatedKey(reasonableSpinStp).toDouble());
      }
    }

    updateSpinValue();
  };

  connect(checkbox, &QCheckBox::stateChanged, updateAxisTicker);
  if (spinDouble)
  {
    connect(spinDouble, static_cast<void (RangeSpinBox::*)(double)>(&RangeSpinBox::valueChanged), updateAxisTicker);
  }
  else
  {
    if (spinDays)
      connect(spinDays, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), updateAxisTicker);
    if (spinHours)
      connect(spinHours, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), updateAxisTicker);
    if (spinMins)
      connect(spinMins, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), updateAxisTicker);
    if (spinSecs)
      connect(spinSecs, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), updateAxisTicker);
    if (spinMSecs)
      connect(spinMSecs, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), updateAxisTicker);
  }

  auto updateConnection = connect(axis, static_cast<void (GPAxis::*)(const GPRange&)>(&GPAxis::rangeChanged), axisStepAction, updateSpinRanges);
  connect(axisStepAction, &QWidget::destroyed, [this, updateConnection]() {
    disconnect(updateConnection);
  });

  updateSpinRanges();

  return axisStepAction;
}

void GraphicsPlotExtended::contextMenuEvent(QContextMenuEvent* event)
{
  if (InDrawingMeasuringPolygon)
    return;

  if (Flags & GP::AAMenu)
  {
    if (ZoomingAxisMoved)
      return GraphicsPlot::contextMenuEvent(event);

    // Global actions
    if (ContextMenu)
    {
      delete ContextMenu;
    }
    ContextMenu = new QMenu(this);
    std::map<QAction*, std::function<void(void)>> actions;

    FillContextMenu(event, *ContextMenu, actions);

    if (!actions.empty())
    {
      connect(ContextMenu, &QMenu::triggered, [actions](QAction* action)
        {
          auto a = actions.find(action);
          if (a != actions.end())
          {
            a->second();
          }
        });

      // Cancel zooming axis
      ZoomingAxis.clear();
      ZoomingAxisMoved = false;

      // Show
      ContextMenu->popup(event->globalPos());
      event->accept();

      // Done
      return;
    }
    GraphicsPlot::contextMenuEvent(event);
  }
}

bool GraphicsPlotExtended::event(QEvent *event)
{
  switch(event->type())
  {
    case QEvent::DragEnter:
    case QEvent::DragMove:
      {
        InDragAndDrop = true;
      }
      break;
    case QEvent::DragLeave:
    case QEvent::Drop:
      {
        QTimer::singleShot(1000, [this](){
          InDragAndDrop = false;
        });
      }
      break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      {
        if (underMouse())
        {
          if (auto keyEvent = static_cast<QKeyEvent*>(event))
          {
            if (keyEvent->key() == Qt::Key_Control || keyEvent->key() == Qt::Key_Shift)
            {
              setCursor(GetCursorAt(mapFromGlobal(QCursor::pos())));
            }

            if (keyEvent->key() == Qt::Key_Alt && (Flags & GP::AAMeasuringPolygon))
            {
              SetMeasuringPolygonDraw(event->type() == QEvent::KeyPress);
              return true;
            }
          }
        }
      }
      break;
    case QEvent::HoverEnter:
      {
        if (InDrawingMeasuringPolygon && !(QApplication::keyboardModifiers() & Qt::AltModifier))
        {
          SetMeasuringPolygonDraw(false);
        }
        replot();
      }
      break;
    case QEvent::HoverLeave:
      {
        if (InDrawingMagnifier)
        {
          SetMagnifierDraw(false);
        }
      }
      break;
    case QEvent::Gesture:
      {
        QGestureEvent *gestureEve = static_cast<QGestureEvent*>(event);
        if( QGesture *pinch = gestureEve->gesture(Qt::PinchGesture))
        {
          QPinchGesture *pinchEve = static_cast<QPinchGesture *>(pinch);
          qreal scaleFactor = pinchEve->totalScaleFactor();
          if (scaleFactor > 1.0)
          {
            scaleFactor *= 27;
          }
          else
          {
            scaleFactor *= -27;
          }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
          QWheelEvent *wheelEve = new QWheelEvent(GestureTouchMousePressPos, scaleFactor, Qt::NoButton, Qt::NoModifier, Qt::Vertical);
          wheelEvent(wheelEve);
#else
          //// @todo
#endif
        }
      }
      return true;
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
      {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
        if(touchPoints.count( ) == 1)
        {
          const QTouchEvent::TouchPoint &pointFirst = touchPoints.first();
          GestureTouchMousePressPos = pointFirst.pos().toPoint();
          QMouseEvent* mouseEve = new QMouseEvent(QEvent::MouseButtonPress, GestureTouchMousePressPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
          if(touchEvent->touchPointStates() == (Qt::TouchPointStates)Qt::TouchPointPressed)
          {
            mousePressEvent(mouseEve);
          }
          else if(touchEvent->touchPointStates() == (Qt::TouchPointStates)Qt::TouchPointMoved)
          {
            mouseMoveEvent(mouseEve);
          }
          else if(touchEvent->touchPointStates() == (Qt::TouchPointStates)Qt::TouchPointReleased)
          {
            mouseReleaseEvent(mouseEve);
          }
          delete mouseEve;
        }
      }
      return true;
    default:
      break;
  }
  return GraphicsPlot::event(event);
}

void GraphicsPlotExtended::wheelEvent(QWheelEvent* event)
{
  // Find axis
  auto pos =
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  event->pos();
#else
  event->position();
#endif
  QList<GPLayerable*> candidates = layerableListAt(pos, false);
  QList<GPAxis*> candidateAxes;
  for (int i = 0; i < candidates.size(); ++i)
  {
    if (auto legend = qobject_cast<GPEXLegend*>(candidates.at(i)))
    {
      legend->wheelEvent(event);
      return;
    }
    if (auto axis = qobject_cast<GPAxis*>(candidates.at(i)))
    {
      candidateAxes.append(axis);
    }
  }

  // As axis
  if (!candidateAxes.empty())
  {
    for (int i = 0; i < candidateAxes.size(); ++i)
      candidateAxes.at(i)->wheelEvent(event);
  }
  else
  {
    // no axis hit, process with AxisRect
    GraphicsPlot::wheelEvent(event);
  }
}

void GraphicsPlotExtended::showEvent(QShowEvent* event)
{
  GraphicsPlot::showEvent(event);
  bool oldblockReplots = getBlockReplots();
  blockReplots(true);
  if (PendingViewAll)
  {
    if (PendingViewAllAxes.isEmpty())
      viewAll(PendingViewAllFlags);
    else
      viewAll(PendingViewAllAxes, PendingViewAllFlags);
  }
  blockReplots(oldblockReplots);
  CurvesUpdated();
  replot();
}

bool GraphicsPlotExtended::eventFilter(QObject* object, QEvent* event)
{
  if (QMenu* menu = qobject_cast<QMenu*>(object))
  {
    switch (event->type())
    {
      //      case QEvent::MouseButtonRelease:
      //        {
      //          if (QAction* action = menu->activeAction())
      //          {
      //            if (!action->menu()
      //                && action->isCheckable()
      //                && action->isEnabled())
      //            {
      //              action->setProperty("Triggered", true);
      //              action->trigger();
      //              return true;
      //            }
      //          }
      //        }
      case QEvent::Timer:
        {
          if (menu->isVisible())
          {
            QList<QMenu*> childMenus = menu->findChildren<QMenu*>(QString(), Qt::FindChildrenRecursively);
            QList<QMenu*>::iterator c = childMenus.begin();
            while (c != childMenus.end())
            {
              bool visible    = (*c)->isVisible();
              bool persistent = (*c)->property("Persistent").toBool();
              if (visible && persistent)
              {
                return true;
              }
              ++c;
            }
          }
        }
      default:
        break;
    }
  }
  return GraphicsPlot::eventFilter(object, event);
}

Qt::CursorShape GraphicsPlotExtended::GetCursorAt(const QPointF &pos)
{
  if ((Flags & GP::AAMagnifier) && (QApplication::queryKeyboardModifiers() == Qt::ShiftModifier))
  {
    return Qt::CrossCursor;
  }

  QMouseEvent ev(QEvent::MouseMove, pos, Qt::NoButton, qApp->mouseButtons(), qApp->queryKeyboardModifiers());
  QList<GPLayerable*> candidates = layerableListAt(pos, false);
  auto newCursor = Qt::ArrowCursor;
  for (const auto& candidate : candidates)
  {
    newCursor = candidate->HitTest(&ev);
    if (newCursor != Qt::ArrowCursor)
      break;
  }
  return newCursor;
}

void GraphicsPlotExtended::axisRemoved(GPAxis *axis)
{
  GraphicsPlot::axisRemoved(axis);
  ZoomingAxis.removeOne(axis);
  ZoomingAxisStartRangeX.remove(axis);
  ZoomingAxisStartRangeY.remove(axis);
  ZoomingAxisStartCenterX.remove(axis);
  ZoomingAxisStartCenterY.remove(axis);
  AxisInteraction.remove(axis);
  Axes.removeOne(axis);
}

///////////////////////////////////////////////////////
// Plot magnifier
GPMagnifier::GPMagnifier(GraphicsPlot *parent)
  : GPLayerable(parent)
{
}

GPMagnifier::~GPMagnifier()
{
}

void GPMagnifier::ApplyZoom()
{
  if (auto cpex = qobject_cast<GraphicsPlotExtended*>(mParentPlot))
  {
    QPoint startPos = cpex->mMousePressPos;
    QPoint endPos = mParentPlot->mapFromGlobal(QCursor::pos());
    if (auto rect = cpex->axisRectAt(startPos))
    {
      QMap<GPAxis*, GPRange> axisToNewRange;
      for (const auto& axis : rect->axes())
        axisToNewRange[axis] = GPRange();
      if (auto tr = qobject_cast<GPTrackRect*>(rect))
        axisToNewRange[tr->keyAxis()] = GPRange();

      for (auto it = axisToNewRange.begin(); it != axisToNewRange.end(); ++it)
      {
        if (it.key()->orientation() == Qt::Vertical)
        {
          it.value().lower = it.key()->pixelToCoord(startPos.y());
          it.value().upper = it.key()->pixelToCoord(endPos.y());
        }
        else
        {
          it.value().lower = it.key()->pixelToCoord(startPos.x());
          it.value().upper = it.key()->pixelToCoord(endPos.x());
        }
        it.value().normalize();
      }
      for (auto it = axisToNewRange.begin(); it != axisToNewRange.end(); ++it)
      {
        it.key()->setRange(it.value());
      }
      cpex->queuedReplot();
    }
  }
}

void GPMagnifier::draw(GPPainter *painter)
{
  if (auto cpex = qobject_cast<GraphicsPlotExtended*>(mParentPlot))
  {
    QRect rect(cpex->mMousePressPos, mParentPlot->mapFromGlobal(QCursor::pos()));
    painter->setPen(QPen(QColor(97, 127, 255), 1.3));
    painter->drawRect(rect.adjusted(-1, -1, 0, 0));
  }
}


///////////////////////////////////////////////////////
// Polygon
GPPolygon::GPPolygon(GPAxis* keyAxis, GPAxis* valueAxis, GPLayerable* parent)
  : GPAbstractPlottable1D<GPCurveData>(keyAxis, valueAxis, parent)
  , Group(qobject_cast<GPPolygonGroup*>(parent))
{
  mDataContainer->setUomProvider(keyAxis, valueAxis);
}

GPPolygon::~GPPolygon()
{
}

void GPPolygon::setData(const QSharedPointer<GPCurveDataContainer>& data)
{
  mDataContainer = data;
  updateRegistration();
}

void GPPolygon::setData(const QVector<double> &t, const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  mDataContainer->clear();
  const int n = qMin(qMin(t.size(), keys.size()), values.size());
  if (n)
  {
    QVector<GPCurveData> tempData(n + 1);
    QVector<GPCurveData>::iterator it = tempData.begin();
    int i = 0;
    double tMin = it->t;
    double tMax = tMin;
    while (it != tempData.end() && i < n)
    {
      it->t = t[i];
      it->key = keys[i];
      it->value = values[i];
      tMax = qMax(tMax, it->t);
      tMin = qMin(tMin, it->t);
      ++it;
      ++i;
    }
    if (!alreadySorted)
    {
      std::sort(tempData.begin(), tempData.end() - 1, [](const GPCurveData& lt, const GPCurveData& rt) -> bool
      {
        if (lt.t < rt.t)
          return true;
        if (lt.t > rt.t)
          return false;
        if (lt.key < rt.key)
          return true;
        if (lt.key > rt.key)
          return false;
        else
          return lt.value < rt.value;
      });
    }
    it->t = tMax + 1;
    it->key = tempData[0].key;
    it->value = tempData[0].value;
    mDataContainer->add(tempData, true);
    updateRegistration();
  }
}

void GPPolygon::setData(const QVector<double> &keys, const QVector<double> &values)
{
  mDataContainer->clear();
  const int n = qMin(keys.size(), values.size());
  if (n)
  {
    double tStart = 0;
    QVector<GPCurveData> tempData(n + 1);
    QVector<GPCurveData>::iterator it = tempData.begin();
    int i = 0;
    while (it != tempData.end() && i < n)
    {
      it->t = tStart + i;
      it->key = keys[i];
      it->value = values[i];
      ++it;
      ++i;
    }
    it->t = tStart + i;
    it->key = keys[0];
    it->value = values[0];
    mDataContainer->add(tempData, true);
    updateRegistration();
  }
}

void GPPolygon::updateRegistration()
{
  if (Group && mDataContainer.data())
  {
    auto data0 = mDataContainer->at(0);
    Group->registerPolygon(data0->key, data0->value, this);
  }
}

GPRange GPPolygon::getKeyRange(bool &foundRange, GP::SignDomain inSignDomain) const
{
  return mDataContainer->keyRange(foundRange, inSignDomain);
}

GPRange GPPolygon::getValueRange(bool &foundRange, GP::SignDomain inSignDomain, const GPRange &inKeyRange) const
{
  return mDataContainer->valueRange(foundRange, inSignDomain, inKeyRange);
}

GPDataRange GPPolygon::getDataRange() const
{
  return mDataContainer->dataRange();
}

void GPPolygon::draw(GPPainter* painter)
{
  if (mDataContainer->isEmpty())
    return;

  // allocate line vector:
  QVector<QPointF> lines;

  // loop over and draw segments of unselected/selected data:
  QList<GPDataRange> selectedSegments, unselectedSegments, allSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  allSegments << unselectedSegments << selectedSegments;
  for (int i = 0; i<allSegments.size(); ++i)
  {
    bool isSelectedSegment = i >= unselectedSegments.size();

    // fill with curve data:
    QPen finalCurvePen = mPen;
    if (isSelectedSegment && mSelectionDecorator)
      finalCurvePen = mSelectionDecorator->pen();

    // unselected segments extend lines to bordering selected data point (safe to exceed total data bounds in first/last segment, getCurveLines takes care)
    GPDataRange lineDataRange = isSelectedSegment ? allSegments.at(i) : allSegments.at(i).adjusted(-1, 1);
    getCurveLines(&lines, lineDataRange, finalCurvePen.widthF());

    // draw curve fill:
    applyFillAntialiasingHint(painter);
    if (isSelectedSegment && mSelectionDecorator)
      mSelectionDecorator->applyBrush(painter);
    else
      painter->setBrush(mBrush);
    painter->setPen(Qt::NoPen);
    if (painter->brush().style() != Qt::NoBrush && painter->brush().color().alpha() != 0)
      painter->drawPolygon(QPolygonF(lines));

    // draw curve line:
    painter->setPen(finalCurvePen);
    painter->setBrush(Qt::NoBrush);
    drawCurveLine(painter, lines);
  }

  // draw other selection decoration that isn't just line/scatter pens and brushes:
  if (mSelectionDecorator)
  {
    mSelectionDecorator->drawDecoration(painter, selection());
  }
}

void GPPolygon::drawLegendIcon(GPPainter* /*painter*/, const QRectF& /*rect*/) const
{
}

void GPPolygon::drawCurveLine(GPPainter* painter, const QVector<QPointF>& lines) const
{
  if (painter->pen().style() != Qt::NoPen && painter->pen().color().alpha() != 0)
  {
    applyDefaultAntialiasingHint(painter);
    drawPolyline(painter, lines);
  }
}

int GPPolygon::getRegion(double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const
{
  if (key < keyMin) // region 123
  {
    if (value > valueMax)
      return 1;
    if (value < valueMin)
      return 3;
    else
      return 2;
  }
  else if (key > keyMax) // region 789
  {
    if (value > valueMax)
      return 7;
    if (value < valueMin)
      return 9;
    else
      return 8;
  }
  else // region 456
  {
    if (value > valueMax)
      return 4;
    if (value < valueMin)
      return 6;
    else
      return 5;
  }
}

void GPPolygon::getCurveLines(QVector<QPointF>* lines, const GPDataRange& dataRange, double penWidth) const
{
  if (!lines)
    return;

  lines->clear();
  GPAxis *keyAxis = mKeyAxis.data();
  GPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis
      || keyAxis->range().size() < std::numeric_limits<double>::epsilon()
      || valueAxis->range().size() < std::numeric_limits<double>::epsilon())
  {
    return;
  }

  // add margins to rect to compensate for stroke width
  const double strokeMargin = qMax(qreal(1.0), qreal(penWidth*0.75)); // stroke radius + 50% safety
  const double keyMin = keyAxis->pixelToCoord(keyAxis->coordToPixel(keyAxis->range().lower) - strokeMargin * keyAxis->pixelOrientation());
  const double keyMax = keyAxis->pixelToCoord(keyAxis->coordToPixel(keyAxis->range().upper) + strokeMargin * keyAxis->pixelOrientation());
  const double valueMin = valueAxis->pixelToCoord(valueAxis->coordToPixel(valueAxis->range().lower) - strokeMargin * valueAxis->pixelOrientation());
  const double valueMax = valueAxis->pixelToCoord(valueAxis->coordToPixel(valueAxis->range().upper) + strokeMargin * valueAxis->pixelOrientation());
  GPCurveDataContainer::const_iterator itBegin = mDataContainer->constBegin();
  GPCurveDataContainer::const_iterator itEnd = mDataContainer->constEnd();
  mDataContainer->limitIteratorsToDataRange(itBegin, itEnd, dataRange);
  if (itBegin == itEnd)
    return;
  GPCurveDataContainer::const_iterator it = itBegin;
  GPCurveDataContainer::const_iterator prevIt = itEnd - 1;
  int prevRegion = getRegion(prevIt->key, prevIt->value, keyMin, valueMax, keyMax, valueMin);
  QVector<QPointF> trailingPoints; // points that must be applied after all other points (are generated only when handling first point to get virtual segment between last and first point right)
  while (it != itEnd)
  {
    const int currentRegion = getRegion(it->key, it->value, keyMin, valueMax, keyMax, valueMin);
    if (currentRegion != prevRegion) // changed region, possibly need to add some optimized edge points or original points if entering R
    {
      if (currentRegion != 5) // segment doesn't end in R, so it's a candidate for removal
      {
        QPointF crossA, crossB;
        if (prevRegion == 5) // we're coming from R, so add this point optimized
        {
          lines->append(getOptimizedPoint(currentRegion, it->key, it->value, prevIt->key, prevIt->value, keyMin, valueMax, keyMax, valueMin));
          // in the situations 5->1/7/9/3 the segment may leave R and directly cross through two outer regions. In these cases we need to add an additional corner point
          *lines << getOptimizedCornerPoints(prevRegion, currentRegion, prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin);
        }
        else if (mayTraverse(prevRegion, currentRegion) &&
                 getTraverse(prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin, crossA, crossB))
        {
          // add the two cross points optimized if segment crosses R and if segment isn't virtual zeroth segment between last and first curve point:
          QVector<QPointF> beforeTraverseCornerPoints, afterTraverseCornerPoints;
          getTraverseCornerPoints(prevRegion, currentRegion, keyMin, valueMax, keyMax, valueMin, beforeTraverseCornerPoints, afterTraverseCornerPoints);
          if (it != itBegin)
          {
            *lines << beforeTraverseCornerPoints;
            lines->append(crossA);
            lines->append(crossB);
            *lines << afterTraverseCornerPoints;
          }
          else
          {
            lines->append(crossB);
            *lines << afterTraverseCornerPoints;
            trailingPoints << beforeTraverseCornerPoints << crossA;
          }
        }
        else // doesn't cross R, line is just moving around in outside regions, so only need to add optimized point(s) at the boundary corner(s)
        {
          *lines << getOptimizedCornerPoints(prevRegion, currentRegion, prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin);
        }
      }
      else // segment does end in R, so we add previous point optimized and this point at original position
      {
        if (it == itBegin) // it is first point in curve and prevIt is last one. So save optimized point for adding it to the lineData in the end
          trailingPoints << getOptimizedPoint(prevRegion, prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin);
        else
          lines->append(getOptimizedPoint(prevRegion, prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin));
        lines->append(coordsToPixels(it->key, it->value));
      }
    }
    else // region didn't change
    {
      if (currentRegion == 5) // still in R, keep adding original points
      {
        lines->append(coordsToPixels(it->key, it->value));
      }
      else // still outside R, no need to add anything
      {
        // see how this is not doing anything? That's the main optimization...
      }
    }
    prevIt = it;
    prevRegion = currentRegion;
    ++it;
  }
  *lines << trailingPoints;
}

QPointF GPPolygon::getOptimizedPoint(int otherRegion, double otherKey, double otherValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const
{
  const double keyMinPx = mKeyAxis->coordToPixel(keyMin);
  const double keyMaxPx = mKeyAxis->coordToPixel(keyMax);
  const double valueMinPx = mValueAxis->coordToPixel(valueMin);
  const double valueMaxPx = mValueAxis->coordToPixel(valueMax);
  const double otherValuePx = mValueAxis->coordToPixel(otherValue);
  const double valuePx = mValueAxis->coordToPixel(value);
  const double otherKeyPx = mKeyAxis->coordToPixel(otherKey);
  const double keyPx = mKeyAxis->coordToPixel(key);
  double intersectKeyPx = keyMinPx; // initial key just a fail-safe
  double intersectValuePx = valueMinPx; // initial value just a fail-safe
  switch (otherRegion)
  {
    case 1: // top and left edge
      {
        intersectValuePx = valueMaxPx;
        intersectKeyPx = otherKeyPx + (keyPx - otherKeyPx) / (valuePx - otherValuePx)*(intersectValuePx - otherValuePx);
        if (intersectKeyPx < qMin(keyMinPx, keyMaxPx) || intersectKeyPx > qMax(keyMinPx, keyMaxPx)) // check whether top edge is not intersected, then it must be left edge (qMin/qMax necessary since axes may be reversed)
        {
          intersectKeyPx = keyMinPx;
          intersectValuePx = otherValuePx + (valuePx - otherValuePx) / (keyPx - otherKeyPx)*(intersectKeyPx - otherKeyPx);
        }
        break;
      }
    case 2: // left edge
      {
        intersectKeyPx = keyMinPx;
        intersectValuePx = otherValuePx + (valuePx - otherValuePx) / (keyPx - otherKeyPx)*(intersectKeyPx - otherKeyPx);
        break;
      }
    case 3: // bottom and left edge
      {
        intersectValuePx = valueMinPx;
        intersectKeyPx = otherKeyPx + (keyPx - otherKeyPx) / (valuePx - otherValuePx)*(intersectValuePx - otherValuePx);
        if (intersectKeyPx < qMin(keyMinPx, keyMaxPx) || intersectKeyPx > qMax(keyMinPx, keyMaxPx)) // check whether bottom edge is not intersected, then it must be left edge (qMin/qMax necessary since axes may be reversed)
        {
          intersectKeyPx = keyMinPx;
          intersectValuePx = otherValuePx + (valuePx - otherValuePx) / (keyPx - otherKeyPx)*(intersectKeyPx - otherKeyPx);
        }
        break;
      }
    case 4: // top edge
      {
        intersectValuePx = valueMaxPx;
        intersectKeyPx = otherKeyPx + (keyPx - otherKeyPx) / (valuePx - otherValuePx)*(intersectValuePx - otherValuePx);
        break;
      }
    case 5:
      {
        break; // case 5 shouldn't happen for this function but we add it anyway to prevent potential discontinuity in branch table
      }
    case 6: // bottom edge
      {
        intersectValuePx = valueMinPx;
        intersectKeyPx = otherKeyPx + (keyPx - otherKeyPx) / (valuePx - otherValuePx)*(intersectValuePx - otherValuePx);
        break;
      }
    case 7: // top and right edge
      {
        intersectValuePx = valueMaxPx;
        intersectKeyPx = otherKeyPx + (keyPx - otherKeyPx) / (valuePx - otherValuePx)*(intersectValuePx - otherValuePx);
        if (intersectKeyPx < qMin(keyMinPx, keyMaxPx) || intersectKeyPx > qMax(keyMinPx, keyMaxPx)) // check whether top edge is not intersected, then it must be right edge (qMin/qMax necessary since axes may be reversed)
        {
          intersectKeyPx = keyMaxPx;
          intersectValuePx = otherValuePx + (valuePx - otherValuePx) / (keyPx - otherKeyPx)*(intersectKeyPx - otherKeyPx);
        }
        break;
      }
    case 8: // right edge
      {
        intersectKeyPx = keyMaxPx;
        intersectValuePx = otherValuePx + (valuePx - otherValuePx) / (keyPx - otherKeyPx)*(intersectKeyPx - otherKeyPx);
        break;
      }
    case 9: // bottom and right edge
      {
        intersectValuePx = valueMinPx;
        intersectKeyPx = otherKeyPx + (keyPx - otherKeyPx) / (valuePx - otherValuePx)*(intersectValuePx - otherValuePx);
        if (intersectKeyPx < qMin(keyMinPx, keyMaxPx) || intersectKeyPx > qMax(keyMinPx, keyMaxPx)) // check whether bottom edge is not intersected, then it must be right edge (qMin/qMax necessary since axes may be reversed)
        {
          intersectKeyPx = keyMaxPx;
          intersectValuePx = otherValuePx + (valuePx - otherValuePx) / (keyPx - otherKeyPx)*(intersectKeyPx - otherKeyPx);
        }
        break;
      }
  }
  if (mKeyAxis->orientation() == Qt::Horizontal)
    return QPointF(intersectKeyPx, intersectValuePx);
  return QPointF(intersectValuePx, intersectKeyPx);
}

QVector<QPointF> GPPolygon::getOptimizedCornerPoints(int prevRegion, int currentRegion, double prevKey, double prevValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const
{
  QVector<QPointF> result;
  switch (prevRegion)
  {
    case 1:
      {
        switch (currentRegion)
        {
          case 2: { result << coordsToPixels(keyMin, valueMax); break; }
          case 4: { result << coordsToPixels(keyMin, valueMax); break; }
          case 3: { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMin, valueMin); break; }
          case 7: { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMax, valueMax); break; }
          case 6: { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMin, valueMin); result.append(result.last()); break; }
          case 8: { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMax, valueMax); result.append(result.last()); break; }
          case 9: { // in this case we need another distinction of cases: segment may pass below or above rect, requiring either bottom right or top left corner points
              if ((value - prevValue) / (key - prevKey)*(keyMin - key) + value < valueMin) // segment passes below R
              {
                result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMin, valueMin); result.append(result.last()); result << coordsToPixels(keyMax, valueMin);
              }
              else
              {
                result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMax, valueMax); result.append(result.last()); result << coordsToPixels(keyMax, valueMin);
              }
              break;
            }
        }
        break;
      }
    case 2:
      {
        switch (currentRegion)
        {
          case 1: { result << coordsToPixels(keyMin, valueMax); break; }
          case 3: { result << coordsToPixels(keyMin, valueMin); break; }
          case 4: { result << coordsToPixels(keyMin, valueMax); result.append(result.last()); break; }
          case 6: { result << coordsToPixels(keyMin, valueMin); result.append(result.last()); break; }
          case 7: { result << coordsToPixels(keyMin, valueMax); result.append(result.last()); result << coordsToPixels(keyMax, valueMax); break; }
          case 9: { result << coordsToPixels(keyMin, valueMin); result.append(result.last()); result << coordsToPixels(keyMax, valueMin); break; }
        }
        break;
      }
    case 3:
      {
        switch (currentRegion)
        {
          case 2: { result << coordsToPixels(keyMin, valueMin); break; }
          case 6: { result << coordsToPixels(keyMin, valueMin); break; }
          case 1: { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMin, valueMax); break; }
          case 9: { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMax, valueMin); break; }
          case 4: { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMin, valueMax); result.append(result.last()); break; }
          case 8: { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMax, valueMin); result.append(result.last()); break; }
          case 7: { // in this case we need another distinction of cases: segment may pass below or above rect, requiring either bottom right or top left corner points
              if ((value - prevValue) / (key - prevKey)*(keyMax - key) + value < valueMin) // segment passes below R
              {
                result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMax, valueMin); result.append(result.last()); result << coordsToPixels(keyMax, valueMax);
              }
              else
              {
                result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMin, valueMax); result.append(result.last()); result << coordsToPixels(keyMax, valueMax);
              }
              break;
            }
        }
        break;
      }
    case 4:
      {
        switch (currentRegion)
        {
          case 1: { result << coordsToPixels(keyMin, valueMax); break; }
          case 7: { result << coordsToPixels(keyMax, valueMax); break; }
          case 2: { result << coordsToPixels(keyMin, valueMax); result.append(result.last()); break; }
          case 8: { result << coordsToPixels(keyMax, valueMax); result.append(result.last()); break; }
          case 3: { result << coordsToPixels(keyMin, valueMax); result.append(result.last()); result << coordsToPixels(keyMin, valueMin); break; }
          case 9: { result << coordsToPixels(keyMax, valueMax); result.append(result.last()); result << coordsToPixels(keyMax, valueMin); break; }
        }
        break;
      }
    case 5:
      {
        switch (currentRegion)
        {
          case 1: { result << coordsToPixels(keyMin, valueMax); break; }
          case 7: { result << coordsToPixels(keyMax, valueMax); break; }
          case 9: { result << coordsToPixels(keyMax, valueMin); break; }
          case 3: { result << coordsToPixels(keyMin, valueMin); break; }
        }
        break;
      }
    case 6:
      {
        switch (currentRegion)
        {
          case 3: { result << coordsToPixels(keyMin, valueMin); break; }
          case 9: { result << coordsToPixels(keyMax, valueMin); break; }
          case 2: { result << coordsToPixels(keyMin, valueMin); result.append(result.last()); break; }
          case 8: { result << coordsToPixels(keyMax, valueMin); result.append(result.last()); break; }
          case 1: { result << coordsToPixels(keyMin, valueMin); result.append(result.last()); result << coordsToPixels(keyMin, valueMax); break; }
          case 7: { result << coordsToPixels(keyMax, valueMin); result.append(result.last()); result << coordsToPixels(keyMax, valueMax); break; }
        }
        break;
      }
    case 7:
      {
        switch (currentRegion)
        {
          case 4: { result << coordsToPixels(keyMax, valueMax); break; }
          case 8: { result << coordsToPixels(keyMax, valueMax); break; }
          case 1: { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMin, valueMax); break; }
          case 9: { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMax, valueMin); break; }
          case 2: { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMin, valueMax); result.append(result.last()); break; }
          case 6: { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMax, valueMin); result.append(result.last()); break; }
          case 3: { // in this case we need another distinction of cases: segment may pass below or above rect, requiring either bottom right or top left corner points
              if ((value - prevValue) / (key - prevKey)*(keyMax - key) + value < valueMin) // segment passes below R
              {
                result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMax, valueMin); result.append(result.last()); result << coordsToPixels(keyMin, valueMin);
              }
              else
              {
                result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMin, valueMax); result.append(result.last()); result << coordsToPixels(keyMin, valueMin);
              }
              break;
            }
        }
        break;
      }
    case 8:
      {
        switch (currentRegion)
        {
          case 7: { result << coordsToPixels(keyMax, valueMax); break; }
          case 9: { result << coordsToPixels(keyMax, valueMin); break; }
          case 4: { result << coordsToPixels(keyMax, valueMax); result.append(result.last()); break; }
          case 6: { result << coordsToPixels(keyMax, valueMin); result.append(result.last()); break; }
          case 1: { result << coordsToPixels(keyMax, valueMax); result.append(result.last()); result << coordsToPixels(keyMin, valueMax); break; }
          case 3: { result << coordsToPixels(keyMax, valueMin); result.append(result.last()); result << coordsToPixels(keyMin, valueMin); break; }
        }
        break;
      }
    case 9:
      {
        switch (currentRegion)
        {
          case 6: { result << coordsToPixels(keyMax, valueMin); break; }
          case 8: { result << coordsToPixels(keyMax, valueMin); break; }
          case 3: { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMin, valueMin); break; }
          case 7: { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMax, valueMax); break; }
          case 2: { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMin, valueMin); result.append(result.last()); break; }
          case 4: { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMax, valueMax); result.append(result.last()); break; }
          case 1: { // in this case we need another distinction of cases: segment may pass below or above rect, requiring either bottom right or top left corner points
              if ((value - prevValue) / (key - prevKey)*(keyMin - key) + value < valueMin) // segment passes below R
              {
                result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMin, valueMin); result.append(result.last()); result << coordsToPixels(keyMin, valueMax);
              }
              else
              {
                result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMax, valueMax); result.append(result.last()); result << coordsToPixels(keyMin, valueMax);
              }
              break;
            }
        }
        break;
      }
  }
  return std::move(result);
}

bool GPPolygon::mayTraverse(int prevRegion, int currentRegion) const
{
  switch (prevRegion)
  {
    case 1:
      {
        switch (currentRegion)
        {
          case 4:
          case 7:
          case 2:
          case 3: return false;
          default: return true;
        }
      }
    case 2:
      {
        switch (currentRegion)
        {
          case 1:
          case 3: return false;
          default: return true;
        }
      }
    case 3:
      {
        switch (currentRegion)
        {
          case 1:
          case 2:
          case 6:
          case 9: return false;
          default: return true;
        }
      }
    case 4:
      {
        switch (currentRegion)
        {
          case 1:
          case 7: return false;
          default: return true;
        }
      }
    case 5: return false; // should never occur
    case 6:
      {
        switch (currentRegion)
        {
          case 3:
          case 9: return false;
          default: return true;
        }
      }
    case 7:
      {
        switch (currentRegion)
        {
          case 1:
          case 4:
          case 8:
          case 9: return false;
          default: return true;
        }
      }
    case 8:
      {
        switch (currentRegion)
        {
          case 7:
          case 9: return false;
          default: return true;
        }
      }
    case 9:
      {
        switch (currentRegion)
        {
          case 3:
          case 6:
          case 8:
          case 7: return false;
          default: return true;
        }
      }
    default: return true;
  }
}

bool GPPolygon::getTraverse(double prevKey, double prevValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin, QPointF &crossA, QPointF &crossB) const
{
  QList<QPointF> intersections;
  const double valueMinPx = mValueAxis->coordToPixel(valueMin);
  const double valueMaxPx = mValueAxis->coordToPixel(valueMax);
  const double keyMinPx = mKeyAxis->coordToPixel(keyMin);
  const double keyMaxPx = mKeyAxis->coordToPixel(keyMax);
  const double keyPx = mKeyAxis->coordToPixel(key);
  const double valuePx = mValueAxis->coordToPixel(value);
  const double prevKeyPx = mKeyAxis->coordToPixel(prevKey);
  const double prevValuePx = mValueAxis->coordToPixel(prevValue);
  if (qFuzzyIsNull(key - prevKey)) // line is parallel to value axis
  {
    // due to region filter in mayTraverse(), if line is parallel to value or key axis, region 5 is traversed here
    intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyPx, valueMinPx) : QPointF(valueMinPx, keyPx)); // direction will be taken care of at end of method
    intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyPx, valueMaxPx) : QPointF(valueMaxPx, keyPx));
  }
  else if (qFuzzyIsNull(value - prevValue)) // line is parallel to key axis
  {
    // due to region filter in mayTraverse(), if line is parallel to value or key axis, region 5 is traversed here
    intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyMinPx, valuePx) : QPointF(valuePx, keyMinPx)); // direction will be taken care of at end of method
    intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyMaxPx, valuePx) : QPointF(valuePx, keyMaxPx));
  }
  else // line is skewed
  {
    double gamma;
    double keyPerValuePx = (keyPx - prevKeyPx) / (valuePx - prevValuePx);
    // check top of rect:
    gamma = prevKeyPx + (valueMaxPx - prevValuePx)*keyPerValuePx;
    if (gamma >= qMin(keyMinPx, keyMaxPx) && gamma <= qMax(keyMinPx, keyMaxPx)) // qMin/qMax necessary since axes may be reversed
      intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(gamma, valueMaxPx) : QPointF(valueMaxPx, gamma));
    // check bottom of rect:
    gamma = prevKeyPx + (valueMinPx - prevValuePx)*keyPerValuePx;
    if (gamma >= qMin(keyMinPx, keyMaxPx) && gamma <= qMax(keyMinPx, keyMaxPx)) // qMin/qMax necessary since axes may be reversed
      intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(gamma, valueMinPx) : QPointF(valueMinPx, gamma));
    const double valuePerKeyPx = 1.0 / keyPerValuePx;
    // check left of rect:
    gamma = prevValuePx + (keyMinPx - prevKeyPx)*valuePerKeyPx;
    if (gamma >= qMin(valueMinPx, valueMaxPx) && gamma <= qMax(valueMinPx, valueMaxPx)) // qMin/qMax necessary since axes may be reversed
      intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyMinPx, gamma) : QPointF(gamma, keyMinPx));
    // check right of rect:
    gamma = prevValuePx + (keyMaxPx - prevKeyPx)*valuePerKeyPx;
    if (gamma >= qMin(valueMinPx, valueMaxPx) && gamma <= qMax(valueMinPx, valueMaxPx)) // qMin/qMax necessary since axes may be reversed
      intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyMaxPx, gamma) : QPointF(gamma, keyMaxPx));
  }

  // handle cases where found points isn't exactly 2:
  if (intersections.size() > 2)
  {
    // line probably goes through corner of rect, and we got duplicate points there. single out the point pair with greatest distance in between:
    double distSqrMax = 0;
    QPointF pv1, pv2;
    for (int i = 0; i<intersections.size() - 1; ++i)
    {
      for (int k = i + 1; k<intersections.size(); ++k)
      {
        QPointF distPoint = intersections.at(i) - intersections.at(k);
        double distSqr = distPoint.x()*distPoint.x() + distPoint.y() + distPoint.y();
        if (distSqr > distSqrMax)
        {
          pv1 = intersections.at(i);
          pv2 = intersections.at(k);
          distSqrMax = distSqr;
        }
      }
    }
    intersections = QList<QPointF>() << pv1 << pv2;
  }
  else if (intersections.size() != 2)
  {
    // one or even zero points found (shouldn't happen unless line perfectly tangent to corner), no need to draw segment
    return false;
  }

  // possibly re-sort points so optimized point segment has same direction as original segment:
  double xDelta = keyPx - prevKeyPx;
  double yDelta = valuePx - prevValuePx;
  if (mKeyAxis->orientation() != Qt::Horizontal)
    qSwap(xDelta, yDelta);
  if (xDelta*(intersections.at(1).x() - intersections.at(0).x()) + yDelta * (intersections.at(1).y() - intersections.at(0).y()) < 0) // scalar product of both segments < 0 -> opposite direction
    intersections.move(0, 1);
  crossA = intersections.at(0);
  crossB = intersections.at(1);
  return true;
}

void GPPolygon::getTraverseCornerPoints(int prevRegion, int currentRegion, double keyMin, double valueMax, double keyMax, double valueMin, QVector<QPointF> &beforeTraverse, QVector<QPointF> &afterTraverse) const
{
  switch (prevRegion)
  {
    case 1:
      {
        switch (currentRegion)
        {
          case 6: { beforeTraverse << coordsToPixels(keyMin, valueMax); break; }
          case 9: { beforeTraverse << coordsToPixels(keyMin, valueMax); afterTraverse << coordsToPixels(keyMax, valueMin); break; }
          case 8: { beforeTraverse << coordsToPixels(keyMin, valueMax); break; }
        }
        break;
      }
    case 2:
      {
        switch (currentRegion)
        {
          case 7: { afterTraverse << coordsToPixels(keyMax, valueMax); break; }
          case 9: { afterTraverse << coordsToPixels(keyMax, valueMin); break; }
        }
        break;
      }
    case 3:
      {
        switch (currentRegion)
        {
          case 4: { beforeTraverse << coordsToPixels(keyMin, valueMin); break; }
          case 7: { beforeTraverse << coordsToPixels(keyMin, valueMin); afterTraverse << coordsToPixels(keyMax, valueMax); break; }
          case 8: { beforeTraverse << coordsToPixels(keyMin, valueMin); break; }
        }
        break;
      }
    case 4:
      {
        switch (currentRegion)
        {
          case 3: { afterTraverse << coordsToPixels(keyMin, valueMin); break; }
          case 9: { afterTraverse << coordsToPixels(keyMax, valueMin); break; }
        }
        break;
      }
    case 5: { break; } // shouldn't happen because this method only handles full traverses
    case 6:
      {
        switch (currentRegion)
        {
          case 1: { afterTraverse << coordsToPixels(keyMin, valueMax); break; }
          case 7: { afterTraverse << coordsToPixels(keyMax, valueMax); break; }
        }
        break;
      }
    case 7:
      {
        switch (currentRegion)
        {
          case 2: { beforeTraverse << coordsToPixels(keyMax, valueMax); break; }
          case 3: { beforeTraverse << coordsToPixels(keyMax, valueMax); afterTraverse << coordsToPixels(keyMin, valueMin); break; }
          case 6: { beforeTraverse << coordsToPixels(keyMax, valueMax); break; }
        }
        break;
      }
    case 8:
      {
        switch (currentRegion)
        {
          case 1: { afterTraverse << coordsToPixels(keyMin, valueMax); break; }
          case 3: { afterTraverse << coordsToPixels(keyMin, valueMin); break; }
        }
        break;
      }
    case 9:
      {
        switch (currentRegion)
        {
          case 2: { beforeTraverse << coordsToPixels(keyMax, valueMin); break; }
          case 1: { beforeTraverse << coordsToPixels(keyMax, valueMin); afterTraverse << coordsToPixels(keyMin, valueMax); break; }
          case 4: { beforeTraverse << coordsToPixels(keyMax, valueMin); break; }
        }
        break;
      }
  }
}

double GPPolygon::pointDistance(const QPointF &pixelPoint, GPCurveDataContainer::const_iterator &closestData) const
{
  closestData = mDataContainer->constEnd();
  if (mDataContainer->isEmpty())
    return -1.0;

  if (mDataContainer->size() == 1)
  {
    QPointF dataPoint = coordsToPixels(mDataContainer->constBegin()->key, mDataContainer->constBegin()->value);
    closestData = mDataContainer->constBegin();
    return GPVector2D(dataPoint - pixelPoint).length();
  }

  // calculate minimum distances to curve data points and find closestData iterator:
  double minDistSqr = std::numeric_limits<double>::max();

  // iterate over found data points and then choose the one with the shortest distance to pos:
  GPCurveDataContainer::const_iterator begin = mDataContainer->constBegin();
  GPCurveDataContainer::const_iterator end = mDataContainer->constEnd();
  for (GPCurveDataContainer::const_iterator it = begin; it != end; ++it)
  {
    const double currentDistSqr = GPVector2D(coordsToPixels(it->key, it->value) - pixelPoint).lengthSquared();
    if (currentDistSqr < minDistSqr)
    {
      minDistSqr = currentDistSqr;
      closestData = it;
    }
  }

  // calculate distance to line if there is one (if so, will probably be smaller than distance to closest data point):
  if (true)
  {
    QVector<QPointF> lines;
    getCurveLines(&lines, GPDataRange(0, dataCount()), mParentPlot->selectionTolerance()*1.2); // optimized lines outside axis rect shouldn't respond to clicks at the edge, so use 1.2*tolerance as pen width
    for (int i = 0; i<lines.size() - 1; ++i)
    {
      double currentDistSqr = GPVector2D(pixelPoint).distanceSquaredToLine(lines.at(i), lines.at(i + 1));
      if (currentDistSqr < minDistSqr)
        minDistSqr = currentDistSqr;
    }
  }

  return qSqrt(minDistSqr);
}

///////////////////////////////////////////////////////
// Polygon group
GPPolygonGroup::GPPolygonGroup(GraphicsPlot* parentPlot, GPAxis* keyAxis, GPAxis* valueAxis)
  : GPLayerable(parentPlot, "polygons")
  , KeyAxs(keyAxis)
  , ValueAxs(valueAxis)
{
}

GPPolygonGroup::~GPPolygonGroup()
{
}

GPPolygon* GPPolygonGroup::addPolygon(GPAxis* kAxis, GPAxis* vAxis)
{
  QPointer<GPPolygon> polygon(new GPPolygon(kAxis ? kAxis : keyAxis(),
                                              vAxis ? vAxis : valueAxis(),
                                              this));
  polygon->setSelectable(GP::stNone);

  if (Pen)
    polygon->setPen(*Pen);
  if (Brush)
    polygon->setBrush(*Brush);

  Polygons.insert(polygon);
  return polygon.data();
}

GPPolygon* GPPolygonGroup::getPolygon(double leftKey, double topValue)
{
  decltype(Index)::iterator i = Index.find(std::pair<double, double>(leftKey, topValue));
  if (i != Index.end())
  {
    return i.value().data();
  }
  return nullptr;
}

GPPolygon *GPPolygonGroup::getDefaultPolygon()
{
  if (Polygons.empty())
    addPolygon();
  return *Polygons.begin();
}

void GPPolygonGroup::registerPolygon(double leftKey, double topValue, GPPolygon* polygon)
{
  Index[std::pair<double, double>(leftKey, topValue)] = polygon;
}

void GPPolygonGroup::removePolygon(GPPolygon* polygon)
{
  decltype(Polygons)::iterator p = std::find_if(Polygons.begin(),
                                                Polygons.end(),
                                                [polygon](const QPointer<GPPolygon>& pg) -> bool { return pg.data() == polygon; } );
  if (p != Polygons.end())
  {
    parentPlot()->removePlottable(p->data());
    Polygons.erase(p);
  }
}

void GPPolygonGroup::clear()
{
  decltype(Polygons)::iterator p = Polygons.begin();
  while (p != Polygons.end())
  {
    parentPlot()->removePlottable(p->data());
    ++p;
  }
  Polygons.clear();
}

size_t GPPolygonGroup::size()
{
  return Polygons.size();
}

void GPPolygonGroup::setPen(const QPen& pen)
{
  Pen.reset(new QPen(pen));
}

void GPPolygonGroup::setBrush(const QBrush& brush)
{
  Brush.reset(new QBrush(brush));
}

void GPPolygonGroup::applyDefaultAntialiasingHint(GPPainter* /*painter*/) const
{
}

void GPPolygonGroup::draw(GPPainter* /*painter*/)
{
}

GPAxis* GPPolygonGroup::keyAxis()
{
  return KeyAxs.data();
}

GPAxis* GPPolygonGroup::valueAxis()
{
  return ValueAxs.data();
}

///////////////////////////////////////////////////////
// Polygon capable layer
GPPolygonLayer::GPPolygonLayer(GraphicsPlot* parentPlot, const QString& layerName)
  : GPLayer(parentPlot, layerName)
{
}

GPPolygonLayer::~GPPolygonLayer()
{
}

GPPolygonGroup* GPPolygonLayer::addPolygonGroup(GPAxis* keyAxis, GPAxis* valueAxis)
{
  if (!keyAxis)
  {
    if (parentPlot()->xAxis)
      keyAxis = parentPlot()->xAxis;
    else if (parentPlot()->xAxis2)
      keyAxis = parentPlot()->xAxis2;
  }
  if (!valueAxis)
  {
    if (parentPlot()->yAxis)
      valueAxis = parentPlot()->yAxis;
    else if (parentPlot()->yAxis2)
      valueAxis = parentPlot()->yAxis2;
  }

  QSharedPointer<GPPolygonGroup> group(new GPPolygonGroup(parentPlot(),
                                                                keyAxis,
                                                                valueAxis));
  PolygonGroups.push_back(group);
  return group.data();
}

void GPPolygonLayer::removePolygonGroup(GPPolygonGroup* group)
{
  decltype(PolygonGroups)::iterator g = std::find_if(PolygonGroups.begin(),
                                                     PolygonGroups.end(),
                                                     [group](const QSharedPointer<GPPolygonGroup>& gr) -> bool { return gr.data() == group; } );
  if (g != PolygonGroups.end())
  {
    auto pg = *g;
    pg->clear();
    PolygonGroups.removeOne(pg);
  }
}


///////////////////////////////////////////////////////
// Extended color map
GPEXColorMap::GPEXColorMap(GPAxis *keyAxis, GPAxis *valueAxis)
  : GPColorMap(keyAxis, valueAxis)
  , mContour(new GPPolygon(keyAxis, valueAxis, this))
{
  mContour->setPen(QPen(QColor("#969696"), 2, Qt::DashLine));
  mParentPlot->installEventFilter(this);
}

GPEXColorMap::~GPEXColorMap()
{
}

void GPEXColorMap::clear()
{
  mMapData->clear();
  mContour->setData(QVector<double>(), QVector<double>());
  mClipPolygon.clear();
  mMapImageInvalidated = true;
}

void GPEXColorMap::setContourVisible(bool visible)
{
  mContour->setVisible(visible);
}

bool GPEXColorMap::contourVisible()
{
  return mContour->visible();
}

void GPEXColorMap::updateMapImage()
{
  GPColorMap::updateMapImage();
  updateContour();
}

void GPEXColorMap::updateContour()
{
  double cellW = mMapData->keyRange().size() / (double)mMapData->keySize();
  double cellH = mMapData->valueRange().size() / (double)mMapData->valueSize();

  QMap<int, QPair<int, int>> dataRanges;

  for (int v = 0; v < mMapData->valueSize(); ++v)
  {
    double min = qQNaN();
    double max = qQNaN();
    for (int k = 0; k < mMapData->keySize(); ++k)
    {
      double d = mMapData->cell(k, v);
      if (!qIsNaN(d))
      {
        if (qIsNaN(min))
          min = k;
        max = k;
      }
    }
    if (!qIsNaN(min) && !qIsNaN(max))
    {
      dataRanges[v] = QPair<int, int>(min, max);
    }
  }

  QVector<double> x;
  QVector<double> y;
  if (!dataRanges.isEmpty())
  {
    auto keys = dataRanges.keys();
    int lastX = dataRanges.begin()->first;
    for (auto it = keys.begin(); it != keys.end(); ++it)
    {
      x << mMapData->keyRange().lower + cellW * lastX;
      y << mMapData->valueRange().lower + cellH * (*it);

      x << mMapData->keyRange().lower + cellW * (dataRanges[*it].second + 1);
      y << mMapData->valueRange().lower + cellH * (*it);

      lastX = dataRanges[*it].second + 1;
    }

    x << mMapData->keyRange().lower + cellW * lastX;
    y << mMapData->valueRange().lower + cellH * (keys.last() + 1);

    lastX = dataRanges[keys.last()].second;
    for (auto it = keys.rbegin(); it != keys.rend(); ++it)
    {
      x << mMapData->keyRange().lower + cellW * lastX;
      y << mMapData->valueRange().lower + cellH * ((*it) + 1);

      x << mMapData->keyRange().lower + cellW * dataRanges[*it].first;
      y << mMapData->valueRange().lower + cellH * ((*it) + 1);

      lastX = dataRanges[*it].first;
    }
  }
  mContour->data()->setInputUomForKeys(mMapData->listenerKeys.uomDescriptor);
  mContour->data()->setInputUomForValues(mMapData->listenerValues.uomDescriptor);
  mContour->setData(x, y);
  mClipPolygon.clear();
  for (int i = 0; i < x.size(); ++i)
    mClipPolygon.append(QPointF(x[i], y[i]));
}

bool GPEXColorMap::eventFilter(QObject *watched, QEvent *event)
{
  if (watched == mParentPlot)
  {
    if (((event->type() == QEvent::MouseMove) && (QApplication::queryKeyboardModifiers() & Qt::ControlModifier))
        || (event->type() == QEvent::KeyPress) || (event->type() == QEvent::KeyRelease))
    {
      if (!mClipPolygon.isEmpty())
      {
        QPoint pos = mParentPlot->mapFromGlobal(QCursor::pos());
        QPolygon p;
        for (int i = 0; i < mClipPolygon.size(); ++i)
          p << coordsToPixels(mClipPolygon[i].x(), mClipPolygon[i].y()).toPoint();
        if (QRegion(p).contains(pos))
        {
          if (event->type() == QEvent::MouseMove)
          {
            ShowValueToolTip(pos);
          }
          else
          {
            auto keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Control)
            {
              if (event->type() == QEvent::KeyPress)
              {
                ShowValueToolTip(pos);
                return false;
              }
            }
          }
        }
        QToolTip::hideText();
        setProperty("showing_x", -1);
        setProperty("showing_y", -1);
      }
    }
  }
  return false;
}

void GPEXColorMap::ShowValueToolTip(const QPoint &pos)
{
  double x = mKeyAxis->pixelToCoord(pos.x());
  double y = mValueAxis->pixelToCoord(pos.y());
  int i, j;
  mMapData->coordToCell(x, y, &i, &j);
  if (i >= 0 && i < mMapData->keySize() && j >= 0 && j < mMapData->valueSize())
  {
    if (i != property("showing_x").toInt() || j != property("showing_y").toInt())
    {
      double value = mMapData->cell(i, j);
      if (!qIsNaN(value) && !qIsInf(value) && value != -1)
      {
        QToolTip::showText(mParentPlot->mapToGlobal(pos), QString::number(value), mParentPlot);
        setProperty("showing_x", i);
        setProperty("showing_y", j);
      }
    }
    return;
  }
  QToolTip::hideText();
  setProperty("showing_x", -1);
  setProperty("showing_y", -1);
}

void GPEXColorMap::draw(GPPainter *painter)
{
  if (mMapData->isEmpty())
    return;
  if (!mKeyAxis || !mValueAxis)
    return;
  applyDefaultAntialiasingHint(painter);

  if (mMapData->mDataModified || mMapImageInvalidated)
    updateMapImage();

  // use buffer if painting vectorized (PDF):
  const bool useBuffer = painter->modes().testFlag(GPPainter::pmVectorized);
  GPPainter *localPainter = painter; // will be redirected to paint on mapBuffer if painting vectorized
  QRectF mapBufferTarget; // the rect in absolute widget coordinates where the visible map portion/buffer will end up in
  QPixmap mapBuffer;
  if (useBuffer)
  {
    const double mapBufferPixelRatio = 3; // factor by which DPI is increased in embedded bitmaps
    mapBufferTarget = painter->clipRegion().boundingRect();
    mapBuffer = QPixmap((mapBufferTarget.size()*mapBufferPixelRatio).toSize());
    mapBuffer.fill(Qt::transparent);
    localPainter = new GPPainter(&mapBuffer);
    localPainter->scale(mapBufferPixelRatio, mapBufferPixelRatio);
    localPainter->translate(-mapBufferTarget.topLeft());
  }

  QRect imageRect = QRect(coordsToPixels(mMapData->keyRange().lower, mMapData->valueRange().lower).toPoint(),
                            coordsToPixels(mMapData->keyRange().upper, mMapData->valueRange().upper).toPoint()).normalized();

  const bool mirrorX = (mKeyAxis->orientation() == Qt::Horizontal ? mKeyAxis : mValueAxis)->rangeReversed();
  const bool mirrorY = (mValueAxis->orientation() == Qt::Vertical ? mValueAxis : mKeyAxis)->rangeReversed();
  const bool smoothBackup = localPainter->renderHints().testFlag(QPainter::SmoothPixmapTransform);
  localPainter->setRenderHint(QPainter::SmoothPixmapTransform, mInterpolate);

  QRegion clipBackup;
  if (mTightBoundary)
  {
    clipBackup = localPainter->clipRegion();
    if (!mClipPolygon.isEmpty())
    {
      QPolygon p;
      for (int i = 0; i < mClipPolygon.size(); ++i)
        p << coordsToPixels(mClipPolygon[i].x(), mClipPolygon[i].y()).toPoint();
      localPainter->setClipRegion(QRegion(p), Qt::IntersectClip);
    }
    else
    {
      QRectF tightClipRect = QRectF(coordsToPixels(mMapData->keyRange().lower, mMapData->valueRange().lower),
                                    coordsToPixels(mMapData->keyRange().upper, mMapData->valueRange().upper)).normalized();
      localPainter->setClipRect(tightClipRect, Qt::IntersectClip);
    }
  }
  localPainter->drawImage(imageRect, mMapImage.mirrored(mirrorX, mirrorY));
  if (mTightBoundary)
    localPainter->setClipRegion(clipBackup);
  localPainter->setRenderHint(QPainter::SmoothPixmapTransform, smoothBackup);

  if (useBuffer) // localPainter painted to mapBuffer, so now draw buffer with original painter
  {
    delete localPainter;
    painter->drawPixmap(mapBufferTarget.toRect(), mapBuffer);
  }
}

///////////////////////////////////////////////////////
// Srv curve
GPSRVCurve::GPSRVCurve(GPAxis *keyAxis, GPAxis *valueAxis)
  : GPCurve(keyAxis, valueAxis, true)
  , mFilledByStress(false)
  , minX(qQNaN())
  , maxX(qQNaN())
  , minY(qQNaN())
  , maxY(qQNaN())
{
  setProperty("ExportUngroupped", true);

  setSelectable(GP::stNone);
  parentPlot()->setOpenGl(false);
  parentPlot()->setPlottingHint(GP::phFastPolylines);

  mPen = QPen(QColor("#447744"), 0.5, Qt::SolidLine);
}

GPSRVCurve::~GPSRVCurve()
{

}

void GPSRVCurve::setData(const QVector<double> &t,
                            const QVector<double> &x,
                            const QVector<double> &y,
                            const QVector<double> &a,
                            const QVector<double> &s,
                            const double &fractureLength,
                            unsigned dataType)
{
  clearData();

  auto checkMinMax = [this](const QPointF &point) {
    if (!qIsNaN(point.x()) && !qIsInf(point.x()))
    {
      if (point.x() < minX || qIsNaN(minX))
        minX = point.x();
      if (point.x() > maxX || qIsNaN(maxX))
        maxX = point.x();
    }

    if (!qIsNaN(point.y()) && !qIsInf(point.y()))
    {
      if (point.y() < minY || qIsNaN(minY))
        minY = point.y();
      if (point.y() > maxY || qIsNaN(maxY))
        maxY = point.y();
    }
  };

  if (!a.isEmpty())
  {
    mFilledByStress = false;
    mPoints.reserve(t.size() * 2);

    for (int i = 0; i < t.size(); ++i)
    {
      const double& px = x[i];
      const double& py = y[i];

      mPoints << rotatePoint(px, py, a[i], px, py + fractureLength / 2.0);
      mPoints << rotatePoint(px, py, a[i] + 180.0, px, py + fractureLength / 2.0);
    }
  }
  else if (!s.isEmpty())
  {
    mFilledByStress = true;
    mPoints.reserve(t.size() * 2);

    double SMin = s[0];
    double SMax = SMin;
    for (int i = 0; i < s.size(); ++i)
    {
      if (!qIsNaN(s[i]) && !qIsInf(s[i]))
      {
        if (SMin < s[i])
          SMin = s[i];
        if (SMax > s[i])
          SMax = s[i];
      }
    }

    for (int i = 0; i < t.size(); ++i)
    {
      const double& cx = x[i];
      const double& cy = y[i];

      double dy = (s[i] - SMin) / (SMax - SMin) * fractureLength / 2.0;
      double dx = abs(dy);
      if (dataType == DataTypeSMin)
        dy = -dy;

      mPoints << QPointF(cx - dx, cy);
      mPoints << QPointF(cx + dx, cy + dy);
    }
  }

  for (int i = 0; i < mPoints.size(); ++i)
    checkMinMax(mPoints[i]);

  //// Also, set data in inherited storage
  {
    QVector<double> t;
    t.reserve(mPoints.size() * 3 / 2 + 1);
    QVector<double> x;
    x.reserve(mPoints.size() * 3 / 2 + 1);
    QVector<double> y;
    y.reserve(mPoints.size() * 3 / 2 + 1);

    for (auto& p : mPoints)
    {
      Q_ASSERT(!std::isnan(p.x()));
      Q_ASSERT(!std::isnan(p.y()));

      x << p.x();
      y << p.y();
      t << t.size();

      /*if (t.size() % 2 == 0)
      {
        x << qQNaN();
        y << qQNaN();
        t << t.size();
      }*/
    }

    GPCurve::setData(t, x, y);
  }
}

void GPSRVCurve::clearData()
{
  mPoints.clear();
  minX = maxX = minY = maxY = qQNaN();

  GPCurve::data()->clear();
}

GPRange GPSRVCurve::getKeyRange(bool &foundRange, GP::SignDomain) const
{
  foundRange = !(qIsNaN(minX) || qIsNaN(maxX));
  return GPRange(minX, maxX);
}

GPRange GPSRVCurve::getValueRange(bool &foundRange, GP::SignDomain, const GPRange &) const
{
  foundRange = !(qIsNaN(minY) || qIsNaN(maxY));
  return GPRange(minY, maxY);
}

void GPSRVCurve::draw(GPPainter *painter)
{
  if (mPoints.isEmpty())
    return;

  painter->setPen(mPen);
  painter->setBrush(Qt::NoBrush);
  auto r = painter->clipBoundingRect();

  if (!mFilledByStress)
  {
    for (int i = 1; i < mPoints.size(); i += 2)
      drawLine(painter, mPoints[i - 1], mPoints[i], r);
  }
  else
  {
    for (int i = 1; i < mPoints.size(); i += 2)
      drawRect(painter, mPoints[i - 1], mPoints[i], r);
  }
}

void GPSRVCurve::drawLine(GPPainter *painter, QPointF p1, QPointF p2, const QRectF &clipRect)
{
  p1 = QPointF(mKeyAxis->coordToPixel(p1.x()), mValueAxis->coordToPixel(p1.y()));
  p2 = QPointF(mKeyAxis->coordToPixel(p2.x()), mValueAxis->coordToPixel(p2.y()));

  bool needPaint = false;
  needPaint |= clipRect.contains(p1);
  needPaint |= clipRect.contains(p2);

  if (needPaint)
    painter->drawLine(p1, p2);
}

void GPSRVCurve::drawRect(GPPainter *painter, QPointF p1, QPointF p2, const QRectF &clipRect)
{
  p1 = QPointF(mKeyAxis->coordToPixel(p1.x()), mValueAxis->coordToPixel(p1.y()));
  p2 = QPointF(mKeyAxis->coordToPixel(p2.x()), mValueAxis->coordToPixel(p2.y()));

  bool needPaint = false;
  needPaint |= clipRect.contains(p1);
  needPaint |= clipRect.contains(p2);

  if (needPaint)
    painter->fillRect(p1.x(), p1.y(), p2.x() - p1.x(), p2.y() - p1.y(), mBrush);
}

QPointF GPSRVCurve::rotatePoint(double cx, double cy, double az, double px, double py)
{
  QPointF result;

  double radians = (90 - az) * M_PI / 180.0;

  double s = sin(radians);
  double c = cos(radians);

  double dx = px - cx;
  double dy = py - cy;

  // rotate
  result.setX(dx * c - dy * s + cx);
  result.setY(dx * s + dy * c + cy);

  return std::move(result);
}

///////////////////////////////////////////////////////
// Default axis ticker
const char* GPAxisAwareTicker::HoursFormat = "h";
const char* GPAxisAwareTicker::MinutesFormat = "min";
const char* GPAxisAwareTicker::SecondsFormat = "sec";

GPAxisAwareTicker::GPAxisAwareTicker(GPAxis* axis)
  : Axis(axis)
  , PreferredStep(0)
{
  setTickStepStrategy(tssMeetTickCount);
}

GPAxisAwareTicker::~GPAxisAwareTicker()
{
}

void GPAxisAwareTicker::SetAxis(GPAxis *axis)
{
  Axis = axis;
}

GPAxis *GPAxisAwareTicker::GetAxis() const
{
  return Axis;
}

QString GPAxisAwareTicker::getQDateTimeFormat(GPAxisTicker *ticker)
{
  if (auto awt = qobject_cast<GPAxisAwareTicker*>(ticker))
  {
    auto format = awt->dateTimeFormat();
    if (format != HoursFormat && format != MinutesFormat && format != SecondsFormat)
      return format;
  }
  return QString();
}

QString GPAxisAwareTicker::getQDateTimeFormat(GPAxis* axis)
{
  if (auto ticker = axis->ticker().data())
    return getQDateTimeFormat(ticker);
  return QString();
}

QVariant GPAxisAwareTicker::toFormatedKey(GPAxisTicker *ticker, double keyValue)
{
  if (auto awt = qobject_cast<GPAxisAwareTicker*>(ticker))
    return awt->toFormatedKey(keyValue);
  return keyValue;
}

double GPAxisAwareTicker::fromFormatedKey(GPAxisTicker *ticker, const QVariant& value)
{
  if (auto awt = qobject_cast<GPAxisAwareTicker*>(ticker))
    return awt->fromFormatedKey(value);
  return value.toDouble();
}

QVariant GPAxisAwareTicker::toFormatedKey(GPAxis *axis, double keyValue)
{
  if (auto awt = qobject_cast<GPAxisAwareTicker*>(axis->ticker().data()))
    return awt->toFormatedKey(keyValue);
  return keyValue;
}

double GPAxisAwareTicker::fromFormatedKey(GPAxis *axis, const QVariant& value)
{
  if (auto awt = qobject_cast<GPAxisAwareTicker*>(axis->ticker().data()))
    return awt->fromFormatedKey(value);
  return value.toDouble();
}

QVariant GPAxisAwareTicker::toFormatedKey(double keyValue) const
{
  if (mDateTimeFormat.isEmpty())
    return keyValue;
  if (mDateTimeFormat == HoursFormat)
    return keyValue / 1000.0 / 60.0 / 60.0;
  if (mDateTimeFormat == MinutesFormat)
    return keyValue / 1000.0 / 60.0;
  if (mDateTimeFormat == SecondsFormat)
    return keyValue / 1000.0;
  else
    return QDateTime::fromMSecsSinceEpoch(keyValue);
}

double GPAxisAwareTicker::fromFormatedKey(const QVariant& value) const
{
  if (mDateTimeFormat.isEmpty())
    return value.toDouble();
  if (mDateTimeFormat == HoursFormat)
    return value.toDouble() * 1000.0 * 60.0 * 60.0;
  if (mDateTimeFormat == MinutesFormat)
    return value.toDouble() * 1000.0 * 60.0;
  if (mDateTimeFormat == SecondsFormat)
    return value.toDouble() * 1000.0;
  else if (value.VARIANT_TYPE_ID() == QMetaType::QDateTime)
    return value.toDateTime().toMSecsSinceEpoch();
  else
    return value.toDouble();
}

bool GPAxisAwareTicker::isFormatedTime() const
{
  return !mDateTimeFormat.isEmpty() && mDateTimeFormat != HoursFormat && mDateTimeFormat != MinutesFormat && mDateTimeFormat != SecondsFormat;
}

double GPAxisAwareTicker::getPreferredStep() const
{
  return PreferredStep;
}

void GPAxisAwareTicker::setPreferredStep(double step)
{
  PreferredStep = step;
}

QVector<double> GPAxisAwareTicker::createTickVector(double tickStep, const GPRange& range)
{
  if (Axis
      && Axis->scaleType() != GPAxis::stLogarithmic
      && fabs(PreferredStep) > std::numeric_limits<double>::epsilon())
  {
    double ps = PreferredStep;
    ps = qMax(range.size() / 100.0, ps);
    return GPAxisTicker::createTickVector(ps, range);
  }

  if (Axis && Axis->scaleType() == GPAxis::stLogarithmic)
  {
    QVector<double> result;
    double mLogBase = 10.0;
    double mLogBaseLnInv = 1.0 / qLn(mLogBase);

    if (range.lower > 0 && range.upper > 0) // positive range
    {
      double exactPowerStep =  qLn(range.upper / range.lower) * mLogBaseLnInv / (mLogBase / 2.0 + 1e-10);
      double newLogBase = qPow(mLogBase, qMax((int)cleanMantissa(exactPowerStep), 1));
      double currentTick = qPow(newLogBase, qFloor(qLn(range.lower) / qLn(newLogBase)));
      result.append(currentTick);
      while (currentTick < range.upper && currentTick > 0) // currentMag might be zero for ranges ~1e-300, just cancel in that case
      {
        currentTick *= newLogBase;
        result.append(currentTick);
      }
    }
    else if (range.lower < 0 && range.upper < 0) // negative range
    {
      double exactPowerStep =  qLn(range.lower / range.upper) * mLogBaseLnInv / (mLogBase / 2.0 + 1e-10);
      double newLogBase = qPow(mLogBase, qMax((int)cleanMantissa(exactPowerStep), 1));
      double currentTick = -qPow(newLogBase, qCeil(qLn(-range.lower) / qLn(newLogBase)));
      result.append(currentTick);
      while (currentTick < range.upper && currentTick < 0) // currentMag might be zero for ranges ~1e-300, just cancel in that case
      {
        currentTick /= newLogBase;
        result.append(currentTick);
      }
    }
    return std::move(result);
  }
  return GPAxisTicker::createTickVector(tickStep, range);
}

QVector<QString> GPAxisAwareTicker::createLabelVector(const QVector<double> &ticks, const QLocale &locale, QChar formatChar, int precision, const GPRange &range)
{
  if (!Axis)
    return QVector<QString>();

  const double padding = 3.0;
  QVector<QString> labels = GPAxisTicker::createLabelVector(ticks, locale,
                                                             formatChar,
                                                             Axis->scaleType() == GPAxis::stLogarithmic ? 3 : precision,
                                                             range);
  if (!ticks.empty() && (ticks.size() == labels.size()))
  {
    std::vector<double> pixels(ticks.size());
    for (int i = 0; i < ticks.size(); ++i)
    {
      if (Axis->orientation() == Qt::Horizontal)
        pixels[i] = Axis->coordToPixel(ticks[i], range);
      else
        pixels[i] = -Axis->coordToPixel(ticks[i], range);
    }

    std::vector<double> measures(ticks.size());
    QFont font = Axis->labelFont();
    QFontMetricsF fmf(font);
    for (int i = 0; i < ticks.size(); ++i)
    {
      if (Axis->orientation() == Qt::Horizontal)
      {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        auto w = fmf.width(labels[i]);
#else
        auto w = fmf.horizontalAdvance(labels[i]);
#endif
        measures[i] = w;
      }
      else
      {
        measures[i] = fmf.height();
      }
    }

    if (Axis->rangeReversed())
    {
      double outermost = pixels[0] - measures[0] / 2 - padding;
      for (int i = 1; i < ticks.size(); ++i)
      {
        bool fit = outermost > pixels[i] + measures[i] / 2;
        if (fit)
        {
          outermost = pixels[i] - measures[i] / 2 - padding;
        }
        else
        {
          labels[i].clear();
        }
      }
    }
    else
    {
      double outermost = pixels[0] + measures[0] / 2 + padding;
      for (int i = 1; i < ticks.size(); ++i)
      {
        bool fit = outermost < pixels[i] - measures[i] / 2;
        if (fit)
        {
          outermost = pixels[i] + measures[i] / 2 + padding;
        }
        else
        {
          labels[i].clear();
        }
      }
    }
  }
  return labels;
}

QString GPAxisAwareTicker::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision)
{
  if (Axis && Axis->scaleType() == GPAxis::stLogarithmic
      && tick <= 1000.0)
    precision = 3;

  if (mDateTimeFormat.isEmpty())
    return GPAxisTicker::getTickLabel(tick, locale, formatChar, precision);
  if (mDateTimeFormat == HoursFormat)
    return GPAxisTicker::getTickLabel(tick / 1000.0 / 60.0 / 60.0, locale, formatChar, precision);
  if (mDateTimeFormat == MinutesFormat)
    return GPAxisTicker::getTickLabel(tick / 1000.0 / 60.0, locale, formatChar, precision);
  if (mDateTimeFormat == SecondsFormat)
    return GPAxisTicker::getTickLabel(tick / 1000.0, locale, formatChar, precision);
  else
    return QDateTime::fromMSecsSinceEpoch(tick).toString(mDateTimeFormat);
}

QVector<double> GPAxisAwareTicker::createSubTickVector(int subTickCount, const QVector<double> &ticks)
{
  if (Axis && Axis->scaleType() == GPAxis::stLogarithmic)
  {
    subTickCount = 8;
    QVector<double> result;
    result.reserve((ticks.size() - 1) * subTickCount);
    for (int i = 1; i < ticks.size(); ++i)
    {
      if (ticks.at(i) / ticks.at(i - 1) > 10.1)
      {
        double kpow = 10.0;
        qint64 delta = ceil(log10(ticks.at(i) / ticks.at(i - 1)));
        if (delta <= 0)
          continue;

        if (delta % 10 != 0 && delta % 5 == 0)
        {
          if ((delta / 5) % 5 != 0)
            kpow = (double)delta / 5.0;
          else
            kpow = 5.0;
        }
        else
        {
          if (delta / 10 > 1)
            kpow = (double)delta / 10.0;
        }
        qint64 k = pow(10, ceil(delta / kpow));
        for (double r = ticks.at(i - 1) * k; r < ticks.at(i); r *= k)
          result.append(r);
      }
      else
      {
        for (int k = 1; k <= subTickCount; ++k)
          result.append(ticks.at(i - 1) + k * ticks.at(i - 1));
      }
    }
    return std::move(result);
  }
  return GPAxisTicker::createSubTickVector(subTickCount, ticks);
}

///////////////////////////////////////////////////////
// Interactive curve
GPInteractiveCurve::GPInteractiveCurve(GPAxis* keyAxis, GPAxis* valueAxis, double slope, bool registerInPlot)
  : GPCurve(keyAxis, valueAxis, registerInPlot)
  , Slope(slope)
  , DraggingDataPoint(qQNaN())
  , DraggingSlope(qQNaN())
{
  setSelectable(GP::stWhole);
  SetColor("#997777");

  mSelectable = GP::stSingleData;
}

GPInteractiveCurve::~GPInteractiveCurve()
{
}

void GPInteractiveCurve::SetColor(const QColor& color)
{
  GPScatterStyle ss;
  ss.setPen(QPen(color, 1.0));
  ss.setBrush(QBrush(color));
  ss.setShape(GPScatterStyle::ssDisc);
  ss.setSize(5.0);
  setScatterStyle(ss);
  setPen(QPen(color, 1.0));

  selectionDecorator()->setPen(QPen(color, 1.7));
  ss.setPen(QPen(color, 1.7));
  selectionDecorator()->setScatterStyle(ss);
}

QSharedPointer<GPDataContainer<GPCurveData>> GPInteractiveCurve::GetDataContainer()
{
  return mDataContainer;
}

void GPInteractiveCurve::setInteractive(bool interactive)
{
  mInteractive = interactive;
}

bool GPInteractiveCurve::isInteractive()
{
  return mInteractive;
}

void GPInteractiveCurve::DragStart(QMouseEvent *event, unsigned part)
{
  GPCurveData* closestPoint = nullptr;
  if (part == HitPartDataPoint)
  {
    auto data = GetDataContainer();
    auto closestDataPoint = data->constEnd();
    pointDistance(event->pos(), closestDataPoint);
    if (closestDataPoint != data->constEnd())
    {
      double pointDistanceSqr = GPVector2D(coordsToPixels(closestDataPoint->key, closestDataPoint->value) - event->pos()).lengthSquared();
      if (pointDistanceSqr < mParentPlot->selectionTolerance()*0.99 * mParentPlot->selectionTolerance()*0.99)
        closestPoint = const_cast<GPCurveData*>(&*closestDataPoint);
    }
  }
  DragStart(event, closestPoint);
}

void GPInteractiveCurve::DragStart(QMouseEvent* event, GPCurveData* dataPoint)
{
  DraggingSlope = qQNaN();
  DraggingStartPos = event->pos();

  DraggingData.clear();
  DraggingDataPoint = -1;
  if (!dataPoint)
  {
    mDraggingPart = HitPartDataLine;

    // full drag
    for (auto it = mDataContainer->begin(); it != mDataContainer->end(); ++it)
      DraggingData << *it;
  }
  else
  {
    mDraggingPart = HitPartDataPoint;

    // one point drag
    DraggingDataPoint = dataPoint->t;
    DraggingData << *dataPoint;

    if (!qIsNaN(Slope))
    {
      DraggingSlope = Slope;
    }
    else if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && (mDataContainer->size() == 2))
    {
      double a = mDataContainer->at(1)->value - mDataContainer->at(0)->value;
      double b = mDataContainer->at(1)->key - mDataContainer->at(0)->key;
      DraggingSlope = a / b;
    }
  }
}

void GPInteractiveCurve::DragMove(QMouseEvent* event)
{
  DragUpdateData(event);
}

void GPInteractiveCurve::DragAccept(QMouseEvent* event)
{
  // Apply data
  DragUpdateData(event);

  // Drop
  mDraggingPart = HitPartNone;
  DraggingSlope = qQNaN();
  DraggingData.clear();
  DraggingDataPoint = -1;
  emit movedComplete();
}

void GPInteractiveCurve::DragUpdateData(QMouseEvent* event)
{
  double dragStartX = 0;
  double dragStartY = 0;
  pixelsToCoords(DraggingStartPos, dragStartX, dragStartY);

  double dragX = 0;
  double dragY = 0;
  pixelsToCoords(event->pos(), dragX, dragY);

  double dx = dragX - dragStartX;
  double dy = dragY - dragStartY;

  if (DraggingDataPoint != -1 && DraggingData.size() == 1)
  {
    //Drag point
    {
      double y = 0;
      if (!qIsNaN(DraggingSlope))
      {
        if (mKeyAxis->scaleType() == GPAxis::stLinear)
        {
          if (fabs(dragX - DraggingData[0].key) > std::numeric_limits<double>::epsilon())
            y = DraggingData[0].value + DraggingSlope * (dragX - DraggingData[0].key);
        }
        else
        {
          if (fabs(dragX / DraggingData[0].key) > std::numeric_limits<double>::epsilon())
            y = pow(dragX / DraggingData[0].key, DraggingSlope) * DraggingData[0].value;
        }
      }
      else
      {
        y = dragY;
      }

      double x = dragX;

      if (auto p = GetDraggingDataPoint())
      {
        p->key = x;
        p->value = y;
      }
      mParentPlot->replot(GraphicsPlot::rpQueuedReplot);
    }
  }
  else if (!DraggingData.isEmpty())
  {
    // Drag full data set
    for (int i = 0; i < mDataContainer->size(); ++i)
    {
      if (mKeyAxis->scaleType() == GPAxis::stLinear)
        mDataContainer->atNonConst(i)->key = DraggingData[i].key + dx;
      else
        mDataContainer->atNonConst(i)->key = DraggingData[i].key * dragX / dragStartX;
      if (mValueAxis->scaleType() == GPAxis::stLinear)
        mDataContainer->atNonConst(i)->value = DraggingData[i].value + dy;
      else
        mDataContainer->atNonConst(i)->value = DraggingData[i].value * dragY / dragStartY;
    }
  }
  emit moved();
}

GPCurveData* GPInteractiveCurve::GetDraggingDataPoint()
{
  auto it = mDataContainer->findBegin(DraggingDataPoint, false);
  if (it != mDataContainer->end())
    return &*it;
  return nullptr;
}

Qt::CursorShape GPInteractiveCurve::HitTest(QMouseEvent* event, unsigned* part)
{
  if (part)
    *part = HitPartNone;

  if (!mInteractive)
    return Qt::ArrowCursor;

  auto data = GetDataContainer();
  GPCurveDataContainer::const_iterator closestDataPoint = data->constEnd();
  double distance = pointDistance(event->pos(), closestDataPoint);

  if (closestDataPoint != data->constEnd())
  {
    double pointDistanceSqr = GPVector2D(coordsToPixels(closestDataPoint->key, closestDataPoint->value) - event->pos()).lengthSquared();
    if (pointDistanceSqr < mParentPlot->selectionTolerance()*0.99 * mParentPlot->selectionTolerance()*0.99)
    {
      // Point hit
      if (part)
        *part = HitPartDataPoint;
      return Qt::SizeAllCursor;
    }
  }

  if (distance < mParentPlot->selectionTolerance()*0.99)
  {
    // Line hit
    if (part)
      *part = HitPartDataLine;
    return Qt::SizeAllCursor;
  }
  return Qt::ArrowCursor;
}

void GPInteractiveCurve::SetSlope(double slope)
{
  Slope = slope;
}

double GPInteractiveCurve::GetSlope()
{
  return Slope;
}

///////////////////////////////////////////////////////
// Curve label
GPCurveLabel::GPCurveLabel(GPCurve *curve)
  : GPLayerable(curve->parentPlot(), curve->layer(), curve)
  , mCurve(curve)
{
  QFont font = parentPlot()->font();
  font.setPointSize(9);
  font.setBold(true);
  setFont(font);
}

GPCurveLabel::~GPCurveLabel()
{
}

void GPCurveLabel::setText(const QString& text)
{
  mText = text;
}

void GPCurveLabel::setFont(const QFont& font)
{
  mFont = font;
}

void GPCurveLabel::setColor(const QColor& color)
{
  mColor = color;
}

void GPCurveLabel::draw(GPPainter *painter)
{
  if (mText.isEmpty() || !mCurve)
    return;

  auto valueAxis = mCurve->valueAxis();
  auto keyAxis = mCurve->keyAxis();

  if (!valueAxis || !keyAxis || !mCurve->data() || mCurve->data()->isEmpty())
    return;

  painter->save();

  painter->setFont(mFont);
  painter->setPen(mColor);

  QRect clipR;
  if (auto rect = valueAxis->axisRect())
  {
    clipR = rect->rect();
    painter->setClipRect(rect->rect());
  }

  QFontMetrics fm(mFont);
  auto textRect = fm.boundingRect(mText);

  double indentX = 7.0;
  double indentY = 5.0;

  QPointF textPos;
  int keyPixel = keyAxis->coordToPixel(mCurve->data()->first()->key);
  int valuePixel = valueAxis->coordToPixel(mCurve->data()->first()->value);
  if (mCurve->data()->size() > 1)
  {
    keyPixel = (keyAxis->coordToPixel(mCurve->data()->first()->key)
                + keyAxis->coordToPixel(mCurve->data()->last()->key)) / 2.0 + 5.0;
    valuePixel = (valueAxis->coordToPixel(mCurve->data()->first()->value)
                  + valueAxis->coordToPixel(mCurve->data()->last()->value)) / 2.0;
  }
  else
  {
    keyPixel = keyAxis->coordToPixel(mCurve->data()->first()->key);
    valuePixel = valueAxis->coordToPixel(mCurve->data()->first()->value);
  }

  if (keyAxis->orientation() == Qt::Horizontal)
  {
    textPos = QPointF(keyPixel + indentX, valuePixel - indentY);
    if (!clipR.isEmpty())
    {
      if (textRect.width() + keyPixel + indentX > clipR.right())
        textPos.setX(keyPixel - textRect.width() - indentX);

      if (valuePixel - indentY - textRect.height() < clipR.top())
        textPos.setY(valuePixel + indentY + textRect.height());
    }
  }
  else
  {
    textPos = QPointF(valuePixel + indentX, keyPixel - indentY);
    if (!clipR.isEmpty())
    {
      if (textRect.width() + valuePixel + indentX > clipR.bottom())
        textPos.setX(valuePixel - textRect.width() - indentX);

      if (keyPixel - indentY - textRect.height() < clipR.top())
        textPos.setY(keyPixel + indentY + textRect.height());
    }
  }
  painter->drawText(textPos, mText);

  painter->restore();
}

///////////////////////////////////////////////////////
// Movable data container
GPMovableDataContainer::GPMovableDataContainer()
  : GPRepresentationDataContainer<GPCurveData>()
  , KeyShift(0)
  , ValueShift(0)
{
}

//// Data
double GPMovableDataContainer::GetKeyShift() const
{
  return KeyShift;
}

double GPMovableDataContainer::GetValueShift() const
{
  return ValueShift;
}

void GPMovableDataContainer::SetKeyShift(double k)
{
  KeyShift = k;
}

void GPMovableDataContainer::SetValueShift(double v)
{
  ValueShift = v;
}

//// Uom suuport for data
void GPMovableDataContainer::applyUomForKeys(const GPUomDescriptor& target)
{
  //// Inherited
  GPRepresentationDataContainer<GPCurveData>::applyUomForKeys(target);

  //// Self
  GPUomDescriptor source = listenerKeys.getUomDescriptor();
  if (source != target)
  {
    if (source.canConvert(target))
    {
      GPUomDataTransformation transformation = listenerKeys.getUomProvider()->getUomTransformation(source);
      KeyShift = transformation(KeyShift, source, target);
    }
  }
}

void GPMovableDataContainer::applyUomForValues(const GPUomDescriptor& target)
{
  //// Inherited
  GPRepresentationDataContainer<GPCurveData>::applyUomForKeys(target);

  //// Self
  GPUomDescriptor source = listenerValues.getUomDescriptor();
  if (source != target)
  {
    if (source.canConvert(target))
    {
      GPUomDataTransformation transformation = listenerValues.getUomProvider()->getUomTransformation(source);
      ValueShift = transformation(ValueShift, source, target);
    }
  }
}

void GPMovableDataContainer::updateData()
{
  mDataLock.lockForWrite();

  mData.resize(mOriginalData.size());

  auto recalculate = [this](QPair<int, int> range) {
    for (auto i = range.first; i <= range.second; ++i)
    {
      const auto& d = mOriginalData.at(i);
      mData[i].t = d->t;
      mData[i].key = d->key + KeyShift;
      mData[i].value = d->value + ValueShift;
    }
  };

#if !defined(DISABLE_CONCURRENT)
  QList<QPair<int, int>> tasks;
  int count = 10000;
  int begin = 0;
  int end = 0;
  while (end != mOriginalData.size() - 1)
  {
    end = qMin(begin + count, mOriginalData.size() - 1);
    tasks.append({ begin, end });
    begin = end + 1;
    if (begin >= mOriginalData.size())
      break;
  }

  QtConcurrent::blockingMap(tasks, [recalculate](QPair<int, int> task) {
    recalculate(task);
  });
#else
  recalculate({0, mOriginalData.size() - 1});
#endif

  mInvalidatedData.clear();
  mDataLock.unlock();
}

///////////////////////////////////////////////////////
// Movable curve
GPMovableCurve::GPMovableCurve(GPAxis *keyAxis, GPAxis *valueAxis, bool registerInPlot)
  : GPCurve(keyAxis, valueAxis, registerInPlot)
  , Dragging(false)
{
  mDataContainer.reset(new GPMovableDataContainer);
  mDataContainer->setUomProvider(keyAxis, valueAxis);
}

void GPMovableCurve::setData(const QVector<double> &t, const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  GPCurve::setData(t, keys, values, alreadySorted);
  UpdateData();
}

void GPMovableCurve::setData(const QVector<double> &keys, const QVector<double> &values)
{
  GPCurve::setData(keys, values);
  UpdateData();
}

double GPMovableCurve::GetKeyShift() const
{
  if (auto d = dynamic_cast<GPMovableDataContainer*>(mDataContainer.data()))
    return d->GetKeyShift();
  return 0;
}

double GPMovableCurve::GetValueShift() const
{
  if (auto d = dynamic_cast<GPMovableDataContainer*>(mDataContainer.data()))
    return d->GetValueShift();
  return 0;
}

void GPMovableCurve::SetKeyShift(double k)
{
  if (auto d = dynamic_cast<GPMovableDataContainer*>(mDataContainer.data()))
  {
    d->SetKeyShift(k);
    emit moved();
  }
}

void GPMovableCurve::SetValueShift(double v)
{
  if (auto d = dynamic_cast<GPMovableDataContainer*>(mDataContainer.data()))
  {
    d->SetValueShift(v);
    emit moved();
  }
}

void GPMovableCurve::UpdateData()
{
  if (auto d = dynamic_cast<GPMovableDataContainer*>(mDataContainer.data()))
    d->updateData();
}

Qt::CursorShape GPMovableCurve::HitTest(QMouseEvent* event, unsigned* /*part*/)
{
  auto modifiers = qApp->queryKeyboardModifiers();
  if (modifiers & Qt::ControlModifier)
  {
    if (selectTest(event->pos(), false) < parentPlot()->selectionTolerance() * 0.99)
    {
      if (modifiers & Qt::ShiftModifier)
        return Qt::SizeAllCursor;
      return Qt::SizeHorCursor;
    }
  }
  return Qt::ArrowCursor;
}

void GPMovableCurve::mousePressEvent(QMouseEvent* /*event*/, const QVariant& /*details*/)
{
  auto modifiers = qApp->queryKeyboardModifiers();
  if (modifiers & Qt::ControlModifier)
  {
    if (!selected())
    {
      setSelection(GPDataSelection(getDataRange()));
      emit mParentPlot->selectionChangedByUser();
    }

    Dragging = true;
    for (int i = 0; i < parentPlot()->curveCount(); ++i)
    {
      if (auto curve = parentPlot()->curve(i))
      {
        if (curve->selected())
        {
          if (auto c = qobject_cast<GPMovableCurve*>(curve))
          {
            curve->setProperty("key_drag_start", c->GetKeyShift());
            curve->setProperty("value_drag_start", c->GetValueShift());
          }
        }
      }
    }
  }
}

void GPMovableCurve::mouseMoveEvent(QMouseEvent* event, const QPointF& startPos)
{
  if (Dragging)
  {
    auto pressPos = pixelsToCoords(startPos);
    auto currPos = pixelsToCoords(event->pos());
    auto modifiers = qApp->queryKeyboardModifiers();
    if ((modifiers & Qt::ControlModifier) && keyAxis())
    {
      QList<GPMovableCurve*> curves;
      for (int i = 0; i < parentPlot()->curveCount(); ++i)
      {
        if (auto curve = parentPlot()->curve(i))
        {
          if (curve->selected())
          {
            if (auto mc = qobject_cast<GPMovableCurve*>(curve))
            {
              curves << mc;
            }
          }
        }
      }

      if (!curves.isEmpty())
      {
        auto move = [modifiers, currPos, pressPos](GPCurve* curve){
          if (auto c = qobject_cast<GPMovableCurve*>(curve))
          {
            if (modifiers & Qt::ControlModifier)
            {
              c->SetKeyShift(curve->property("key_drag_start").toDouble() + currPos.x() - pressPos.x());
              if (modifiers & Qt::ShiftModifier)
                c->SetValueShift(curve->property("value_drag_start").toDouble() + currPos.y() - pressPos.y());
            }
            c->UpdateData();
          }
        };

#if !defined(DISABLE_CONCURRENT)
        QtConcurrent::blockingMap(curves, move);
#else
        for (auto& curve : curves)
          move(curve);
#endif
      }
    }
    parentPlot()->replot();
  }
}

void GPMovableCurve::mouseReleaseEvent(QMouseEvent* /*event*/, const QPointF& /*startPos*/)
{
  Dragging = false;
  setSelection(GPDataSelection(getDataRange()));
  emit parentPlot()->selectionChangedByUser();
}


///////////////////////////////////////////////////////
// Curve mark point
GPCurveMarkPoint::GPCurveMarkPoint(GPCurve* curve, QPoint pixelPos)
  : GPLayerable(curve->parentPlot(), curve->layer(), curve)
  , Curve(curve)
  , KeyPos(0)
  , Dragging(false)
{
  double coord = curve->keyAxis()->pixelToCoord(curve->keyAxis()->orientation() == Qt::Horizontal ? pixelPos.x() : pixelPos.y());
  if (!Curve->data()->isEmpty())
    KeyPos = coord - curve->data()->first()->key;

  connect(curve, &GPCurve::destroyed, this, &GPCurveMarkPoint::deleteLater);

  setUomProvider(curve->keyAxis(), curve->valueAxis());
}

GPCurveMarkPoint::GPCurveMarkPoint(GPCurve* curve, double keyPos)
  : GPLayerable(curve->parentPlot(), curve->layer(), curve)
  , Curve(curve)
  , KeyPos(0)
  , Dragging(false)
{
  if (!Curve->data()->isEmpty())
    KeyPos = keyPos - curve->data()->first()->key;

  connect(curve, &GPCurve::destroyed, this, &GPCurveMarkPoint::deleteLater);

  setUomProvider(curve->keyAxis(), curve->valueAxis());
}

double GPCurveMarkPoint::selectTest(const QPointF& pos, bool /*onlySelectable*/, QVariant* /*details*/) const
{
  if (Curve && Curve->keyAxis() && !Curve->data()->isEmpty())
  {
    double keyPos = qMin(KeyPos + Curve->data()->first()->key, Curve->data()->last()->key);
    double valuePos = GraphicsPlotExtendedUtils::interpolate(Curve->data(), keyPos, true);
    if (!qIsNaN(valuePos))
    {
      QPointF pointPos = Curve->coordsToPixels(keyPos, valuePos);
      if ((pos - pointPos).manhattanLength() < parentPlot()->selectionTolerance())
        return 0;
    }
  }
  return - 1;
}

Qt::CursorShape GPCurveMarkPoint::HitTest(QMouseEvent* event, unsigned* /*part*/)
{
  if (selectTest(event->pos(), false) < parentPlot()->selectionTolerance())
    return Qt::SizeAllCursor;
  return Qt::ArrowCursor;
}

void GPCurveMarkPoint::draw(GPPainter* painter)
{
  if (Curve && Curve->valueAxis() && Curve->keyAxis() && !Curve->data()->isEmpty())
  {
    painter->save();

    double pos = qMin(KeyPos + Curve->data()->first()->key, Curve->data()->last()->key);
    double valuePos = GraphicsPlotExtendedUtils::interpolate(Curve->data(), pos, false);
    QPointF pixelPos = Curve->coordsToPixels(pos, valuePos);

    auto plotRect = Curve->keyAxis()->axisRect()->rect();
    painter->setClipRect(plotRect);

    painter->setPen(QPen(Qt::black, 1));
    painter->drawLine(QPoint(pixelPos.x(), plotRect.top()), QPoint(pixelPos.x(), plotRect.bottom()));
    painter->drawLine(QPoint(plotRect.left(), pixelPos.y()), QPoint(plotRect.right(), pixelPos.y()));

    painter->setPen(Qt::red);
    painter->setBrush(Qt::red);
    painter->drawEllipse(pixelPos, 4.2, 4.2);

    painter->setFont(parentPlot()->font());
    painter->setPen(Qt::black);
    painter->drawText(pixelPos.x() + 6.0, pixelPos.y() - 10.0, Curve->keyAxis()->tickLabel(pos)
                      + ", " + Curve->valueAxis()->tickLabel(valuePos));

    painter->restore();
  }
}

void GPCurveMarkPoint::mousePressEvent(QMouseEvent* /*event*/, const QVariant& /*details*/)
{
  Dragging = true;
}

void GPCurveMarkPoint::mouseMoveEvent(QMouseEvent* event, const QPointF& /*startPos*/)
{
  if (Dragging && Curve && !Curve->data()->isEmpty())
  {
    double key, value;
    Curve->pixelsToCoords(event->pos(), key, value);
    bool foundRange = false;
    auto range = Curve->getKeyRange(foundRange);
    KeyPos = qBound(range.lower, key, range.upper) - Curve->data()->first()->key;
    parentPlot()->replot();
  }
}

void GPCurveMarkPoint::mouseReleaseEvent(QMouseEvent* /*event*/, const QPointF& /*startPos*/)
{
  Dragging = false;
}

void GPCurveMarkPoint::applyUomForKeys(const GPUomDescriptor& target)
{
  GPUomDescriptor source = listenerKeys.getUomDescriptor();
  GPUomDataTransformation transformation = listenerKeys.getUomProvider()->getUomTransformation(source);

  KeyPos = transformation(KeyPos, source, target);
}

void GPCurveMarkPoint::applyUomForValues(const GPUomDescriptor& /*descriptor*/)
{
  //// just for symmetry w/ common plot impl which has .mValuePos
}

///////////////////////////////////////////////////////
// Interactive point
GPInteractivePoint::GPInteractivePoint(GPAxis *keyAxis, GPAxis *valueAxis, bool interactive)
  : GPInteractiveCurve(keyAxis, valueAxis)
  , mLabel(new GPCurveLabel(this))
{
  setInteractive(interactive);

  setLineStyle(GPCurve::lsNone);
  setPen(QPen(QBrush(Qt::red), 5.5));
  setScatterStyle(GPScatterStyle::ssDisc);
  selectionDecorator()->setPen(QPen(QBrush(Qt::red), 5.5));
  selectionDecorator()->setScatterStyle(GPScatterStyle::ssCircle);
}

GPInteractivePoint::~GPInteractivePoint()
{
}

void GPInteractivePoint::setLabelText(const QString& name)
{
  mLabel->setText(name);
}

void GPInteractivePoint::setLabelFont(const QFont& font)
{
  mLabel->setFont(font);
}

void GPInteractivePoint::setLabelColor(const QColor& color)
{
  mLabel->setColor(color);
}

void GPInteractivePoint::setCoord(const QPointF& point)
{
  mDataContainer->clear();
  addData(point.x(), point.y());
  emit movedComplete();
}

void GPInteractivePoint::setCoord(double key, double val)
{
  mDataContainer->clear();
  addData(key, val);
  emit movedComplete();
}

QPointF GPInteractivePoint::getCoord() const
{
  if (!mDataContainer->isEmpty())
    return QPointF(mDataContainer->at(0)->key, mDataContainer->at(0)->value);
  return QPointF();
}

QPointF GPInteractivePoint::getPixelCoord() const
{
  if (keyAxis() && valueAxis())
    return QPointF(keyAxis()->coordToPixel(mDataContainer->at(0)->key), valueAxis()->coordToPixel(mDataContainer->at(0)->value));
  return QPointF();
}

double GPInteractivePoint::key()
{
  if (!mDataContainer->isEmpty())
    return mDataContainer->at(0)->key;
  return qQNaN();
}

double GPInteractivePoint::value()
{
  if (!mDataContainer->isEmpty())
    return mDataContainer->at(0)->value;
  return qQNaN();
}

///////////////////////////////////////////////////////
// Infinite rect
GPInfiniteRect::GPInfiniteRect(GraphicsPlot* parentPlot, Qt::Orientation orientation)
  : GPItemRect(parentPlot)
  , mOrientation(orientation)
{
  setPen(QPen(Qt::transparent));
}

GPInfiniteRect::GPInfiniteRect(GraphicsPlot* parentPlot, Qt::Orientation orientation, GPAxis* keyAxis, GPAxis* valueAxis)
  : GPInfiniteRect(parentPlot, orientation)
{
  topLeft->setAxes(keyAxis, valueAxis);
  bottomRight->setAxes(keyAxis, valueAxis);
}

GPInfiniteRect::~GPInfiniteRect()
{
}

void GPInfiniteRect::setColors(const QBrush &rectBrush, const QPen &linesPen)
{
  setBrush(rectBrush);
  setPen(linesPen);
}

void GPInfiniteRect::draw(GPPainter* painter)
{
  QRect clip = clipRect();

  QPointF p1 = topLeft->pixelPosition();
  QPointF p2 = bottomRight->pixelPosition();

  if (mOrientation == Qt::Horizontal)
  {
    p1.setX(clip.left());
    p2.setX(clip.right());
  }
  else
  {
    p1.setY(clip.top());
    p2.setY(clip.bottom());
  }

  if (p1.toPoint() == p2.toPoint())
    return;

  QRectF rect = QRectF(p1, p2).normalized();
  double clipPad = mainPen().widthF();
  QRectF boundingRect = rect.adjusted(-clipPad, -clipPad, clipPad, clipPad);
  if (boundingRect.intersects(clip)) // only draw if bounding rect of rect item is visible in cliprect
  {
    painter->setPen(Qt::transparent);
    painter->setBrush(mainBrush());
    painter->drawRect(rect);

    painter->setPen(mainPen());
    painter->setBrush(QBrush(Qt::transparent));
    painter->drawLine(rect.topLeft(), rect.topRight());
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
  }
}

///////////////////////////////////////////////////////
// Infinite line
GPAxisInfiniteLine::GPAxisInfiniteLine(GraphicsPlot* parentPlot)
  : GPItemLine(parentPlot)
{
}

GPAxisInfiniteLine::GPAxisInfiniteLine(GraphicsPlot *parentPlot, GPAxis *keyAxis, GPAxis *valueAxis)
  : GPAxisInfiniteLine(parentPlot)
{
  start->setAxes(keyAxis, valueAxis);
  end->setAxes(keyAxis, valueAxis);
}

GPAxisInfiniteLine::GPAxisInfiniteLine(GPAxis* axis)
  : GPItemLine(axis->parentPlot())
{
  setAntialiased(false);

  start->setAxes(axis, axis->orientation() == Qt::Horizontal
    ? axis->parentPlot()->getDefaultAxisY()
    : axis->parentPlot()->getDefaultAxisX());
  end->setAxes(axis, axis->orientation() == Qt::Horizontal
    ? axis->parentPlot()->getDefaultAxisY()
    : axis->parentPlot()->getDefaultAxisX());

  setClipAxisRect(axis->axisRect());
}


GPAxisInfiniteLine::~GPAxisInfiniteLine()
{
}

void GPAxisInfiniteLine::setValue(const double& value)
{
  start->setCoords(value, -1e7);
  end->setCoords(value, 1e7);

  parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

double GPAxisInfiniteLine::getValue() const
{
  return start->key();
}

Qt::CursorShape GPAxisInfiniteLine::HitTest(QMouseEvent* event, unsigned* part)
{
  if (mInteractive)
  {
    double pointToStartDistanceSqr = GPVector2D(start->pixelPosition() - event->pos()).lengthSquared();
    if (pointToStartDistanceSqr < parentPlot()->selectionTolerance()*0.99 * parentPlot()->selectionTolerance()*0.99)
    {
      if (part)
        *part = HitPartStart;
      return Qt::SizeAllCursor;
    }

    double pointToEndDistanceSqr = GPVector2D(end->pixelPosition() - event->pos()).lengthSquared();
    if (pointToEndDistanceSqr < parentPlot()->selectionTolerance()*0.99 * parentPlot()->selectionTolerance()*0.99)
    {
      if (part)
        *part = HitPartEnd;
      return Qt::SizeAllCursor;
    }

    GPVector2D eventPosVector(event->pos());
    GPVector2D startPosVector(start->pixelPosition());
    GPVector2D endPosVector(end->pixelPosition());
    double distance = eventPosVector.distanceToStraightLine(startPosVector, endPosVector);
    if (distance < parentPlot()->selectionTolerance()*0.99 * parentPlot()->selectionTolerance()*0.99)

    {
      if (part)
        *part = HitPartLine;
      return Qt::SizeAllCursor;
    }
  }
  return Qt::ArrowCursor;
}

void GPAxisInfiniteLine::DragStart(QMouseEvent* event, unsigned part)
{
  if (!mInteractive)
    return;
  DraggingStartPos      = event->pos();
  DraggingStartPosStart = start->pixelPosition();
  DraggingStartPosEnd   = end->pixelPosition();
  mDraggingPart = part;
}

void GPAxisInfiniteLine::DragMove(QMouseEvent* event)
{
  if (!mInteractive)
    return;
  DragUpdateData(event);
}

void GPAxisInfiniteLine::DragAccept(QMouseEvent* event)
{
  if (!mInteractive)
    return;
  DragUpdateData(event);

  mDraggingPart = HitPartNone;
}

void GPAxisInfiniteLine::DragUpdateData(QMouseEvent* event)
{
  if (!mInteractive)
    return;
  if (mDraggingPart == HitPartStart)
    start->setPixelPosition(event->pos());
  else if (mDraggingPart == HitPartEnd)
    end->setPixelPosition(event->pos());
  else if (mDraggingPart == HitPartLine)
  {
    QPointF delta(event->pos().x() - DraggingStartPos.x(),
                  event->pos().y() - DraggingStartPos.y());
    QPointF pixelStart = DraggingStartPosStart + delta;
    QPointF pixelEnd   = DraggingStartPosEnd + delta;
    start->setPixelPosition(pixelStart);
    end->setPixelPosition(pixelEnd);
  }
}

bool GPAxisInfiniteLine::IsDragging() const
{
  return mDraggingPart != HitPartNone;
}

QLineF GPAxisInfiniteLine::getRectClippedLine(const GPVector2D &start, const GPVector2D &end, const QRect &rect) const
{
  GPVector2D base = start;
  GPVector2D vec = end - start;
  double bx, by;
  double gamma, mu;
  QList<GPVector2D> pointVectors;

  if (!qFuzzyIsNull(vec.y())) // line is not horizontal
  {
    // check top of rect:
    bx = rect.left();
    by = rect.top();
    mu = (by - base.y()) / vec.y();
    /*if (mu >= 0 && mu <= 1)*/
    {
      gamma = base.x() - bx + mu * vec.x();
      if (gamma >= 0 && gamma <= rect.width())
        pointVectors.append(GPVector2D(bx + gamma, by));
    }
    // check bottom of rect:
    bx = rect.left();
    by = rect.bottom();
    mu = (by - base.y()) / vec.y();
    /*if (mu >= 0 && mu <= 1)*/
    {
      gamma = base.x() - bx + mu * vec.x();
      if (gamma >= 0 && gamma <= rect.width())
        pointVectors.append(GPVector2D(bx + gamma, by));
    }
  }

  if (!qFuzzyIsNull(vec.x())) // line is not vertical
  {
    // check left of rect:
    bx = rect.left();
    by = rect.top();
    mu = (bx - base.x()) / vec.x();
    /*if (mu >= 0 && mu <= 1)*/
    {
      gamma = base.y() - by + mu * vec.y();
      if (gamma >= 0 && gamma <= rect.height())
        pointVectors.append(GPVector2D(bx, by + gamma));
    }
    // check right of rect:
    bx = rect.right();
    by = rect.top();
    mu = (bx - base.x()) / vec.x();
    /*if (mu >= 0 && mu <= 1)*/
    {
      gamma = base.y() - by + mu * vec.y();
      if (gamma >= 0 && gamma <= rect.height())
        pointVectors.append(GPVector2D(bx, by + gamma));
    }
  }

  QLineF result;
  if (pointVectors.size() == 2)
  {
    result.setPoints(pointVectors.at(0).toPointF(), pointVectors.at(1).toPointF());
  }
  else if (pointVectors.size() > 2)
  {
    // line probably goes through corner of rect, and we got two points there. single out the point pair with greatest distance:
    double distSqrMax = 0;
    GPVector2D pv1, pv2;
    for (int i = 0; i<pointVectors.size() - 1; ++i)
    {
      for (int k = i + 1; k<pointVectors.size(); ++k)
      {
        double distSqr = (pointVectors.at(i) - pointVectors.at(k)).lengthSquared();
        if (distSqr > distSqrMax)
        {
          pv1 = pointVectors.at(i);
          pv2 = pointVectors.at(k);
          distSqrMax = distSqr;
        }
      }
    }
    result.setPoints(pv1.toPointF(), pv2.toPointF());
  }
  return std::move(result);
}


///////////////////////////////////////////////////////
//  Selection rect
GPSelectionIntervals::GPSelectionIntervals(GPSelectionIntervals::SelectionMode workmode, GPAxis *keyAxis, GraphicsPlot *parentPlot)
  : GPLayerable(parentPlot ? parentPlot : keyAxis->parentPlot())
  , mMode(workmode)
  , mKeyAxis(keyAxis)
  , mLeftSideAxisRect(nullptr)
  , mSelectionBrush(QBrush(Qt::transparent))
  , mMargin(0)
  , mStartLine(nullptr)
  , mStopLine(nullptr)
{
  setUomProvider(mKeyAxis);

  if (mMode == InteractiveRange)
  {
    mStartLine = new GPOrientatedInfiniteLine(mParentPlot, Qt::Horizontal);
    mStartLine->setParentLayerable(this);
    mStartLine->setKeyAxis(nullptr);
    mStartLine->setValueAxis(mKeyAxis);
    mStartLine->start->setTypeX(GPItemPosition::ptAbsolute);
    mStartLine->start->setTypeY(GPItemPosition::ptPlotCoords);
    mStartLine->end->setTypeX(GPItemPosition::ptAbsolute);
    mStartLine->end->setTypeY(GPItemPosition::ptPlotCoords);

    mStopLine = new GPOrientatedInfiniteLine(mParentPlot, Qt::Horizontal);
    mStopLine->setParentLayerable(this);
    mStopLine->setKeyAxis(nullptr);
    mStopLine->setValueAxis(mKeyAxis);
    mStopLine->start->setTypeX(GPItemPosition::ptAbsolute);
    mStopLine->start->setTypeY(GPItemPosition::ptPlotCoords);
    mStopLine->end->setTypeX(GPItemPosition::ptAbsolute);
    mStopLine->end->setTypeY(GPItemPosition::ptPlotCoords);

    connect(mStartLine, &GPOrientatedInfiniteLine::moved, this, &GPSelectionIntervals::OnStartLineMoved);
    connect(mStopLine, &GPOrientatedInfiniteLine::moved, this, &GPSelectionIntervals::OnStopLineMoved);
  }
}

GPSelectionIntervals::GPSelectionIntervals(GPAxis* keyAxis)
  : GPSelectionIntervals(InteractiveRange, keyAxis)
{
}

GPSelectionIntervals::~GPSelectionIntervals()
{
}

void GPSelectionIntervals::setHorizontalLinesPen(const QPen& pen)
{
  mHorizontalLinesPen = pen;
  if (mStartLine && mStopLine)
  {
    mStartLine->setPen(pen);
    mStartLine->setSelectedPen(pen);
    mStopLine->setPen(pen);
    mStopLine->setSelectedPen(pen);
  }
}

void GPSelectionIntervals::setVerticalLinePen(const QPen& pen)
{
  mVerticalLinePen = pen;
}

void GPSelectionIntervals::setSelectionBrush(const QBrush& brush)
{
  mSelectionBrush = brush;
}

void GPSelectionIntervals::setMargin(int margin)
{
  mMargin = margin;
}

void GPSelectionIntervals::setLeftSideAxisRect(GPAxisRect *rect)
{
  mLeftSideAxisRect = rect;
}

void GPSelectionIntervals::setInteractive(bool interactive)
{
  if (mStartLine && mStopLine)
  {
    mStartLine->setInteractive(interactive);
    mStopLine->setInteractive(interactive);
  }
}

void GPSelectionIntervals::setSelection(const QVector<QPair<double, double>>& ranges)
{
  mSelections = ranges;

  applyUom(mInputUom, uomDescriptor, mSelections.begin(), mSelections.end());

  setVisible(!mSelections.isEmpty());
}

void GPSelectionIntervals::setNames(const QVector<QString>& names)
{
  mNames = names;
}

void GPSelectionIntervals::OnStartLineMoved()
{
  if (mStartLine && mStopLine && mStartLine->getValue() > mStopLine->getValue())
    mStartLine->setValue(mStopLine->getValue());
  emit rangeChanged();
}

void GPSelectionIntervals::OnStopLineMoved()
{
  if (mStartLine && mStopLine && mStopLine->getValue() < mStartLine->getValue())
    mStopLine->setValue(mStartLine->getValue());
  emit rangeChanged();
}

void GPSelectionIntervals::setStartStop(double start, double stop)
{
  if (mStartLine && mStopLine)
  {
    mStartLine->setValue(start);
    mStopLine->setValue(stop);
  }
}

double GPSelectionIntervals::getStart() const
{
  return mStartLine ? mStartLine->getValue() : 0;
}

double GPSelectionIntervals::getStop() const
{
  return mStopLine ? mStopLine->getValue() : 0;
}

void GPSelectionIntervals::applyDefaultAntialiasingHint(GPPainter* /*painter*/) const
{
}

void GPSelectionIntervals::draw(GPPainter *painter)
{
  if (mVisible && mKeyAxis)
  {
    QRect clipRect;
    if (mLeftSideAxisRect)
    {
      clipRect.setTopLeft(mLeftSideAxisRect->rect().topRight() + QPoint(5, 0));
      clipRect.setBottomRight(QPoint(parentPlot()->viewport().right() - mMargin, mLeftSideAxisRect->rect().bottom()));
    }
    else
    {
      clipRect = parentPlot()->viewport();
    }

    painter->save();
    painter->setClipRect(clipRect);
    painter->setAntialiasing(false);
    if (mMode == InteractiveRange)
    {
      if (mStartLine && mStopLine)
      {
        mStartLine->setClipRect(clipRect);
        mStopLine->setClipRect(clipRect);

        QPoint pT(clipRect.right() - mVerticalLinePen.widthF() / 2 + 1, mKeyAxis->coordToPixel(mStartLine->getValue()) + mVerticalLinePen.widthF() / 2);
        QPoint pB(clipRect.right() - mVerticalLinePen.widthF() / 2 + 1, mKeyAxis->coordToPixel(mStopLine->getValue()));
        painter->setPen(mVerticalLinePen);
        painter->drawLine(pT, pB);
      }
    }
    else
    {
      for (int i = 0; i < mSelections.size(); ++i)
      {
        double startPx = mKeyAxis->coordToPixel(mSelections[i].first);
        double stopPx = mKeyAxis->coordToPixel(mSelections[i].second);

        QRect selectionRect;
        selectionRect.setTopLeft(QPoint(clipRect.left(), startPx + mHorizontalLinesPen.widthF() / 2));
        selectionRect.setBottomRight(QPoint(clipRect.right() - mVerticalLinePen.widthF(), stopPx + mHorizontalLinesPen.widthF() / 2));

        painter->setPen(Qt::transparent);
        painter->setBrush(mSelectionBrush);
        painter->drawRect(selectionRect);

        painter->setPen(mHorizontalLinesPen);
        painter->drawLine(selectionRect.topLeft(), selectionRect.topRight());
        painter->drawLine(selectionRect.bottomLeft(), selectionRect.bottomRight());

        painter->setPen(mVerticalLinePen);
        QPoint pT(selectionRect.right(), selectionRect.top() + mHorizontalLinesPen.widthF() / 2 + 1);
        QPoint pB(selectionRect.right(), selectionRect.bottom() - mHorizontalLinesPen.widthF() / 2 - 1);
        painter->drawLine(pT, pB);

        QString text = mNames.value(i);
        if (text.isEmpty())
          continue;

        painter->save();
        {
          QFont font;
          font.setFamily("Segoe UI");
          font.setPixelSize(11);
          QFontMetrics fm(font);
          auto textRect = fm.boundingRect(text);
          textRect.moveTo(0, 0);

          painter->setPen(QColor(0,0,0));
          painter->setRenderHints(painter->renderHints(), false);
          painter->translate(pB.x(), pB.y());
          painter->rotate(-90);
          painter->drawText(qMax((pB.y() - pT.y() - textRect.width()) / 2, 0), -5 - textRect.height(), pB.y() - pT.y(), textRect.height(), Qt::AlignLeft | Qt::AlignVCenter, text);
        }

        painter->restore();
      }
    }
    painter->restore();
  }
}

void GPSelectionIntervals::setInputUom(const GPUomDescriptor& descriptor)
{
  mInputUom = descriptor;

  if (mStartLine && mStopLine)
  {
    mStartLine->setInputUomForValues(descriptor);
    mStopLine->setInputUomForValues(descriptor);
  }
}

void GPSelectionIntervals::setInputUom(int quantity, int units)
{
  GPUomDescriptor descriptor;
  descriptor.quantity = quantity;
  descriptor.units = units;
  setInputUom(descriptor);
}

void GPSelectionIntervals::applyUom(const GPUomDescriptor& descriptor)
{
  applyUom(uomDescriptor, descriptor, mSelections.begin(), mSelections.end());
}

void GPSelectionIntervals::applyUom(const GPUomDescriptor& source, const GPUomDescriptor& target,
                                     QVector<QPair<double, double>>::iterator from,
                                     QVector<QPair<double, double>>::iterator to)
{
  if (mSelections.size() != 0)
  {
    if (source != target)
    {
      GPUomDataTransformation transformation = uomProvider->getUomTransformation(source);
      for (auto i = from; i != to; ++i)
      {
        i->first  = transformation(i->first,  source, target);
        i->second = transformation(i->second, source, target);
      }
    }
  }
}

///////////////////////////////////////////////////////////
// Vertical Line
GPOrientatedInfiniteLine::GPOrientatedInfiniteLine(GraphicsPlot* parentPlot, Qt::Orientation orientation)
  : GPAxisInfiniteLine(parentPlot)
  , mOrientation(orientation)
{
  mInteractive = true;
}

GPOrientatedInfiniteLine::~GPOrientatedInfiniteLine()
{
}

void GPOrientatedInfiniteLine::setKeyAxis(GPAxis *axis)
{
  start->setAxes(axis, start->keyAxis());
  end->setAxes(axis, end->keyAxis());
}

void GPOrientatedInfiniteLine::setValueAxis(GPAxis *axis)
{
  start->setAxes(start->keyAxis(), axis);
  end->setAxes(end->keyAxis(), axis);
}

void GPOrientatedInfiniteLine::setClipRect(const QRect& rect)
{
  mClipRect = rect;
}

Qt::CursorShape GPOrientatedInfiniteLine::HitTest(QMouseEvent* event, unsigned* part)
{
  if (mInteractive)
  {
    double distance;

    if (clipToAxisRect())
    {
      if (auto rect = parentPlot()->axisRectAt(event->pos()))
        if (!QRectF(rect->topLeft(), rect->bottomRight()).contains(event->pos()))
          return Qt::ArrowCursor;
    }

    if (mOrientation == Qt::Horizontal)
      distance = abs(event->pos().y() - start->pixelPosition().y());
    else
      distance = abs(event->pos().x() - start->pixelPosition().x());

    if (distance < parentPlot()->selectionTolerance()*0.99 * parentPlot()->selectionTolerance()*0.99)
    {
      if (part)
        *part = HitPartLine;

      if (mOrientation == Qt::Horizontal)
        return Qt::SplitVCursor;
      return Qt::SplitHCursor;
    }
  }
  return Qt::ArrowCursor;
}

double GPOrientatedInfiniteLine::selectTest(const QPointF &pos, bool, QVariant *) const
{
  if (!mSelectable || !mInteractive)
    return -1;

  if (clipToAxisRect())
  {
    auto rect = parentPlot()->axisRectAt(pos);
    if (rect == nullptr || !QRectF(rect->topLeft(), rect->bottomRight()).contains(pos))
      return -1;
  }

  return abs(mOrientation == Qt::Horizontal ? pos.y() - start->pixelPosition().y() : pos.x() - start->pixelPosition().x());
}

void GPOrientatedInfiniteLine::DragUpdateData(QMouseEvent* event)
{
  if (!mInteractive)
    return;
  if (mDraggingPart == HitPartLine || mDraggingPart == HitPartStart || mDraggingPart == HitPartEnd)
  {
    QPointF delta;
    if (mOrientation == Qt::Vertical)
      delta = QPointF(event->pos().x() - DraggingStartPos.x(), 0);
    else
      delta = QPointF(0, event->pos().y() - DraggingStartPos.y());

    QPointF pixelStart = DraggingStartPosStart + delta;
    QPointF pixelEnd = DraggingStartPosEnd + delta;
    start->setPixelPosition(pixelStart);
    end->setPixelPosition(pixelEnd);
    emit moved();
  }
}

GP::Interaction GPOrientatedInfiniteLine::selectionCategory() const
{
  return GP::iSelectItems;
}

QRect GPOrientatedInfiniteLine::clipRect() const
{
  if (mClipRect.isNull())
    return GPAxisInfiniteLine::clipRect();
  return mClipRect;
}

void GPOrientatedInfiniteLine::DragAccept(QMouseEvent* event)
{
  if (!mInteractive)
    return;
  DragUpdateData(event);
  mDraggingPart = HitPartNone;
  emit movedComplete();
}

void GPOrientatedInfiniteLine::setValue(const double &value)
{
  if (mOrientation == Qt::Horizontal)
  {
    start->setCoords(-1e7, value);
    end->setCoords(1e7, value);
  }
  else
  {
    start->setCoords(value, -1e7);
    end->setCoords(value, 1e7);
  }
}

double GPOrientatedInfiniteLine::getValue() const
{
  QPointF point = start->coords();
  return (mOrientation == Qt::Horizontal) ? point.y() : point.x();
}


///////////////////////////////////////////////////////
// ruler line
GPRulerLine::GPRulerLine(GraphicsPlot *parentPlot, Qt::Orientation orientation)
  : GPOrientatedInfiniteLine(parentPlot, orientation)
  , CurrentKeyAxis(nullptr)
{
  setSelectable(true);

  start->setAxes(nullptr, nullptr);
  end->setAxes(nullptr, nullptr);
  if (orientation == Qt::Horizontal)
  {
    start->setTypeX(GPItemPosition::ptAbsolute);
    end->setTypeX(GPItemPosition::ptAbsolute);
  }
  else
  {
    start->setTypeY(GPItemPosition::ptAbsolute);
    end->setTypeY(GPItemPosition::ptAbsolute);
  }

  ValueWidget = new QWidget(parentPlot);
  ValueWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  QVBoxLayout* layout = new QVBoxLayout(ValueWidget.data());

  AxisBox = new QComboBox(ValueWidget.data());
  ValueBox = new QDoubleSpinBox(ValueWidget.data());
  ValueBox->setMinimum(-1e7);
  ValueBox->setMaximum(1e7);
  ValueBox->setDecimals(2);

  TimeBox = new QDateTimeEdit(ValueWidget.data());
  TimeBox->hide();

  layout->addWidget(AxisBox);
  layout->addWidget(ValueBox);
  layout->addWidget(TimeBox);
  layout->setContentsMargins(0,0,0,0);
  ValueWidget->show();

  UpdateAxisList();

  connect(this, &GPRulerLine::moved, this, &GPRulerLine::UpdateWidgetPosition);
  connect(this, &GPRulerLine::moved, this, &GPRulerLine::ClearPoints);
  connect(this, &GPRulerLine::movedComplete, this, &GPRulerLine::UpdatePoints);

  connect(ValueBox, &QDoubleSpinBox::editingFinished, this, &GPRulerLine::UpdateValue);
  connect(ValueBox, &QDoubleSpinBox::editingFinished, this, &GPRulerLine::UpdatePoints);

  connect(TimeBox, &QDateTimeEdit::editingFinished, this, &GPRulerLine::UpdateValue);
  connect(TimeBox, &QDateTimeEdit::editingFinished, this, &GPRulerLine::UpdatePoints);

  connect(AxisBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetCurrentAxis()));

  connect(parentPlot, SIGNAL(viewportResized()), this, SLOT(UpdateWidgetPosition()));

  setPen(QPen(QColor("#dd4444"), 2));
  setSelectedPen(QPen(QColor("#aa9144dd"), 2));
  setSelectable(true);
}

GPRulerLine::GPRulerLine(GraphicsPlot *parentPlot, Qt::Orientation orientation, double keyCoord)
  : GPRulerLine(parentPlot, orientation)
{
  if (CurrentKeyAxis)
  {
    setValue(keyCoord);
  }
}

GPRulerLine::GPRulerLine(GraphicsPlot *parentPlot, Qt::Orientation orientation, QPoint pos)
  : GPRulerLine(parentPlot, orientation)
{
  if (CurrentKeyAxis)
  {
    double posPix = (orientation == Qt::Horizontal) ? pos.y() : pos.x();
    setValue(CurrentKeyAxis->pixelToCoord(posPix));
  }
}

GPRulerLine::~GPRulerLine()
{
  if (ValueWidget)
    delete ValueWidget.data();

  ClearPoints();
}

void GPRulerLine::setValue(const double &value)
{
  if (CurrentKeyAxis)
  {
    GPOrientatedInfiniteLine::setValue(value);
    UpdateWidgetPosition();
    UpdatePoints();
    parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
  }
}

double GPRulerLine::getValue() const
{
  if (CurrentKeyAxis)
  {
    if (mOrientation == Qt::Horizontal)
      return CurrentKeyAxis->pixelToCoord(start->pixelPosition().y());
    return CurrentKeyAxis->pixelToCoord(start->pixelPosition().x());
  }
  return qQNaN();
}


void GPRulerLine::OnAxisReseted()
{
  if (parentPlot()->isVisible())
  {
    UpdateAxisList();
    UpdateWidgetPosition();
    UpdatePoints();
  }
}

void GPRulerLine::OnCurveUpdated()
{
  if (parentPlot()->isVisible())
  {
    UpdateWidgetPosition();
    UpdatePoints();
  }
}

void GPRulerLine::UpdateValue()
{
  if (CurrentKeyAxis)
  {
    GPUomDescriptor uomOut = CurrentKeyAxis->getUomDescriptor();
    GPUomDescriptor uomIn;

    double value = 0.0;
    if (TimeBox->isVisible())
    {
      if (uomOut.isNull())
      {
        value = GPAxisAwareTicker::fromFormatedKey(CurrentKeyAxis, TimeBox->dateTime());
      }
      else
      {
        value = TimeBox->dateTime().toMSecsSinceEpoch();
        uomIn.quantity = UOM::PhysicalQuantityTime;
        uomIn.units    = UOM::TimeMilliSeconds;
      }
    }
    else
    {
      if (uomOut.isNull())
      {
        value = GPAxisAwareTicker::fromFormatedKey(CurrentKeyAxis, ValueBox->value());
      }
      else
      {
        value = ValueBox->value();
        uomIn = uomOut;
      }
    }

    if (!uomOut.isNull())
    {
      setInputUomForKeys(uomIn);
    }
    setValue(value);
  }
}

void GPRulerLine::UpdateAxisList()
{
  GPAxis::AxisType f, s;
  f = (mOrientation == Qt::Horizontal) ? GPAxis::atLeft : GPAxis::atTop;
  s = (mOrientation == Qt::Horizontal) ? GPAxis::atRight : GPAxis::atBottom;

  QList<GPAxis*> allAxes;
  for (const auto& rect : parentPlot()->axisRects())
  {
    for (const auto& axis : rect->axes())
    {
      if (axis->axisType() == f || axis->axisType() == s)
      {
        allAxes << axis;
      }
    }
  }

  AxisBox->clear();
  for (const auto& axis : allAxes)
  {
    if (!axis->label().isEmpty())
    {
      AxisBox->addItem(axis->label(), (long long)axis);
    }
  }

  if (AxisBox->count() == 0)
    AxisBox->addItem((mOrientation == Qt::Horizontal) ? "Y" : "X", (long long)((mOrientation == Qt::Horizontal) ? parentPlot()->yAxis : parentPlot()->xAxis));

  AxisBox->setVisible(AxisBox->count() > 1);

  ValueWidget->adjustSize();

  SetCurrentAxis();
}

void GPRulerLine::UpdateWidgetPosition()
{
  if (parentPlot()->axisRect() && CurrentKeyAxis)
  {
    double x, y;
    if (mOrientation == Qt::Vertical)
    {
      x = start->pixelPosition().x() + 5;
      y = parentPlot()->axisRect()->rect().top() + 3;
    }
    else
    {
      x = parentPlot()->axisRect()->rect().left() + 5;
      y = start->pixelPosition().y() - ValueWidget->height() - 3;
    }

    ValueWidget->move(x, y);

    QVariant value = GPAxisAwareTicker::toFormatedKey(CurrentKeyAxis, getValue());
    if (value.VARIANT_TYPE_ID() == QMetaType::QDateTime)
      TimeBox->setDateTime(value.toDateTime());
    else
      ValueBox->setValue(value.toDouble());
  }
}

void GPRulerLine::UpdatePoints()
{
  ClearPoints();

  auto plot = parentPlot();

  bool needShowAxisLabel = false;
  if (auto cpex = qobject_cast<GraphicsPlotExtended*>(plot))
    needShowAxisLabel = cpex->GetRulerShowAxesLabels();

  for (int i = 0; i < plot->curveCount(); ++i)
  {
    auto curve = plot->curve(i);
    if (curve->data()->size() > 2 && curve->visible())
    {
      auto kAxis = curve->keyAxis();
      auto vAxis = curve->valueAxis();
      if (kAxis && vAxis)
      {
        double pix = (mOrientation == Qt::Horizontal) ? start->pixelPosition().y() : start->pixelPosition().x();
        double val = GraphicsPlotExtendedUtils::interpolate(curve->data(), kAxis->pixelToCoord(pix), true);
        if (!qIsNaN(val))
        {
          QPointF coords;
          if (mOrientation == Qt::Horizontal)
            coords.setX(kAxis->pixelToCoord(start->pixelPosition().y()));
          else
            coords.setX(kAxis->pixelToCoord(start->pixelPosition().x()));
          coords.setY(val);

          QString text;
          if (needShowAxisLabel)
          {
            QString valueName = vAxis->label();
            if (valueName.isEmpty())
              valueName = (mOrientation == Qt::Horizontal) ? "x" : "y";
            text = valueName + QString(": ");
          }

          if (kAxis->ticker() && vAxis->ticker())
            text += vAxis->ticker()->getTickLabel(coords.y(), QLocale::system(), 'f', 3);

          GPInteractivePoint* point = new GPInteractivePoint(kAxis, vAxis, false);
          point->setPen(QPen(QBrush(Qt::blue), 2));
          point->selectionDecorator()->setPen(QPen(QBrush(Qt::blue), 2));
          point->setSelectable(GP::stNone);
          point->data()->setInputUomForKeys(kAxis->getUomDescriptor());
          point->data()->setInputUomForValues(vAxis->getUomDescriptor());
          point->setCoord(coords);
          point->setLabelText(text);

          Points.append(point);

          connect(kAxis, SIGNAL(destroyed(QObject*)), this, SLOT(UpdatePoints()), Qt::UniqueConnection);
          connect(vAxis, SIGNAL(destroyed(QObject*)), this, SLOT(UpdatePoints()), Qt::UniqueConnection);
        }
      }
    }
  }
  plot->replot(GraphicsPlot::rpQueuedReplot);
}

void GPRulerLine::ClearPoints()
{
  for (const auto& point : Points)
  {
    if (parentPlot()->hasPlottable(point))
      parentPlot()->removePlottable(point);
  }
  Points.clear();
}

void GPRulerLine::SetCurrentAxis()
{
  if (CurrentKeyAxis)
    disconnect(CurrentKeyAxis, SIGNAL(rangeChanged(GPRange)), this, SLOT(UpdateWidgetPosition()));

  bool ok = true;
  CurrentKeyAxis = (GPAxis*)AxisBox->currentData().toLongLong(&ok);
  if (!ok)
    CurrentKeyAxis = nullptr;
  else
    connect(CurrentKeyAxis, SIGNAL(rangeChanged(GPRange)), this, SLOT(UpdateWidgetPosition()));

  if (mOrientation == Qt::Horizontal)
  {
    start->setAxes(nullptr, CurrentKeyAxis);
    end->setAxes(nullptr, CurrentKeyAxis);
  }
  else
  {
    start->setAxes(CurrentKeyAxis, nullptr);
    end->setAxes(CurrentKeyAxis, nullptr);
  }

  if (CurrentKeyAxis)
  {
    auto time = dynamic_cast<GPAxisAwareTicker*>(CurrentKeyAxis->ticker().data());
    if (time && time->isFormatedTime())
    {
      TimeBox->show();
      ValueBox->hide();
      TimeBox->setDisplayFormat(time->dateTimeFormat());
      TimeBox->setMinimumWidth(120);
      ValueWidget->setMinimumWidth(120);
    }
    else
    {
      TimeBox->hide();
      ValueBox->show();
      ValueWidget->setMinimumWidth(80);
    }
    UpdateValue();
  }
}


///////////////////////////////////////////////////////
//  distance ruler
GPRulerDistance::GPRulerDistance(GPAxis *axisX, GPAxis *axisY, QPoint pos)
  : GPInteractiveCurve(axisX, axisY)
{
  double x1 = axisX->pixelToCoord(pos.x() - 50);
  double x2 = axisX->pixelToCoord(pos.x() + 50);
  double y1 = axisY->pixelToCoord(pos.y());
  double y2 = y1;

  addData(x1, y1);
  addData(x2, y2);

  setPen(QPen(QColor("#dd4444"), 2));

  mLabel = new GPCurveLabel(this);
  UpdateDistanceLabel();
}

GPRulerDistance::~GPRulerDistance()
{

}

void GPRulerDistance::DragUpdateData(QMouseEvent *event)
{
  GPInteractiveCurve::DragUpdateData(event);
  UpdateDistanceLabel();
}

void GPRulerDistance::UpdateDistanceLabel()
{
  if (data()->size() == 2)
  {
    double dy = data()->at(1)->key - data()->at(0)->key;
    double dx = data()->at(1)->value - data()->at(0)->value;
    double d = sqrt(dy * dy + dx * dx);
    mLabel->setText(QString::number(d));
  }
}


///////////////////////////////////////////////////////
// Simple text legend item
GPTextLegendItem::GPTextLegendItem(GPLegend *parent, const QString& text)
  : GPAbstractLegendItem(parent)
  , mText(text)
{
  parent->addItem(this);
  setMargins(QMargins(0, 0, 4, 4));
}

GPTextLegendItem::~GPTextLegendItem()
{

}

void GPTextLegendItem::setText(const QString& text)
{
  mText = text;
}

QString GPTextLegendItem::getText() const
{
  return mText;
}

void GPTextLegendItem::draw(GPPainter *painter)
{
  painter->setFont(mFont);
  QColor textColor = mTextColor;
  painter->setPen(QPen(textColor));
  QRectF textRect = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, mText);
  painter->drawText(mRect.x() + mParentLegend->iconTextPadding()
                    , mRect.y()
                    , textRect.width()
                    , mRect.height()
                    , Qt::TextDontClip | Qt::AlignVCenter
                    , mText);
}

QSize GPTextLegendItem::minimumOuterSizeHint() const
{
  QSize result(0, 0);
  QRect textRect;
  QFontMetrics fontMetrics(mFont);
  textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip, mText);
  result.setWidth(mParentLegend->iconTextPadding() + textRect.width());
  result.setHeight(textRect.height());
  result.rwidth() += mMargins.left() + mMargins.right();
  result.rheight() += mMargins.top() + mMargins.bottom();
  return std::move(result);
}


///////////////////////////////////////////////////////
// legend with scrollable plottables section
GPEXLegend::GPEXLegend(GraphicsPlot* parent)
  : GPLegend()
  , mNeedUpdate(false)
  , mVisibleScrolledCount(10)
  , mScrollBar(nullptr)
  , mScrolledItemsSectionStartIndex(-1)
  , mScrolledItemsSectionTop(0)
{
  setParentPlot(parent);
  if (auto rect = parent->axisRect())
    if (auto layout = rect->insetLayout())
      layout->addElement(this, Qt::AlignRight|Qt::AlignTop);
  setLayer(QLatin1String("legend"));
}

GPEXLegend::~GPEXLegend()
{
}

bool GPEXLegend::addItem(GPAbstractLegendItem* item)
{
  if (item->property("non_persistent").toBool() || qobject_cast<GPPlottableLegendItem*>(item))
    item->setVisible(false);
  return GPLegend::addItem(item);
}

QSize GPEXLegend::minimumOuterSizeHint() const
{
  QSize size = GPLegend::minimumOuterSizeHint();
  int maxWidth = 0;
  for (int i = 0; i < mElements.size(); ++i)
  {
    QPointer<GPLayoutElement> item = mElements[i].value(0, nullptr);
    if (item)
      maxWidth = qMax(maxWidth, getFinalMinimumOuterSize(item).width());
  }
  size.setWidth(maxWidth + mMargins.left() + mMargins.right());
  *(const_cast<QSize*>(&mMinimumSize)) = QSize(size.width(), mMinimumSize.height());
  return size;
}

void GPEXLegend::setOuterRect(const QRect &rect)
{
  if (mOuterRect != rect)
  {
    mOuterRect = rect;

    if (mMinimumSize.width() > 0)
      mOuterRect.setWidth(qMax(mOuterRect.width(), mMinimumSize.width()));
    mRect = mOuterRect.adjusted(mMargins.left(), mMargins.top(), -mMargins.right(), -mMargins.bottom());

    GPLegend::sizeConstraintsChanged();
    updateScrollBarGeometry();
  }
}

void GPEXLegend::sizeConstraintsChanged()
{
  mNeedUpdate = true;
  GPLegend::sizeConstraintsChanged();
}

void GPEXLegend::updateLayout()
{
  if (mNeedUpdate)
  {
    updateScrollAreaItems();
    mNeedUpdate = false;
  }
  GPLegend::updateLayout();
}

void GPEXLegend::updateScrollAreaItems()
{
  simplify();

  int firstVisible = 0;
  for (int i = 0; i < mScrolledItems.size(); ++i)
  {
    if (mScrolledItems[i]->visible())
    {
      firstVisible = i;
      break;
    }
  }

  mScrolledItems.clear();
  QMap<int, GPLayoutElement*> persistentItems;
  int maxWidth = 0;
  mScrolledItemsSectionStartIndex = -1;
  for (int i = 0; i < mElements.size(); ++i)
  {
    QPointer<GPAbstractLegendItem> item = qobject_cast<GPAbstractLegendItem*>(mElements[i].value(0, nullptr));
    if (item)
    {
      maxWidth = qMax(maxWidth, getFinalMinimumOuterSize(item).width());

      if (qobject_cast<GPPlottableLegendItem*>(item) || item->property("non_persistent").toBool())
      {
        item->setVisible(true);
        mScrolledItems.append(item);
        if (mScrolledItemsSectionStartIndex == -1)
          mScrolledItemsSectionStartIndex = i;
      }
      else
      {
        persistentItems[i] = item;
      }
    }
  }

  mMinimumSize.setWidth(maxWidth + mMargins.left() + mMargins.right());
  setScrollBarVisible(mScrolledItems.size() > mVisibleScrolledCount);

  if (mScrolledItemsSectionStartIndex == -1 || mScrolledItems.size() <= mVisibleScrolledCount)
  {
    mScrolledItemsSectionTop = 0;
    return;
  }

  for (auto it = persistentItems.begin(); it != persistentItems.end(); ++it)
  {
    if (it.key() > mScrolledItemsSectionStartIndex)
    {
      auto row = mElements.takeAt(it.key());
      mElements.insert(mScrolledItemsSectionStartIndex, row);
      mScrolledItemsSectionStartIndex++;
    }
  }

  QVector<int> minColWidths, minRowHeights, maxColWidths, maxRowHeights;
  getMinimumRowColSizes(&minColWidths, &minRowHeights);
  getMaximumRowColSizes(&maxColWidths, &maxRowHeights);

  int totalRowHeight = (rowCount() - 1) * mRowSpacing + std::accumulate(minRowHeights.begin(), minRowHeights.end(), 0);
  QVector<int> rowHeights = getSectionSizes(maxRowHeights, minRowHeights, mRowStretchFactors.toVector(), totalRowHeight);

  mScrolledItemsSectionTop = 0;
  for (int row = 0; row < rowCount() && row < mScrolledItemsSectionStartIndex; ++row)
  {
    mScrolledItemsSectionTop += rowHeights.at(row);
    if (row > 0)
      mScrolledItemsSectionTop += mRowSpacing;
  }

  if (mScrollBar)
  {
    mScrollBar->setPageStep(mVisibleScrolledCount);
    mScrollBar->setRange(0, mScrolledItems.size() - mVisibleScrolledCount);
    mScrollBar->setValue(firstVisible);

    onScrollBarValueChanged();
  }
}

void GPEXLegend::selectItem(GPPlottableLegendItem* item, Qt::KeyboardModifiers modifier)
{
  if (modifier & Qt::ControlModifier)
  {
    item->setSelected(true);
  }
  else if (modifier & Qt::ShiftModifier)
  {
    if (!mLastSelectedItem.isNull() && item != mLastSelectedItem)
    {
      QSet<GPAbstractLegendItem*> selectionBorders;
      selectionBorders << item << mLastSelectedItem;
      for (int i = 0; i < mScrolledItems.size(); ++i)
      {
        if (auto im = mScrolledItems[i])
        {
          if (selectionBorders.contains(im))
          {
            im->setSelected(true);
            selectionBorders.remove(im);
          }
          else
          {
            im->setSelected(selectionBorders.size() == 1);
          }
        }
      }
    }
    else
    {
      if (mLastSelectedItem)
        mLastSelectedItem->setSelected(false);
      item->setSelected(true);
    }
  }
  else
  {
    for (int i = 0; i < mScrolledItems.size(); ++i)
    {
      if (auto im = mScrolledItems[i])
      {
        im->setSelected(false);
      }
    }
    item->setSelected(true);
  }
  mLastSelectedItem = item;
}

void GPEXLegend::setScrollBarVisible(bool visible)
{
  if (!mScrollBar)
  {
    if (visible)
    {
      mScrollBar = new QScrollBar(parentPlot());
      connect(mScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onScrollBarValueChanged()));
      updateScrollBarGeometry();
      mScrollBar->setVisible(visible);
    }
  }
  else
  {
    mScrollBar->setVisible(visible);
  }
}

void GPEXLegend::updateScrollBarGeometry()
{
  if (mScrollBar)
  {
    mScrollBar->setGeometry(mOuterRect.right() - mScrollBarWidth
                            , mRect.top() + mScrolledItemsSectionTop
                            , mScrollBarWidth
                            , mRect.bottom() - mRect.top() - mScrolledItemsSectionTop - mMargins.bottom());
  }
}

void GPEXLegend::wheelEvent(QWheelEvent *event)
{
  auto pos =
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  event->pos();
#else
  event->position();
#endif
  if (mScrollBar && mRect.contains(pos.x(), pos.y()))
    QApplication::sendEvent(mScrollBar, event);
}

void GPEXLegend::onScrollBarValueChanged()
{
  if (mScrollBar)
  {
    bool changed = false;
    for (int i = 0; i < mScrolledItems.size(); ++i)
    {
      bool oldVisible = mScrolledItems[i]->visible();
      mScrolledItems[i]->setVisible(!mScrollBar->isVisible() ||
                                     ((i >= mScrollBar->value()) && (i < mScrollBar->value() + mScrollBar->pageStep())));
      if (mScrolledItemsSectionStartIndex + i < mRowStretchFactors.size())
        mRowStretchFactors[mScrolledItemsSectionStartIndex + i] = mScrolledItems[i]->visible() ? 1 : 0;
      changed |= (oldVisible == mScrolledItems[i]->visible());
    }
    if (changed)
      parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
  }
}


///////////////////////////////////////////////////////
// Compass legend item
GPCompassItem::GPCompassItem(GPLegend *parent)
  : GPAbstractLegendItem(parent)
{
  mRect.setSize(QSize(70, 70));
  mOuterRect.setSize(QSize(70, 70));
}

GPCompassItem::~GPCompassItem()
{

}

void GPCompassItem::draw(GPPainter *painter)
{
  painter->setFont(mFont);
  QColor textColor = mTextColor;
  painter->setPen(QPen(textColor, 1));

  mRect.setSize(QSize(70, 70));
  painter->drawEllipse(mRect.left() + 1, mRect.top(), mRect.width() - 2, mRect.height() - 2);

  QRect rect = mRect;
  rect.moveTopLeft(rect.topLeft() + QPoint(21, 20));
  rect.setSize(QSize(30, 30));

  {
    QPoint left(rect.left(), rect.center().y());
    QPoint right(rect.right(), rect.center().y());
    QPoint top(rect.center().x(), rect.top());
    QPoint bottom(rect.center().x(), rect.bottom());

    painter->drawLine(left, right);
    painter->drawLine(top, bottom);

    painter->drawLine(left, left + QPoint(4, -2));
    painter->drawLine(left, left + QPoint(4, 2));

    painter->drawLine(right, right + QPoint(-4, -2));
    painter->drawLine(right, right + QPoint(-4, 2));

    painter->drawLine(top, top + QPoint(-2, 4));
    painter->drawLine(top, top + QPoint(2, 4));

    painter->drawLine(bottom, bottom + QPoint(-2, -4));
    painter->drawLine(bottom, bottom + QPoint(2, -4));
  }

  rect = mRect;
  rect.moveTopLeft(rect.topLeft() + QPoint(4, 2));
  rect.setSize(rect.size() - QSize(4, 4));
  auto font = painter->font();
  font.setBold(true);
  painter->setFont(font);
  {
    QRect textRect(QPoint(rect.left(), rect.top()), QPoint(rect.left() + 16, rect.bottom() - 2));
    painter->drawText(textRect, Qt::AlignCenter, tr("W"));
  }
  {
    QRect textRect(QPoint(rect.left(), rect.top()), QPoint(rect.right() - 2, rect.top() + 16));
    painter->drawText(textRect, Qt::AlignCenter, tr("N"));
  }
  {
    QRect textRect(QPoint(rect.right() - 16, rect.top()), QPoint(rect.right(), rect.bottom() - 2));
    painter->drawText(textRect, Qt::AlignCenter, tr("E"));
  }
  {
    QRect textRect(QPoint(rect.left(), rect.bottom() - 16), QPoint(rect.right() - 2, rect.bottom()));
    painter->drawText(textRect, Qt::AlignCenter, tr("S"));
  }
}

QSize GPCompassItem::minimumOuterSizeHint() const
{
  return QSize(70, 70);
}

///////////////////////////////////////////////////////
// Caption item
GPCaptionItem::GPCaptionItem(GraphicsPlot* parent)
  : GPLayerable(parent)
  , mId(QUuid::createUuid())
  , mFont(parent->font())
  , mHasLine(false)
  , mPadding(5)
{
}

GPCaptionItem::GPCaptionItem(GPLayerable* layerable, const QString& text, bool hasLine, const QPointF& pos)
  : GPCaptionItem(layerable->parentPlot())
{
  setParent(layerable);
  mTextPos = pixelsToCoords(pos);
  setText(text);
  setHasLine(hasLine);
}

GPCaptionItem::~GPCaptionItem()
{
}

void GPCaptionItem::setParent(GPLayerable *layerable)
{
  mParentLayerable = layerable;
  if (auto curve = qobject_cast<GPCurve*>(layerable))
  {
    setTarget(curve);
  }
  else if (auto axis = qobject_cast<GPAxis*>(layerable))
  {
    GPAxis* valueAxis = axis;
    GPAxis* keyAxis = 0;
    if (valueAxis->orientation() == Qt::Vertical)
    {
      if (auto ka = valueAxis->axisRect()->axis(GPAxis::atBottom))
        keyAxis = ka;
      else if (auto ka = valueAxis->axisRect()->axis(GPAxis::atTop))
        keyAxis = ka;
    }
    else
    {
      if (auto ka = valueAxis->axisRect()->axis(GPAxis::atLeft))
        keyAxis = ka;
      else if (auto ka = valueAxis->axisRect()->axis(GPAxis::atRight))
        keyAxis = ka;
    }
    setAxes(keyAxis, valueAxis);
  }
}

void GPCaptionItem::setParent(const QString& type, const QString& id)
{
  GPLayerable* layerable = nullptr;

  if (type == GPCurve::staticMetaObject.className())
  {
    for (int i = 0; i < mParentPlot->curveCount(); ++i)
    {
      if (auto curve = mParentPlot->curve(i))
      {
        if (curve->property(OKCurveId).toString() == id)
        {
          layerable = curve;
          break;
        }
      }
    }
  }
  else if (type == GPAxis::staticMetaObject.className())
  {
    for (int i = 0; i < mParentPlot->axisRectCount(); ++i)
    {
      if (auto rect = mParentPlot->axisRect(i))
      {
        for (const auto& axis : rect->axes())
        {
          if (axis && axis->property(OKAxisId).toString() == id)
          {
            layerable = axis;
            break;
          }
        }
      }
    }
  }

  if (layerable)
  {
    setParent(layerable);
  }
}

QString GPCaptionItem::getParentType() const
{
  return mParentLayerable->metaObject()->className();
}

QString GPCaptionItem::getParentId() const
{
  if (qobject_cast<GPCurve*>(mParentLayerable))
    return mParentLayerable->property(OKCurveId).toString();
  if (qobject_cast<GPAxis*>(mParentLayerable))
    return mParentLayerable->property(OKAxisId).toString();
  return QString();
}

void GPCaptionItem::setId(const QUuid& id)
{
  mId = id;
}

QUuid GPCaptionItem::getId() const
{
  return mId;
}

void GPCaptionItem::setText(const QString &text)
{
  mText = text;
}

QString GPCaptionItem::getText() const
{
  return mText;
}

void GPCaptionItem::setFont(const QFont &font)
{
  mFont = font;
}

QFont GPCaptionItem::getFont() const
{
  return mFont;
}

void GPCaptionItem::setTextColor(const QColor &color)
{
  mTextColor = color;
}

QColor GPCaptionItem::getTextColor() const
{
  return mTextColor;
}

void GPCaptionItem::setTarget(GPCurve* target)
{
  mTarget = target;
  setAxes(target->keyAxis(), target->valueAxis());
}

GPCurve* GPCaptionItem::getTarget() const
{
  return mTarget;
}

void GPCaptionItem::setHasLine(bool has)
{
  if (mHasLine != has)
  {
    if (!mHasLine)
    {
      mPointPos = mTextPos;
      if (mValueAxis && mKeyAxis)
      {
        mPointPos = coordsToPixels(mPointPos);
        mPointPos.setY(mPointPos.y() + 30);
        mPointPos = pixelsToCoords(mPointPos);
      }
      else
      {
        mPointPos.setY(mPointPos.y() + 30);
      }
    }
    mHasLine = has;
  }
}

bool GPCaptionItem::getHasLine() const
{
  return mHasLine;
}

void GPCaptionItem::setTextPos(const QPointF& pos)
{
  mTextPos = pos;
}

QPointF GPCaptionItem::getTextPos() const
{
  return mTextPos;
}

void GPCaptionItem::setPointPos(const QPointF& pos)
{
  mPointPos = pos;
}

QPointF GPCaptionItem::getPointPos()
{
  return mPointPos;
}

void GPCaptionItem::DragStart(QMouseEvent*, unsigned part)
{
  mDraggingPart = part;
}

void GPCaptionItem::DragMove(QMouseEvent *event)
{
  if (mKeyAxis && mValueAxis)
  {
    QPointF pos = pixelsToCoords(event->pos());

    switch (mDraggingPart)
    {
      case HitPartText:
        {
          mTextPos = pos;
        }
        break;
      case HitPartPoint:
        {
          if (mTarget)
          {
            bool hasRange = false;
            auto range = mTarget->getKeyRange(hasRange);
            if (hasRange && range.contains(pos.x()))
            {
              double dy = GraphicsPlotExtendedUtils::interpolate(mTarget->data(), pos.x());
              if (fabs(mValueAxis->coordToPixel(dy) - mValueAxis->coordToPixel(pos.y())) < 10)
                pos.setY(dy);
            }
          }
          mPointPos = pos;
        }
        break;
      default:
        break;
    }
  }
}

void GPCaptionItem::DragAccept(QMouseEvent* /*event*/)
{
  mDraggingPart = HitPartNone;
}

Qt::CursorShape GPCaptionItem::HitTest(QMouseEvent* event, unsigned* part)
{
  if (mKeyAxis && mValueAxis && mKeyAxis->axisRect()->rect().contains(event->pos()))
  {
    if (part)
      *part = HitPartNone;

    QFontMetrics fontMetrics(mFont);
    QRectF textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignCenter, mText);
    QRectF textBoxRect = textRect.adjusted(-mPadding, -mPadding, mPadding, mPadding);
    textBoxRect.moveCenter(coordsToPixels(mTextPos));
    if (textBoxRect.contains(event->pos()))
    {
      if (part)
        *part = HitPartText;
      return Qt::SizeAllCursor;
    }

    if (mHasLine)
    {
      if ((coordsToPixels(mPointPos) - event->pos()).manhattanLength() < parentPlot()->selectionTolerance() * 0.99)
      {
        if (part)
          *part = HitPartPoint;
        return Qt::SizeAllCursor;
      }
    }
  }
  return Qt::ArrowCursor;
}

double GPCaptionItem::selectTest(const QPointF &pos, bool /*onlySelectable*/, QVariant* /*details*/) const
{
  if (mKeyAxis && mValueAxis && mKeyAxis->axisRect()->rect().contains(pos.toPoint()))
  {
    QFontMetrics fontMetrics(mFont);
    QRectF textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignCenter, mText);
    QRectF textBoxRect = textRect.adjusted(-mPadding, -mPadding, mPadding, mPadding);
    textBoxRect.moveCenter(coordsToPixels(mTextPos));
    if (textBoxRect.contains(pos))
      return 0.0;

    if (mHasLine)
    {
      if ((coordsToPixels(mPointPos) - pos).manhattanLength() < parentPlot()->selectionTolerance() * 0.99)
        return 0.0;
    }
  }

  return  -1;
}

bool GPCaptionItem::eventFilter(QObject* obj, QEvent* e)
{
  if (mRunningMenu && mRunningMenu == obj)
  {
    if (e->type() == QEvent::KeyPress)
    {
      QKeyEvent* k = (QKeyEvent*)e;
      if (k->key() == Qt::Key_Alt)
      {
        return true;
      }
    }
  }
  return GPLayerable::eventFilter(obj, e);
}

void GPCaptionItem::mouseDoubleClickEvent(QMouseEvent* event, const QVariant& /*details*/)
{
  QMenu menu;
  menu.installEventFilter(this);
  mRunningMenu = &menu;

  QLineEdit* editor = new QLineEdit;
  editor->setFixedSize(100, 24);
  editor->setFrame(false);
  editor->setText(mText);

  QWidgetAction* action = new QWidgetAction(&menu);
  action->setDefaultWidget(editor);

  menu.addAction(action);
  menu.move(event->globalPos());
  menu.show();
  editor->setFocus();
  connect(editor, &QLineEdit::editingFinished, this, [&menu](){ menu.close(); });
  menu.exec();

  mText = editor->text();
}

void GPCaptionItem::draw(GPPainter* painter)
{
  painter->save();

  QColor color;
  if (mTarget)
    color = mTarget->pen().color();
  painter->setPen(QPen(color, 2, Qt::SolidLine));
  painter->setBrush(color);

  QRect rect = mParentPlot->rect();
  if (mValueAxis)
    rect = mValueAxis->axisRect()->rect();
  painter->setClipRect(rect);

  QPointF textPixelPos = coordsToPixels(mTextPos);

  if (mHasLine)
  {
    QPointF pointPixelPos = coordsToPixels(mPointPos);
    painter->drawLine(textPixelPos, pointPixelPos);
    painter->drawEllipse(pointPixelPos, 2, 2);
  }

  QFontMetrics fontMetrics(mFont);
  QRectF textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignCenter, mText);
  QRectF textBoxRect = textRect.adjusted(-mPadding, -mPadding, mPadding, mPadding);
  textBoxRect.moveCenter(textPixelPos);

  painter->setBrush(Qt::white);
  painter->drawRect(textBoxRect);

  painter->setFont(mFont);
  painter->setPen(mTextColor);
  painter->drawText(textBoxRect, Qt::AlignCenter, mText);

  painter->restore();
}

QPointF GPCaptionItem::pixelsToCoords(const QPointF& point) const
{
  if (mValueAxis && mKeyAxis)
  {
    if (mValueAxis->orientation() == Qt::Vertical)
      return QPointF(mKeyAxis->pixelToCoord(point.x()), mValueAxis->pixelToCoord(point.y()));
    return QPointF(mValueAxis->pixelToCoord(point.x()), mKeyAxis->pixelToCoord(point.y()));
  }
  return QPointF();
}

QPointF GPCaptionItem::coordsToPixels(const QPointF& point) const
{
  if (mValueAxis && mKeyAxis)
  {
    if (mValueAxis->orientation() == Qt::Vertical)
      return QPointF(mKeyAxis->coordToPixel(point.x()), mValueAxis->coordToPixel(point.y()));
    return QPointF(mValueAxis->coordToPixel(point.x()), mKeyAxis->coordToPixel(point.y()));
  }
  return QPointF();
}

void GPCaptionItem::setAxes(GPAxis* keyAxes, GPAxis* valueAxes)
{
  mKeyAxis = keyAxes;
  mValueAxis = valueAxes;
  setUomProvider(mKeyAxis, mValueAxis);
}

void GPCaptionItem::applyUomForKeys(const GPUomDescriptor& target)
{
  double ktp = mTextPos.x();
  double kpp = mPointPos.x();

  GPUomDescriptor source = listenerKeys.getUomDescriptor();
  GPUomDataTransformation transformation = listenerKeys.getUomProvider()->getUomTransformation(source);
  ktp = transformation(ktp, source, target);
  kpp = transformation(kpp, source, target);

  mTextPos.setX(ktp);
  mPointPos.setX(kpp);
}

void GPCaptionItem::applyUomForValues(const GPUomDescriptor& target)
{
  double vtp = mTextPos.y();
  double vpp = mPointPos.y();

  GPUomDescriptor source = listenerValues.getUomDescriptor();
  GPUomDataTransformation transformation = listenerValues.getUomProvider()->getUomTransformation(source);
  vtp = transformation(vtp, source, target);
  vpp = transformation(vpp, source, target);

  mTextPos.setY(vtp);
  mPointPos.setY(vpp);
}

///////////////////////////////////////////////////////
// grid layout with vertical/horizontal splitters
GPLayoutSplitterGrid::GPLayoutSplitterGrid(GraphicsPlot* parentPlot, Qt::Orientation orientation)
  : GPLayoutGrid()
  , mOrientation(orientation)
  , mSplitterWidth(10)
  , mSplitterLineWidth(2)
  , mSplitterLineColor(Qt::gray)
  , mSplitterLineStyle(Qt::SolidLine)
  , mDraggingSplitterIndex(-1)
  , mDraggingSizeLeft(0)
  , mDraggingSizeRight(0)
  , mBlockLayoutUpdate(false)
{
  mParentPlot = parentPlot;
  if (mParentPlot)
    setLayer(mParentPlot->currentLayer());
}

GPLayoutSplitterGrid::~GPLayoutSplitterGrid()
{

}

void GPLayoutSplitterGrid::setBlockLayoutUpdate(bool block)
{
  mBlockLayoutUpdate = block;
  if (!mBlockLayoutUpdate)
    sizeConstraintsChanged();
}

void GPLayoutSplitterGrid::setColumnStretchFactor(int column, double factor)
{
  if (column >= 0 && column < columnCount())
  {
    mColumnStretchFactors[column] = factor;
    checkFactors();
    if (column < mDefaultColumnStretchFactors.size())
      mDefaultColumnStretchFactors[column] = factor;
  }
}

void GPLayoutSplitterGrid::setColumnStretchFactors(const QList<double> &factors)
{
  if (factors.size() == mColumnStretchFactors.size())
    mColumnStretchFactors = factors;
  else
    qDebug() << Q_FUNC_INFO << "Column count not equal to passed stretch factor count:" << factors;
  mDefaultColumnStretchFactors = factors;
}

void GPLayoutSplitterGrid::setRowStretchFactor(int row, double factor)
{
  if (row >= 0 && row < rowCount())
  {
    mRowStretchFactors[row] = factor;
    checkFactors();
    if (row < mDefaultRowStretchFactors.size())
      mDefaultRowStretchFactors[row] = factor;
  }
}

void GPLayoutSplitterGrid::setRowStretchFactors(const QList<double> &factors)
{
  if (factors.size() == mRowStretchFactors.size())
    mRowStretchFactors = factors;
  else
    qDebug() << Q_FUNC_INFO << "Row count not equal to passed stretch factor count:" << factors;
  mDefaultRowStretchFactors = factors;
}

void GPLayoutSplitterGrid::swapColumns(int left, int right)
{
  if (left >= 0 && left < columnCount() && right >= 0 && right < columnCount())
  {
    checkFactors();
    mDefaultColumnStretchFactors.swapItemsAt(left, right);
    mColumnStretchFactors.swapItemsAt(left, right);
    for (int r = 0; r < rowCount(); ++r)
    {
      if (mElements[r].size() > left && mElements[r].size() > right)
        mElements[r].swapItemsAt(left, right);
    }
  }
}

void GPLayoutSplitterGrid::swapRows(int top, int bottom)
{
  if (top >= 0 && top < rowCount() && bottom >= 0 && bottom < rowCount())
  {
    checkFactors();
    mDefaultRowStretchFactors.swapItemsAt(top, bottom);
    mRowStretchFactors.swapItemsAt(top, bottom);
    mElements.swapItemsAt(top, bottom);
  }
}

void GPLayoutSplitterGrid::insertSplitter(int index)
{
  mSplitters.insert(index);
}

void GPLayoutSplitterGrid::removeSplitter(int index)
{
  mSplitters.remove(index);
}

void GPLayoutSplitterGrid::removeAllSplitters()
{
  mSplitters.clear();
  mSplittersCoords.clear();
  resetSplitters();
}

void GPLayoutSplitterGrid::resetSplitters()
{
  while (mDefaultColumnStretchFactors.size() < columnCount())
    mDefaultColumnStretchFactors.append(1);
  while (mDefaultRowStretchFactors.size() < rowCount())
    mDefaultRowStretchFactors.append(1);

  mColumnStretchFactors = mDefaultColumnStretchFactors;
  mRowStretchFactors = mDefaultRowStretchFactors;
  parentPlot()->replot();
}

void GPLayoutSplitterGrid::setSplitterWidth(int width)
{
  mSplitterWidth = width;
}

void GPLayoutSplitterGrid::setSplitterLineWidth(int width)
{
  mSplitterLineWidth = width;
}

void GPLayoutSplitterGrid::setSplitterLineColor(const QColor& color)
{
  mSplitterLineColor = color;
}

void GPLayoutSplitterGrid::setSplitterLineStyle(Qt::PenStyle style)
{
  mSplitterLineStyle = style;
}

void GPLayoutSplitterGrid::insertRow(int newIndex)
{
  GPLayoutGrid::insertRow(newIndex);

  mDefaultRowStretchFactors.insert(newIndex, 1);
  if (mOrientation == Qt::Horizontal)
    moveSplitterIndexes(newIndex, 1);
}

void GPLayoutSplitterGrid::insertColumn(int newIndex)
{
  GPLayoutGrid::insertColumn(newIndex);

  mDefaultColumnStretchFactors.insert(newIndex, 1);
  if (mOrientation == Qt::Vertical)
    moveSplitterIndexes(newIndex, 1);
}

void GPLayoutSplitterGrid::removeRow(int index)
{
  GPLayoutGrid::removeRow(index);

  mDefaultRowStretchFactors.removeAt(index);
  mSplitters.remove(index);
  if (mOrientation == Qt::Horizontal)
    moveSplitterIndexes(index, -1);
}

void GPLayoutSplitterGrid::removeColumn(int index)
{
  GPLayoutGrid::removeColumn(index);

  mDefaultColumnStretchFactors.removeAt(index);
  mSplitters.remove(index);
  if (mOrientation == Qt::Vertical)
    moveSplitterIndexes(index, -1);
}

void GPLayoutSplitterGrid::moveSplitterIndexes(int fromIndex, int increment)
{
  QSet<int> newSplitters;
  for (const auto& index : mSplitters)
  {
    if (index >= fromIndex)
      newSplitters.insert(index + increment);
    else
      newSplitters.insert(index);
  }
  mSplitters = newSplitters;
}

void GPLayoutSplitterGrid::calculateSectionSizes(QVector<int> &rowHeights, QVector<int> &colWidths)
{
  bool isVertical = (mOrientation == Qt::Vertical);

  QVector<int> minColWidths, minRowHeights, maxColWidths, maxRowHeights;
  getMinimumRowColSizes(&minColWidths, &minRowHeights);
  getMaximumRowColSizes(&maxColWidths, &maxRowHeights);

  int splitterRowSpacing = qMax(mRowSpacing, mSplitterWidth);
  int splitterColSpacing = qMax(mColumnSpacing, mSplitterWidth);
  int splittersHeight = (!isVertical) ? (splitterRowSpacing - mRowSpacing) * mSplitters.size() : 0;
  int splittersWidth = (isVertical) ? (splitterColSpacing - mColumnSpacing) * mSplitters.size() : 0;
  int totalRowSpacing = (rowCount() - 1) * mRowSpacing + splittersHeight;
  int totalColSpacing = (columnCount() - 1) * mColumnSpacing + splittersWidth;

  colWidths = getSectionSizes(maxColWidths, minColWidths, mColumnStretchFactors.toVector(), mRect.width() - totalColSpacing);
  rowHeights = getSectionSizes(maxRowHeights, minRowHeights, mRowStretchFactors.toVector(), mRect.height() - totalRowSpacing);
}

void GPLayoutSplitterGrid::updateLayout()
{
  bool isVertical = (mOrientation == Qt::Vertical);

  int splitterRowSpacing = qMax(mRowSpacing, mSplitterWidth);
  int splitterColSpacing = qMax(mColumnSpacing, mSplitterWidth);

  QVector<int> colWidths;
  QVector<int> rowHeights;
  calculateSectionSizes(rowHeights, colWidths);

  mSplittersCoords.clear();

  // go through cells and set rects accordingly:
  int yOffset = mRect.top();
  for (int row = 0; row < rowCount(); ++row)
  {
    if (row > 0)
    {
      if (!isVertical && mSplitters.contains(row - 1))
      {
        yOffset += rowHeights.at(row - 1);
        mSplittersCoords[row - 1] = yOffset + splitterRowSpacing / 2;
        yOffset += splitterRowSpacing;
      }
      else
      {
        yOffset += rowHeights.at(row - 1) + mRowSpacing;
      }
    }

    int xOffset = mRect.left();
    for (int col = 0; col < columnCount(); ++col)
    {
      if (col > 0)
      {
        if (isVertical && mSplitters.contains(col - 1))
        {
          xOffset += colWidths.at(col - 1);
          mSplittersCoords[col - 1] = xOffset + splitterColSpacing / 2;
          xOffset += splitterColSpacing;
        }
        else
        {
          xOffset += colWidths.at(col - 1) + mColumnSpacing;
        }
      }

      if (row < mElements.size() && col < mElements[row].size())
        if (mElements.at(row).at(col))
          mElements.at(row).at(col)->setOuterRect(QRect(xOffset, yOffset, colWidths.at(col), rowHeights.at(row)));
    }
  }
}

QSize GPLayoutSplitterGrid::minimumOuterSizeHint() const
{
  QSize size = GPLayoutGrid::minimumOuterSizeHint();
  int splittersSize = mSplitterWidth * mSplitters.size();
  if (mOrientation == Qt::Vertical)
    size.setWidth(size.width() + splittersSize);
  else
    size.setHeight(size.height() + splittersSize);
  return size;
}

void GPLayoutSplitterGrid::moveSplitter(double delta)
{
  bool oldBlock = GraphicsPlot::getBlockReplots();
  GraphicsPlot::blockReplots(true);

  double leftMinWidth = 1, rightMinWidth = 1;
  if (mOrientation == Qt::Vertical)
  {
    leftMinWidth = getColumnMinimumWidth(mDraggingSplitterIndex);
    rightMinWidth = getColumnMinimumWidth(mDraggingSplitterIndex + 1);
  }
  else
  {
    leftMinWidth = getRowMinimumHeight(mDraggingSplitterIndex);
    rightMinWidth = getRowMinimumHeight(mDraggingSplitterIndex + 1);
  }

  if (mDraggingSizeLeft - leftMinWidth > -delta && mDraggingSizeRight - rightMinWidth > delta)
  {
    double newLeftFactor = (mDraggingSizeLeft + delta) / mDraggingSizeLeft * mDraggingStretchFactors.value(mDraggingSplitterIndex);
    double newRightFactor = (mDraggingSizeRight - delta) / mDraggingSizeRight * mDraggingStretchFactors.value(mDraggingSplitterIndex + 1);
    if (mOrientation == Qt::Vertical)
    {
      mColumnStretchFactors[mDraggingSplitterIndex] = newLeftFactor;
      mColumnStretchFactors[mDraggingSplitterIndex + 1] = newRightFactor;
    }
    else
    {
      mRowStretchFactors[mDraggingSplitterIndex] = newLeftFactor;
      mRowStretchFactors[mDraggingSplitterIndex + 1] = newRightFactor;
    }
  }

  GraphicsPlot::blockReplots(oldBlock);
}

int GPLayoutSplitterGrid::splitterAt(QPoint pos)
{
  int p = (mOrientation == Qt::Vertical) ? pos.x() : pos.y();
  for (const auto& index : mSplittersCoords.keys())
  {
    if (mSplittersCoords[index] - mSplitterLineWidth / 2 <= p && mSplittersCoords[index] + mSplitterLineWidth / 2 >= p)
      return index;
  }
  return -1;
}

void GPLayoutSplitterGrid::checkFactors()
{
  while (mRowStretchFactors.size() < rowCount())
    mRowStretchFactors.append(1);
  while (mColumnStretchFactors.size() < columnCount())
    mColumnStretchFactors.append(1);
  while (mDefaultRowStretchFactors.size() < rowCount())
    mDefaultRowStretchFactors.append(1);
  while (mDefaultColumnStretchFactors.size() < columnCount())
    mDefaultColumnStretchFactors.append(1);
}

int GPLayoutSplitterGrid::getColumnMinimumWidth(int column)
{
  int width = 0;
  for (int r = 0; r < rowCount(); ++r)
    if (auto el = mElements.value(r).value(column))
      width = qMax(width, getFinalMinimumOuterSize(el).width());
  return width;
}

int GPLayoutSplitterGrid::getRowMinimumHeight(int row)
{
  int height = 0;
  for (int c = 0; c < rowCount(); ++c)
    if (auto el = mElements.value(row).value(c))
      height = qMax(height, getFinalMinimumOuterSize(el).height());
  return height;
}

void GPLayoutSplitterGrid::sizeConstraintsChanged()
{
  if (!mBlockLayoutUpdate)
    GPLayoutGrid::sizeConstraintsChanged();
}

void GPLayoutSplitterGrid::mousePressEvent(QMouseEvent *event, const QVariant &details)
{
  if (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier)
  {
    int index = splitterAt(event->pos());
    if (index != -1)
    {
      mDraggingSizeLeft = 0;
      mDraggingSizeRight = 0;
      if (mOrientation == Qt::Vertical)
      {
        for (int i = 0; i < rowCount(); ++i)
        {
          auto el = mElements.value(i).value(index);
          auto er = mElements.value(i).value(index + 1);
          if (el && er)
          {
            mDraggingSizeLeft = el->outerRect().width();
            mDraggingSizeRight = er->outerRect().width();
          }
        }
      }
      else
      {
        for (int i = 0; i < columnCount(); ++i)
        {
          auto el = mElements.value(index).value(i);
          auto er = mElements.value(index + 1).value(i);
          if (el && er)
          {
            mDraggingSizeLeft = el->outerRect().height();
            mDraggingSizeRight = er->outerRect().height();
          }
        }
      }

      mDraggingStretchFactors = (mOrientation == Qt::Vertical) ? mColumnStretchFactors : mRowStretchFactors;
      mDraggingSplitterIndex = index;
      mBlockLayoutUpdate = true;
      return;
    }
  }
  GPLayoutGrid::mousePressEvent(event, details);
}

void GPLayoutSplitterGrid::mouseMoveEvent(QMouseEvent *event, const QPointF &startPos)
{
  if (mDraggingSplitterIndex != -1)
  {
    double delta = (mOrientation == Qt::Vertical) ? event->pos().x() - startPos.x() : event->pos().y() - startPos.y();
    moveSplitter(delta);
    if (delta > 0)
      GPLayoutGrid::sizeConstraintsChanged();
    mParentPlot->replot(GraphicsPlot::rpQueuedReplot);
    return;
  }
  GPLayoutGrid::mouseMoveEvent(event, startPos);
}

void GPLayoutSplitterGrid::mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos)
{
  if (mDraggingSplitterIndex != -1)
  {
    mBlockLayoutUpdate = false;
    mDraggingSplitterIndex = -1;
    sizeConstraintsChanged();
    return;
  }
  GPLayoutGrid::mouseReleaseEvent(event, startPos);
}

void GPLayoutSplitterGrid::mouseDoubleClickEvent(QMouseEvent *event, const QVariant &details)
{
  if (event->modifiers() & Qt::ControlModifier)
  {
    int index = splitterAt(event->pos());
    if (index != -1)
    {
      resetSplitters();
      return;
    }
  }
  GPLayoutGrid::mouseDoubleClickEvent(event, details);
}

Qt::CursorShape GPLayoutSplitterGrid::HitTest(QMouseEvent *event, unsigned*)
{
  bool onSplitter = mDraggingSplitterIndex != -1 || splitterAt(event->pos()) != -1;
  if (onSplitter)
    return mOrientation == Qt::Vertical ? Qt::SizeHorCursor : Qt::SizeVerCursor;
  return Qt::ArrowCursor;
}

void GPLayoutSplitterGrid::draw(GPPainter *painter)
{
  GPLayoutGrid::draw(painter);

  painter->setPen(QPen(mSplitterLineColor, mSplitterLineWidth, mSplitterLineStyle));
  for (const auto& coord : mSplittersCoords)
  {
    QPoint ls, le;
    if (mOrientation == Qt::Vertical)
    {
      ls = QPoint(coord, mRect.top());
      le = QPoint(coord, mRect.bottom());
    }
    else
    {
      ls = QPoint(mRect.left(), coord);
      le = QPoint(mRect.right(), coord);
    }
    painter->drawLine(ls, le);
  }
}


GPEXLayoutInfiniteGrid::GPEXLayoutInfiniteGrid(GraphicsPlot *parentPlot, Qt::Orientation orientation)
  : GPLayoutSplitterGrid(parentPlot, orientation)
  , mHasInfiniteRect(false)
  , mDefaultSectionSize(0)
{

}

GPEXLayoutInfiniteGrid::~GPEXLayoutInfiniteGrid()
{

}

void GPEXLayoutInfiniteGrid::setDefaultSectionSize(int size)
{
  mDefaultSectionSize = size;
}

int GPEXLayoutInfiniteGrid::getDefaultSectionSize() const
{
  return mDefaultSectionSize;
}

void GPEXLayoutInfiniteGrid::resetSplitters()
{
  for (int r = 0; r < rowCount(); ++r)
  {
    for (int c = 0; c < columnCount(); ++c)
    {
      if (auto el = mElements.value(r).value(c))
      {
        QSize size;
        if (mOrientation == Qt::Vertical)
          size = QSize(mDefaultSectionSize, el->minimumSize().height());
        else
          size = QSize(el->minimumSize().height(), mDefaultSectionSize);
        el->setMinimumSize(size);
      }
    }
  }

  parentPlot()->replot();
}

bool GPEXLayoutInfiniteGrid::addElement(int row, int column, GPLayoutElement *element)
{
  QSize msize;
  if (mOrientation == Qt::Vertical)
  {
    if (fabs(mColumnStretchFactors.value(column, 0.)) > std::numeric_limits<double>::epsilon())
      msize = QSize(mDefaultSectionSize, 0);
  }
  else
  {
    if (fabs(mRowStretchFactors.value(row, 0.)) > std::numeric_limits<double>::epsilon())
      msize = QSize(0, mDefaultSectionSize);
  }
  element->setMinimumSize(msize);
  return GPLayoutSplitterGrid::addElement(row, column, element);
}

void GPEXLayoutInfiniteGrid::moveSplitter(double delta)
{
  bool oldBlock = GraphicsPlot::getBlockReplots();
  GraphicsPlot::blockReplots(true);

  if (mOrientation == Qt::Vertical)
  {
    for (int i = 0; i < rowCount(); ++i)
    {
      if (auto el = mElements.value(i).value(mDraggingSplitterIndex))
      {
        QSize size(mDraggingSizeLeft + delta, el->minimumSize().height());
        el->setMinimumSize(size);
      }
    }
  }
  else
  {
    for (int i = 0; i < columnCount(); ++i)
    {
      if (auto el = mElements.value(mDraggingSplitterIndex).value(i))
      {
        QSize size(el->minimumSize().width(), mDraggingSizeLeft + delta);
        el->setMinimumSize(size);
      }
    }
  }

  GraphicsPlot::blockReplots(oldBlock);
}

void GPEXLayoutInfiniteGrid::calculateSectionSizes(QVector<int> &rowHeights, QVector<int> &colWidths)
{
  bool isVertical = (mOrientation == Qt::Vertical);

  QVector<int> minColWidths, minRowHeights, maxColWidths, maxRowHeights;
  getMinimumRowColSizes(&minColWidths, &minRowHeights);
  getMaximumRowColSizes(&maxColWidths, &maxRowHeights);

  int splitterRowSpacing = qMax(mRowSpacing, mSplitterWidth);
  int splitterColSpacing = qMax(mColumnSpacing, mSplitterWidth);

  if (mOrientation == Qt::Vertical)
  {
    int splittersHeight = (!isVertical) ? (splitterRowSpacing - mRowSpacing) * mSplitters.size() : 0;
    int totalRowSpacing = (rowCount() - 1) * mRowSpacing + splittersHeight;
    colWidths = minColWidths;
    rowHeights = getSectionSizes(maxRowHeights, minRowHeights, mRowStretchFactors.toVector(), mRect.height() - totalRowSpacing);
  }
  else
  {
    int splittersWidth = (isVertical) ? (splitterColSpacing - mColumnSpacing) * mSplitters.size() : 0;
    int totalColSpacing = (columnCount() - 1) * mColumnSpacing + splittersWidth;
    colWidths = getSectionSizes(maxColWidths, minColWidths, mColumnStretchFactors.toVector(), mRect.width() - totalColSpacing);
    rowHeights = minRowHeights;
  }
}


GPAxisContainer::GPAxisContainer(GPAxis::AxisType type, GPAxisRect* parentRect, bool acceptDrops)
  : GPLayoutElement(parentRect->parentPlot())
  , mType(type)
  , mAxisRect(parentRect)
  , mMaximumAxes(acceptDrops ? -1 : 0)
  , mInsertIndex(-1)
  , mAxisUnderDrop(nullptr)
{

}

GPAxisContainer::~GPAxisContainer()
{
  clear();
}

GPAxisRect *GPAxisContainer::getAxisRect() const
{
  return mAxisRect;
}

void GPAxisContainer::setAxisRect(GPAxisRect* rect)
{
  mAxisRect = rect;
  for (const auto& axis : mAxes)
    axis->setAxisRect(rect);
}

void GPAxisContainer::setAxesType(GPAxis::AxisType type)
{
  mType = type;
  for (const auto& it : mAxes)
    if (it.data())
      it->setAxisType(type);
  mAxisDrawRect = QRect();
}

void GPAxisContainer::addAxis(GPAxis *axis)
{
  if (!mAxes.contains(axis))
  {
    mAxes.append(axis);
    updateSize();
  }
}

void GPAxisContainer::addAxis(int index, GPAxis *axis)
{
  mAxes.removeOne(axis);
  mAxes.insert(index, axis);
  updateSize();
}

QList<QPointer<GPAxis>> GPAxisContainer::axesList()
{
  return mAxes;
}

void GPAxisContainer::moveAxis(GPAxis *axis)
{
  if (axis)
  {
    auto rect = axis->axisRect();
    if (rect && rect == mAxisRect)
    {
      int oldIndex = rect->indexOf(axis);
      if (oldIndex < mInsertIndex)
        mInsertIndex--;
    }

    auto oldPlot = qobject_cast<GraphicsPlotExtended*>(axis->parentPlot());
    auto newPlot = qobject_cast<GraphicsPlotExtended*>(parentPlot());
    if (!oldPlot || !newPlot)
      return;

    if (oldPlot != newPlot)
    {
      auto setParent = [this, &newPlot](GPLayerable* l) {
        if (l->layer())
          l->layer()->removeChild(l);
        l->setParent(newPlot);
        l->setParentPlot(newPlot);
        l->setParentLayerable(mAxisRect);
        l->setLayer(mAxisRect->layer());
      };

      auto setListParent = [&setParent](QList<GPLayerable*>& oldList,
                                              QList<GPLayerable*>& newList,
                                              const QList<GPLayerable*>& items) {
        for (int i = oldList.size() - 1; i >= 0; --i)
        {
          if (items.contains(oldList[i]))
          {
            oldList.removeAt(i);
          }
        }
        for (const auto& i : items)
        {
          newList.append(i);
          setParent(i);
        }
      };

      {
        QList<GPLayerable*> psOld;
        for (auto i : oldPlot->mPlottables)
          psOld << i;

        QList<GPLayerable*> psNew;
        for (auto i : newPlot->mPlottables)
          psNew << i;

        QList<GPLayerable*> ps;
        auto items = axis->plottables();
        for (auto i : items)
          ps << i;

        setListParent(psOld, psNew, ps);
      }
      {
        QList<GPLayerable*> psOld;
        for (auto i : oldPlot->mGraphs)
          psOld << i;

        QList<GPLayerable*> psNew;
        for (auto i : newPlot->mGraphs)
          psNew << i;

        QList<GPLayerable*> ps;
        auto items = axis->graphs();
        for (auto i : items)
          ps << i;

        setListParent(psOld, psNew, ps);
      }
      {
        QList<GPLayerable*> psOld;
        for (auto i : oldPlot->mCurves)
          psOld << i;

        QList<GPLayerable*> psNew;
        for (auto i : newPlot->mCurves)
          psNew << i;

        QList<GPLayerable*> ps;
        auto items = axis->curves();
        for (auto i : items)
          ps << i;

        setListParent(psOld, psNew, ps);
      }
      {
        QList<GPLayerable*> psOld;
        for (auto i : oldPlot->mPointAttachedTextElements)
          psOld << i;

        QList<GPLayerable*> psNew;
        for (auto i : newPlot->mPointAttachedTextElements)
          psNew << i;

        QList<GPLayerable*> ps;
        auto items = axis->pointAttachedTextElements();
        for (auto i : items)
          ps << i;

        setListParent(psOld, psNew, ps);
      }
      {
        QList<GPLayerable*> psOld;
        for (auto i : oldPlot->mItems)
          psOld << i;

        QList<GPLayerable*> psNew;
        for (auto i : newPlot->mItems)
          psNew << i;

        QList<GPLayerable*> ps;
        auto items = axis->items();
        for (auto i : items)
          ps << i;

        setListParent(psOld, psNew, ps);
      }


      if (rect)
        rect->takeAxis(axis);
      setParent(axis);
      axis->setLayer(QLatin1String("axes"));
      newPlot->SetAxisInteraction(axis, mAxisRect, (GP::Interaction)(int)(GP::iRangeDrag|GP::iRangeZoom));
      newPlot->Axes.append(axis);
    }
    else
    {
      if (rect)
        rect->takeAxis(axis);
    }

    if (auto track = qobject_cast<GPTrackRect*>(mAxisRect))
    {
      track->addAxis(mInsertIndex, axis);
      auto keyAxis = track->keyAxis();
      if (keyAxis && rect != track)
      {
        auto move = [this, &keyAxis](const QList<GPAbstractPlottable*> &list) {
          for (const auto& item : list)
          {
            item->setKeyAxis(keyAxis);
            item->setParentLayerable(mAxisRect);
            item->setLayer(mAxisRect->layer());
          }
        };
        {
          QList<GPAbstractPlottable*> ps;
          for (auto p : axis->plottables())
            ps << p;

          for (auto p : axis->graphs())
            ps << p;

          for (auto p : axis->curves())
            ps << p;

          for (auto p : axis->pointAttachedTextElements())
            ps << p;

          move(ps);
        }

        for (const auto& im : axis->items())
        {
          for (const auto& pos : im->positions())
            pos->setAxes(keyAxis, axis);
          im->setParentLayerable(mAxisRect);
          im->setLayer(mAxisRect->layer());
        }
      }
    }
    else
    {
      mAxisRect->addAxis(axis->axisType(), axis);
    }

    if (rect)
    {
      if (rect->parentPlot() == parentPlot())
      {
        emit axisMovedFromRect(rect);
      }
      else
      {
        if (auto oldPlot = qobject_cast<GraphicsPlotExtended*>(rect->parentPlot()))
        {
          if (auto oldTrackPlot = qobject_cast<QTracksPlot*>(oldPlot))
            oldTrackPlot->onAxisMovedFromRect(rect);
          oldPlot->GraphicsPlotExtended::axisRemoved(axis);
          oldPlot->queuedReplot();
        }
      }
    }
    updateSize();
  }
}

void GPAxisContainer::takeAxis(GPAxis *axis)
{
  mAxes.removeOne(axis);
  updateSize();
}

void GPAxisContainer::setMaximumAxes(int count)
{
  mMaximumAxes = count;
}

int GPAxisContainer::getMaximumAxes() const
{
  return mMaximumAxes;
}

bool GPAxisContainer::acceptDrops()
{
  return mMaximumAxes != 0;
}

int GPAxisContainer::count() const
{
  return mAxes.count();
}

GPAxis *GPAxisContainer::axisAt(int index)
{
  return mAxes.value(index).data();
}

void GPAxisContainer::clear()
{
  mAxes.clear();
  updateSize();
}

int GPAxisContainer::calculateAutoMargin(GP::MarginSide side)
{
  if (mType == GPAxis::marginSideToAxisType(side))
  {
    if (GPAxis::orientation(mType) == Qt::Vertical)
      return minimumOuterSizeHint().width();
    return minimumOuterSizeHint().height();
  }
  return qMax(GP::getMarginValue(mMargins, side), GP::getMarginValue(mMinimumMargins, side));
}

void GPAxisContainer::update(GPLayoutElement::UpdatePhase phase)
{
  if (phase == upMargins)
  {
    // set the margins of this layout element according to automatic margin calculation, either directly or via a margin group:
    QMargins newMargins = mMargins;
    QList<GP::MarginSide> allMarginSides = QList<GP::MarginSide>() << GP::msLeft << GP::msRight << GP::msTop << GP::msBottom;
    for (const auto& side : allMarginSides)
    {
      if (mAutoMargins.testFlag(side)) // this side's margin shall be calculated automatically
      {
        if (mMarginGroups.contains(side))
          GP::setMarginValue(newMargins, side, mMarginGroups[side]->commonMargin(side)); // this side is part of a margin group, so get the margin value from that group
        else
          GP::setMarginValue(newMargins, side, calculateAutoMargin(side)); // this side is not part of a group, so calculate the value directly
        // apply minimum margin restrictions:
        if (GP::getMarginValue(newMargins, side) < GP::getMarginValue(mMinimumMargins, side))
          GP::setMarginValue(newMargins, side, GP::getMarginValue(mMinimumMargins, side));
      }
    }
    if (GPAxis::orientation(mType) == Qt::Vertical)
      mMinimumSize.setWidth(GP::getMarginValue(newMargins, GPAxis::axisTypeToMarginSide(mType)));
    else
      mMinimumSize.setHeight(GP::getMarginValue(newMargins, GPAxis::axisTypeToMarginSide(mType)));
  }
}

void GPAxisContainer::setOuterRect(const QRect &rect)
{
  if (mOuterRect != rect)
  {
    mOuterRect = rect;
    mRect = mOuterRect.adjusted(mMargins.left(), mMargins.top(), -mMargins.right(), -mMargins.bottom());
    mAxisDrawRect = QRect();
  }
}

QSize GPAxisContainer::minimumOuterSizeHint() const
{
  int axisSize = 0;
  for (const auto& axis : mAxes)
  {
    if (axis.data())
      axisSize += axis->calculateMargin();
  }
  QSize size;
  if (GPAxis::orientation(mType) == Qt::Horizontal)
    size = QSize(mMargins.left() + mMargins.right(), axisSize + mMargins.top() + mMargins.bottom());
  else
    size = QSize(axisSize + mMargins.left() + mMargins.right(), mMargins.top() + mMargins.bottom());

  if (mCachedSize != size)
  {
    mCachedSize = size;
    updateSize();
  }
  return size;
}

const QRect& GPAxisContainer::axisDrawRect()
{
  if (mAxisDrawRect.isNull())
  {
    switch (mType)
    {
      case GPAxis::atLeft:
        mAxisDrawRect = QRect(mRect.right(), mRect.top(), 0, mRect.height());
        break;
      case GPAxis::atTop:
        mAxisDrawRect = QRect(mRect.left(), mRect.bottom() + 1, mRect.width(), 0);
        break;
      case GPAxis::atRight:
        mAxisDrawRect = QRect(mRect.left() + 1, mRect.top(), 0, mRect.height());
        break;
      case GPAxis::atBottom:
        mAxisDrawRect = QRect(mRect.left(), mRect.top() + 2, mRect.width(), 0);
        break;
      default:
        mAxisDrawRect = QRect();
        break;
    }
  }
  return mAxisDrawRect;
}

QList<GPAxis*> GPAxisContainer::dropAxes(const QMimeData* mimeData, int fromIndex)
{
  QList<GPAxis*> axes;

  int count = mimeData->data("application/gp-dragging-axes/count").toInt();
  for (int i = fromIndex; (i < count) && (mMaximumAxes == -1 || mAxes.size() < mMaximumAxes); ++i)
  {
    QPointer<GPAxis> axis = (GPAxis*)(mimeData->data("application/gp-dragging-axes/" + QString::number(i)).toLongLong());
    if (axis)
    {
      moveAxis(axis);
      axes << axis;
    }
  }
  return std::move(axes);
}

void GPAxisContainer::dragEnterEvent(QDragEnterEvent *event)
{
  bool needReplot = false;

  if ((mMaximumAxes == -1 || mAxes.size() < mMaximumAxes)
      && event->mimeData()->hasFormat("application/gp-dragging-axes/count"))
  {
    needReplot |= mInsertIndex != 0;

    mInsertIndex = 0;
    mAxisUnderDrop = mAxes.value(0);

    moveToTop();

    event->accept();
  }
  else
  {
    needReplot |= mInsertIndex != -1;

    mInsertIndex = -1;
    mAxisUnderDrop = nullptr;
    event->ignore();
  }

  if (needReplot)
    parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

void GPAxisContainer::dragMoveEvent(QDragMoveEvent *event)
{
  bool needReplot = false;

  if ((mMaximumAxes == -1 || mAxes.size() < mMaximumAxes)
      && event->mimeData()->hasFormat("application/gp-dragging-axes/count"))
  {
    needReplot = true;
    moveToTop();

    double delta = 0;
    const QRect& rect = axisDrawRect();
    switch (mType)
    {
      case GPAxis::atLeft:
        delta = rect.right() - event->pos().x();
        break;
      case GPAxis::atTop:
        delta = rect.bottom() - event->pos().y();
        break;
      case GPAxis::atRight:
        delta = event->pos().x() - rect.left();
        break;
      case GPAxis::atBottom:
        delta = event->pos().y() - rect.top();
        break;
      default:
        break;
    }

    mInsertIndex = 0;
    mAxisUnderDrop = mAxes.value(0);
    for (int i = 0; i < mAxes.size(); ++i)
    {
      int offset = mAxes[i]->offset();
      int margin = mAxes[i]->calculateMargin();
      if (offset + margin / 2 > delta)
        break;
      mInsertIndex = i + 1;
      mAxisUnderDrop = mAxes.value(i + 1, mAxes.last());
    }

    event->accept();
  }
  else
  {
    needReplot |= mInsertIndex != -1;

    mInsertIndex = -1;
    mAxisUnderDrop = nullptr;

    event->ignore();
  }

  if (needReplot)
    parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

void GPAxisContainer::dropEvent(QDropEvent *event)
{
  bool needReplot = false;

  if (mInsertIndex != -1 && (mMaximumAxes == -1 || mAxes.size() < mMaximumAxes)
      && event->mimeData()->hasFormat("application/gp-dragging-axes/count"))
  {
    dropAxes(event->mimeData());
    event->acceptProposedAction();
    needReplot = true;
  }
  else
  {
    event->ignore();
  }
  needReplot |= mInsertIndex != -1;

  mInsertIndex = -1;
  mAxisUnderDrop = nullptr;

  if (needReplot)
    parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

void GPAxisContainer::dragLeaveEvent(QDragLeaveEvent *event)
{
  bool needReplot = mInsertIndex != -1;
  mInsertIndex = -1;
  mAxisUnderDrop = nullptr;
  event->accept();

  if (needReplot)
    parentPlot()->replot(GraphicsPlot::rpQueuedReplot);
}

void GPAxisContainer::draw(GPPainter *painter)
{
  int padding = 7;
  int lineWidth = 4;

  if (mInsertIndex != -1)
  {
    auto rect = mOuterRect.adjusted(-padding, -padding, padding - 1, padding - 1);
    painter->setPen(QPen(DCInsertingRect, 3, Qt::SolidLine));
    painter->drawRect(rect);

    painter->setPen(QPen(DCInsertingPosition, lineWidth, Qt::SolidLine));

    int offset = 0;
    if (!mAxes.isEmpty())
    {
      if (mInsertIndex < mAxes.size())
        offset = mAxes[mInsertIndex]->offset();
      else
        offset = mAxes.last()->offset() + mAxes.last()->calculateMargin();
    }

    QPoint ls, le;
    switch (mType)
    {
      case GPAxis::atLeft:
        {
          double l = mOuterRect.right() - mMargins.right() - offset;
          ls = QPoint(l, rect.bottom());
          le = QPoint(l, rect.top());
          break;
        }
      case GPAxis::atTop:
        {
          double l = mOuterRect.bottom() - mMargins.bottom() - offset;
          ls = QPoint(rect.left(), l);
          le = QPoint(rect.right(), l);
          break;
        }
      case GPAxis::atRight:
        {
          double l = mOuterRect.left() + mMargins.left() + offset;
          ls = QPoint(l, rect.bottom());
          le = QPoint(l, rect.top());
          break;
        }
      case GPAxis::atBottom:
        {
          double l = mOuterRect.top() + mMargins.top() + offset;
          ls = QPoint(rect.left(), l);
          le = QPoint(rect.right(), l);
          break;
        }
      default:
        break;
    }
    painter->drawLine(ls, le);
  }
}

void GPAxisContainer::moveToTop()
{
  if (mParentPlot)
    if (auto layer = mParentPlot->layer(mParentPlot->layerCount() - 1))
      if (layer->children().value(layer->count() - 1) != this)
        moveToLayer(layer, false);
}

void GPAxisContainer::updateSize() const
{
  if (mParentLayout)
    mParentLayout->sizeConstraintsChanged();
}

GPTrackAxisPainterPrivate::GPTrackAxisPainterPrivate(GraphicsPlot *parentPlot)
  : GPAxisPainterPrivate(parentPlot)
  , minimumSize(25)
  , rangeLabelMargin(5)
  , labelOnTop(false)
  , htmlLabel(false)
  , hasCuttedLabel(false)
  , hasCuttedRange(false)
{
  tickLengthIn = 0;
  subTickLengthIn = 0;
}

GPTrackAxisPainterPrivate::~GPTrackAxisPainterPrivate()
{

}

QImage GPTrackAxisPainterPrivate::drawToBuffer(int w, int h, QFont font, const QColor& color, const QString& text) const
{
  font.setStyleStrategy(QFont::NoSubpixelAntialias);
  QImage buffer(w, h, QImage::Format_RGBA8888_Premultiplied);
  buffer.fill(Qt::transparent);
  {
    QPainter bufferPainter(&buffer);
    bufferPainter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    bufferPainter.setFont(font);
    bufferPainter.setPen(QPen(color));
    bufferPainter.drawText(0, 0, w, h, Qt::TextDontClip | Qt::AlignCenter, text);
  }
  return buffer;
}

QImage GPTrackAxisPainterPrivate::drawHTMLToBuffer(QFont font, const QColor& color, const QString& text) const //-V813
{
  if (font.pixelSize() != -1)
    font.setPixelSize(font.pixelSize() + 3);
  else
    font.setPointSize(font.pointSize() + 3);

  QTextDocument document;
  document.setDefaultFont(font);
  document.setDefaultStyleSheet(QString("body { color: %1; }").arg(color.name()));
  QString htmlFormatted = QString("<body>%1</body>").arg(text);
  document.setHtml(htmlFormatted);

  QSize size = document.size().toSize();
  document.setTextWidth(size.width()); // QTBUG-22851 fix

  QImage buffer(size, QImage::Format_RGBA8888_Premultiplied);
  buffer.fill(QColor(Qt::transparent));

  QPainter p(&buffer);
  p.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
  QRect documentRect(0, 0, size.width(), size.height());
  document.drawContents(&p, documentRect);

  return buffer;
}

void GPTrackAxisPainterPrivate::draw(GPPainter *painter)
{
  QPoint origin;
  switch (type)
  {
    case GPAxis::atLeft:   origin = axisRect.bottomLeft() + QPoint(-offset, 0); break;
    case GPAxis::atRight:  origin = axisRect.bottomRight() + QPoint(+offset, 0); break;
    case GPAxis::atTop:    origin = axisRect.topLeft() + QPoint(0, -offset); break;
    case GPAxis::atBottom: origin = axisRect.bottomLeft() + QPoint(0, +offset); break;
  }

  QString lower = QString::number(axisRange.lower, 'g', 3);
  QString upper = QString::number(axisRange.upper, 'g', 3);

  auto bottomLabel = reversedEndings ? upper : lower;
  auto topLabel = reversedEndings ? lower : upper;

  painter->setFont(labelFont);
  painter->setPen(QPen(labelColor));
  painter->setBackground(Qt::transparent);
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

  QRect bottomRect = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, bottomLabel);
  QRect topRect = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, topLabel);

  QRect labelBounds;
  if (!label.isEmpty())
  {
    if (htmlLabel)
    {
      QByteArray newHash = generateLabelParameterHash();
      if (mLabelParameterHash != newHash)
      {
        mNameLabelCache = drawHTMLToBuffer(labelFont, labelColor, label);
        mLabelParameterHash = std::move(newHash);
      }
      labelBounds = mNameLabelCache.rect();
    }
    else
    {
      labelBounds = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, label);
    }
  }

  int labelHeight = labelBounds.height();
  int rangeHeight = bottomRect.height();
  int textHeight = labelHeight + labelPadding + rangeHeight + rangeLabelMargin;

  if (labelOnTop && textHeight < minimumSize)
    textHeight = minimumSize;
  if (!labelOnTop && labelHeight < minimumSize)
    labelHeight = minimumSize;

  {
    int selLabelSize = labelOnTop ? textHeight - 1 : labelHeight - 1;
    switch (type)
    {
      case GPAxis::atLeft:
        mAxisSelectionBox.setCoords(origin.x() - selLabelSize, axisRect.top(), origin.x(), axisRect.bottom());
        break;
      case GPAxis::atTop:
        mAxisSelectionBox.setCoords(axisRect.left(), origin.y() - selLabelSize - 2, axisRect.right(), origin.y() - 1);
        break;
      case GPAxis::atRight:
        mAxisSelectionBox.setCoords(origin.x(), axisRect.top(), origin.x() + selLabelSize, axisRect.bottom());
        break;
      case GPAxis::atBottom:
        mAxisSelectionBox.setCoords(axisRect.left(), origin.y() - 1, axisRect.right(), origin.y() + selLabelSize - 1);
        break;
      default:
        break;
    }
    mAxisSelectionBox = mAxisSelectionBox.normalized();
    mAxisFullBox = mAxisSelectionBox;

    painter->setClipRect(mAxisFullBox);
    painter->fillRect(mAxisFullBox, basePen.color());
    if (selectedParts != GPAxis::spNone)
    {
      int penWidth = 2;
      painter->setPen(QPen(DCSelectionRect, penWidth, Qt::SolidLine));
      painter->drawRect(mAxisFullBox.adjusted(penWidth/2, penWidth/2, -penWidth/2, -penWidth/2));
      painter->setPen(QPen(labelColor));
    }
  }

  //TODO: доделать проверку размеров текста для вертикальных надписей
  {
    if (GPAxis::orientation(type) == Qt::Vertical)
    {
      {
        QByteArray newHash = generateLabelParameterHash();
        if (mLabelParameterHash != newHash || (!htmlLabel && mNameLabelCache.size() != labelBounds.size()))
        {
          mLabelParameterHash = std::move(newHash);
          mNameLabelCache = drawToBuffer(labelBounds.width(), labelHeight, labelFont, labelColor, label);
        }

        if (fabs(mOldRange.lower - axisRange.lower) > std::numeric_limits<double>::epsilon() || rangeHeight != mBottomLabelCache.height())
          mBottomLabelCache = drawToBuffer(bottomRect.width(), rangeHeight, labelFont, labelColor, bottomLabel);

        if (fabs(mOldRange.upper - axisRange.upper) > std::numeric_limits<double>::epsilon() || rangeHeight != mTopLabelCache.height())
          mTopLabelCache = drawToBuffer(topRect.width(), rangeHeight, labelFont, labelColor, topLabel);
      }

      QPointF transformCenter;
      QPointF bottomPoint;
      QPointF topPoint;
      if (type == GPAxis::atRight)
      {
        if (labelOnTop)
        {
          transformCenter = QPointF(origin.x() + textHeight, origin.y() - axisRect.height() / 2 - labelBounds.width() / 2);
          bottomPoint = QPointF(axisRect.height() / 2 + labelBounds.width() / 2 - mBottomLabelCache.width() - rangeLabelMargin, labelHeight + labelPadding);
          topPoint = QPointF(-axisRect.height() / 2 + labelBounds.width() / 2 + rangeLabelMargin, labelHeight + labelPadding);
        }
        else
        {
          transformCenter = QPointF(origin.x() + labelBounds.height(), origin.y() - axisRect.height() / 2 - labelBounds.width() / 2);
          bottomPoint = QPointF(axisRect.height() / 2 + labelBounds.width() / 2 - mBottomLabelCache.width() - rangeLabelMargin, rangeLabelMargin);
          topPoint = QPointF(-axisRect.height() / 2 + labelBounds.width() / 2 + rangeLabelMargin, rangeLabelMargin);
        }
      }
      else
      {
        if (labelOnTop)
        {
          transformCenter = QPointF(origin.x() - textHeight, origin.y() - axisRect.height() / 2 + labelBounds.width() / 2);
          bottomPoint = QPointF(-axisRect.height() / 2 + labelBounds.width() / 2 + rangeLabelMargin, labelHeight + labelPadding);
          topPoint = QPointF(axisRect.height() / 2 + labelBounds.width() / 2 - mTopLabelCache.width() - rangeLabelMargin, labelHeight + labelPadding);
        }
        else
        {
          transformCenter = QPointF(origin.x() - labelBounds.height(), origin.y() - axisRect.height() / 2 + labelBounds.width() / 2);
          bottomPoint = QPointF(-axisRect.height() / 2 + labelBounds.width() / 2 + rangeLabelMargin, 0);
          topPoint = QPointF(axisRect.height() / 2 + labelBounds.width() / 2 - mTopLabelCache.width() - rangeLabelMargin, 0);
        }
      }
      QTransform oldTransform = painter->transform();
      painter->translate(transformCenter);
      painter->rotate(90);

      painter->drawImage(0, 0, mNameLabelCache);
      painter->drawImage(bottomPoint, mBottomLabelCache);
      painter->drawImage(topPoint, mTopLabelCache);

      painter->setTransform(oldTransform);
    }
    else
    {
      QRect labelRect;
      QRect rangeRect;

      QFontMetrics fm(painter->font());
      int bottomLabelWidth = fm.boundingRect(bottomLabel).width();
      int topLabelWidth = fm.boundingRect(topLabel).width();

      int labelMargin = qMax(bottomLabelWidth, topLabelWidth) + rangeLabelMargin * 2;
      int minLabelMargin = 2;

      if (labelOnTop)
        labelRect = QRect(origin.x() + minLabelMargin, origin.y(), axisRect.width() - minLabelMargin * 2, labelHeight);
      else
        labelRect = QRect(origin.x() + labelMargin, origin.y(), axisRect.width() - labelMargin * 2, mAxisFullBox.height());

      if (!labelOnTop && labelRect.width() < 30)
      {
        labelRect = QRect(origin.x() + minLabelMargin, origin.y(), axisRect.width() - minLabelMargin * 2, labelRect.height());
      }
      else
      {
        if (type == GPAxis::atTop)
        {
          if (labelOnTop)
            rangeRect = QRect(origin.x() + rangeLabelMargin, origin.y() - rangeHeight - rangeLabelMargin, axisRect.width() - rangeLabelMargin * 2, rangeHeight);
          else
            rangeRect = QRect(origin.x() + rangeLabelMargin, origin.y() - labelHeight, axisRect.width() - rangeLabelMargin * 2, labelHeight);
        }
        else if (type == GPAxis::atBottom)
        {
          if (labelOnTop)
            rangeRect = QRect(origin.x() + rangeLabelMargin, origin.y() + labelHeight, axisRect.width() - rangeLabelMargin * 2, rangeHeight);
          else
            rangeRect = QRect(origin.x() + rangeLabelMargin, origin.y(), axisRect.width() - rangeLabelMargin * 2, labelHeight);
        }
      }
      if (type == GPAxis::atTop)
      {
        if (labelOnTop)
          labelRect.moveTo(labelRect.topLeft() - QPoint(0, textHeight));
        else
          labelRect.moveTo(labelRect.topLeft() - QPoint(0, labelRect.height()));
      }

      if (htmlLabel)
      {
        if (labelRect.width() > mNameLabelCache.width())
          painter->drawImage(labelRect.center().x() - labelBounds.width() / 2, labelRect.y(), mNameLabelCache);
        else
          painter->drawImage(labelRect, mNameLabelCache, QRect(0, 0, labelRect.width(), labelRect.height()));
        hasCuttedLabel = labelRect.width() < mNameLabelCache.width();
      }
      else
      {
        if (labelRect.width() > labelBounds.width())
          labelRect = QRect(labelRect.center().x() - labelBounds.width() / 2, labelRect.y(), labelBounds.width(), labelRect.height());
        painter->drawText(labelRect, Qt::AlignVCenter | Qt::AlignLeft, label);
        hasCuttedLabel = labelRect.width() < labelBounds.width();
      }

      if (rangeRect.width() != 0)
      {
        painter->drawText(rangeRect, Qt::AlignVCenter | Qt::AlignLeft, bottomLabel);
        if (rangeRect.width() >= bottomLabelWidth + topLabelWidth + 3)
          painter->drawText(rangeRect, Qt::AlignVCenter | Qt::AlignRight, topLabel);
      }
      hasCuttedRange = !((rangeRect.width() != 0) && (rangeRect.width() >= bottomLabelWidth + topLabelWidth + 3));
    }
  }
}

int GPTrackAxisPainterPrivate::size()
{
  QRect bounds;
  if (!label.isEmpty())
  {
    QFontMetrics labelMetrics(labelFont);
    if (htmlLabel)
    {
      QByteArray newHash = generateLabelParameterHash();
      if (mLabelParameterHash != newHash)
      {
        mNameLabelCache = drawHTMLToBuffer(labelFont, labelColor, label);
        mLabelParameterHash = std::move(newHash);
      }
      bounds = mNameLabelCache.rect();
    }
    else
    {
      bounds = labelMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignCenter, label);
    }

    if (labelOnTop)
    {
      bounds.adjust(0, 0, 0, labelPadding + rangeLabelMargin);
      bounds.adjust(0, 0, 0, labelMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignCenter, "0").height());
    }
  }

  return qMax(minimumSize, bounds.height());
}

GPTrackAxis::GPTrackAxis(GPAxisRect *parent, GPAxis::AxisType type)
  : GPAxis(parent, type)
  , mDraggingState(dsBlocked)
{
  delete mAxisPainter;
  mAxisPainter = new GPTrackAxisPainterPrivate(parent->parentPlot());
  mShowCoords = true;

  if (auto qtp = qobject_cast<GraphicsPlotExtended*>(parent->parentPlot()))
  {
    if (qtp->getFlags() & QTracksPlot::ExMoveAxes)
      mDraggingState = dsNone;
  }

  setSelectableParts((mDraggingState == dsNone) ? spAxisFull : spNone);

  setPadding(0);
  setLabelColor(Qt::white);
  setAntialiased(false);
  setLayer(mParentPlot->currentLayer());
}

GPTrackAxis::~GPTrackAxis()
{
  if (mCurve && parentPlot())
    parentPlot()->removeCurve(mCurve);
}

void GPTrackAxis::setCurve(GPCurve *curve)
{
  mCurve = curve;
  if (mCurve)
    setBasePen(mCurve->pen());
}

GPCurve* GPTrackAxis::getCurve() const
{
  return mCurve;
}

void GPTrackAxis::setHasHTMLLabel(bool html)
{
  if (auto axisPainter = dynamic_cast<GPTrackAxisPainterPrivate*>(mAxisPainter))
  {
    if (axisPainter->htmlLabel != html)
    {
      axisPainter->htmlLabel = html;
      axisPainter->labelOnTop = html;
      mCachedMarginValid = false;
    }
  }
}

QString GPTrackAxis::ToolTipTest(const QPointF& /*pos*/) const
{
  QString toolTip;
  if (mAxisPainter)
  {
    if (auto axisPainter = dynamic_cast<GPTrackAxisPainterPrivate*>(mAxisPainter))
    {
      if (axisPainter->hasCuttedLabel)
        toolTip += mLabel;
      if (axisPainter->hasCuttedLabel && axisPainter->hasCuttedRange)
        toolTip += ": ";
      if (axisPainter->hasCuttedRange)
        toolTip += tickLabel(mRange.lower) + " - " + tickLabel(mRange.upper);
    }
  }
  return toolTip;
}

int GPTrackAxis::calculateMargin()
{
  if (!mVisible || !mCurve)
    return 0;

  if (mCachedMarginValid)
    return mCachedMargin;

  int margin = 0;
  int selectionOffset = (mAxisType == GPAxis::atTop) ? 1 : 0;

  mAxisPainter->type = mAxisType;
  mAxisPainter->labelFont = getLabelFont();
  mAxisPainter->label = mLabelVisible ? mLabel : QString();
  mAxisPainter->tickLabelFont = mTickLabelFont;
  mAxisPainter->axisRect = mAxisRect->rect();
  mAxisPainter->viewportRect = mParentPlot->viewport();
  margin += mAxisPainter->size() + selectionOffset;
  margin += mPadding;
  margin = qMax(((GPTrackAxisPainterPrivate*)mAxisPainter)->minimumSize + selectionOffset, margin);

  mCachedMargin = margin;
  mCachedMarginValid = true;
  return margin;
}

void GPTrackAxis::draw(GPPainter *painter)
{
  if (auto axisPainter = dynamic_cast<GPTrackAxisPainterPrivate*>(mAxisPainter))
  {
    // GPAxis params
    axisPainter->type = mAxisType;
    axisPainter->labelFont = getLabelFont();
    axisPainter->labelColor = getLabelColor();
    axisPainter->label = getLabelDisplayText();
    axisPainter->substituteExponent = mNumberBeautifulPowers;
    axisPainter->axisRect = mAxisRect->axisDrawRect(mAxisType);
    axisPainter->viewportRect = mParentPlot->viewport();
    axisPainter->abbreviateDecimalPowers = mScaleType == stLogarithmic;
    axisPainter->reversedEndings = mRangeReversed;

    // GPTrackAxis params
    if (mCurve)
      axisPainter->basePen = mCurve->pen();
    else
      axisPainter->basePen = getBasePen();
    axisPainter->axisRange = range();
    axisPainter->selectedParts = mSelectedParts;

    axisPainter->draw(painter);
  }
}

void GPTrackAxis::mousePressEvent(QMouseEvent *event, const QVariant &details)
{
  if (mDraggingState != dsBlocked && event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier))
  {
    mDraggingState = dsStart;
    event->accept();
  }
  else
  {
    GPAxis::mousePressEvent(event, details);
  }
}

void GPTrackAxis::mouseMoveEvent(QMouseEvent *event, const QPointF &startPos)
{
  if (mDraggingState != dsBlocked && (event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ControlModifier))
  {
    if (mDraggingState == dsStart)
    {
      bool oldBlock = GraphicsPlot::getBlockReplots();
      GraphicsPlot::blockReplots(true);

      auto axes = parentPlot()->selectedAxes();
      if (!axes.isEmpty())
        mSelectedParts |= spAxis;
      if (axes.indexOf(this) == -1)
        axes.append(this);

      QDrag* drag = new QDrag(this);
      QMimeData *mimeData = new QMimeData;
      mimeData->setData("application/gp-dragging-axes/count", QByteArray::number(axes.size()));
      for (int i = 0; i < axes.size(); ++i)
        mimeData->setData("application/gp-dragging-axes/" + QString::number(i), QByteArray::number((long long)axes[i]));
      drag->setMimeData(mimeData);

      QPixmap pixmap(parentPlot()->size());
      pixmap.fill(Qt::transparent);
      GPPainter painter(&pixmap);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.setOpacity(0.9);

      auto curve = mCurve;
      for (int i = axes.size() - 1; i >= 0; --i)
      {
        if (auto axis = qobject_cast<GPTrackAxis*>(axes[i]))
        {
          if (axis != this)
          {
            setCurve(axis->getCurve());
            draw(&painter);
            painter.translate(6, 6);
          }
        }
      }
      setCurve(curve);
      draw(&painter);

      drag->setPixmap(pixmap);
      drag->setHotSpot(event->pos());
      event->accept();
      mDraggingState = dsMove;

      GraphicsPlot::blockReplots(oldBlock);

      drag->exec(Qt::MoveAction);
    }
  }
  else
  {
    GPAxis::mouseMoveEvent(event, startPos);
  }
}

GPEXTrackGrid::GPEXTrackGrid(GPAxisRect* parentRect, GPAxis *parentAxis)
  : GPGrid(parentAxis)
  , mValueTicksCount(5)
  , mOrientation(Qt::Horizontal)
  , mParentRect(parentRect)
{
  setParent(parentRect);
  QPen lightGreyPen("#1E000000");
  setPen(lightGreyPen);
  setSubGridPen(lightGreyPen);
  setLayer(QLatin1String("grid"));
}

GPEXTrackGrid::GPEXTrackGrid(GPAxisRect *parentRect, GPAxis::AxisType keyAxisType)
  : GPGrid(parentRect)
  , mValueTicksCount(5)
  , mOrientation(GPAxis::orientation(keyAxisType))
  , mParentRect(parentRect)
{
  mParentAxis = nullptr;
  QPen lightGreyPen("#1E000000");
  setPen(lightGreyPen);
  setSubGridPen(lightGreyPen);
}

bool GPEXTrackGrid::realVisibility() const
{
  return mVisible && layer()->visible();
}

int GPEXTrackGrid::getLinesCount()
{
  return mValueTicksCount;
}

void GPEXTrackGrid::setLinesCount(int count)
{
  mValueTicksCount = count;
}

void GPEXTrackGrid::draw(GPPainter *painter)
{
  if (mParentAxis && mParentAxis->subTicks() && mSubGridVisible)
    drawSubGridLines(painter);
  drawGridLines(painter);
}

void GPEXTrackGrid::drawGridLines(GPPainter *painter) const
{
  if (!mParentRect)
    return;

  if (mParentAxis)
  {
    auto tickVector = mParentAxis->tickVector();
    auto range = mParentAxis->range();
    const int tickCount = tickVector.size();
    double t; // helper variable, result of coordinate-to-pixel transforms
    if (mParentAxis->orientation() == Qt::Horizontal)
    {
      // draw zeroline:
      int zeroLineIndex = -1;
      if (mZeroLinePen.style() != Qt::NoPen && range.lower < 0 && range.upper > 0)
      {
        applyAntialiasingHint(painter, mAntialiasedZeroLine, GP::aeZeroLine);
        painter->setPen(mZeroLinePen);
        double epsilon = mParentAxis->range().size() * 1E-6; // for comparing double to zero
        for (int i = 0; i < tickCount; ++i)
        {
          if (qAbs(tickVector.at(i)) < epsilon)
          {
            zeroLineIndex = i;
            t = mParentAxis->coordToPixel(tickVector.at(i)); // x
            painter->drawLine(QLineF(t, mParentRect->bottom(), t, mParentRect->top()));
            break;
          }
        }
      }
      // draw grid lines:
      applyDefaultAntialiasingHint(painter);
      painter->setPen(mPen);
      for (int i = 0; i < tickCount; ++i)
      {
        if (i == zeroLineIndex) continue; // don't draw a gridline on top of the zeroline
        t = mParentAxis->coordToPixel(tickVector.at(i)); // x
        painter->drawLine(QLineF(t, mParentRect->bottom(), t, mParentRect->top()));
      }
    }
    else
    {
      // draw zeroline:
      int zeroLineIndex = -1;
      if (mZeroLinePen.style() != Qt::NoPen && range.lower < 0 && range.upper > 0)
      {
        applyAntialiasingHint(painter, mAntialiasedZeroLine, GP::aeZeroLine);
        painter->setPen(mZeroLinePen);
        double epsilon = range.size()*1E-6; // for comparing double to zero
        for (int i=0; i<tickCount; ++i)
        {
          if (qAbs(tickVector.at(i)) < epsilon)
          {
            zeroLineIndex = i;
            t = mParentAxis->coordToPixel(tickVector.at(i)); // y
            painter->drawLine(QLineF(mParentRect->left(), t, mParentRect->right(), t));
            break;
          }
        }
      }
      // draw grid lines:
      applyDefaultAntialiasingHint(painter);
      painter->setPen(mPen);
      for (int i=0; i<tickCount; ++i)
      {
        if (i == zeroLineIndex) continue; // don't draw a gridline on top of the zeroline
        t = mParentAxis->coordToPixel(tickVector.at(i)); // y
        painter->drawLine(QLineF(mParentRect->left(), t, mParentRect->right(), t));
      }
    }
  }
  else
  {
    applyDefaultAntialiasingHint(painter);
    if (mOrientation == Qt::Horizontal)
    {
      double t = mParentRect->top();
      double delta = ((double)mParentRect->height()) / (mValueTicksCount + 1);

      painter->setPen(mPen);
      for (int i = 0; i < mValueTicksCount; ++i)
      {
        t += delta;
        painter->drawLine(QLineF(mParentRect->left(), t, mParentRect->right(), t));
      }

      painter->setPen(mZeroLinePen);
      painter->drawLine(QLineF(mParentRect->topLeft(), mParentRect->topRight()));
      painter->drawLine(QLineF(mParentRect->bottomLeft(), mParentRect->bottomRight()));
    }
    else
    {
      double t = mParentRect->left();
      double delta = ((double)mParentRect->width()) / (mValueTicksCount + 1);

      painter->setPen(mPen);
      for (int i = 0; i < mValueTicksCount; ++i)
      {
        t += delta;
        painter->drawLine(QLineF(t, mParentRect->bottom(), t, mParentRect->top()));
      }

      painter->setPen(mZeroLinePen);
      painter->drawLine(QLineF(mParentRect->topLeft(), mParentRect->bottomLeft()));
      painter->drawLine(QLineF(mParentRect->topRight(), mParentRect->bottomRight()));
    }
  }
}

void GPEXTrackGrid::drawSubGridLines(GPPainter *painter) const
{
  GPGrid::drawSubGridLines(painter);
}

GPTrackRect::GPTrackRect(GraphicsPlot *parentPlot,
                               GPAxis::AxisType keyAxisType,
                               GPAxis::AxisType valueAxisType,
                               GPAxis* keyAxis)
  : GPAxisRect(parentPlot, false)
  , mFlags(TRDefault)
  , mMargin(0)
  , mKeyAxisType(keyAxisType)
  , mValueAxisType(valueAxisType)
{
  auto gpex = qobject_cast<GraphicsPlotExtended*>(mParentPlot);
  if (keyAxis)
  {
    mKeyAxis = keyAxis;
    addRangeZoomDragAxis(keyAxis);
  }
  else
  {
    if (gpex)
      mKeyAxis = gpex->addAxis(this, keyAxisType);
    else
      mKeyAxis = GPAxisRect::addAxis(keyAxisType);
  }

  mValueGrid = new GPEXTrackGrid(this, keyAxisType);
  mKeyGrid = new GPEXTrackGrid(this, mKeyAxis);

  mMinimumMargins = QMargins(mMargin, mMargin, mMargin, mMargin);
}

GPTrackRect::~GPTrackRect()
{
  delete mValueGrid;
  delete mKeyGrid;
  qDeleteAll(mContainers);
}

void GPTrackRect::setFlags(unsigned flags)
{
  mFlags = flags;
  updateContainerMaximumAxes();
}

unsigned GPTrackRect::getFlags() const
{
  return mFlags;
}

GPAxis *GPTrackRect::addAxis(GPAxis::AxisType type, GPAxis *axis)
{
  auto newAxis = GPAxisRect::addAxis(type, axis);

  if (mContainers.contains(type))
    mContainers[type]->addAxis(newAxis);

  addRangeZoomDragAxis(newAxis);
  updateAxesOffset(type);

  return newAxis;
}

void GPTrackRect::addAxis(int index, GPAxis *axis)
{
  if (axis)
  {
    axis->setAxisRect(this);
    auto type = axis->axisType();

    if (!mAxes.contains(type))
      mAxes[type] = QList<GPAxis*>();
    mAxes[type].removeOne(axis);
    mAxes[type].insert(index, axis);

    if (mContainers.contains(type))
      mContainers[type]->addAxis(index, axis);

    addRangeZoomDragAxis(axis);
    updateAxesOffset(type);
  }
}

bool GPTrackRect::takeAxis(GPAxis *axis)
{
  if (mContainers.contains(axis->axisType()))
    mContainers[axis->axisType()]->takeAxis(axis);
  return GPAxisRect::takeAxis(axis);
}

int GPTrackRect::axisCount(GPAxis::AxisType type) const
{
  if (mContainers.contains(type))
    return mContainers[type]->count();
  return GPAxisRect::axisCount(type);
}

int GPTrackRect::getValueGridLinesCount()
{
  return mValueGrid->getLinesCount();
}

void GPTrackRect::setValueGridLinesCount(int count)
{
  mValueGrid->setLinesCount(count);
}

int GPTrackRect::getKeyGridLinesCount()
{
  return mKeyGrid->getLinesCount();
}

void GPTrackRect::setKeyGridLinesCount(int count)
{
  mKeyGrid->setLinesCount(count);
}

bool GPTrackRect::getKeyGridVisible() const
{
  return mKeyGrid->visible();
}

bool GPTrackRect::getValueGridVisible() const
{
  return mValueGrid->visible();
}

void GPTrackRect::setKeyGridVisible(bool visible)
{
  mKeyGrid->setVisible(visible);
}

void GPTrackRect::setValueGridVisible(bool visible)
{
  mValueGrid->setVisible(visible);
}

GPAxisContainer* GPTrackRect::valueAxisContainer()
{
  return mContainers.value(mValueAxisType);
}

GPAxis *GPTrackRect::keyAxis() const
{
  return mKeyAxis;
}

GPAxisContainer* GPTrackRect::addContainer(GPAxis::AxisType type, bool acceptDrops)
{
  auto container = new GPAxisContainer(type, this, acceptDrops);
  container->setMargins(QMargins(mMargin, mMargin, mMargin, mMargin));
  mContainers.insert(type, container);

  if (type == mValueAxisType)
    updateContainerMaximumAxes();

  auto newMargins = mMargins;
  switch (type)
  {
    case GPAxis::atLeft:
      newMargins.setLeft(mMargin);
      break;
    case GPAxis::atTop:
      newMargins.setTop(mMargin);
      break;
    case GPAxis::atRight:
      newMargins.setRight(mMargin);
      break;
    case GPAxis::atBottom:
      newMargins.setBottom(mMargin);
      break;
    default:
      break;
  }
  setMargins(newMargins);

  if (mAxes.contains(type))
  {
    for (const auto& axis : mAxes[type])
      container->addAxis(axis);
  }

  return container;
}

void GPTrackRect::addContainer(GPAxis::AxisType type, GPAxisContainer *container)
{
  if (!mContainers.contains(type))
  {
    mContainers.insert(type, container);
    container->setAxisRect(this);
    if (type == mKeyAxisType)
      updateContainerMaximumAxes();
  }
}

GPAxisContainer *GPTrackRect::takeContainer(GPAxis::AxisType type)
{
  if (auto container = mContainers.value(type))
  {
    for (const auto& axis : container->axesList())
      GPAxisRect::takeAxis(axis);
    container->setAxisRect(nullptr);
    mContainers.remove(type);
    return container;
  }
  return nullptr;
}

int GPTrackRect::calculateAutoMargin(GP::MarginSide side)
{
  auto type = GPAxis::marginSideToAxisType(side);
  if (mContainers.contains(type))
  {
    updateAxesOffset(type);
    return mMargin;
  }
  return GPAxisRect::calculateAutoMargin(side);
}

const QRect& GPTrackRect::axisDrawRect(GPAxis::AxisType type) const
{
  if (mContainers.contains(type))
    return mContainers[type]->axisDrawRect();
  return mRect;
}

void GPTrackRect::dragEnterEvent(QDragEnterEvent *event)
{
  for (const auto& container : mContainers)
  {
    if (container->acceptDrops())
    {
      container->dragEnterEvent(event);
    }
  }
}

void GPTrackRect::dragMoveEvent(QDragMoveEvent *event)
{
  for (const auto& container : mContainers)
  {
    if (container->acceptDrops())
    {
      container->dragMoveEvent(event);
    }
  }
}

void GPTrackRect::dropEvent(QDropEvent *event)
{
  for (const auto& container : mContainers)
  {
    if (container->acceptDrops())
    {
      container->dropEvent(event);
    }
  }
}

void GPTrackRect::dragLeaveEvent(QDragLeaveEvent *event)
{
  for (const auto& container : mContainers)
  {
    if (container->acceptDrops())
    {
      container->dragLeaveEvent(event);
    }
  }
}

void GPTrackRect::updateContainerMaximumAxes()
{
  if (auto c = mContainers.value(mValueAxisType))
  {
    int maximum = 0;
    if (mFlags & TRCanDropAxis)
    {
      if (mFlags & TROnlyOneAxis)
        maximum = 1;
      else
        maximum = -1;
    }
    else
      maximum = 0;
    c->setMaximumAxes(maximum);
  }
}

namespace  {
  static QMap<GPAxis::AxisType, QPair<GPAxis::AxisType, GPAxis::AxisType>> MainOrientationToKeyOrientations
  {
    {GPAxis::atLeft,   {GPAxis::atTop,  GPAxis::atBottom}},
    {GPAxis::atTop,    {GPAxis::atLeft, GPAxis::atRight}},
    {GPAxis::atRight,  {GPAxis::atTop,  GPAxis::atBottom}},
    {GPAxis::atBottom, {GPAxis::atLeft, GPAxis::atRight}}
  };

  /*
    default layout for "atTop"

    !----------------------!----------------------------!---------------------------!
    !                      ! title                      !                           !
    !----------------------!----------------------------!---------------------------!
    !                      ! main                       !                           !
    !----------------------!----------------------------!---------------------------!
    ! main key             ! rect                       ! second key                !
    !----------------------!----------------------------!---------------------------!
    !                      !                            !                           !
    !----------------------!----------------------------!---------------------------!
  */

  // QPoint(row, column)
  static QMap<unsigned, QPoint> TrackOrientationToTitlePosition {
    {GPAxis::atTop,    {0, 1}},
    {GPAxis::atBottom, {0, 1}},
    {GPAxis::atLeft,   {1, 0}},
    {GPAxis::atRight,  {1, 0}},
  };

  static QMap<unsigned, QPoint> TrackOrientationToRectPosition {
    {GPAxis::atTop,    {2, 1}},
    {GPAxis::atBottom, {1, 1}},
    {GPAxis::atLeft,   {1, 2}},
    {GPAxis::atRight,  {1, 1}},
  };

  static QMap<unsigned, QPoint> TrackOrientationToMainPosition {
    {GPAxis::atTop,    {1, 1}},
    {GPAxis::atBottom, {2, 1}},
    {GPAxis::atLeft,   {1, 1}},
    {GPAxis::atRight,  {1, 2}},
  };

  static QMap<unsigned, QPoint> TrackOrientationToMainKeyPosition {
    {GPAxis::atTop,    {2, 0}},
    {GPAxis::atBottom, {1, 0}},
    {GPAxis::atLeft,   {0, 2}},
    {GPAxis::atRight,  {0, 1}},
  };

  static QMap<unsigned, QPoint> TrackOrientationToSecondKeyPosition {
    {GPAxis::atTop,    {2, 2}},
    {GPAxis::atBottom, {1, 2}},
    {GPAxis::atLeft,   {2, 2}},
    {GPAxis::atRight,  {2, 1}},
  };
}

QTracksPlot::QTracksPlot(QWidget *parent, unsigned flags)
  : GraphicsPlotExtended(parent, flags)
  , mTrackAxisOrientation(GPAxis::atTop)
  , mDefaultTrackRectFlags(GPTrackRect::TRDefault)
  , mKeyAxesReversed(true)
  , mHasInfiniteScene(false)
  , mDefaultTrackSize(250)
  , mTracksMarginGroup(this)
  , mGridLayout(nullptr)
  , mInsertionGridIndex(-1)
  , mInsertionRectIndex(-1)
  , mInsertionGrid(false)
  , mInsertionRect(false)
{
  mInteractions |= GP::iSelectAxes;
  setAcceptDrops(true);
  setNoAntialiasingOnDrag(true);
}

QTracksPlot::~QTracksPlot()
{

}

GPLayoutSplitterGrid *QTracksPlot::gridLayout() const
{
  return mGridLayout;
}

void QTracksPlot::setTrackAxisOrientation(GPAxis::AxisType type)
{
  mTrackAxisOrientation = type;
  if (GPAxis::orientation(mTrackAxisOrientation) == Qt::Horizontal)
    KeyAxisOrientation = Qt::Vertical;
  else
    KeyAxisOrientation = Qt::Horizontal;

  mPlotLayout->clear();

  if (mHasInfiniteScene)
  {
    mGridLayout = new GPEXLayoutInfiniteGrid(this, KeyAxisOrientation);
    mGridLayout->setColumnSpacing(50);
    if (KeyAxisOrientation == Qt::Vertical)
      mGridLayout->setMargins(QMargins(45, 5, 45, 5));
    else
      mGridLayout->setMargins(QMargins(5, 45, 5, 45));
  }
  else
  {
    mGridLayout = new GPLayoutSplitterGrid(this, KeyAxisOrientation);
    mGridLayout->setColumnSpacing(20);
    mGridLayout->setMargins(QMargins(5, 5, 5, 5));
  }
  mGridLayout->setParent(this);
  setCurrentLayer(mLayers.last());
  mGridLayout->moveToLayer(mLayers.last(), true);
  mGridLayout->setSplitterLineWidth(4);
  mGridLayout->setSplitterLineColor("#eaeaea");
  mGridLayout->setMaximumSize(maximumSize());

  mPlotLayout->setMargins(QMargins(5,5,5,5));
  mPlotLayout->addElement(0, 0, mGridLayout);
}

GPAxis::AxisType QTracksPlot::getTrackAxisOrientation() const
{
  return mTrackAxisOrientation;
}

void QTracksPlot::setDefaultTrackRectFlags(unsigned flags)
{
  mDefaultTrackRectFlags = flags;
}

unsigned QTracksPlot::getDefaultTrackRectFlags() const
{
  return mDefaultTrackRectFlags;
}

void QTracksPlot::setHasInfiniteScene()
{
  mHasInfiniteScene = true;
}

bool QTracksPlot::getHasInfiniteScene() const
{
  return mHasInfiniteScene;
}

void QTracksPlot::setKeyAxesDefaultName(const QString& name)
{
  mKeyAxesDefaultName = name;
  for (const auto& axis : mTrackGridToMainKeyAxis)
    axis->setLabel(mKeyAxesDefaultName);
  for (const auto& axis : mTrackGridToSecondKeyAxis)
    axis->setLabel(mKeyAxesDefaultName);
}

GPLayoutSplitterGrid *QTracksPlot::addTracksGroup(int gridIndex, QString titleText)
{
  if (!mGridLayout)
    return nullptr;
  mGridLayout->setBlockLayoutUpdate(true);

  GPLayoutSplitterGrid* grid;
  if (mHasInfiniteScene)
  {
    auto t = new GPEXLayoutInfiniteGrid(this, KeyAxisOrientation);
    t->setDefaultSectionSize(mDefaultTrackSize);
    grid = t;
  }
  else
  {
    grid = new GPLayoutSplitterGrid(this, KeyAxisOrientation);
  }
  grid->setMargins(QMargins(0, 10, 0, 10));
  grid->setColumnSpacing(0);
  grid->setRowSpacing(0);

  auto mainKeyType = MainOrientationToKeyOrientations[mTrackAxisOrientation].first;
  auto secondKeyType = MainOrientationToKeyOrientations[mTrackAxisOrientation].second;

  GPTrackRect* rect = new GPTrackRect(this, mainKeyType, mTrackAxisOrientation);
  rect->setFlags(mDefaultTrackRectFlags);

  auto main = rect->addContainer(mTrackAxisOrientation, true);
  auto left = rect->addContainer(mainKeyType);
  auto right = rect->addContainer(secondKeyType);

  main->setMarginGroup(GPAxis::axisTypeToMarginSide(mTrackAxisOrientation), &mTracksMarginGroup);
  connect(main, SIGNAL(axisMovedFromRect(GPAxisRect*)), this, SLOT(onAxisMovedFromRect(GPAxisRect*)));

  grid->expandTo(3, 3);

  auto titlePos = TrackOrientationToTitlePosition[mTrackAxisOrientation];
  auto rectPos = TrackOrientationToRectPosition[mTrackAxisOrientation];
  auto mainPos = TrackOrientationToMainPosition[mTrackAxisOrientation];
  auto leftPos = TrackOrientationToMainKeyPosition[mTrackAxisOrientation];
  auto rightPos = TrackOrientationToSecondKeyPosition[mTrackAxisOrientation];

  if (mHasInfiniteScene)
  {
    grid->setRowStretchFactor(titlePos.x(), 0);
    grid->setRowStretchFactor(mainPos.x(), 0);
    grid->setRowStretchFactor(leftPos.x(), 0);
    grid->setRowStretchFactor(rightPos.x(), 0);
    grid->setRowStretchFactor(rectPos.x(), 1);

    grid->setColumnStretchFactor(titlePos.y(), 0);
    grid->setColumnStretchFactor(mainPos.y(), 0);
    grid->setColumnStretchFactor(leftPos.y(), 0);
    grid->setColumnStretchFactor(rightPos.y(), 0);
    grid->setColumnStretchFactor(rectPos.y(), 1);
  }
  else
  {
    grid->setRowStretchFactor(rectPos.x(), 10000);
    grid->setColumnStretchFactor(rectPos.y(), 10000);
  }

  auto title = new GPTextElement(this);
  title->setText(titleText);
  title->setVisible(!titleText.isEmpty());

  grid->addElement(titlePos.x(), titlePos.y(), title);
  grid->addElement(mainPos.x(), mainPos.y(), main);
  grid->addElement(leftPos.x(), leftPos.y(), left);
  grid->addElement(rightPos.x(), rightPos.y(), right);
  grid->addElement(rectPos.x(), rectPos.y(), rect);


  if (KeyAxisOrientation == Qt::Vertical)
  {
    mGridLayout->insertColumn(gridIndex);
    mGridLayout->addElement(0, gridIndex, grid);
  }
  else
  {
    mGridLayout->insertRow(gridIndex);
    mGridLayout->addElement(gridIndex, 0, grid);
  }

  if (gridIndex - 1 != 0 && !mHasInfiniteScene)
    mGridLayout->insertSplitter(gridIndex - 1);
  if (mHasInfiniteScene)
    grid->insertSplitter(KeyAxisOrientation == Qt::Vertical ? rectPos.y() : rectPos.x());

  auto mainKeyAxis = rect->keyAxis();
  auto secondKeyAxis = addAxis(rect, secondKeyType);
  mainKeyAxis->setLabelPadding(0);
  secondKeyAxis->setLabelPadding(0);
  mainKeyAxis->setSelectableParts(GPAxis::spNone);
  secondKeyAxis->setSelectableParts(GPAxis::spNone);
  mainKeyAxis->setRangeReversed(mKeyAxesReversed);
  secondKeyAxis->setRangeReversed(mKeyAxesReversed);
  mainKeyAxis->setLabel(mKeyAxesDefaultName);
  secondKeyAxis->setLabel(mKeyAxesDefaultName);
  bindAxes(mainKeyAxis, secondKeyAxis);

  if ((Flags & ExBoundedTrackKeyAxes))
  {
    for (const auto& axis : mTrackGridToMainKeyAxis)
      bindAxes(axis, mainKeyAxis);
  }
  if (!mTrackGridToMainKeyAxis.isEmpty())
  {
    auto axis = mTrackGridToMainKeyAxis.first();
    auto ticker = axis->ticker();
    mainKeyAxis->setTicker(ticker);
    secondKeyAxis->setTicker(ticker);

    mainKeyAxis->setLabelFont(axis->labelFont());
    mainKeyAxis->setTickLabelFont(axis->tickLabelFont());
    mainKeyAxis->setLabelColor(axis->labelColor());
    mainKeyAxis->setTickLabelColor(axis->tickLabelColor());

    secondKeyAxis->setLabelFont(axis->labelFont());
    secondKeyAxis->setTickLabelFont(axis->tickLabelFont());
    secondKeyAxis->setLabelColor(axis->labelColor());
    secondKeyAxis->setTickLabelColor(axis->tickLabelColor());
  }

  mTrackGrids.insert(gridIndex, grid);
  mTrackGridToMainKeyAxis[grid] = mainKeyAxis;
  mTrackGridToSecondKeyAxis[grid] = secondKeyAxis;

  mTrackRects[grid] = QList<GPTrackRect*>{ rect };

  mGridLayout->setBlockLayoutUpdate(false);
  queuedReplot();

  return grid;
}

int QTracksPlot::getTracksGroupCount() const
{
  return mTrackGrids.size();
}

GPTrackRect* QTracksPlot::addTrackRect(int gridIndex, int rectIndex, bool fixedSize)
{
  if (auto grid = mTrackGrids.value(gridIndex))
  {
    if (!mGridLayout)
      return nullptr;
    mGridLayout->setBlockLayoutUpdate(true);

    {
      if (rectIndex < 0)
        rectIndex = 0;
      int size = mTrackRects[grid].size();
      if (rectIndex > size)
        rectIndex = size;
    }

    QPoint rectPos = TrackOrientationToRectPosition[mTrackAxisOrientation];
    QPoint axisPos = TrackOrientationToMainPosition[mTrackAxisOrientation];
    int splitterIndex = -1;
    if (KeyAxisOrientation == Qt::Vertical)
    {
      axisPos.setY(axisPos.y() + rectIndex);
      rectPos.setY(rectPos.y() + rectIndex);
      grid->insertColumn(rectPos.y());
      splitterIndex = rectPos.y();
    }
    else
    {
      axisPos.setX(axisPos.x() + rectIndex);
      rectPos.setX(rectPos.x() + rectIndex);
      grid->insertRow(rectPos.x());
      splitterIndex = rectPos.x();
    }

    if (!fixedSize)
    {
      if (rectIndex > 0)
        grid->insertSplitter(splitterIndex - 1);
      if (rectIndex < mTrackRects[grid].size() || mHasInfiniteScene)
        grid->insertSplitter(splitterIndex);
    }

    GPTrackRect* rect = new GPTrackRect(this,
                                              MainOrientationToKeyOrientations[mTrackAxisOrientation].first,
                                              mTrackAxisOrientation,
                                              mTrackGridToMainKeyAxis[grid]);
    rect->setFlags(mDefaultTrackRectFlags);

    auto main = rect->addContainer(mTrackAxisOrientation, true);
    mTrackRects[grid].insert(rectIndex, rect);

    main->setMarginGroup(GPAxis::axisTypeToMarginSide(mTrackAxisOrientation), &mTracksMarginGroup);
    connect(main, SIGNAL(axisMovedFromRect(GPAxisRect*)), this, SLOT(onAxisMovedFromRect(GPAxisRect*)));

    if (mHasInfiniteScene)
    {
      double rectRowStretch = 1;
      double rectColStretch = 1;

      if (fixedSize)
      {
        if (KeyAxisOrientation == Qt::Vertical)
          rectColStretch = 0;
        else
          rectRowStretch = 0;
      }

      grid->setRowStretchFactor(rectPos.x(), rectRowStretch);
      grid->setColumnStretchFactor(rectPos.y(), rectColStretch);
    }
    else
    {
      grid->setRowStretchFactor(rectPos.x(), 10000);
      grid->setColumnStretchFactor(rectPos.y(), 10000);
    }

    grid->addElement(rectPos.x(), rectPos.y(), rect);
    grid->addElement(axisPos.x(), axisPos.y(), main);

    mGridLayout->setBlockLayoutUpdate(false);

    queuedReplot();
    return rect;
  }
  return nullptr;
}

void QTracksPlot::removeTracksGroup(int gridIndex)
{
  if (mGridLayout)
  {
    if (auto grid = mTrackGrids.value(gridIndex))
    {
      auto mainAxisTicker = mTrackGridToMainKeyAxis.first()->ticker();
      if (auto aat = qobject_cast<GPAxisAwareTicker*>(mainAxisTicker.data()))
      {
        auto axis = mTrackGridToMainKeyAxis.value(grid);
        if (aat->GetAxis() == axis)
        {
          if (auto newAxis = mTrackGridToMainKeyAxis.value(mTrackGrids.value(gridIndex + 1)))
            aat->SetAxis(newAxis);
        }
      }

      removeTrackGroupPlottables(gridIndex);

      auto rects = mTrackRects.value(grid);
      qDeleteAll(rects);

      mTrackRects.remove(grid);
      mTrackGridToMainKeyAxis.remove(grid);
      mTrackGridToSecondKeyAxis.remove(grid);

      mTrackGrids.removeAt(gridIndex);
      delete grid;
      mGridLayout->removeColumn(gridIndex);

      if (mTrackGrids.size() == 1)
        mGridLayout->removeAllSplitters();

      queuedReplot();
    }
  }
}

void QTracksPlot::removeTrackRect(int gridIndex, int rectIndex)
{
  if (auto grid = mTrackGrids.value(gridIndex))
  {
    auto rects = mTrackRects.value(grid);
    if (auto rect = rects.value(rectIndex))
    {
      if (!(rect->getFlags() & GPTrackRect::TRCanRemove))
        return;

      if (rects.size() <= 1)
      {
        if ((Flags & ExAddRemoveGrids))
          removeTracksGroup(gridIndex);
        return;
      }

      removeTrackPlottables(gridIndex, rectIndex);

      auto mainType = MainOrientationToKeyOrientations[mTrackAxisOrientation].first;
      auto secondType = MainOrientationToKeyOrientations[mTrackAxisOrientation].second;
      {
        auto mainAxis = mTrackGridToMainKeyAxis[grid];
        auto secondAxis = mTrackGridToSecondKeyAxis[grid];
        if ((mainAxis && mainAxis->axisRect() == rect)
         || (secondAxis && secondAxis->axisRect() == rect))
        {
          auto cm = rect->takeContainer(mainType);
          auto cs = rect->takeContainer(secondType);

          GPTrackRect* nextRect = (rectIndex == 0) ? rects.value(1) : rects.value(0);

          nextRect->addContainer(mainType, cm);
          nextRect->addContainer(secondType, cs);

          nextRect->addAxis(mainType, mainAxis);
          nextRect->addAxis(secondType, secondAxis);
        }
      }

      delete rect;
      int rectRow, rectColumn;
      getRectPositionInGrid(rectIndex, rectRow, rectColumn);
      if (KeyAxisOrientation == Qt::Vertical)
      {
        grid->removeColumn(rectColumn);
        if (rectIndex == rects.size() - 1 && !mHasInfiniteScene)
          grid->removeSplitter(rectColumn - 1);
      }
      else
      {
        grid->removeRow(rectRow);
        if (rectIndex == rects.size() - 1 && !mHasInfiniteScene)
          grid->removeSplitter(rectRow - 1);
      }
      mTrackRects[grid].removeAt(rectIndex);

      queuedReplot();
    }
  }
}

int QTracksPlot::getTrackRectsCount(int gridIndex)
{
  return mTrackRects.value(mTrackGrids.value(gridIndex)).size();
}

void QTracksPlot::setTrackFlags(int gridIndex, int rectIndex, unsigned flags)
{
  if (auto rect = mTrackRects.value(mTrackGrids.value(gridIndex)).value(rectIndex))
    rect->setFlags(flags);
}

void QTracksPlot::removeTrackPlottables(int gridIndex, int rectIndex)
{
  if (Flags & ExAutoRemoveTrackPlottables)
  {
    auto rect = mTrackRects.value(mTrackGrids.value(gridIndex)).value(rectIndex);

    for (const auto& plottable : mPlottables)
    {
      if (plottable->valueAxis() && plottable->valueAxis()->axisRect() == rect)
        removePlottable(plottable);
    }
  }
}

void QTracksPlot::removeTrackGroupPlottables(int gridIndex)
{
  if (Flags & ExAutoRemoveTrackPlottables)
  {
    for (int rectIndex = 0; rectIndex < mTrackRects.value(mTrackGrids.value(gridIndex)).size(); ++rectIndex)
      removeTrackPlottables(gridIndex, rectIndex);
  }
}

void QTracksPlot::clearPlot()
{
  for (const auto& plottable : mPlottables)
    removePlottable(plottable);
  while(!mTrackGrids.isEmpty())
    removeTracksGroup(0);
  if (mGridLayout)
  {
    while (mGridLayout->rowCount() > 0)
      mGridLayout->removeRow(0);
    while (mGridLayout->columnCount() > 0)
      mGridLayout->removeColumn(0);
  }
}

GPTrackAxis* QTracksPlot::addTrackAxis(int gridIndex, int rectIndex, GPTrackAxis *axis)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  if (auto rect = rects.value(rectIndex))
  {
    if (axis == nullptr)
      axis = new GPTrackAxis(rect, mTrackAxisOrientation);
    addAxis(rect, mTrackAxisOrientation, axis);
    return axis;
  }
  return nullptr;
}

GPTrackAxis *QTracksPlot::addTrackAxis(int gridIndex, int rectIndex, const QString& label, const QString& id)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  if (auto rect = rects.value(rectIndex))
  {
    auto axis = new GPTrackAxis(rect, mTrackAxisOrientation);
    addAxis(rect, mTrackAxisOrientation, axis);
    axis->setLabel(label);
    axis->setProperty(OKAxisId, id);
    return axis;
  }
  return nullptr;
}

GPAxis* QTracksPlot::addAxis(int gridIndex, int rectIndex, GPAxis::AxisType type, GPAxis* axis)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  if (auto rect = rects.value(rectIndex))
    return addAxis(rect, type, axis);
  return nullptr;
}

void QTracksPlot::removeAxis(GPAxis* axis)
{
  if (axis)
  {
    auto oldblockReplots = getBlockReplots();
    blockReplots(true);
    for (const auto& plottable : axis->plottables())
      removePlottable(plottable);
    if (auto rect = axis->axisRect())
      rect->removeAxis(axis);
    blockReplots(oldblockReplots);
    queuedReplot();
  }
}

GPAxis* QTracksPlot::getKeyAxisForTrack(GPTrackAxis *trackAxis)
{
  if (auto rect = qobject_cast<GPTrackRect*>(trackAxis->axisRect()))
    return rect->keyAxis();
  return nullptr;
}

GPAxis* QTracksPlot::getKeyAxisForGrid(int gridIndex)
{
  return mTrackGridToMainKeyAxis.value(mTrackGrids.value(gridIndex));
}

GPCurve* QTracksPlot::addCurve(int index, GPTrackAxis* trackAxis)
{
  if (auto keyAxis = getKeyAxisForTrack(trackAxis))
  {
    GPCurve* curve = GraphicsPlotExtended::addCurve(index, keyAxis, trackAxis);
    curve->setName(trackAxis->label());
    trackAxis->setCurve(curve);
    return curve;
  }
  return nullptr;
}

GPCurve* QTracksPlot::addCurve(GPTrackAxis* trackAxis)
{
  return addCurve(-1, trackAxis);
}

int QTracksPlot::getValueGridLinesCount(int gridIndex)
{
  if (auto rect = mTrackRects.value(mTrackGrids.value(gridIndex)).value(0))
    return rect->getValueGridLinesCount();
  return 0;
}

void QTracksPlot::setValueGridLinesCount(int gridIndex, int count)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  for (const auto& rect : rects)
  {
    if (rect)
    {
      rect->setValueGridLinesCount(count);
    }
  }
  queuedReplot();
}

bool QTracksPlot::getKeyGridVisible(int gridIndex)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  if (auto rect = rects.value(0))
    return rect->getKeyGridVisible();
  return false;
}

bool QTracksPlot::getValueGridVisible(int gridIndex)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  if (auto rect = rects.value(0))
    return rect->getValueGridVisible();
  return false;
}

void QTracksPlot::setKeyGridVisible(int gridIndex, bool visible)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  for (const auto& rect : rects)
    rect->setKeyGridVisible(visible);
  queuedReplot();
}

void QTracksPlot::setValueGridVisible(int gridIndex, bool visible)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  for (const auto& rect : rects)
    rect->setValueGridVisible(visible);
  queuedReplot();
}

int QTracksPlot::gridIndexAt(QPoint pos)
{
  for (int i = 0; i < mTrackGrids.size(); ++i)
  {
    if (auto grid = mTrackGrids[i])
    {
      auto rect = grid->outerRect();
      if (KeyAxisOrientation == Qt::Vertical)
      {
        rect.setTop(0);
        rect.setBottom(height());
      }
      else
      {
        rect.setLeft(0);
        rect.setRight(width());
      }
      if (rect.contains(pos))
        return i;
    }
  }
  return -1;
}

int QTracksPlot::rectIndexAt(QPoint pos)
{
  return rectIndexAt(gridIndexAt(pos), pos);
}

int QTracksPlot::rectIndexAt(int gridIndex, QPoint pos)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  for (int i = 0; i < rects.size(); ++i)
  {
    if (auto rect = rects[i])
    {
      if (KeyAxisOrientation == Qt::Vertical)
      {
        if (rect->outerRect().left() <= pos.x() && rect->outerRect().right() >= pos.x())
          return i;
      }
      else
      {
        if (rect->outerRect().top() <= pos.y() && rect->outerRect().bottom() >= pos.y())
          return i;
      }
    }
  }
  return -1;
}

GPLayoutSplitterGrid *QTracksPlot::gridAt(QPoint pos)
{
  return mTrackGrids.value(gridIndexAt(pos));
}

GPTrackRect *QTracksPlot::rectAt(QPoint pos)
{
  return rectAt(gridIndexAt(pos), pos);
}

GPTrackRect *QTracksPlot::rectAt(int gridIndex, QPoint pos)
{
  auto rects = mTrackRects.value(mTrackGrids.value(gridIndex));
  for (int i = 0; i < rects.size(); ++i)
  {
    if (auto rect = rects[i])
    {
      if (KeyAxisOrientation == Qt::Vertical)
      {
        if (rect->outerRect().left() <= pos.x() && rect->outerRect().right() >= pos.x())
          return rect;
      }
      else
      {
        if (rect->outerRect().top() <= pos.y() && rect->outerRect().bottom() >= pos.y())
          return rect;
      }
    }
  }
  return nullptr;
}

int QTracksPlot::getGridIndex(GPLayoutSplitterGrid *grid)
{
  return mTrackGrids.indexOf(grid);
}

int QTracksPlot::getRectIndex(GPLayoutSplitterGrid* grid, GPTrackRect* rect)
{
  return mTrackRects.value(grid).indexOf(rect);
}

void QTracksPlot::getInsertPosition(QPoint pos)
{
  mInsertionGridIndex = -1;
  mInsertionRectIndex = -1;
  mInsertionGrid = false;
  mInsertionRect = false;

  auto isElemOnRight = [this](GPLayoutElement* elem, QPoint pos)
  {
    if (!elem)
      return false;
    if (KeyAxisOrientation == Qt::Vertical)
      return elem->outerRect().left() > pos.x();
    return elem->outerRect().top() > pos.y();
  };
  auto isInElem = [this](GPLayoutElement* elem, QPoint pos)
  {
    if (!elem)
      return false;
    if (KeyAxisOrientation == Qt::Vertical)
      return elem->outerRect().right() > pos.x();
    return elem->outerRect().bottom() > pos.y();
  };


  for (int i = 0; i < mTrackGrids.size(); ++i)
  {
    if (isElemOnRight(mTrackGrids[i], pos))
    {
      mInsertionGridIndex = i;
      if (Flags & ExAddGrids)
      {
        mInsertionGrid = true;
      }
      else if (Flags & ExAddTracks)
      {
        mInsertionRectIndex = 0;
        mInsertionRect = true;
      }
      return;
    }
    if ((Flags & ExAddTracks) && isInElem(mTrackGrids[i], pos))
    {
      auto rects = mTrackRects.value(mTrackGrids[i]);
      if (!rects.isEmpty())
      {
        if (isElemOnRight(rects.value(rects.size()/2), pos))
          mInsertionRectIndex = 0;
        else
          mInsertionRectIndex = getTrackRectsCount(i);
      }

      if (mInsertionRectIndex != -1)
      {
        mInsertionGridIndex = i;
        mInsertionRect = true;
      }
      return;
    }
  }
  if ((Flags & ExAddGrids) && (mTrackGrids.isEmpty() || !isInElem(mTrackGrids.last(), pos)))
  {
    mInsertionGridIndex = mTrackGrids.size();
    mInsertionGrid = true;
  }
}

void QTracksPlot::onAxisMovedFromRect(GPAxisRect *oldAxisRect)
{
  if (auto rect = qobject_cast<GPTrackRect*>(oldAxisRect))
  {
    if (auto grid = qobject_cast<GPLayoutSplitterGrid*>(rect->layout()))
    {
      int gridIndex = mTrackGrids.indexOf(grid);
      int rectIndex = mTrackRects.value(grid).indexOf(rect);
      if (gridIndex != -1 && rectIndex != -1)
      {
        if (rect->axisCount(mTrackAxisOrientation) == 0)
        {
          if (((Flags & (ExAddRemoveGrids | ExAddRemoveTracks)) || (Flags & (ExAutoRemoveGrids | ExAutoRemoveTracks)))
               && mTrackRects[grid].size() == 1)
            removeTracksGroup(gridIndex);
          else if ((Flags & ExAddRemoveTracks) || (Flags & ExAutoRemoveTracks))
            removeTrackRect(gridIndex, rectIndex);
        }
      }
    }
  }
}

void QTracksPlot::FillContextMenu(QContextMenuEvent *event, QMenu &menu, std::map<QAction *, std::function<void ()>> &actions)
{
  GraphicsPlotExtended::FillContextMenu(event, menu, actions);

  int gridIndex = gridIndexAt(event->pos());
  int rectIndex = rectIndexAt(gridIndex, event->pos());

  auto grid = mTrackGrids.value(gridIndex);
  auto rect = mTrackRects.value(grid).value(rectIndex);

  if ((gridIndex != -1 && (Flags & (ExAddGrids | ExRemoveGrids | ExMoveGrids))) ||
      (rectIndex != -1 && (Flags & (ExAddTracks | ExRemoveTracks | ExMoveTracks))))
    menu.addSeparator();

  if (Flags & (ExAddGrids | ExRemoveGrids | ExMoveGrids) && mGridLayout)
  {
    QMenu* groupMenu = menu.addMenu(tr("Tracks group"));

    if (Flags & ExAddGrids)
    {
      int insertIndex = gridIndex != -1 ? gridIndex : mGridLayout->columnCount();
      QAction* addGridAction = groupMenu->addAction(tr("Add"));
      actions[addGridAction] = std::bind(&QTracksPlot::addTracksGroup, this, insertIndex, QString());
    }
    if (Flags & ExRemoveGrids && gridIndex != -1)
    {
      QAction* removeGridAction = groupMenu->addAction(tr("Remove"));
      actions[removeGridAction] = std::bind(&QTracksPlot::removeTracksGroup, this, gridIndex);
    }
    if (Flags & ExMoveGrids)
    {
      auto swapGrids = [this, gridIndex](int incr)
      {
        if (mGridLayout)
        {
          int left = gridIndex;
          int right = gridIndex + incr;

          mGridLayout->swapColumns(left, right);
          mTrackGrids.swapItemsAt(left, right);

          queuedReplot();
        }
      };

      if (gridIndex > 0)
      {
        QAction* moveGridToLeft = groupMenu->addAction(tr("Move left"));
        actions[moveGridToLeft] = std::bind(swapGrids, -1);
      }
      if (gridIndex < mTrackGrids.size() - 1)
      {
        QAction* moveGridToRight = groupMenu->addAction(tr("Move right"));
        actions[moveGridToRight] = std::bind(swapGrids, 1);
      }
    }
  }

  if (gridIndex != -1 && rectIndex != -1 && Flags & (ExAddTracks | ExRemoveTracks | ExMoveTracks))
  {
    QMenu* tracksMenu = menu.addMenu(tr("Track"));

    if (Flags & ExAddTracks)
    {
      QAction* removeRectAction = tracksMenu->addAction(tr("Add"));
      actions[removeRectAction] = std::bind(&QTracksPlot::addTrackRect, this, gridIndex, rectIndex + 1, false);
    }
    if (Flags & ExRemoveTracks && rect->getFlags() & GPTrackRect::TRCanRemove)
    {
      QAction* removeRectAction = tracksMenu->addAction(tr("Remove"));
      actions[removeRectAction] = std::bind(&QTracksPlot::removeTrackRect, this, gridIndex, rectIndex);
    }
    if (Flags & ExMoveTracks && rect->getFlags() & GPTrackRect::TRCanMove)
    {
      auto swapTracks = [this, rectIndex, grid](int incr)
      {
        int row, column;
        getRectPositionInGrid(rectIndex, row, column);
        if (KeyAxisOrientation == Qt::Vertical)
          grid->swapColumns(column, column + incr);
        else
          grid->swapRows(row, row + incr);
        mTrackRects[grid].swapItemsAt(rectIndex, rectIndex + incr);
        queuedReplot();
      };

      if (rectIndex > 0)
      {
        QAction* moveTrackToLeft = tracksMenu->addAction(tr("Move left"));
        actions[moveTrackToLeft] = std::bind(swapTracks, -1);
      }
      if (rectIndex < mTrackRects.value(grid).size() - 1)
      {
        QAction* moveTrackToRight = tracksMenu->addAction(tr("Move right"));
        actions[moveTrackToRight] = std::bind(swapTracks, 1);
      }
    }
  }

  auto mainAxis = mTrackGridToMainKeyAxis.value(grid);
  auto secondAxis = mTrackGridToSecondKeyAxis.value(grid);
  if (mainAxis || secondAxis)
  {
    menu.addSeparator();
    auto toggleAxisVisibility = [this](GPAxis* axis, bool visible)
    {
      axis->setVisible(visible);
      queuedReplot();
    };

    if (mainAxis)
    {
      QString text = tr("Show %1 axis").arg(KeyAxisOrientation == Qt::Vertical ? tr("left") : tr("top"));
      QAction* actionToggleMainAxis = menu.addAction(text);
      actionToggleMainAxis->setCheckable(true);
      actionToggleMainAxis->setChecked(mainAxis->visible());
      actions[actionToggleMainAxis] = std::bind(toggleAxisVisibility, mainAxis, !mainAxis->visible());
    }
    if (secondAxis)
    {
      QString text = tr("Show %1 axis").arg(KeyAxisOrientation == Qt::Vertical ? tr("right") : tr("bottom"));
      QAction* actionToggleSecondAxis = menu.addAction(text);
      actionToggleSecondAxis->setCheckable(true);
      actionToggleSecondAxis->setChecked(secondAxis->visible());
      actions[actionToggleSecondAxis] = std::bind(toggleAxisVisibility, secondAxis, !secondAxis->visible());
    }
  }
}

void QTracksPlot::CreateGridActions(QContextMenuEvent *event, QMenu &menu, std::map<QAction *, std::function<void ()>> &actions)
{
  int gridIndex = gridIndexAt(event->pos());
  int rectIndex = rectIndexAt(gridIndex, event->pos());

  auto grid = mTrackGrids.value(gridIndex);
  auto rect = mTrackRects.value(grid).value(rectIndex);

  if (Flags & GP::AAGrid)
  {
    if (rect)
    {
      QMenu* gridActions = menu.addMenu(tr("Grid"));
      {
        QMenu* actionKeyAxisGridSetLineCount = gridActions->addMenu(tr("Set key axis grid lines count"));
        actionKeyAxisGridSetLineCount->setProperty("Persistent", true);

        QGridLayout* grid = new QGridLayout(actionKeyAxisGridSetLineCount);
        grid->setContentsMargins(4, 4, 4, 4);

        QSpinBox* spinLines = new QSpinBox(actionKeyAxisGridSetLineCount);
        spinLines->setMinimum(0);
        spinLines->setMaximum(100);
        spinLines->setValue(rect->getValueGridLinesCount());
        grid->addWidget(spinLines);

        auto updateGridLines = [this, gridIndex, spinLines]()
        {
          setValueGridLinesCount(gridIndex, spinLines->value());
        };
        connect(spinLines, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), updateGridLines);
      }

      {
        QAction* actionToggleKeyAxisGrid = gridActions->addAction(tr("Show key axis grid"));
        actionToggleKeyAxisGrid->setCheckable(true);
        actionToggleKeyAxisGrid->setChecked(rect->getKeyGridVisible());
        actions[actionToggleKeyAxisGrid] = std::bind(&QTracksPlot::setKeyGridVisible, this, gridIndex, !rect->getKeyGridVisible());
      }

      {
        QAction* actionToggleValueAxisGrid = gridActions->addAction(tr("Show value axis grid"));
        actionToggleValueAxisGrid->setCheckable(true);
        actionToggleValueAxisGrid->setChecked(rect->getValueGridVisible());
        actions[actionToggleValueAxisGrid] = std::bind(&QTracksPlot::setValueGridVisible, this, gridIndex, !rect->getValueGridVisible());
      }
    }
  }
}

void QTracksPlot::keyPressEvent(QKeyEvent *event)
{
  if ((Flags & ExRemoveAxes) && event->key() == Qt::Key_Delete)
  {
    auto axes = selectedAxes();
    if (!axes.isEmpty())
    {
      for (const auto& axis : axes)
        removeAxis(axis);

      queuedReplot();
      event->accept();
      return;
    }
  }
  return GraphicsPlotExtended::keyPressEvent(event);
}

void QTracksPlot::dragEnterEvent(QDragEnterEvent *event)
{
  if (!(Flags & ExAddAxes))
    return;

  blockReplots(true);
  event->ignore();

  QList<GPLayerable*> candidates = layerableListAt(event->pos(), false);
  for (const auto& candidate : candidates)
  {
    if (auto dropableTarget = qobject_cast<GPLayoutElement*>(candidate))
    {
      dropableTarget->dragEnterEvent(event);
      if (event->isAccepted())
        resetLastDropTargetItemState(dropableTarget);
    }
  }

  if (!event->isAccepted())
  {
    resetLastDropTargetItemState();
    getInsertPosition(event->pos());
    event->accept();
  }

  blockReplots(false);
  if (mReplotQueued)
    replot();
}

void QTracksPlot::dragMoveEvent(QDragMoveEvent *event)
{
  if (!(Flags & ExAddAxes))
    return;

  blockReplots(true);
  event->ignore();

  QList<GPLayerable*> candidates = layerableListAt(event->pos(), false);
  for (const auto& candidate : candidates)
  {
    if (auto dropableTarget = qobject_cast<GPLayoutElement*>(candidate))
    {
      dropableTarget->dragMoveEvent(event);
      if (event->isAccepted())
      {
        resetLastDropTargetItemState(dropableTarget);
        break;
      }
    }
  }

  if (!event->isAccepted())
  {
    resetLastDropTargetItemState();
    getInsertPosition(event->pos());
    event->accept();
  }

  blockReplots(false);
  if (mReplotQueued)
    replot();
}

void QTracksPlot::dropEvent(QDropEvent *event)
{
  if (!(Flags & ExAddAxes))
    return;

  blockReplots(true);
  event->ignore();

  if (mInsertionGrid || mInsertionRect)
  {
    GPTrackRect* rect = nullptr;

    if (mInsertionGrid)
    {
      auto grid = addTracksGroup(mInsertionGridIndex);
      if (mTrackRects.contains(grid))
      {
        rect = mTrackRects[grid].value(0);
        mInsertionRectIndex = 0;
      }
    }
    else
    {
      rect = addTrackRect(mInsertionGridIndex, mInsertionRectIndex);
    }

    if (rect)
    {
      if (auto container = rect->valueAxisContainer())
      {
        container->dropAxes(event->mimeData());

        if (container->getMaximumAxes() > 0)
        {
          int axesCount = event->mimeData()->data("application/gp-dragging-axes/count").toInt();
          int droped = container->count();

          while (axesCount - droped > 0)
          {
            mInsertionRectIndex++;
            if (auto nextRect = addTrackRect(mInsertionGridIndex, mInsertionRectIndex))
            {
              if (auto nextContainer = nextRect->valueAxisContainer())
              {
                nextContainer->dropAxes(event->mimeData(), droped);
                if (nextContainer->count() == 0)
                  break;
                droped += nextContainer->count();
              }
            }
          }
        }

        if (mInsertionGrid)
        {
          updateLayout();
          viewAll(QSet<GPAxis*>({ rect->keyAxis() }));
        }
        event->accept();
      }
    }
    resetLastDropTargetItemState();
  }
  else
  {
    QList<GPLayerable*> candidates = layerableListAt(event->pos(), false);
    for (const auto& candidate : candidates)
    {
      if (auto dropableTarget = qobject_cast<GPLayoutElement*>(candidate))
      {
        dropableTarget->dropEvent(event);
        if (event->isAccepted())
        {
          resetLastDropTargetItemState(dropableTarget);
          break;
        }
      }
    }
  }
  blockReplots(false);
  if (mReplotQueued)
    replot();
}

void QTracksPlot::dragLeaveEvent(QDragLeaveEvent *event)
{
  if (!(Flags & ExAddAxes))
    return;

  resetLastDropTargetItemState();
  event->accept();
}

void QTracksPlot::axisRemoved(GPAxis *axis)
{
  onAxisMovedFromRect(axis->axisRect());
  GraphicsPlotExtended::axisRemoved(axis);
}

void QTracksPlot::resetLastDropTargetItemState(GPLayoutElement *newItem)
{
  bool needReplot = false;
  if (mLastDropTarget != newItem)
  {
    if (mLastDropTarget)
    {
      QDragLeaveEvent le;
      mLastDropTarget->dragLeaveEvent(&le);
    }
    mLastDropTarget = newItem;
    needReplot = true;
  }
  needReplot |= mInsertionGrid || mInsertionRect;
  mInsertionGrid = mInsertionRect = false;

  if (needReplot)
    replot(rpQueuedReplot);
}

void QTracksPlot::getRectPositionInGrid(int rectIndex, int &row, int &column)
{
  QPoint rectPos = TrackOrientationToRectPosition[mTrackAxisOrientation];
  if (KeyAxisOrientation == Qt::Vertical)
  {
    row = rectPos.x();
    column = rectPos.y() + rectIndex;
  }
  else
  {
    row = rectPos.x() + rectIndex;
    column = rectPos.y();
  }
}

void QTracksPlot::paintEvent(QPaintEvent *event)
{
  GraphicsPlotExtended::paintEvent(event);

  QRect insertionRect;

  if (mInsertionGrid)
  {
    insertionRect = plotLayout()->outerRect();
    if (KeyAxisOrientation == Qt::Vertical)
    {
      if (auto left = mTrackGrids.value(mInsertionGridIndex - 1))
        insertionRect.setLeft(left->outerRect().right());
      if (auto right = mTrackGrids.value(mInsertionGridIndex))
        insertionRect.setRight(right->outerRect().left());
    }
    else
    {
      if (auto top = mTrackGrids.value(mInsertionGridIndex - 1))
        insertionRect.setTop(top->outerRect().bottom());
      if (auto bottom = mTrackGrids.value(mInsertionGridIndex))
        insertionRect.setBottom(bottom->outerRect().top());
    }
  }
  if (mInsertionRect)
  {
    if (auto grid = mTrackGrids.value(mInsertionGridIndex))
    {
      insertionRect = grid->outerRect();
      auto rects = mTrackRects.value(grid);
      if (KeyAxisOrientation == Qt::Vertical)
      {
        if (mInsertionRectIndex <= rects.size() / 2)
          insertionRect.setRight(rects.value(0)->outerRect().left());
        else
          insertionRect.setLeft(rects.value(rects.size() - 1)->outerRect().right());
      }
      else
      {
        if (mInsertionRectIndex <= rects.size() / 2)
          insertionRect.setBottom(rects.value(0)->outerRect().top());
        else
          insertionRect.setTop(rects.value(rects.size() - 1)->outerRect().bottom());
      }
    }
  }

  if (!insertionRect.isEmpty())
  {
    auto color = DCInsertingRect;
    color.setAlpha(50);

    GPPainter painter(this);
    painter.fillRect(insertionRect, color);
  }
}
