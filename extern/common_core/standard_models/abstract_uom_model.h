#pragma once

#include <QAbstractItemModel>

#include "standard_uom_model.h"

class AbstractUOMModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  
#include "abstract_uom_model_traits.inc"

public:
  AbstractUOMModel(QObject* parent = nullptr);
  ~AbstractUOMModel();

public slots:
  //// Specific column key
  void            SetColumnKey(int column, const QString& key);
  virtual QString GetColumnKey(const QModelIndex& index) const;

  int             GetColumnIndex(const QString& key) const;

  virtual QString GetStaticColumnKey(const QModelIndex& modelIndex) const;
  virtual QString GetStaticColumnKey(int key) const;
  virtual QString GetDynamicColumnKey(const QModelIndex& modelIndex) const;

  //// Centralized Semantics change handler
  void HandleCentralizedSemanticsUiUnitsChange(const QString& semantics, int units);

  void NotifyUOMChanged(int columnIndex);
  void NotifyUOMChanged(const QString& columnKey);
  void NotifyModelInformationChanged();
  
public:
  //////////////////////////////////////////////////////////////////////
  //// Reimplemented
  virtual QVariant  data(const QModelIndex& index, int role) const override;
  virtual bool      setData(const QModelIndex& index, const QVariant& data, int role = Qt::EditRole) override;
  virtual QVariant  headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  virtual bool      setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

signals:
  void UnitsUiChanged(int columnIndex, int units);
  void ModelInformationChanged();
};
