#pragma once
#ifndef PASSPORT_COMPONENT_CONTROL_H
#define PASSPORT_COMPONENT_CONTROL_H

#include <QDate>
#include "itab_order_list.h"
#include <logic/structures/passport_form_structures.h>

namespace Ui
{
  class PassportComponentControl;
}

class PassportModelsManager;
class QStandardItemModel;
class PassportComponentControl : public iTabOrderWidget
{
  Q_OBJECT

public:
  PassportComponentControl(QWidget* parent = nullptr);
  virtual ~PassportComponentControl();

  void SetSummarySubstance(double summary);
  double Substance();

  void FillLayer(const QStringList& layers);
  QString SelectedLayer() const;

  double PercentSubstance() const;

  void FillWellCluster(const QStringList& clusters);
  QString SelectedWellCluster() const;

  void FillWell(const QStringList& wells);
  QString SelectedWell() const;

  double Depth() const;

  QString DepthType() const;

  QDateTime DateTime() const;

  QString DataSource() const;

  void FillAltitude(const QStringList& altitudes);
  QString SelectedAltitude() const;

  double PerforationInterval() const;

  QString Thickness() const;
  QString FluidType() const;

  void FillClusterStation(const QStringList& clusterStations);
  QString SelectedClusterStation() const;

  void FillBoosterStation(const QStringList& boosterStations);
  QString SelectedBoosterStation() const;

  void FillOilGasProdDep(const QStringList& oilGasProdDeps);
  QString SelectedOilGasProdDep() const;

  void SetData(const Passport::MixedSampleGeoComponent &data);
  Passport::MixedSampleGeoComponent Data() const;

  void onFluidChanged(Passport::FluidType fluid);
  void onFieldChanged(int fieldId, QPointer<QStandardItemModel> fieldChildModelPtr);

  void Clear();

  void setModelsManager(PassportModelsManager* manager);
  bool IsDataValid();
  void HighlightInvalid();

signals:
  void substanceChanged(double newSubstance);

protected:
  Ui::PassportComponentControl* ui = nullptr;
  PassportModelsManager* mManager;

  void SetupUi();

  // iTabOrderList interface
public:
  QWidgetList tabOrderWidgets() override;
};
#endif
