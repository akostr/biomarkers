
  
  //////////////////////////////////////////////////////////////////////
  //// Properties
  QVariantMap GetUOM() const
  {
    return StandardUOMModel::GetUOM(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this));
  }

  void SetUOM(const QVariantMap& uom)
  {
    return StandardUOMModel::SetUOM(this, uom);
  }

  QVariantMap GetSemantics() const
  {
    return StandardUOMModel::GetSemantics(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this));
  }

  void SetSemantics(const QVariantMap& semantics)
  {
    return StandardUOMModel::SetSemantics(this, semantics);
  }

  QVariantMap GetQuantities() const
  {
    return StandardUOMModel::GetQuantities(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this));
  }

  void SetQuantities(const QVariantMap& quantities)
  {
    return StandardUOMModel::SetQuantities(this, quantities);
  }

  QVariantMap GetUnitsUI() const
  {
    return StandardUOMModel::GetUnitsUI(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this));
  }

  void SetUnitsUI(const QVariantMap& units)
  {
    return StandardUOMModel::SetUnitsUI(this, units);
  }

  QVariantMap GetUnitsStored() const
  {
    return StandardUOMModel::GetUnitsStored(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this));
  }

  void SetUnitsStored(const QVariantMap& units)
  {
    return StandardUOMModel::SetUnitsStored(this, units);
  }

  //////////////////////////////////////////////////////////////////////
  //// Semantics
  QString GetColumnSemantics(const QString& columnKey) const
  {
    return StandardUOMModel::GetColumnSemantics(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), columnKey);
  }

  QString GetColumnSemantics(const int& columnIndex) const
  {
    return StandardUOMModel::GetColumnSemantics(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), columnIndex);
  }

  QString GetColumnSemantics(const QModelIndex& modelIndex) const
  {
    return StandardUOMModel::GetColumnSemantics(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), GetColumnKey(modelIndex));
  }

  void SetColumnSemantics(const QString& columnKey, const QString& semantic)
  {
    return StandardUOMModel::SetColumnSemantics(this, columnKey, semantic);
  }

  void SetColumnSemantics(const int& columnIndex, const QString& semantics)
  {
    return StandardUOMModel::SetColumnSemantics(this, columnIndex, semantics);
  }

  //////////////////////////////////////////////////////////////////////
  //// Quantities
  UOM::PhysicalQuantity GetColumnQuantity(const QString& columnKey) const
  {
    return StandardUOMModel::GetColumnQuantity(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), columnKey);
  }

  UOM::PhysicalQuantity GetColumnQuantity(const int& columnIndex) const
  {
    return StandardUOMModel::GetColumnQuantity(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), columnIndex);
  }

  UOM::PhysicalQuantity GetColumnQuantity(const QModelIndex& modelIndex) const
  {
    return StandardUOMModel::GetColumnQuantity(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), GetColumnKey(modelIndex));
  }

  void SetColumnQuantity(const QString& columnKey, UOM::PhysicalQuantity quantity)
  {
    return StandardUOMModel::SetColumnQuantity(this, columnKey, quantity);
  }

  void SetColumnQuantity(const int& columnIndex, UOM::PhysicalQuantity quantity)
  {
    return StandardUOMModel::SetColumnQuantity(this, columnIndex, quantity);
  }

  //////////////////////////////////////////////////////////////////////
  //// Units in UI
  int GetColumnUnitsUI(const QString& columnKey) const
  {
    return StandardUOMModel::GetColumnUnitsUI(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), columnKey);
  }

  int GetColumnUnitsUI(const int& columnIndex) const
  {
    return StandardUOMModel::GetColumnUnitsUI(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), GetStaticColumnKey(columnIndex));
  }

  int GetColumnUnitsUI(const QModelIndex& modelIndex) const
  {
    return StandardUOMModel::GetColumnUnitsUI(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), GetColumnKey(modelIndex));
  }

  void SetColumnUnitsUI(const QString& columnKey, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsUI(this, columnKey, units);
  }

  void SetColumnUnitsUI(const int& columnIndex, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsUI(this, columnIndex, units);
  }

  void SetColumnUnitsUI(const QModelIndex& modelIndex, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsUI(this, GetColumnKey(modelIndex), units);
  }

  //// Units as stored
  int GetColumnUnitsStored(const QString& columnKey) const
  {
    return StandardUOMModel::GetColumnUnitsStored(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), columnKey);
  }

  int GetColumnUnitsStored(const int& columnIndex) const
  {
    return StandardUOMModel::GetColumnUnitsStored(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), columnIndex);
  }

  int GetColumnUnitsStored(const QModelIndex& modelIndex) const
  {
    return StandardUOMModel::GetColumnUnitsStored(const_cast<QAbstractItemModel*>((const QAbstractItemModel*)this), GetColumnKey(modelIndex));
  }

  void SetColumnUnitsStored(const QString& columnKey, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsStored(this, columnKey, units);
  }

  void SetColumnUnitsStored(const int& columnIndex, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsStored(this, columnIndex, units);
  }

  void SetColumnUnitsStored(const QModelIndex& modelIndex, const int& units)
  {
    return StandardUOMModel::SetColumnUnitsStored(this, GetColumnKey(modelIndex), units);
  }

  void SetColumnUnitsStored(const QModelIndex& parentIndex, const int& columnIndex, const int& units)
  {
    QModelIndex columnModelIndex = index(0, columnIndex, parentIndex);
    if (columnModelIndex.isValid())
    {
      SetColumnUnitsStored(columnModelIndex, units); 
    }
    else if (!parentIndex.isValid())
    {
      SetColumnUnitsStored(GetStaticColumnKey(columnIndex), units);
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
    return IndexHasConversion(GetStaticColumnKey(index));
  }

  bool IndexHasConversion(const QModelIndex& index) const
  {
    return IndexHasConversion(GetColumnKey(index));
  }

  bool IndexHasUOM(const QString& columnKey) const
  {
    UOM::PhysicalQuantity quantity = GetColumnQuantity(columnKey);
    return quantity != UOM::PhysicalQuantityNone;
  }

  bool IndexHasUOM(const int& index) const
  {
    return IndexHasUOM(GetStaticColumnKey(index));
  }

  bool IndexHasUOM(const QModelIndex& index) const
  {
    return IndexHasUOM(GetColumnKey(index));
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
    return ConvertToData(GetStaticColumnKey(index), data);
  }

  QVariant ConvertToData(const QModelIndex& index, const QVariant& data) const
  {
    return ConvertToData(GetColumnKey(index), data);
  }

  double ConvertToData(const QString& columnKey, const double& data) const
  {
    return ConvertToData(columnKey, QVariant(data)).toDouble();
  }

  double ConvertToData(const int& index, const double& data) const
  {
    return ConvertToData(GetStaticColumnKey(index), data);
  }

  double ConvertToData(const QModelIndex& index, const double& data) const
  {
    return ConvertToData(GetColumnKey(index), data);
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
    return ConvertToUi(GetStaticColumnKey(index), data);
  }

  QVariant ConvertToUi(const QModelIndex& index, const QVariant& data) const
  {
    return ConvertToUi(GetColumnKey(index), data);
  }

  double ConvertToUi(const QString& columnKey, const double& data) const
  {
    return ConvertToUi(columnKey, QVariant(data)).toDouble();
  }

  double ConvertToUi(const int& index, const double& data) const
  {
    return ConvertToUi(GetStaticColumnKey(index), data);
  }

  double ConvertToUi(const QModelIndex& index, const double& data) const
  {
    return ConvertToUi(GetColumnKey(index), data);
  }

  QVariant Convert(const QVariant& value, UOM::PhysicalQuantity quantity, const unsigned& sourceUOM, const unsigned& targetUOM) const
  {
    if (sourceUOM == UOM::AnyNone)
      return QVariant();

    if (targetUOM == UOM::AnyNone)
      return QVariant();

    return UOM::Convert(value.toDouble(), quantity, sourceUOM, targetUOM);
  }
