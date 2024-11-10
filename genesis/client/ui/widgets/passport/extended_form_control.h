#pragma once
#ifndef PASSPORT_EXTENDED_FORM_CONTROL_H
#define PASSPORT_EXTENDED_FORM_CONTROL_H

#include "itab_order_list.h"
#include <logic/structures/passport_form_structures.h>
#include <QPointer>

namespace Ui
{
  class PassportExtendedFormControl;
}

class PassportModelsManager;
class PassportExtendedFormControl : public iTabOrderWidget
{
  Q_OBJECT

public:

  explicit PassportExtendedFormControl(QWidget* parent = nullptr);
  virtual ~PassportExtendedFormControl();

  QString DataSource() const;

  void FillAltitude(const QStringList& altitudes);
  QString SelectedAltitude() const;

  double PerforationInterval() const;

  QString Stratum() const;
  QString FluidType() const;

  void FillPumpStations(const QStringList& stations);
  QString SelectedPumpStation() const;

  void FillClusterStations(const QStringList& stations);
  QString SelectedClusterStation() const;

  void FillProdDepStations(const QStringList& stations);
  QString SelectedProdDepStation() const;

  void SetData(const Passport::SampleGeoExtended& data);
  Passport::SampleGeoExtended Data();

  void onFluidChanged(Passport::FluidType fluid);
  void Clear();
  void SetModelsManager(QPointer<PassportModelsManager> manager);

private:
  Ui::PassportExtendedFormControl* ui = nullptr;
  QPointer<PassportModelsManager> mModelsManager;

  void SetupUi();


  // iTabOrderList interface
public:
  QWidgetList tabOrderWidgets() override;
};
#endif
