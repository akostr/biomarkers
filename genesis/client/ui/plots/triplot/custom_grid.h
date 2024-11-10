#pragma once
#ifndef CUSTOM_GRID_H
#define CUSTOM_GRID_H


#include "graphicsplot/graphicsplot.h"

class CustomGrid final : public GPGrid
{
  Q_OBJECT
public:
  explicit CustomGrid(GPAxis* parentAxis) : GPGrid(parentAxis) {};
  explicit CustomGrid(GPLayerable* parent) : GPGrid(parent) {};

protected:
  // virtual methods:
  virtual void drawGridLines(GPPainter* painter) const override;
  virtual void drawSubGridLines(GPPainter* painter) const override;
};
#endif