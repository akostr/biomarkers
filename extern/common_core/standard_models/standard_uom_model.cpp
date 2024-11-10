#include "standard_uom_model.h"
#include "uom_settings.h"

#ifdef USE_UBJSON
#ifndef DISABLE_UBJSON
#include <utils/json_utils/base.h>
#endif // !DISABLE_UBJSON
#endif

#include <QJsonObject>
#include <QPointer>
#include <QTimer>
#include <QDebug>

#pragma warning(1 : 4061)
#pragma warning(1 : 4062)

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define VARIANT_TYPE_ID typeId
#else
#define VARIANT_TYPE_ID type
#endif

///////////////////////////////////////////////////////
//// Standard UOM Model
StandardUOMModel::StandardUOMModel(QObject* parent)
  : StandardItemModel(parent)
{
  UOMSettings* centralized = &UOMSettings::Get();
  connect(centralized, &UOMSettings::UnitsChanged, this, &StandardUOMModel::HandleCentralizedSemanticsUiUnitsChange);
}

StandardUOMModel::~StandardUOMModel()
{
}

//////////////////////////////////////////////////////////////////////
//// Static sccessors - UOM properties
QVariantMap StandardUOMModel::GetUOM(QAbstractItemModel* model)
{  
  auto uom = model->property(UOM::KeyUOM);
  if (uom.VARIANT_TYPE_ID() == QMetaType::QVariantMap)
    return std::move(*reinterpret_cast<const QVariantMap*>(&uom));
  return std::move(QVariantMap());
}

void StandardUOMModel::SetUOM(QAbstractItemModel* model, const QVariantMap& uom)
{
  model->setProperty(UOM::KeyUOM, uom);
}

QVariantMap StandardUOMModel::GetSemantics(QAbstractItemModel* model)
{
  QVariantMap uom = GetUOM(model);
  if (uom.contains(UOM::KeySemantics))
  {
    auto& v = uom[UOM::KeySemantics];
    if (v.VARIANT_TYPE_ID() == QMetaType::QVariantMap)
      return std::move(*reinterpret_cast<const QVariantMap*>(&v));
  }
  return std::move(QVariantMap());
}

void StandardUOMModel::SetSemantics(QAbstractItemModel* model, const QVariantMap& semantics)
{
  //// Ensure normalized
  NormalizeKeys(model);

  QVariantMap uom = GetUOM(model);
  uom[UOM::KeySemantics] = semantics;
  SetUOM(model, uom);
}

QVariantMap StandardUOMModel::GetQuantities(QAbstractItemModel* model)
{
  QVariantMap uom = GetUOM(model);
  if (uom.contains(UOM::KeyQuantities))
  {
    const auto& quantities = uom[UOM::KeyQuantities];
    if (quantities.VARIANT_TYPE_ID() == QMetaType::QVariantMap)
      return std::move(*reinterpret_cast<const QVariantMap*>(&uom[UOM::KeyQuantities]));
  }
  return std::move(QVariantMap());
}

void StandardUOMModel::SetQuantities(QAbstractItemModel* model, const QVariantMap& quantities)
{
  //// Ensure normalized
  NormalizeKeys(model);

  QVariantMap uom = GetUOM(model);
  uom[UOM::KeyQuantities] = quantities;
  SetUOM(model, uom);
}

QVariantMap StandardUOMModel::GetUnitsUI(QAbstractItemModel* model)
{
  QVariantMap uom = GetUOM(model);
  if (uom.contains(UOM::KeyUnitsUI))
  {
    auto& v = uom[UOM::KeyUnitsUI];
    if (v.VARIANT_TYPE_ID() == QMetaType::QVariantMap)
      return std::move(*reinterpret_cast<const QVariantMap*>(&v));
  }
  return std::move(QVariantMap());
}

void StandardUOMModel::SetUnitsUI(QAbstractItemModel* model, const QVariantMap& units)
{
  //// Ensure normalized
  NormalizeKeys(model);

  QVariantMap uom = GetUOM(model);
  uom[UOM::KeyUnitsUI] = units;
  SetUOM(model, uom);
}

QVariantMap StandardUOMModel::GetUnitsStored(QAbstractItemModel* model)
{
  QVariantMap uom = GetUOM(model);
  if (uom.contains(UOM::KeyUnitsStored))
  {
    auto& v = uom[UOM::KeyUnitsStored];
    if (v.VARIANT_TYPE_ID() == QMetaType::QVariantMap)
      return std::move(*reinterpret_cast<const QVariantMap*>(&v));
  }
  return std::move(QVariantMap());
}

void StandardUOMModel::SetUnitsStored(QAbstractItemModel* model, const QVariantMap& units)
{
  //// Ensure normalized
  NormalizeKeys(model);

  QVariantMap uom = GetUOM(model);
  uom[UOM::KeyUnitsStored] = units;
  SetUOM(model, uom);
}

//// Static sccessors - UOM properties - Semantics
QString StandardUOMModel::GetColumnSemantics(QAbstractItemModel* model, const QString& columnKey)
{
  QVariantMap semantics = GetSemantics(model);
  QVariantMap::iterator semantic = semantics.find(columnKey); // Find by string key
  if (semantic == semantics.end())
  {
    semantic = semantics.find(QString::number(GetColumnIndex(model, columnKey))); // Find by integer key
  }
  if (semantic != semantics.end())
  {
    return std::move(semantic.value().toString());
  }
  return std::move(QString());
}

QString StandardUOMModel::GetColumnSemantics(QAbstractItemModel* model, int columnIndex)
{
  return std::move(GetColumnSemantics(model, GetStaticColumnKey(model, columnIndex)));
}

void StandardUOMModel::SetColumnSemantics(QAbstractItemModel* model, const QString& columnKey, const QString& semantic)
{
  //// Ensure normalized
  NormalizeKeys(model);

  //// Set
  QVariantMap semantics = GetSemantics(model);
  semantics[columnKey] = semantic;                        // by string key
  int columnIndex = GetColumnIndex(model, columnKey);
  if (columnIndex != -1)
    semantics[QString::number(columnIndex)] = semantic;   // by integer key
  SetSemantics(model, semantics);

  //// Update quantities & units accordingly
  {
    UOM::PhysicalQuantity quantity = UOMSettings::Get().GetQuantityForSemantics(semantic);
    int units = UOMSettings::Get().GetUiUnitsForSemantics(semantic);
    SetColumnQuantity(model, columnKey, quantity);
    SetColumnUnitsUI(model, columnKey, units);
    SetColumnUnitsStored(model, columnKey, UOM::GetSIUnitsForQuantity(quantity));
  }
}

void StandardUOMModel::SetColumnSemantics(QAbstractItemModel* model, int columnIndex, const QString& semantics)
{
  SetColumnSemantics(model, GetStaticColumnKey(model, columnIndex), semantics);
}

//// Static sccessors - UOM properties - Quantities
UOM::PhysicalQuantity StandardUOMModel::GetColumnQuantity(QAbstractItemModel* model, const QString& columnKey)
{
  QVariantMap quantities = GetQuantities(model);
  QVariantMap::iterator quantity = quantities.find(columnKey);    // by string key
  if (quantity == quantities.end())
  {
    quantity = quantities.find(QString::number(GetColumnIndex(model, columnKey))); // by int key
  }
  if (quantity != quantities.end())
  {
    return (UOM::PhysicalQuantity)quantity.value().toInt();
  }
  return UOM::PhysicalQuantityNone;
}

UOM::PhysicalQuantity StandardUOMModel::GetColumnQuantity(QAbstractItemModel* model, int columnIndex)
{
  return GetColumnQuantity(model, GetStaticColumnKey(model, columnIndex));
}

void StandardUOMModel::SetColumnQuantity(QAbstractItemModel* model, const QString& columnKey, UOM::PhysicalQuantity quantity)
{
  //// Ensure normalized
  NormalizeKeys(model);

  QVariantMap quantities = GetQuantities(model);
  quantities[columnKey] = quantity;                       // by string key
  int columnIndex = GetColumnIndex(model, columnKey);
  if (columnIndex != -1)
    quantities[QString::number(columnIndex)] = quantity;  // by int key
  SetQuantities(model, quantities);

  NotifyUOMChanged(model, columnKey);
}

void StandardUOMModel::SetColumnQuantity(QAbstractItemModel* model, int columnIndex, UOM::PhysicalQuantity quantity)
{
  SetColumnQuantity(model, GetStaticColumnKey(model, columnIndex), quantity);
}

//// Static sccessors - UOM properties - Units in UI
int StandardUOMModel::GetColumnUnitsUI(QAbstractItemModel* model, const QString& columnKey)
{
  QVariantMap units = GetUnitsUI(model);
  QVariantMap::iterator u = units.find(columnKey); // by string key
  if (u == units.end())
  {
    u = units.find(QString::number(GetColumnIndex(model, columnKey))); // by int key
  }
  if (u != units.end())
  {
    return u.value().toInt();
  }
  return UOM::AnyNone;
}

int StandardUOMModel::GetColumnUnitsUI(QAbstractItemModel* model, int columnIndex)
{
  return GetColumnUnitsUI(model, GetStaticColumnKey(model, columnIndex));
}

void StandardUOMModel::SetColumnUnitsUI(QAbstractItemModel* model, const QString& columnKey, int units)
{
  //// Ensure normalized
  NormalizeKeys(model);

  QVariantMap allUnits = GetUnitsUI(model);
  allUnits[columnKey] = units;                          // by string key
  int columnIndex = GetColumnIndex(model, columnKey);
  if (columnIndex != -1)
    allUnits[QString::number(columnIndex)] = units;     // by int key
  SetUnitsUI(model, allUnits);

  NotifyUOMChanged(model, columnKey);
}

void StandardUOMModel::SetColumnUnitsUI(QAbstractItemModel* model, int columnIndex, int units)
{
  SetColumnUnitsUI(model, GetStaticColumnKey(model, columnIndex), units);
}

//// Static sccessors - UOM properties - Units as stored
int StandardUOMModel::GetColumnUnitsStored(QAbstractItemModel* model, const QString& columnKey)
{
  if (columnKey.isEmpty())
    return UOM::AnyNone;

  QVariantMap units = GetUnitsStored(model);
  QVariantMap::iterator u = units.find(columnKey);  // by string key
  if (u == units.end())
  {
    int columnIndex = GetColumnIndex(model, columnKey);
    if (columnIndex != -1)
    {
      u = units.find(QString::number(columnIndex)); // by int key
    }
  }
  if (u != units.end())
  {
    return u->toInt();
  }
  return UOM::AnyNone;
}

int StandardUOMModel::GetColumnUnitsStored(QAbstractItemModel* model, int columnIndex)
{
  return GetColumnUnitsStored(model, GetStaticColumnKey(model, columnIndex));
}

void StandardUOMModel::SetColumnUnitsStored(QAbstractItemModel* model, const QString& columnKey, int units)
{
  //// Ensure normalized
  NormalizeKeys(model);

  QVariantMap allUnits = GetUnitsStored(model);
  allUnits[columnKey] = units;                        // by string key
  int columnIndex = GetColumnIndex(model, columnKey);
  if (columnIndex != -1)
    allUnits[QString::number(columnIndex)] = units;   // by int key
  SetUnitsStored(model, allUnits);
}

void StandardUOMModel::SetColumnUnitsStored(QAbstractItemModel* model, int columnIndex, int units)
{
  SetColumnUnitsStored(model, GetStaticColumnKey(model, columnIndex), units);
}

//// Static sccessors - UOM properties - Notifications
void StandardUOMModel::NotifyUOMChanged(QAbstractItemModel* model, int columnIndex)
{
  StandardUOMModel* uomModel = qobject_cast<StandardUOMModel*>(model);
  if (uomModel)
    uomModel->NotifyUOMChanged(columnIndex);
}

void StandardUOMModel::NotifyUOMChanged(QAbstractItemModel* model, const QString& columnKey)
{
  NotifyUOMChanged(model, GetColumnIndex(model, columnKey));
}

#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
///////////////////////////////////////////////////////
//// StandardItemModel reimpl
void StandardUOMModel::Load(const rapidjson::Value& json)
{
  /////////////////////////////////////////////////////////////////////
  // BEGIN load
  if (json.IsObject() && json.HasMember(UOM::KeyUOM) && json[UOM::KeyUOM].IsObject())
  {
    beginResetModel();
    blockSignals(true);

    // UOM settings root
#if defined(USE_UBJSON)
    QJsonObject uom = JsonUtils::Convert::RapidJsonToQJson(json[UOM::KeyUOM]).toObject();
#else
    QJsonObject uom;
    return;
#endif
    QJsonObject unitsStored = uom[UOM::KeyUnitsStored].toObject();

    // Collect keys
    QSet<QString> columnKeysSet;
    QVariantMap columnKeys = GetColumnKeys();
    for (auto c = columnKeys.begin(); c != columnKeys.end(); ++c)
    {
      columnKeysSet << c.value().toString();
    }

    QSet<QString> semanticsKeySet;
    QVariantMap semantics = GetSemantics();
    for (auto sk = semantics.begin(); sk != semantics.end(); ++sk)
    {
      bool isInt = false;
      sk.key().toInt(&isInt);
      if (!isInt)
        semanticsKeySet << sk.key();
    }
    columnKeysSet += semanticsKeySet;

    // Go through all indexed columns
    for (auto c = columnKeysSet.begin(); c != columnKeysSet.end(); ++c)
    {
      // Column
      QString columnKey = *c;
      if (!columnKey.isEmpty())
      {
        // Semantics
        QString columnSemantics = GetColumnSemantics(columnKey);
        if (!columnSemantics.isEmpty())
        {
          int explicitlyStoredUnit = UOM::KeysUnitsBackwards.value(unitsStored.value(columnKey).toString(), UOM::AnyNone);
          if (explicitlyStoredUnit == UOM::AnyNone)
          {
            // Units not indicated, load as legacy
            int legacyUnits = UOMSettings::Get().GetLegacyUntaggedUnitsForematics(columnSemantics);
            if (legacyUnits != UOM::AnyNone)
            {
              qDebug() << "defaulting to legacy for model" << metaObject()->className() << columnKey << legacyUnits;
              SetColumnUnitsStored(columnKey, legacyUnits);
            }
          }
        }
      }
    }
  }

  /////////////////////////////////////////////////////////////////////
  // DO load
  StandardItemModel::Load(json);

  /////////////////////////////////////////////////////////////////////
  // END load
  if (json.IsObject() && json.HasMember(UOM::KeyUOM) && json[UOM::KeyUOM].IsObject())
  {
    // UOM settings root
#if defined(USE_UBJSON)
    QJsonObject uom = JsonUtils::Convert::RapidJsonToQJson(json[UOM::KeyUOM]).toObject();
#else
    QJsonObject uom;
    return;
#endif
    QSet<QString> keys;

    // Load Units stored
    {
      QJsonObject unitsStored = uom[UOM::KeyUnitsStored].toObject();
      for (QJsonObject::iterator u = unitsStored.begin(); u != unitsStored.end(); ++u)
      {
        QString columnKey = u.key();
        keys << columnKey;
        int units = UOM::KeysUnitsBackwards.value(u.value().toString(), UOM::AnyNone);
        if (units != UOM::AnyNone)
        {
          SetColumnUnitsStored(columnKey, units);
        }
      }
    }

    //// Normalize loaded
    {
      // Convert columns w/ dynamic keys
      QSet<QString> dynamicColumnKeys = CollectDynamicColumnKeys();
      // qDebug() << "dynamic keys" << dynamicColumnKeys;
      for (auto dck = dynamicColumnKeys.begin(); dck != dynamicColumnKeys.end(); ++dck)
      {
        ConvertStoredDataToSI(QModelIndex(), *dck);
      }

      // Columns by indexes in flat model
      for (int c = 0; c < columnCount(); ++c)
      {
        ConvertStoredDataToSI(QModelIndex(), c);
      }

      // Columns by keys
      for (QSet<QString>::iterator k = keys.begin(); k != keys.end(); ++k)
      {
        ConvertStoredDataToSI(*k);
      }

      // Columns by keys from quantities
      auto qs = GetQuantities();
      for (auto q = qs.begin(); q != qs.end(); ++q)
      {
        ConvertStoredDataToSI(q.key());
      }
    }

    blockSignals(false);
    endResetModel();
  }
}

void StandardUOMModel::Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const
{
  /////////////////////////////////////////////////////////////////////
  // DO save
  StandardItemModel::Save(json, allocator);

  // Save UOM
  {
    // UOM settings root
    QJsonObject uom;

    // Store Quantities
    {
      QJsonObject quantities;
      QVariantMap quantitiesMap = GetQuantities();
      for (QVariantMap::iterator q = quantitiesMap.begin(); q != quantitiesMap.end(); ++q)
      {
        if (q.key() != "-1")
          quantities[q.key()] = UOM::KeysQuantities.value((UOM::PhysicalQuantity)q.value().toInt(), "PhysicalQuantityNone");
      }
      uom[UOM::KeyQuantities] = quantities;
    }

    // Store Units stored
    {
      QJsonObject unitsStored;
      QVariantMap unitsStoredMap = GetUnitsStored();
      QVariantMap quantitiesMap = GetQuantities();
      for (QVariantMap::iterator u = unitsStoredMap.begin(); u != unitsStoredMap.end(); ++u)
      {
        UOM::PhysicalQuantity q = (UOM::PhysicalQuantity)quantitiesMap.value(u.key(), (int)UOM::PhysicalQuantityNone).toInt();
        if (u.key() != "-1")
          unitsStored[u.key()] = UOM::KeysUnits.value({ q, u.value().toInt() }, "UOMAnyNone");
      }
      uom[UOM::KeyUnitsStored] = unitsStored;
    }

    // Store Semantics (preferred) -=XOR=- Units ui
    {
      QJsonObject semantics;
      QVariantMap semanticsMap = GetSemantics();
      for (QVariantMap::iterator s = semanticsMap.begin(); s != semanticsMap.end(); ++s)
      {
        semantics[s.key()] = s.value().toString();
      }
      uom[UOM::KeySemantics] = semantics;

      QJsonObject unitsUI;
      QVariantMap unitsUIMap = GetUnitsUI();
      QVariantMap quantitiesMap = GetQuantities();
      for (QVariantMap::iterator u = unitsUIMap.begin(); u != unitsUIMap.end(); ++u)
      {
        if (!semanticsMap.contains(u.key()))
        {
          UOM::PhysicalQuantity q = (UOM::PhysicalQuantity)quantitiesMap.value(u.key(), (int)UOM::PhysicalQuantityNone).toInt();
          if (u.key() != "-1")
            unitsUI[u.key()] = UOM::KeysUnits.value({ q, u.value().toInt() }, "UOMAnyNone");
        }
      }
      uom[UOM::KeyUnitsUI] = unitsUI;
    }

#if defined(USE_UBJSON)
    rapidjson::Value rapidUOM = JsonUtils::Convert::QJsonToRapidJson(uom, allocator);
    json.AddMember(rapidjson::Value(UOM::KeyUOM, allocator), rapidUOM, allocator);
#endif
  }
}
#endif // !DISABLE_STANDARD_MODELS_SERIALIZING

//////////////////////////////////////////////////////////////////////
//// Properties
void StandardUOMModel::HandleCentralizedSemanticsUiUnitsChange(const QString& semantics, int units)
{
  QStringList affectedColumns = GetColumnIdsForSemantics(semantics);
  for (auto c = affectedColumns.begin(); c != affectedColumns.end(); ++c)
  {
    SetColumnUnitsUI(*c, units);
  }
}

void StandardUOMModel::NotifyUOMChanged(int columnIndex)
{
  QVariantMap queuedColumnUpdates = property("QueuedColumnUpdates").toMap();
  if (queuedColumnUpdates.value(QString::number(columnIndex)).toBool())
    return;

  queuedColumnUpdates[QString::number(columnIndex)] = true;
  setProperty("QueuedColumnUpdates", queuedColumnUpdates);

  QPointer<StandardUOMModel> instance = this;
  QTimer::singleShot(0, instance, [instance]()
  {
    if (instance)
    {
      QVariantMap queuedColumnUpdates = instance->property("QueuedColumnUpdates").toMap();
      for (auto u = queuedColumnUpdates.begin(); u != queuedColumnUpdates.end(); ++u)
      {
        if (u.value().toBool())
        {
          // Header data changed
          int column = u.key().toInt();
          instance->headerDataChanged(Qt::Horizontal, column, column);

          // Model data changed
          int rc = instance->rowCount();
          if (rc)
          {
            QVector<int> dataRoles;
            dataRoles.reserve(3);
            dataRoles << Qt::DisplayRole;
            dataRoles << Qt::WhatsThisRole;
            dataRoles << Qt::ToolTipRole;
            dataRoles << StandardItemModel::UserRoleUOMSemantics;
            dataRoles << StandardItemModel::UserRoleUOMPhysicalQuantity;
            dataRoles << StandardItemModel::UserRoleUOMUnitsUI;

            QModelIndex topLeft     = instance->index(0, column);
            QModelIndex bottomRight = instance->index(rc, column);
            
            instance->dataChanged(topLeft, bottomRight, dataRoles);
            instance->UnitsUiChanged(column, instance->GetColumnUnitsUI(column));
          }
        }
      }
      instance->setProperty("QueuedColumnUpdates", QVariantMap());
    }
  });
}

void StandardUOMModel::NotifyUOMChanged(const QString& columnKey)
{
  NotifyUOMChanged(GetColumnIndex(columnKey));
}

void StandardUOMModel::ConvertStoredData(const QModelIndex& parent, int column, UOM::PhysicalQuantity quantity, int unitsFrom, int unitsTo)
{
  blockSignals(true);
  for (int r = 0; r < rowCount(parent); ++r)
  {
    QModelIndex mi = index(r, column, parent);
    QVariant storedValue = data(mi, Qt::EditRole);
    QVariant convertedValue = Convert(storedValue, quantity, unitsFrom, unitsTo);
    setData(mi, convertedValue, Qt::EditRole);
  }
  blockSignals(false);
}

void StandardUOMModel::ConvertStoredDataToSI(const QModelIndex& parent, int column)
{
  QModelIndex columnIndex = index(0, column, parent);
  UOM::PhysicalQuantity quantity = UOM::PhysicalQuantityNone;
  if (columnIndex.isValid())
  {
    quantity = GetColumnQuantity(columnIndex);
  }
  else if (!parent.isValid())
  {
    quantity = GetColumnQuantity(GetStaticColumnKey(column));
  }
  
  if (quantity != UOM::PhysicalQuantityNone)
  {
    int storedUnitsSource = UOM::AnyNone; 
    if (columnIndex.isValid())
    {
      storedUnitsSource = GetColumnUnitsStored(columnIndex);
    }
    else if (!parent.isValid())
    {
      storedUnitsSource = GetColumnUnitsStored(GetStaticColumnKey(column));
    }
    int storedUnitsTarget = UOM::GetSIUnitsForQuantity(quantity);

    if (storedUnitsSource != storedUnitsTarget
     && storedUnitsSource != UOM::AnyNone
     && storedUnitsTarget != UOM::AnyNone)
    {
      ConvertStoredData(parent, column, quantity, storedUnitsSource, storedUnitsTarget);
    }
    SetColumnUnitsStored(parent, column, storedUnitsTarget);
  }
}

void StandardUOMModel::ConvertStoredDataToSI(const QModelIndex& root, const QString& columnKey)
{
  if (columnKey.isEmpty())
    return;

  // Convert
  if (root.isValid() && root.row() == 0)
  {
    QString dck = GetDynamicColumnKey(root);
    if (dck == columnKey)
    {
      UOM::PhysicalQuantity quantity = GetColumnQuantity(root);
      int storedUnitsSource = GetColumnUnitsStored(root);
      int storedUnitsTarget = UOM::GetSIUnitsForQuantity(quantity);
      if (storedUnitsSource != storedUnitsTarget
        && storedUnitsSource != UOM::AnyNone
        && storedUnitsTarget != UOM::AnyNone)
      {
        ConvertStoredData(parent(root), root.column(), quantity, storedUnitsSource, storedUnitsTarget);
      }
    }
  }
  
  // Reenter
  int rc = rowCount(root);
  int cc = columnCount(root);
  for (int r = 0; r < rc; ++r)
  {
    for (int c = 0; c < cc; ++c)
    {
      ConvertStoredDataToSI(index(r, c, root), columnKey);
    }
  }

  // Save units
  if (!root.isValid() && !columnKey.isEmpty())
  {
    QString semantics = GetColumnSemantics(columnKey);
    if (!semantics.isEmpty())
    {
      UOM::PhysicalQuantity quantity = UOMSettings::Get().GetQuantityForSemantics(semantics);
      if (quantity != UOM::PhysicalQuantityNone)
      {
        int storedUnitsTarget = UOM::GetSIUnitsForQuantity(quantity);
        if (storedUnitsTarget != UOM::AnyNone)
        {
          SetColumnUnitsStored(columnKey, storedUnitsTarget);
        }
      }
    }
  }
}

void StandardUOMModel::ConvertStoredDataToSI(const QString& columnKey)
{
  int columnIndex = GetColumnIndex(columnKey);
  if (columnIndex != -1)
  {
    ConvertStoredDataToSI(QModelIndex(), columnIndex);
  }
}

QSet<QString> StandardUOMModel::CollectAllUOMColumnKeys()
{
  QSet<QString> result;
  auto quantities = GetQuantities();
  for (auto it = quantities.begin(); it != quantities.end(); ++it)
    result.insert(it.key());

  auto semantics = GetSemantics();
  for (auto it = semantics.begin(); it != semantics.end(); ++it)
    result.insert(it.key());

  auto unitsStored = GetQuantities();
  for (auto it = unitsStored.begin(); it != unitsStored.end(); ++it)
    result.insert(it.key());

  return std::move(result);
}

QSet<QString> StandardUOMModel::CollectDynamicColumnKeys()
{
  QSet<QString> result;
  CollectDynamicColumnKeys(QModelIndex(), result);

  QSet<QString> uomKeys = CollectAllUOMColumnKeys();
  auto skk = GetStaticColumnKeys();
  uomKeys -= QSet<QString>(skk.begin(), skk.end());

  result += uomKeys;
  return std::move(result);
}

void StandardUOMModel::CollectDynamicColumnKeys(const QModelIndex& root, QSet<QString>& keys)
{
  QString dynamicColumnKey = GetDynamicColumnKey(root);
  if (!dynamicColumnKey.isEmpty())
    keys << dynamicColumnKey;

  // Reenter
  int rc = rowCount(root);
  int cc = columnCount(root);
  for (int r = 0; r < rc; ++r)
  {
    for (int c = 0; c < cc; ++c)
    {
      CollectDynamicColumnKeys(index(r, c, root), keys);
    }
  }
}

//////////////////////////////////////////////////////////////////////
//// Reimplemented
QVariant StandardUOMModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid())
  {
    //// Override display role & uom specific roles for views
    switch (role)
    {
    case StandardItemModel::UserRoleClipboardDataRoleId:
      {
        if (IndexHasConversion(index))
          return StandardItemModel::UserRoleClipboardDataRole;
        return StandardItemModel::data(index, role);
      }
      break;

    case StandardItemModel::UserRoleClipboardDataRole:
      {
        if (IndexHasUOM(index))
          return data(index, Qt::DisplayRole);
        return StandardItemModel::data(index, role);
      }
      break;

    case Qt::DisplayRole:
    case StandardItemModel::UserRoleUOMConvertToUI:
      {
        if (IndexHasUOM(index))
        {
          QVariant stored = data(index, Qt::EditRole);
          if (stored.canConvert(QMetaType::Double))
            return ConvertToUi(index, stored);
        }
        else if (role == StandardItemModel::UserRoleUOMConvertToUI)
        {
          return data(index, Qt::EditRole);
        }
      }
      break;

    //// UOM Roles
    case StandardItemModel::UserRoleUOMSemantics:
      {
        return GetColumnSemantics(index);
      }
      break;
    case StandardItemModel::UserRoleUOMPhysicalQuantity:
      {
        return GetColumnQuantity(index);
      }
      break;
    case StandardItemModel::UserRoleUOMUnitsStored:
      {
        return GetColumnUnitsStored(index);
      }
      break;
    case StandardItemModel::UserRoleUOMUnitsUI:
      {
        return GetColumnUnitsUI(index);
      }
      break;
    default:
      break;
    }
  }
  return StandardItemModel::data(index, role);
}

bool StandardUOMModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
  if (index.isValid())
  {
    switch (role)
    {
    case StandardItemModel::UserRoleUOMConvertFromUI:
      {
        if (IndexHasUOM(index))
        {
          return StandardItemModel::setData(index, ConvertToData(index, data), Qt::EditRole);
        }
        else
        {
          return StandardItemModel::setData(index, data, Qt::EditRole);
        }
      }
      break;
    case StandardItemModel::UserRoleClipboardDataRole:
      {
        if (IndexHasUOM(index))
          return StandardItemModel::setData(index, ConvertToData(index, data), role);
      }
      break;

    case StandardItemModel::UserRoleUOMUnitsUI:
      {
        QString key = GetColumnKey(index);
        if (!key.isEmpty())
        {
          SetColumnUnitsUI(key, data.toInt());
          return true;
        }
        return false;
      }
      break;
    default:
      break;
    }
  }
  return StandardItemModel::setData(index, data, role);
}

QVariant StandardUOMModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  // Override uom specific roles
  if (orientation == Qt::Horizontal)
  {
    switch (role)
    {
    case StandardItemModel::UserRoleClipboardDataRoleId:
      {
        if (IndexHasConversion(section))
          return StandardItemModel::UserRoleClipboardDataRole;
        return StandardItemModel::headerData(section, orientation, role);
      }
      break;

    case StandardItemModel::UserRoleUOMSemantics:
      {
        QString sematnics = GetColumnSemantics(section);
        if (!sematnics.isEmpty())
          return sematnics;
      }
      break;
    case StandardItemModel::UserRoleUOMPhysicalQuantity:
      {
        UOM::PhysicalQuantity quantity = GetColumnQuantity(section);
        if (quantity != UOM::PhysicalQuantityNone)
          return quantity;
      }
      break;
    case StandardItemModel::UserRoleUOMUnitsStored:
      {
        int units = GetColumnUnitsStored(section);
        if (units != UOM::AnyNone)
          return units;
      }
      break;
    case StandardItemModel::UserRoleUOMUnitsUI:
      {
        int units = GetColumnUnitsUI(section);
        if (units != UOM::AnyNone)
          return units;
      }
      break;
    default:
      break;
    }
  }
  return StandardItemModel::headerData(section, orientation, role);
}

bool StandardUOMModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
  switch (role)
  {
  case StandardItemModel::UserRoleUOMSemantics:
    {
      SetColumnSemantics(section, value.toString());
      return true;
    }
    break;
  case StandardItemModel::UserRoleUOMPhysicalQuantity:
    {
      SetColumnQuantity(section, (UOM::PhysicalQuantity)value.toInt());
      return true;
    }
    break;
  case StandardItemModel::UserRoleUOMUnitsStored:
    {
      SetColumnUnitsStored(section, value.toInt());
      return true;
    }
    break;
  case StandardItemModel::UserRoleUOMUnitsUI:
    {
      SetColumnUnitsUI(section, value.toInt());
      return true;
    }
    break;
  default:
    break;
  }
  return StandardItemModel::setHeaderData(section, orientation, value, role);
}
