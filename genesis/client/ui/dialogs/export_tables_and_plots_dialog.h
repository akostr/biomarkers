#pragma once

#include "ui/dialogs/templates/dialog.h"
#include <QStandardItemModel>

namespace Ui {
  class ExportTablesAndPlotsDialog;
}

class ExportTreeModel : public QStandardItemModel
{
public:
  ExportTreeModel(QObject* parent = nullptr);
  ~ExportTreeModel();

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
};

using ItemSelected = QVector<QPair<QString, Qt::CheckState>>;

class ExportTablesAndPlotsDialog : public Dialogs::Templates::Dialog
{
  Q_OBJECT
public:
  ExportTablesAndPlotsDialog(const ItemSelected& tables, const ItemSelected& plots,
    QWidget* parent = nullptr);
  ~ExportTablesAndPlotsDialog();

  ItemSelected Tables() const;
  ItemSelected Plots() const;

private:
  void setupUi();

private:
  Ui::ExportTablesAndPlotsDialog* ui;
  QWidget* mContent;
  ExportTreeModel* mModel = nullptr;

  ItemSelected mTables;
  ItemSelected mPlots;
};
