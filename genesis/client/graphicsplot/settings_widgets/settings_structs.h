#pragma once

#include <QFont>
#include <QPen>
#include <QMap>

#include "../graphicsplot_extended.h"

struct CurveSettings
{
  CurveSettings(){}
  CurveSettings(GPCurve* curve);
  void Load(GPCurve* curve);
  void Apply(GPCurve* curve);
  void ApplyDiff(const CurveSettings& newSettings, QMap<GPCurve*, CurveSettings>& curveMap);

  QPen linePen;
  GPScatterStyle pointStyle;
  QBrush curveBrush;
};

struct FontSettings
{
  FontSettings(){}
  FontSettings(GPAxis* axis);
  FontSettings(GPLegend* legend);
  FontSettings(GPTextElement* elem);
  FontSettings(GPZonesRect* zones);

  void Load(GPAxis* axis);
  void Load(GPLegend* axis);
  void Load(GPTextElement* elem);
  void Load(GPZonesRect* zones);

  void Apply(GPAxis* axis);
  void Apply(GPLegend* legend);
  void Apply(GPTextElement* elem);
  void Apply(GPZonesRect* zones);

  void ApplyDiff(const FontSettings& newSettings, QMap<QObject*, FontSettings>& objectsMap);

  QFont font;
  QColor color;
};
