#pragma once

#include "standard_item_model.h"
#include "uom.h"

///////////////////////////////////////////////////////
//// Standard UOM Model
class StandardUOMModel : public StandardItemModel
{
  Q_OBJECT
public:

#include "abstract_uom_model_traits.inc"

public:
#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
  using StandardItemModel::Save;
  using StandardItemModel::Load;
#endif // !DISABLE_STANDARD_MODELS_SERIALIZING

public:
  StandardUOMModel(QObject* parent = nullptr);
  ~StandardUOMModel();

  //////////////////////////////////////////////////////////////////////
  //// Static sccessors - UOM properties
  static QVariantMap GetUOM(QAbstractItemModel* model);
  static void SetUOM(QAbstractItemModel* model, const QVariantMap& uom);

  static QVariantMap GetSemantics(QAbstractItemModel* model);
  static void SetSemantics(QAbstractItemModel* model, const QVariantMap& semantics);

  static QVariantMap GetQuantities(QAbstractItemModel* model);
  static void SetQuantities(QAbstractItemModel* model, const QVariantMap& quantities);

  static QVariantMap GetUnitsUI(QAbstractItemModel* model);
  static void SetUnitsUI(QAbstractItemModel* model, const QVariantMap& units);

  static QVariantMap GetUnitsStored(QAbstractItemModel* model);
  static void SetUnitsStored(QAbstractItemModel* model, const QVariantMap& units);

  //// Static sccessors - UOM properties - Semantics
  static QString GetColumnSemantics(QAbstractItemModel* model, const QString& columnKey);
  static QString GetColumnSemantics(QAbstractItemModel* model, int columnIndex);

  static void SetColumnSemantics(QAbstractItemModel* model, const QString& columnKey, const QString& semantics);
  static void SetColumnSemantics(QAbstractItemModel* model, int columnIndex, const QString& semantics);

  //// Static sccessors - UOM properties - Quantities
  static UOM::PhysicalQuantity GetColumnQuantity(QAbstractItemModel* model, const QString& columnKey);
  static UOM::PhysicalQuantity GetColumnQuantity(QAbstractItemModel* model, int columnIndex);

  static void SetColumnQuantity(QAbstractItemModel* model, const QString& columnKey, UOM::PhysicalQuantity quantity);
  static void SetColumnQuantity(QAbstractItemModel* model, int columnIndex, UOM::PhysicalQuantity quantity);

  //// Static sccessors - UOM properties - Units in UI
  static int GetColumnUnitsUI(QAbstractItemModel* model, const QString& columnKey);
  static int GetColumnUnitsUI(QAbstractItemModel* model, int columnIndex);

  static void SetColumnUnitsUI(QAbstractItemModel* model, const QString& columnKey, int units);
  static void SetColumnUnitsUI(QAbstractItemModel* model, int columnIndex, int units);

  //// Units as stored
  static int GetColumnUnitsStored(QAbstractItemModel* model, const QString& columnKey);
  static int GetColumnUnitsStored(QAbstractItemModel* model, int columnIndex);

  static void SetColumnUnitsStored(QAbstractItemModel* model, const QString& columnKey, int units);
  static void SetColumnUnitsStored(QAbstractItemModel* model, int columnIndex, int units);

  //// Notifications
  static void NotifyUOMChanged(QAbstractItemModel* model, int columnIndex);
  static void NotifyUOMChanged(QAbstractItemModel* model, const QString& columnKey);

public slots:
#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
  //// StandardItemModel reimpl
  Q_INVOKABLE virtual void Load(const rapidjson::Value& json); 
  Q_INVOKABLE virtual void Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const;
#endif

  //// Centralized Semantics change handler
  void        HandleCentralizedSemanticsUiUnitsChange(const QString& semantics, int units);

  void        NotifyUOMChanged(int columnIndex);
  void        NotifyUOMChanged(const QString& columnKey); 

  //// Convert stored data
  void             ConvertStoredData(const QModelIndex& parent, int column, UOM::PhysicalQuantity quantity, int unitsFrom, int unitsTo);
  void             ConvertStoredDataToSI(const QModelIndex& parent, int column);
  void             ConvertStoredDataToSI(const QString& columnKey);
  void             ConvertStoredDataToSI(const QModelIndex& root, const QString& columnKey);

  QSet<QString>    CollectAllUOMColumnKeys();
  QSet<QString>    CollectDynamicColumnKeys();
  void             CollectDynamicColumnKeys(const QModelIndex& root, QSet<QString>& keys);

  //////////////////////////////////////////////////////////////////////
  //// Reimplemented
  virtual QVariant  data(const QModelIndex& index, int role) const override;
  virtual bool      setData(const QModelIndex& index, const QVariant& data, int role = Qt::EditRole) override;
  virtual QVariant  headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  virtual bool      setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

signals:
  void UnitsUiChanged(int columnIndex, int units);
};
