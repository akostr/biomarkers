#include "settings_structs.h"

CurveSettings::CurveSettings(GPCurve *curve)
{
  Load(curve);
}

void CurveSettings::Load(GPCurve *curve)
{
  linePen = curve->pen();
  pointStyle = curve->scatterStyle();
  curveBrush = curve->brush();
}

void CurveSettings::Apply(GPCurve *curve)
{
  curve->setPen(linePen);
  if (auto decorator = curve->selectionDecorator())
  {
    QPen selectionPen = linePen;
    selectionPen.setColor(selectionPen.color().lighter(120));
    selectionPen.setWidthF(selectionPen.widthF() + 1.5);
    decorator->setPen(selectionPen);
  }
  curve->setScatterStyle(pointStyle);
  curve->setBrush(curveBrush);
}

void CurveSettings::ApplyDiff(const CurveSettings& newSettings, QMap<GPCurve*, CurveSettings> &curveMap)
{
  bool clr, wdth, stl, pclr, psz, pshp, bstl, bclr;
  clr = wdth = stl = pclr = psz = pshp = bstl = bclr = false;
  if (newSettings.linePen.color() != linePen.color())
    clr = true;
  if (newSettings.linePen.widthF() != linePen.widthF())
    wdth = true;
  if (newSettings.linePen.style() != linePen.style())
    stl = true;
  if (newSettings.pointStyle.pen().color() != pointStyle.pen().color())
    pclr = true;
  if (newSettings.pointStyle.size() != pointStyle.size())
    psz = true;
  if (newSettings.pointStyle.shape() != pointStyle.shape())
    pshp = true;
  if (newSettings.curveBrush.style() != curveBrush.style())
    bstl = true;
  if (newSettings.curveBrush.color() != curveBrush.color())
    bclr = true;

  for (auto& curve : curveMap)
  {
    if (clr)
      curve.linePen.setColor(newSettings.linePen.color());
    if (wdth)
      curve.linePen.setWidthF(newSettings.linePen.widthF());
    if (stl)
      curve.linePen.setStyle(newSettings.linePen.style());
    if (pclr)
      curve.pointStyle.setPen(newSettings.pointStyle.pen());
    if (psz)
      curve.pointStyle.setSize(newSettings.pointStyle.size());
    if (pshp)
      curve.pointStyle.setShape(newSettings.pointStyle.shape());
    if (bclr)
      curve.curveBrush.setColor(newSettings.curveBrush.color());
    if (bstl)
      curve.curveBrush.setStyle(newSettings.curveBrush.style());
  }
}

FontSettings::FontSettings(GPAxis *axis)
{
  Load(axis);
}

FontSettings::FontSettings(GPLegend *legend)
{
  Load(legend);
}

FontSettings::FontSettings(GPTextElement *elem)
{
  Load(elem);
}

FontSettings::FontSettings(GPZonesRect *zones)
{
  Load(zones);
}

void FontSettings::Load(GPAxis *axis)
{
  font = axis->labelFont();
  color = axis->labelColor();
}

void FontSettings::Load(GPLegend *legend)
{
  font = legend->font();
  color = legend->textColor();
}

void FontSettings::Load(GPTextElement *elem)
{
  font = elem->font();
  color = elem->textColor();
}

void FontSettings::Load(GPZonesRect *zones)
{
  auto style = zones->getStyle();
  font = style.textFont;
  color = style.textColor;
}

void FontSettings::Apply(GPAxis *axis)
{
  axis->setLabelColor(color);
  axis->setTickLabelColor(color);

  axis->setLabelFont(font);
  axis->setTickLabelFont(font);
  axis->setSelectedLabelFont(font);
  axis->setSelectedTickLabelFont(font);
}

void FontSettings::Apply(GPLegend *legend)
{
  legend->setTextColor(color);
  legend->setFont(font);
  legend->setSelectedFont(font);
}

void FontSettings::Apply(GPTextElement *elem)
{
  elem->setTextColor(color);
  elem->setFont(font);
  elem->setSelectedFont(font);
}

void FontSettings::Apply(GPZonesRect *zones)
{
  auto style = zones->getStyle();
  style.textColor = color;
  style.textFont = font;
  zones->setStyle(style);
}

void FontSettings::ApplyDiff(const FontSettings& newSettings, QMap<QObject*, FontSettings> &objectsMap)
{
  bool family, size, style, weight, color;
  family = size = style = weight = color = false;
  if (newSettings.font.family() != font.family())
    family = true;
  if (newSettings.font.pointSize() != font.pointSize())
    size = true;
  if (newSettings.font.weight() != font.weight())
    weight = true;
  if (newSettings.font.style() != font.style())
    style = true;
  if (newSettings.color != this->color)
    color = true;

  for (auto& axis : objectsMap)
  {
    if (family)
      axis.font.setFamily(newSettings.font.family());
    if (size)
      axis.font.setPointSize(newSettings.font.pointSize());
    if (weight)
      axis.font.setWeight(newSettings.font.weight());
    if (style)
      axis.font.setStyle(newSettings.font.style());
    if (color)
      axis.color = newSettings.color;
 }
}
