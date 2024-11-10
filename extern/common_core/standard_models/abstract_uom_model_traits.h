#pragma once

#include <QAbstractItemModel>

#include "standard_item_model.h"
#include "standard_uom_model.h"

template<class T>
class AbstractUOMModelTraits
{
public:
  AbstractUOMModelTraits(T* model)
    : Model(model)
  {
  }

  //////////////////////////////////////////////////////////////////////
  //// Properties
  QVariantMap GetUOM() const
  {
    return StandardUOMModel::GetUOM(Model);
  }

  void SetUOM(const QVariantMap& uom)
  {
    return StandardUOMModel::SetUOM(Model, uom);
  }

  QVariantMap GetSemantics() const
  {
    return StandardUOMModel::GetSemantics(Model);
  }

  void SetSemantics(const QVariantMap& semantics)
  {
    return StandardUOMModel::SetSemantics(Model, semantics);
  }

  QVariantMap GetQuantities() const
  {
    return StandardUOMModel::GetQuantities(Model);
  }

  void SetQuantities(const QVariantMap& quantities)
  {
    return StandardUOMModel::SetQuantities(Model, quantities);
  }

  QVariantMap GetUnitsUI() const
  {
    return StandardUOMModel::GetUnitsUI(Model);
  }

  void SetUnitsUI(const QVariantMap& units)
  {
    return StandardUOMModel::SetUnitsUI(Model, units);
  }

  QVariantMap GetUnitsStored() const
  {
    return StandardUOMModel::GetUnitsStored(Model);
  }

  void SetUnitsStored(const QVariantMap& units)
  {
    return StandardUOMModel::SetUnitsStored(Model, units);
  }

  //////////////////////////////////////////////////////////////////////
  //// Semantics
  QString GetColumnSemantics(const QString& columnKey) const
  {
    return StandardUOMModel::GetColumnSemantics(Model, columnKey);
  }

  QString GetColumnSemantics(const int& columnIndex) const
  {
    return StandardUOMModel::GetColumnSemantics(Model, columnIndex);
  }

  QString GetColumnSemantics(const QModelIndex& modelIndex) const
  {
    return StandardUOMModel::GetColumnSemantics(Model, Model->GetColumnKey(modelIndex));
  }

  void SetColumnSemantics(const QString& columnKey, const QString& semantic)
  {
    return StandardUOMModel::SetColumnSemantics(Model, columnKey, semantic);
  }

  void SetColumnSemantics(const int& columnIndex, const QString& semantics)
  {
    return StandardUOMModel::SetColumnSemantics(Model, columnIndex, semantics);
  }

  //////////////////////////////////////////////////////////////////////
  //// Quantities
  UOM::PhysicalQuantity GetColumnQuantity(const QString& columnKey) const
  {
    return StandardUOMModel::GetColumnQuantity(Model, columnKey);
  }

  UOM::PhysicalQuantity GetColumnQuantity(const int& columnIndex) const
  {
    return StandardUOMModel::GetColumnQuantity(Model, columnIndex);
  }

  UOM::PhysicalQuantity GetColumnQuantity(const QModelIndex& modelIndex) const
  {
    return StandardUOMModel::GetColumnQuantity(Model, Model->GetColumnKey(modelIndex));
  }

  void SetColumnQuantity(const QString& columnKey, UOM::PhysicalQuantity quantity)
  {
    return StandardUOMModel::SetColumnQuantity(Model, columnKey, quantity);
  }

  void SetColumnQuantity(const int& columnIndex, UOM::PhysicalQuantity quantity)
  {
    return StandardUOMModel::SetColumnQuantity(Model, columnIndex, quantity);
  }

  //////////////////////////////////////////////////////////////////////
  //// Units in UI
  int GetColumnUnitsUI(const QString& columnKey) const
  {
    return StandardUOMModel::GetColumnUnitsUI(Model, columnKey);
  }

  int GetColumnUnitsUI(const int& columnIndex) const
  {
    return StandardUOMModel::GetColumnUnitsUI(Model, Model->GetStaticColumnKey(columnIndex));
  }

  int GetColumnUnitsUI(const QModelIndex& modelIndex) const
  {
    return StandardUOMModel::GetColumnUnitsUI(Model, Model->GetColumnKey(modelIndex));
  }

  void SetColumnUnitsUI(const QString& columnKey, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsUI(Model, columnKey, units);
  }

  void SetColumnUnitsUI(const int& columnIndex, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsUI(Model, columnIndex, units);
  }

  void SetColumnUnitsUI(const QModelIndex& modelIndex, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsUI(Model, GetColumnKey(modelIndex), units);
  }

  //// Units as stored
  int GetColumnUnitsStored(const QString& columnKey) const
  {
    return StandardUOMModel::GetColumnUnitsStored(Model, columnKey);
  }

  int GetColumnUnitsStored(const int& columnIndex) const
  {
    return StandardUOMModel::GetColumnUnitsStored(Model, columnIndex);
  }

  int GetColumnUnitsStored(const QModelIndex& modelIndex) const
  {
    return StandardUOMModel::GetColumnUnitsStored(Model, Model->GetColumnKey(modelIndex));
  }

  void SetColumnUnitsStored(const QString& columnKey, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsStored(Model, columnKey, units);
  }

  void SetColumnUnitsStored(const int& columnIndex, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsStored(Model, columnIndex, units);
  }

  void SetColumnUnitsStored(const QModelIndex& modelIndex, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsStored(Model, Model->GetColumnKey(modelIndex), units);
  }

  void SetColumnUnitsStored(const QModelIndex& parentIndex, const int& columnIndex, const int& units)
  {
    QModelIndex columnModelIndex = Model->index(0, columnIndex, parentIndex);
    if (columnModelIndex.isValid())
    {
      SetColumnUnitsStored(columnModelIndex, units); 
    }
    else if (!parentIndex.isValid())
    {
      SetColumnUnitsStored(Model->GetStaticColumnKey(columnIndex), units);
    }
  }

  QStringList GetColumnIdsForSemantics(const QString& centralizedSemantics)
  {
    QStringList columnIds;
    QVariantMap semantics = GetSemantics();
    for (QVariantMap::iterator s = semantics.begin(); s != semantics.end(); ++s)
    {
      if (s.value() == centralizedSemantics)
      {
        columnIds << s.key();
      }
    }
    return columnIds;
  }

  //////////////////////////////////////////////////////////////////////
  //// Converersions
  bool IndexHasConversion(const QString& columnKey) const
  {
    UOM::PhysicalQuantity quantity = GetColumnQuantity(columnKey);
    if (quantity == UOM::PhysicalQuantityNone)
      return false;

    int unitsStorage = GetColumnUnitsStored(columnKey);
    int unitsUi = GetColumnUnitsUI(columnKey);

    return unitsStorage != unitsUi;
  }

  bool IndexHasConversion(const int& index) const
  {
    return IndexHasConversion(Model->GetStaticColumnKey(index));
  }

  bool IndexHasConversion(const QModelIndex& index) const
  {
    return IndexHasConversion(Model->GetColumnKey(index));
  }

  bool IndexHasUOM(const QString& columnKey) const
  {
    UOM::PhysicalQuantity quantity = GetColumnQuantity(columnKey);
    return quantity != UOM::PhysicalQuantityNone;
  }

  bool IndexHasUOM(const int& index) const
  {
    return IndexHasUOM(Model->GetStaticColumnKey(index));
  }

  bool IndexHasUOM(const QModelIndex& index) const
  {
    return IndexHasUOM(Model->GetColumnKey(index));
  }

  QVariant ConvertToData(const QString& columnKey, const QVariant& data) const
  {
    if (!IndexHasConversion(columnKey))
      return data;

    UOM::PhysicalQuantity quantity = GetColumnQuantity(columnKey);
    int unitsStorage = GetColumnUnitsStored(columnKey);
    int unitsUi = GetColumnUnitsUI(columnKey);

    return Convert(data, quantity, unitsUi, unitsStorage);
  }

  QVariant ConvertToData(const int& index, const QVariant& data) const
  {
    return ConvertToData(Model->GetStaticColumnKey(index), data);
  }

  QVariant ConvertToData(const QModelIndex& index, const QVariant& data) const
  {
    return ConvertToData(Model->GetColumnKey(index), data);
  }

  double ConvertToData(const QString& columnKey, const double& data) const
  {
    return ConvertToData(columnKey, QVariant(data)).toDouble();
  }

  double ConvertToData(const int& index, const double& data) const
  {
    return ConvertToData(Model->GetStaticColumnKey(index), data);
  }

  double ConvertToData(const QModelIndex& index, const double& data) const
  {
    return ConvertToData(Model->GetColumnKey(index), data);
  }

  QVariant ConvertToUi(const QString& columnKey, const QVariant& data) const
  {
    if (!IndexHasConversion(columnKey))
      return data;

    UOM::PhysicalQuantity quantity = GetColumnQuantity(columnKey);
    int unitsStorage = GetColumnUnitsStored(columnKey);
    int unitsUi = GetColumnUnitsUI(columnKey);

    return Convert(data, quantity, unitsStorage, unitsUi);
  }

  QVariant ConvertToUi(const int& index, const QVariant& data) const
  {
    return ConvertToUi(Model->GetStaticColumnKey(index), data);
  }

  QVariant ConvertToUi(const QModelIndex& index, const QVariant& data) const
  {
    return ConvertToUi(Model->GetColumnKey(index), data);
  }

  double ConvertToUi(const QString& columnKey, const double& data) const
  {
    return ConvertToUi(columnKey, QVariant(data)).toDouble();
  }

  double ConvertToUi(const int& index, const double& data) const
  {
    return ConvertToUi(Model->GetStaticColumnKey(index), data);
  }

  double ConvertToUi(const QModelIndex& index, const double& data) const
  {
    return ConvertToUi(Model->GetColumnKey(index), data);
  }

  QVariant Convert(const QVariant& value, UOM::PhysicalQuantity quantity, const unsigned& sourceUOM, const unsigned& targetUOM) const
  {
    if (sourceUOM == UOM::AnyNone)
      return QVariant();

    if (targetUOM == UOM::AnyNone)
      return QVariant();

    return UOM::Convert(value.toDouble(), quantity, sourceUOM, targetUOM);
  }

private:
  T* Model;
};
