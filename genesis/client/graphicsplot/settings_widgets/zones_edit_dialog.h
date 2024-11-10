#pragma once

#include <QDialog>
#include <QPointer>
#include <QStandardItemModel>

#ifndef DISABLE_STANDARD_VIEWS

namespace Ui {
  class ZonesEditDialog;
}

class GraphicsPlotExtended;
class GPZonesRect;
class QStandardItem;

class ZonesTableModel : public QStandardItemModel
{
  Q_OBJECT

public:
  enum Columns
  {
    ColumnName,
    ColumnPosLeft,
    ColumnPosRight,
    ColumnVisible,
    ColumnColor
  };

  explicit ZonesTableModel(QObject *parent, const QString& dateTimeFormat);

  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole);

protected:
  QString DateTimeFormat;
};

class ZonesEditDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZonesEditDialog(GraphicsPlotExtended *parent, QPointer<GPZonesRect> zonesRect);
  ~ZonesEditDialog();

public slots:
  void accept();

protected:
  void SetupUi();
  void UpdateUi();
  void MakeConnects();
  QVector<double> UpdateZonesLines();

protected slots:
  void Apply();
  void OnColorClicked(QModelIndex index);
  void OnInsertRows(const QModelIndex &parent, int first, int last);
  void OnRemoveRows(const QModelIndex &parent, int first, int last);
  void SetZonesPos(const QVector<double>& positions);

private:
  Ui::ZonesEditDialog* Ui;
  GraphicsPlotExtended* ParentPlot;

  QPointer<GPZonesRect> ZonesRect;
  ZonesTableModel* ZonesModel;
};

#endif // ~!DISABLE_STANDARD_VIEWS
