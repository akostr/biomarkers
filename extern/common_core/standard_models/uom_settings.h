#pragma once

#include "uom.h"

#include <QObject>
#include <QVariant>
#include <QSet>

///////////////////////////////////////////////////////
//// Centralized UOM settings
class UOMSettings : public QObject
{
  Q_OBJECT
public:
  explicit UOMSettings(QObject* parent = nullptr);
  ~UOMSettings();

  static UOMSettings& Get();
  static void SetSavePath(const QString& savePath);

  static QString                Normalize(const QString& key);
  static QString                GetStringKey(UOM::PhysicalQuantity quantity);
  static QString                GetStringKey(UOM::PhysicalQuantity quantity, unsigned units);
  static UOM::PhysicalQuantity  GetQuantityFromStringKey(const QString& key);
  static unsigned               GetUnitsFromStringKey(const QString& key);
  static UOM::Descriptor        Parse(const QString& uiUnits);

public slots:
  void SetupDefautSettings();
  void SetupSemanticsDisplayNames();
  void SetupSemanticsAcceptableUiUnits();
  void RetranslateUi();

  //// Semantic properties setters / getters
  UOM::PhysicalQuantity GetQuantityForSemantics(const QString& semantics);
  void                  SetQuantityForSemantics(const QString& semantics, UOM::PhysicalQuantity quantity);

  static QString        GetDefaultSemanticsForQuantity(UOM::PhysicalQuantity quantity);

  int  GetUiUnitsForSemantics(const QString& semantics);
  void SetUiUnitsForSemantics(const QString& semantics, int units);

  QSet<int> GetAcceptableUiUnitsForSemantics(const QString& semantics);

  int  GetLegacyUntaggedUnitsForematics(const QString& semantics);

  void Load(const QVariantMap& object);
  void Save(QVariantMap &object);

  void Load();
  void Save();

  void Export(QString filePath);
  void Import(QString filePath);

protected:
  void Setup();

signals:
  void QuantitiesChanged(const QString& semantics, int quantity);
  void UnitsChanged(const QString& semantics, int units);

private:
  QVariantMap Settings;
};
