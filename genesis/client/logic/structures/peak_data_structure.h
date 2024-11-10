#pragma once
#ifndef PEAK_DATA_STRUCTURE_H
#define PEAK_DATA_STRUCTURE_H
#include <QList>

namespace Structures
{
  enum class PeakType
  {
    PTPeak,
    PTMarker,
    PTInterMarker,
    PTFake,
    None
  };


  struct Interval
  {
    double Start;
    double End;
  };

  struct Rectangle
  {
    double XTop;
    double XBottom;

    double YTop;
    double YBottom;
  };

  struct Point
  {
    double X;
    double Y;
  };

  struct Color
  {
    int Red;
    int Green;
    int Blue;
    int Alpha;
  };

  struct GeoParams
  {
    double RetantionTime;
    double CovatsIndex;
    double MarkerWindow;
    double Area;
    double Height;
  };

  struct ChromaData
  {
      QList<Point> CurveData;
      QList <int> ions;
  };
}
#endif
