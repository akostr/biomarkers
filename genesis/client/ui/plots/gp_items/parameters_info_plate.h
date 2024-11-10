#pragma once
#ifndef PARAMETERS_INFO_PLATE_H

#include "gp_rich_text_item.h"
#include <unordered_map>

using ParamsContainer = std::unordered_map<QString, QString>;

class ParametersInfoPlate : public GPRichTextItem
{
  Q_OBJECT

public:

  ParametersInfoPlate(GraphicsPlot* parentPlot);

  void SetContentFontSize(uint fontSize);
  void SetParametersList(const ParamsContainer& list);
  void ClearParametersMap();
  void AppendParameter(const QString& parameter, const QString& value);
  QString GetParameterValue(const QString& parameter) const;
  void SetParameterValue(const QString& parameter, const QString& value);
  void update();

  void SetActiveParameterColor(const QColor& newActiveParameterColor);
  void SetInactiveParameterColor(const QColor& newInactiveParameterColor);
  void SetActiveValueColor(const QColor& newActiveValueColor);
  void SetInactiveValueColor(const QColor& newInactiveValueColor);
  void SetIsInactive(bool newIsInactive);

  uint GetContentFontSize() const;
  const QColor& getActiveParameterColor() const;
  const QColor& getInactiveParameterColor() const;
  const QColor& getActiveValueColor() const;
  const QColor& getInactiveValueColor() const;
  bool getIsInactive() const;

  void resetActiveParameterColor();
  void resetInactiveParameterColor();
  void resetActiveValueColor();
  void resetInactiveValueColor();
  void resetIsInactive();

public slots:
  void showParameters(bool isShown);

private:
  ParamsContainer Parameters;
  QColor ActiveParameterColor;
  QColor InactiveParameterColor;
  QColor ActiveValueColor;
  QColor InactiveValueColor;
  uint ContentFontSize;

  bool IsInactive;
  bool IsParametersShown;
};

#endif
