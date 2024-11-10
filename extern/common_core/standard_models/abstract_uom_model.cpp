#include "abstract_uom_model.h"
#include "uom_settings.h"
#include "standard_item_model.h"

#include <QTimer>

AbstractUOMModel::AbstractUOMModel(QObject* parent)
  : QAbstractItemModel(parent)
{
  UOMSettings* centralized = &UOMSettings::Get();
  connect(centralized, &UOMSettings::UnitsChanged, this, &AbstractUOMModel::HandleCentralizedSemanticsUiUnitsChange);
}

AbstractUOMModel::~AbstractUOMModel()
{
}

void AbstractUOMModel::SetColumnKey(int column, const QString& key)
{
  StandardItemModel::SetColumnKey(this, column, key);
}

QString AbstractUOMModel::GetColumnKey(const QModelIndex& modelIndex) const
{
  if (!modelIndex.isValid())
    return QString();
  QString dynamicKey = GetDynamicColumnKey(modelIndex);
  if (!dynamicKey.isEmpty())
    return dynamicKey;
  return GetStaticColumnKey(modelIndex);
}

int AbstractUOMModel::GetColumnIndex(const QString& key) const
{
  return StandardItemModel::GetColumnIndex(const_cast<AbstractUOMModel*>(this), key);
}

QString AbstractUOMModel::GetStaticColumnKey(const QModelIndex& modelIndex) const
{
  return std::move(GetStaticColumnKey(modelIndex.column()));
}

QString AbstractUOMModel::GetStaticColumnKey(int key) const
{
  return std::move(StandardItemModel::GetStaticColumnKey(const_cast<AbstractUOMModel*>(this), key));
}

QString AbstractUOMModel::GetDynamicColumnKey(const QModelIndex& /*modelIndex*/) const
{
  return QString();
}

void AbstractUOMModel::HandleCentralizedSemanticsUiUnitsChange(const QString& semantics, int units)
{
  QStringList affectedColumns = GetColumnIdsForSemantics(semantics);
  for (auto c = affectedColumns.begin(); c != affectedColumns.end(); ++c)
  {
    SetColumnUnitsUI(*c, units);
  }
}

void AbstractUOMModel::NotifyUOMChanged(int columnIndex)
{
  QVariantMap queuedColumnUpdates = property("QueuedColumnUpdates").toMap();
  if (queuedColumnUpdates.value(QString::number(columnIndex)).toBool())
    return;

  queuedColumnUpdates[QString::number(columnIndex)] = true;
  setProperty("QueuedColumnUpdates", queuedColumnUpdates);

  QPointer<AbstractUOMModel> instance = this;
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

            QModelIndex topLeft = instance->index(0, column);
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

void AbstractUOMModel::NotifyUOMChanged(const QString& columnKey)
{
  NotifyUOMChanged(StandardItemModel::GetColumnIndex(this, columnKey));
}

//////////////////////////////////////////////////////////////////////
//// Reimplemented
QVariant AbstractUOMModel::data(const QModelIndex& index, int role) const
{ 
  if (index.isValid())
  {
    switch (role)
    {
    case StandardItemModel::UserRoleClipboardDataRoleId:
      {
        if (IndexHasConversion(index))
          return StandardItemModel::UserRoleClipboardDataRole;
      }
      break;

    //// Override display role & uom specific roles for views
    case StandardItemModel::UserRoleClipboardDataRole:
    case Qt::DisplayRole:
      {
        QVariant stored = data(index, Qt::EditRole);
        if (stored.canConvert(QMetaType::Double))
          return ConvertToUi(index, stored);
      }
      break;

    //// Key role
    case StandardItemModel::UserRoleColumnKey:
      return GetColumnKey(index);

    //// UOM roles
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
  return QVariant();
}

bool AbstractUOMModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
  if (index.isValid())
  {
    switch (role)
    {
    case StandardItemModel::UserRoleClipboardDataRole:
      return setData(index, ConvertToData(index, data), Qt::EditRole);
    default:
      break;
    }
  }
  return false;
}

QVariant AbstractUOMModel::headerData(int section, Qt::Orientation orientation, int role) const
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
      }
      break;

    //// Key role
    case StandardItemModel::UserRoleColumnKey:
      return GetStaticColumnKey(section);

    //// UOM roles
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
    case StandardItemModel::UserRoleStringSizeHint:
      {
        return StandardItemModel::GetColumnInformation(const_cast<AbstractUOMModel*>(this), section).Scheme.TypicalValue;
      }
    default:
      break;
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

bool AbstractUOMModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
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
  return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

//// Notifications
void AbstractUOMModel::NotifyModelInformationChanged()
{
  emit ModelInformationChanged();
}
