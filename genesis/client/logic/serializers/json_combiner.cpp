#include "json_combiner.h"

#include <qvariantmap>

namespace Service
{
  QJsonObject JSonCombiner::Combine(const QJsonObject& left, const QJsonObject& right) const
  {
    auto map = left.toVariantMap();
    map.insert(right.toVariantMap());
    return QJsonObject::fromVariantMap(map);
  }

  QJsonObject JSonCombiner::Combine(const QVector<QJsonObject>& items) const
  {
    auto first = items.begin();
    auto map = first->toVariantMap();
    while (first != items.end())
    {
      map.insert(first->toVariantMap());
      first++;
    }
    return QJsonObject::fromVariantMap(map);
  }
}