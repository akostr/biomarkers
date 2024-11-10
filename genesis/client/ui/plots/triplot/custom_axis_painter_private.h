#pragma once
#ifndef CUSTOM_AXIS_PAINTER_PRIVATE_H
#define CUSTOM_AXIS_PAINTER_PRIVATE_H

#include "graphicsplot/graphicsplot.h"

class CustomAxisPainterPrivate : public GPAxisPainterPrivate
{
public:
  explicit CustomAxisPainterPrivate(GraphicsPlot* parent);
  ~CustomAxisPainterPrivate() = default;

  void draw(GPPainter* painter) override;
};
#endif