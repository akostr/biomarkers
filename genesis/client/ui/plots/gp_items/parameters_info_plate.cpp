#include "parameters_info_plate.h"
#include <genesis_style/style.h>

ParametersInfoPlate::ParametersInfoPlate(GraphicsPlot* parentPlot)
  : GPRichTextItem(parentPlot)
  , ActiveParameterColor(QColor(102, 121, 133))
  , InactiveParameterColor(QColor(224, 228, 231))
  , ActiveValueColor(QColor(0, 32, 51))
  , InactiveValueColor(QColor(204, 210, 214))
  , ContentFontSize(qApp->font().pointSize())
  , IsInactive(false)
{
  QFont font(Style::GetSASSValue("fontFaceNormal"),
             -1,
             Style::GetSASSValue("fontWeight").toInt());
  font.setPixelSize(Style::GetSASSValue("fontSizeSecondaryTextScalableFont").toInt());
  SetFont(font);
  GPItemRect::setPen(Qt::NoPen);
  GPItemRect::setBrush(Qt::NoBrush);
  GPItemRect::setSelectedPen(Qt::NoPen);
  GPItemRect::setSelectedBrush(Qt::NoBrush);
}

void ParametersInfoPlate::SetContentFontSize(uint fontSize)
{
  if (ContentFontSize == fontSize)
    return;
  ContentFontSize = fontSize;
  update();
}

void ParametersInfoPlate::SetParametersList(const ParamsContainer& list)
{
  if (Parameters == list)
    return;
  Parameters = list;
  update();
}

void ParametersInfoPlate::ClearParametersMap()
{
  if (Parameters.empty())
    return;
  Parameters.clear();
  update();
}

void ParametersInfoPlate::AppendParameter(const QString& parameter, const QString& value)
{
  Parameters.emplace(parameter, value);
  update();
}

QString ParametersInfoPlate::GetParameterValue(const QString& parameter) const
{
  if (const auto& it = Parameters.find(parameter); it != Parameters.end())
  {
    return it->second;
  }
  return "";
}

void ParametersInfoPlate::SetParameterValue(const QString& parameter, const QString& value)
{
  Parameters[parameter] = value;
  update();
}

void ParametersInfoPlate::SetActiveParameterColor(const QColor& newActiveParameterColor)
{
  if (ActiveParameterColor == newActiveParameterColor)
    return;
  ActiveParameterColor = newActiveParameterColor;
  update();
  //  emit ActiveParameterColorChanged();
}

void ParametersInfoPlate::SetInactiveParameterColor(const QColor& newInactiveParameterColor)
{
  if (InactiveParameterColor == newInactiveParameterColor)
    return;
  InactiveParameterColor = newInactiveParameterColor;
  update();
  //  emit InactiveParameterColorChanged();
}

void ParametersInfoPlate::SetActiveValueColor(const QColor& newActiveValueColor)
{
  if (ActiveValueColor == newActiveValueColor)
    return;
  ActiveValueColor = newActiveValueColor;
  update();
  //  emit ActiveValueColorChanged();
}

void ParametersInfoPlate::SetInactiveValueColor(const QColor& newInactiveValueColor)
{
  if (InactiveValueColor == newInactiveValueColor)
    return;
  InactiveValueColor = newInactiveValueColor;
  update();
  //  emit InactiveValueColorChanged();
}

void ParametersInfoPlate::SetIsInactive(bool newIsInactive)
{
  if (IsInactive == newIsInactive)
    return;
  IsInactive = newIsInactive;
  update();
  //  emit IsInactiveChanged();
}

uint ParametersInfoPlate::GetContentFontSize() const
{
  return ContentFontSize;
}

const QColor& ParametersInfoPlate::getActiveParameterColor() const
{
  return ActiveParameterColor;
}

const QColor& ParametersInfoPlate::getInactiveParameterColor() const
{
  return InactiveParameterColor;
}

const QColor& ParametersInfoPlate::getActiveValueColor() const
{
  return ActiveValueColor;
}

const QColor& ParametersInfoPlate::getInactiveValueColor() const
{
  return InactiveValueColor;
}

bool ParametersInfoPlate::getIsInactive() const
{
  return IsInactive;
}

void ParametersInfoPlate::resetActiveParameterColor()
{
  SetActiveParameterColor(QColor(128, 128, 128));
}

void ParametersInfoPlate::resetInactiveParameterColor()
{
  SetInactiveParameterColor(QColor(192, 192, 192));
}

void ParametersInfoPlate::resetActiveValueColor()
{
  SetActiveValueColor(QColor(0, 0, 0));
}

void ParametersInfoPlate::resetInactiveValueColor()
{
  SetInactiveValueColor(QColor(128, 128, 128));
}

void ParametersInfoPlate::resetIsInactive()
{
  SetIsInactive(false);
}

void ParametersInfoPlate::showParameters(bool isShown)
{
  IsParametersShown = isShown;
  update();
}

void ParametersInfoPlate::update()
{
  QColor cl, cr, cap;

  if (IsInactive)
  {
    cl = InactiveParameterColor;
    cr = InactiveValueColor;
  }
  else
  {
    cl = ActiveParameterColor;
    cr = ActiveValueColor;
  }

  QStringList html;

  QString templateStr = QString("<font color=%1>%2</font>  <font color=%3>%4</font>");
  if (IsParametersShown)
  {
    for (const auto& [name, value] : Parameters)
    {
      auto str = templateStr
        .arg(cl.name())
        .arg(name)
        .arg(cr.name())
        .arg(value);
      if (!html.empty())
      {
        str.prepend("<br>");
        str.append("</br>");
      }
      html.append(str);
    }
  }

  SetText(html.join("\r\n"));
  parentPlot()->replot(GraphicsPlot::RefreshPriority::rpQueuedReplot);

  //  emit TextFormatChanged();
}
