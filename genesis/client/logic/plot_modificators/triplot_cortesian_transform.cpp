#include "triplot_cortesian_transform.h"

#include <algorithm>

namespace
{
  const double triplotSumAxisValue = 100;
}

namespace Service
{
  QVector<QPointF> TriplotCortesianTransform::toCortesian(const QVector<QPointF>& values) const
  {
    QVector<QPointF> coords;
    coords.reserve(values.size());
    std::transform(values.begin(), values.end(), std::back_inserter(coords),
      [](const QPointF& point) { return QPointF{ (point.y() + (triplotSumAxisValue - point.x() - point.y())/2) , sqrt(3) * point.y() / 2 }; });
    return coords;
  }
}