#include "extended_form_control.h"
#include <logic/models/passport_combo_models.h>
#include "ui_extended_form_control.h"
#include <QStandardItemModel>
#include <logic/notification.h>

PassportExtendedFormControl::PassportExtendedFormControl(QWidget* parent)
  : iTabOrderWidget(parent),
    ui(new Ui::PassportExtendedFormControl())
{
  ui->setupUi(this);
  SetupUi();
}

PassportExtendedFormControl::~PassportExtendedFormControl()
{
  delete ui;
}

QString PassportExtendedFormControl::DataSource() const
{
  return ui->sourceLineEdit->text();
}

void PassportExtendedFormControl::FillAltitude(const QStringList& altitudes)
{
  ui->altitudeComboBox->addItems(altitudes);
}

QString PassportExtendedFormControl::SelectedAltitude() const
{
  return ui->altitudeComboBox->currentText();
}

double PassportExtendedFormControl::PerforationInterval() const
{
  return ui->perforationIntervalSpinBox->value();
}

QString PassportExtendedFormControl::Stratum() const
{
  return ui->stratumLineEdit->text();
}

QString PassportExtendedFormControl::FluidType() const
{
  return ui->fluidTypeLineEdit->text();
}

void PassportExtendedFormControl::FillPumpStations(const QStringList& stations)
{
  ui->pumpComboBox->addItems(stations);
}

QString PassportExtendedFormControl::SelectedPumpStation() const
{
  return ui->pumpComboBox->currentText();
}

void PassportExtendedFormControl::FillClusterStations(const QStringList& stations)
{
  ui->clusterComboBox->addItems(stations);
}

QString PassportExtendedFormControl::SelectedClusterStation() const
{
  return ui->clusterComboBox->currentText();
}

void PassportExtendedFormControl::FillProdDepStations(const QStringList &stations)
{
  ui->oilAndGasProdDepComboBox->addItems(stations);
}

QString PassportExtendedFormControl::SelectedProdDepStation() const
{
  return ui->oilAndGasProdDepComboBox->currentText();
}

void PassportExtendedFormControl::SetData(const Passport::SampleGeoExtended &data)
{
  ui->sourceLineEdit->setText(data.source);
  ui->altitudeComboBox->setCurrentText(data.altitude);
  ui->stratumLineEdit->setText(data.stratum);
  ui->fluidTypeLineEdit->setText(data.fluidType);
  ui->pumpComboBox->setCurrentText(data.boosterPumpingStation);
  ui->clusterComboBox->setCurrentText(data.clusterPumpingStation);
  ui->oilAndGasProdDepComboBox->setCurrentText(data.oilGasProductionStation);
  ui->perforationIntervalSpinBox->setValue(data.perforationInterval);
}

Passport::SampleGeoExtended PassportExtendedFormControl::Data()
{
  Passport::SampleGeoExtended data;
  data.source = ui->sourceLineEdit->text();
  data.altitude = ui->altitudeComboBox->currentText();
  data.stratum = ui->stratumLineEdit->text();
  data.fluidType = ui->fluidTypeLineEdit->text();
  data.boosterPumpingStation = ui->pumpComboBox->currentText();
  data.clusterPumpingStation = ui->clusterComboBox->currentText();
  data.oilGasProductionStation = ui->oilAndGasProdDepComboBox->currentText();
  data.perforationInterval = ui->perforationIntervalSpinBox->value();
  return data;
}

void PassportExtendedFormControl::onFluidChanged(Passport::FluidType fluid)
{
  switch(fluid)
  {
  case Passport::InvalidFluid:
  case Passport::FGas:
  case Passport::FOil:
  case Passport::FCondensate:
  case Passport::FMixed:
  case Passport::FBitumoid:
  case Passport::FEmpty:
    ui->oilAndGasProdDepComboBox->setEnabled(true);
    ui->pumpComboBox->setEnabled(false);
    ui->clusterComboBox->setEnabled(false);
    break;
  case Passport::FWater:
    ui->oilAndGasProdDepComboBox->setEnabled(false);
    ui->pumpComboBox->setEnabled(true);
    ui->clusterComboBox->setEnabled(true);
    break;
  }
}

void PassportExtendedFormControl::Clear()
{
  ui->sourceLineEdit->clear();
  ui->altitudeComboBox->clear();
  ui->stratumLineEdit->clear();
  ui->fluidTypeLineEdit->clear();
  ui->pumpComboBox->clear();
  ui->clusterComboBox->clear();
  ui->oilAndGasProdDepComboBox->clear();
}

void PassportExtendedFormControl::SetModelsManager(QPointer<PassportModelsManager> manager)
{
  mModelsManager = manager;
  if(mModelsManager)
  {
    ui->altitudeComboBox->setModel(mModelsManager->getAltitudesModel());
    ui->clusterComboBox->setModel(mModelsManager->getClusterPumpingStationsModel());
    ui->pumpComboBox->setModel(mModelsManager->getBoosterPumpingStationsModel());
    ui->oilAndGasProdDepComboBox->setModel(mModelsManager->getOilAndGasProductionStationsModel());

    ui->altitudeComboBox->setEnabled(true);
    ui->clusterComboBox->setEnabled(true);
    ui->pumpComboBox->setEnabled(true);
    ui->oilAndGasProdDepComboBox->setEnabled(false);
  }
}

void PassportExtendedFormControl::SetupUi()
{
  ui->altitudeComboBox->setEnabled(false);
  ui->clusterComboBox->setEnabled(false);
  ui->pumpComboBox->setEnabled(false);
  ui->oilAndGasProdDepComboBox->setEnabled(false);
}


QWidgetList PassportExtendedFormControl::tabOrderWidgets()
{
  return {ui->sourceLineEdit,
          ui->altitudeComboBox,
          ui->perforationIntervalSpinBox,
          ui->stratumLineEdit,
          ui->fluidTypeLineEdit,
          ui->pumpComboBox,
          ui->clusterComboBox,
          ui->oilAndGasProdDepComboBox};
}
