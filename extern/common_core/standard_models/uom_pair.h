#ifndef UOM_PAIR_H
#define UOM_PAIR_H

#include <QMetaType>

#include "uom.h"

class QJsonObject;

class UOMPair : public std::pair<UOM::PhysicalQuantity, int>
{
public:
  UOMPair(int quantity, int unit);
  UOMPair(const QJsonObject&);
  UOMPair();

  bool isValid() const;
  bool isValuable() const;

  UOM::PhysicalQuantity quantity() const { return this->first; }
  int unit() const { return this->second; }
  double convertToSI(const double v) const { return UOM::ConvertAnyToSI(v, this->first, this->second); }
  double convertFromSI(const double v) const { return UOM::ConvertSIToAny(v, this->first, this->second); }
  QString displayName() const { return UOM::GetUnitsDisplayName(this->first, this->second); }
  QString displayNameSimplified() const { return UOM::GetUnitsDisplayNameSimplified(this->first, this->second); }
  QString displayNameSimplifiedAlternative() const { return UOM::GetUnitsDisplayNameSimplifiedAlternative(this->first, this->second); }
  QString displayNameUnicoded() const { return UOM::GetUnitsDisplayNameUnicoded(this->first, this->second); }
  QString quantityDisplayName() const { return UOM::GetPhysicalQuantityDisplayName(this->first); }
  QJsonObject toJsonObject() const;
  QVariant toVariant() const;
  static bool checkJson(const QJsonObject&);
};

Q_DECLARE_METATYPE(UOMPair);

QDebug operator<<(QDebug, const UOMPair&);

#endif // UOM_PAIR_H
