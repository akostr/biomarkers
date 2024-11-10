#pragma once
#ifndef CUSTOM_AXIS_H
#define CUSTOM_AXIS_H

#include <QObject>

#include "graphicsplot/graphicsplot.h"

class CustomAxis final : public GPAxis
{
  Q_OBJECT
public:
  explicit CustomAxis(GPAxisRect* parent, AxisType type);
  ~CustomAxis() = default;

  QVector<double> subTickVector() { return mSubTickVector; };
};
#endif