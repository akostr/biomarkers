#include "uom_pair.h"

#include <QJsonObject>
#include <QDebug>

namespace {

const char QUANTITY_KEY[] = "quantity";
const char UNIT_KEY[] = "unit";

}

UOMPair::UOMPair(int quantity, int unit)
  : std::pair<UOM::PhysicalQuantity, int>{
      static_cast<UOM::PhysicalQuantity>(quantity),
      unit
      }
{}

UOMPair::UOMPair(const QJsonObject& obj)
  : std::pair<UOM::PhysicalQuantity, int>{
      static_cast<UOM::PhysicalQuantity>(obj.value(QUANTITY_KEY).toInt(-1)),
      obj.value(UNIT_KEY).toInt(-1)
      }
{}

UOMPair::UOMPair() : std::pair<UOM::PhysicalQuantity, int>{UOM::PhysicalQuantityNone, 0}{}

bool UOMPair::isValid() const {
  if (this->second < 0 || this->first < 0) {
    return false;
  }
  if (UOM::GetUnitsCount(this->first) < this->second) {
    return false;
  }
  return true;
}

bool UOMPair::isValuable() const
{
  return isValid() && this->first != UOM::PhysicalQuantityNone;
}

QJsonObject UOMPair::toJsonObject() const {
  if (!isValid()) {
    return QJsonObject{};
  }
  QJsonObject result;
  result.insert(QUANTITY_KEY, this->first);
  result.insert(UNIT_KEY, this->second);
  return result;
}

QVariant UOMPair::toVariant() const
{
  QVariant r;
  r.setValue(*this);
  return r;
}

QDebug operator<<(QDebug out, const UOMPair& pair)
{
  QString description;
  if (pair.isValid()) {
    description = pair.displayNameSimplified();
    if (pair.first == 0) {
      description = "undefined";
    } else if (pair.second == 0) {
      description = QString("unmeasured %1").arg(pair.quantityDisplayName());
    }
  } else {
    description = QString("invalid(%1,%2)").arg(pair.first, pair.second);
  }
  return out << QString("UOMPair{%1}").arg(description);
}

bool UOMPair::checkJson(const QJsonObject& o)
{
  if (!o.contains(QUANTITY_KEY)) {
    return false;
  }
  if (!o.contains(UNIT_KEY)) {
    return false;
  }
  if (!o[QUANTITY_KEY].isDouble() || !o[UNIT_KEY].isDouble()) {
    return false;
  }
  const int q(o[QUANTITY_KEY].toDouble());
  const int u(o[UNIT_KEY].toDouble());
  return UOMPair(q, u).isValid();
}
