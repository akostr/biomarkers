#include "component_control.h"
#include <logic/models/passport_combo_models.h>
#include "ui_component_control.h"
#include <QStandardItemModel>
#include <logic/notification.h>
#include <genesis_style/style.h>

PassportComponentControl::PassportComponentControl(QWidget* parent)
  : iTabOrderWidget(parent),
    ui(new Ui::PassportComponentControl())
{
  ui->setupUi(this);
  SetupUi();
}

PassportComponentControl::~PassportComponentControl()
{
  delete ui;
}

void PassportComponentControl::SetSummarySubstance(double summary)
{
  ui->summarySubstanceLabel->setText(tr("Sum:") + QString(" %1%").arg(summary));
}

double PassportComponentControl::Substance()
{
  return ui->substanceSpinBox->value();
}

void PassportComponentControl::FillLayer(const QStringList& layers)
{
  ui->layerComboBox->addItems(layers);
}

QString PassportComponentControl::SelectedLayer() const
{
  return ui->layerComboBox->currentText();
}

double PassportComponentControl::PercentSubstance() const
{
  return ui->substanceSpinBox->value();
}

void PassportComponentControl::FillWellCluster(const QStringList& clusters)
{
  ui->wellClusterComboBox->addItems(clusters);
}

QString PassportComponentControl::SelectedWellCluster() const
{
  return ui->wellClusterComboBox->currentText();
}

void PassportComponentControl::FillWell(const QStringList& wells)
{
  ui->wellComboBox->addItems(wells);
}

QString PassportComponentControl::SelectedWell() const
{
  return ui->wellComboBox->currentText();
}

double PassportComponentControl::Depth() const
{
  return ui->depthSpinBox->value();
}

QString PassportComponentControl::DepthType() const
{
  return QString();
//  return ui->depthTypeSwitcher->tabText(ui->depthTypeSwitcher->currentIndex());
}

QDateTime PassportComponentControl::DateTime() const
{
  return ui->dateEdit->dateTime();
}

QString PassportComponentControl::DataSource() const
{
//  return ui->sourceLineEdit->text();
  return QString();
}

void PassportComponentControl::FillAltitude(const QStringList& altitudes)
{
  ui->altitudeComboBox->addItems(altitudes);
}

QString PassportComponentControl::SelectedAltitude() const
{
  return ui->altitudeComboBox->currentText();
}

double PassportComponentControl::PerforationInterval() const
{
  return ui->perforationIntervalSpinBox->value();
}

QString PassportComponentControl::Thickness() const
{
  return ui->stratumLineEdit->text();
}

QString PassportComponentControl::FluidType() const
{
  return ui->fluidTypeLineEdit->text();
}

void PassportComponentControl::FillClusterStation(const QStringList& clusterStations)
{
  ui->clusterComboBox->addItems(clusterStations);
}

QString PassportComponentControl::SelectedClusterStation() const
{
  return ui->clusterComboBox->currentText();
}

void PassportComponentControl::FillBoosterStation(const QStringList &boosterStations)
{
  ui->boosterComboBox->addItems(boosterStations);
}

QString PassportComponentControl::SelectedBoosterStation() const
{
  return ui->boosterComboBox->currentText();
}

void PassportComponentControl::FillOilGasProdDep(const QStringList &oilGasProdDeps)
{
  ui->oilGasProdDepComboBox->addItems(oilGasProdDeps);
}

QString PassportComponentControl::SelectedOilGasProdDep() const
{
  return ui->oilGasProdDepComboBox->currentText();
}

void PassportComponentControl::SetData(const Passport::MixedSampleGeoComponent &data)
{
  ui->layerComboBox->setCurrentIndex(ui->layerComboBox->findData(data.layerId, PassportModelsManager::idRole));
  ui->substanceSpinBox->setValue(data.percentSubstance);
  ui->wellClusterComboBox->setCurrentIndex(ui->wellClusterComboBox->findData(data.wellClusterId, PassportModelsManager::idRole));
  ui->wellComboBox->setCurrentText(data.well);
  ui->depthSpinBox->setValue(data.depth);
  ui->depthTVDSpinBox->setValue(data.depth_tvd);
//  ui->depthTypeSwitcher->setCurrentIndex(data.depthType == Passport::MD ? 0 : 1);
  ui->dateEdit->setDateTime(data.sampleDate);
//  ui->sourceLineEdit->setText(data.source);
  ui->altitudeComboBox->setCurrentText(data.altitude);
  ui->perforationIntervalSpinBox->setValue(data.perforationInterval);
  ui->stratumLineEdit->setText(data.stratum);
  ui->fluidTypeLineEdit->setText(data.fluidType);
  ui->clusterComboBox->setCurrentText(data.clusterPumpingStation);
  ui->boosterComboBox->setCurrentText(data.boosterPumpingStation);
  ui->oilGasProdDepComboBox->setCurrentText(data.oilGasProductionStation);
  setProperty("passportId", data.passportId);
}

Passport::MixedSampleGeoComponent PassportComponentControl::Data() const
{
  Passport::MixedSampleGeoComponent data;
  bool ok = false;
  if(ui->layerComboBox->currentIndex() == -1 || ui->layerComboBox->currentText().isEmpty())
  {
    data.layer = "";
    data.layerId = -1;
  }
  else
  {
    data.layer = ui->layerComboBox->currentText();
    data.layerId = ui->layerComboBox->currentData(PassportModelsManager::idRole).toInt(&ok);
    if(!ok) data.layerId = -1;
  }
  data.percentSubstance = ui->substanceSpinBox->value();
  data.wellCluster = ui->wellClusterComboBox->currentText();
  data.wellClusterId = ui->wellClusterComboBox->currentData(PassportModelsManager::idRole).toInt(&ok);
  if(!ok) data.wellClusterId = -1;
  data.well = ui->wellComboBox->currentText();
  data.wellId = ui->wellComboBox->currentData(PassportModelsManager::idRole).toInt(&ok);
  if(!ok) data.wellId = -1;;
  data.depth = ui->depthSpinBox->value();
  data.depth_tvd = ui->depthTVDSpinBox->value();
//  data.depthType = Passport::MD;
  data.sampleDate = ui->dateEdit->dateTime();
//  data.source = ui->sourceLineEdit->text();
  data.altitude = ui->altitudeComboBox->currentText();
  data.perforationInterval = ui->perforationIntervalSpinBox->value();
  data.stratum = ui->stratumLineEdit->text();
  data.fluidType = ui->fluidTypeLineEdit->text();
  data.clusterPumpingStation = ui->clusterComboBox->currentText();
  data.boosterPumpingStation = ui->boosterComboBox->currentText();
  data.oilGasProductionStation = ui->oilGasProdDepComboBox->currentText();
  data.passportId = property("passportId").toInt();
  return data;
}

void PassportComponentControl::onFluidChanged(Passport::FluidType fluid)
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
    ui->oilGasProdDepComboBox->setEnabled(true);
    ui->boosterComboBox->setEnabled(false);
    ui->clusterComboBox->setEnabled(false);
    break;
  case Passport::FWater:
    ui->oilGasProdDepComboBox->setEnabled(false);
    ui->boosterComboBox->setEnabled(true);
    ui->clusterComboBox->setEnabled(true);
    break;
  }
}

void PassportComponentControl::onFieldChanged(int fieldId, QPointer<QStandardItemModel> fieldChildModelPtr)
{
  if(!mManager)
    return;

  ui->wellClusterComboBox->setModel(fieldChildModelPtr);
//  if(!ui->wellClusterComboBox->isEnabled())
//    ui->wellClusterComboBox->setEnabled(true);
}

void PassportComponentControl::Clear()
{
  ui->layerComboBox->clear();
  ui->substanceSpinBox->setValue(0);
  ui->wellClusterComboBox->clear();
  ui->wellComboBox->clear();
//  ui->depthSpinBox->setValue(0);
//  ui->depthTypeSwitcher->setCurrentIndex(0);
  ui->dateEdit->setDateTime(QDateTime::currentDateTime());
//  ui->sourceLineEdit->clear();
  ui->altitudeComboBox->clear();
  ui->perforationIntervalSpinBox->setValue(0);
  ui->stratumLineEdit->clear();
  ui->fluidTypeLineEdit->clear();
  ui->clusterComboBox->clear();
  ui->boosterComboBox->clear();
  ui->oilGasProdDepComboBox->clear();
}

void PassportComponentControl::setModelsManager(PassportModelsManager *manager)
{
  mManager = manager;
  if(mManager)
  {
    ui->layerComboBox->setCurrentIndex(-1);
    ui->layerComboBox->setModel(mManager->getLayersModel());
    ui->altitudeComboBox->setCurrentIndex(-1);
    ui->altitudeComboBox->setModel(mManager->getAltitudesModel());
    ui->clusterComboBox->setCurrentIndex(-1);
    ui->clusterComboBox->setModel(mManager->getClusterPumpingStationsModel());
    ui->boosterComboBox->setCurrentIndex(-1);
    ui->boosterComboBox->setModel(mManager->getBoosterPumpingStationsModel());
    ui->oilGasProdDepComboBox->setCurrentIndex(-1);
    ui->oilGasProdDepComboBox->setModel(mManager->getOilAndGasProductionStationsModel());

//    ui->layerComboBox->setEnabled(true);
    ui->altitudeComboBox->setEnabled(true);
    ui->clusterComboBox->setEnabled(true);
    ui->boosterComboBox->setEnabled(true);
    ui->oilGasProdDepComboBox->setEnabled(true);
  }
}

auto check = [](QComboBox* box)->bool
{
  if(box->currentIndex() == -1 || box->currentText().isEmpty())
    return false;
  auto idVariant = box->currentData(PassportModelsManager::idRole);
  if(!idVariant.isValid() || idVariant.isNull())
    return false;
  if(idVariant.toInt() == -1)
    return false;
  return true;
};
bool PassportComponentControl::IsDataValid()
{
  return check(ui->layerComboBox) && (ui->substanceSpinBox->value() >= 0.0 && ui->substanceSpinBox->value() <= 100.0);
}

void PassportComponentControl::HighlightInvalid()
{
  if(ui->layerComboBox->currentIndex() == -1 ||
     ui->layerComboBox->currentData(PassportModelsManager::ComboItemDataRole::idRole).toInt() == -1 ||
     ui->layerComboBox->currentText().isEmpty())
  {
    ui->layerComboBox->setStyleSheet("QComboBox {border-color: red;}");
    ui->layerComboBox->style()->unpolish(ui->layerComboBox);
    ui->layerComboBox->style()->polish(ui->layerComboBox);
    QPointer context{new QObject};
    QObject::connect(ui->layerComboBox, &QComboBox::currentIndexChanged, context, [this, context]()
    {
      if(ui->layerComboBox->currentIndex() != -1 &&
         ui->layerComboBox->currentData(PassportModelsManager::ComboItemDataRole::idRole).toInt() != -1 &&
         !ui->layerComboBox->currentText().isEmpty())
      {
        ui->layerComboBox->setStyleSheet("");
        ui->layerComboBox->style()->unpolish(ui->layerComboBox);
        ui->layerComboBox->style()->polish(ui->layerComboBox);
        context->deleteLater();
      }
    });
  }
  if(ui->substanceSpinBox->value() == 0)
  {
    ui->substanceSpinBox->setStyleSheet(Style::ApplySASS("QDoubleSpinBox {border-color: @errorColor;}"));
    ui->substanceSpinBox->style()->unpolish(ui->substanceSpinBox);
    ui->substanceSpinBox->style()->polish(ui->substanceSpinBox);
    QPointer context{new QObject};
    QObject::connect(ui->substanceSpinBox, &QDoubleSpinBox::valueChanged, context, [this, context](double value)
    {
      if(value != 0)
      {
        ui->substanceSpinBox->setStyleSheet("");
        ui->substanceSpinBox->style()->unpolish(ui->substanceSpinBox);
        ui->substanceSpinBox->style()->polish(ui->substanceSpinBox);
        context->deleteLater();
      }
    });
  }
}

void PassportComponentControl::SetupUi()
{
  ui->summarySubstanceLabel->setStyleSheet(Style::ApplySASS("QLabel {color: @brandColor;}"));
  ui->summarySubstanceLabel->style()->polish(ui->summarySubstanceLabel);
//  ui->depthTypeSwitcher->addTab("MD");
//  ui->depthTypeSwitcher->addTab("TVD")
      ;
//  ui->layerComboBox->setEnabled(false);
  ui->altitudeComboBox->setEnabled(false);
  ui->clusterComboBox->setEnabled(false);
  ui->boosterComboBox->setEnabled(false);
  ui->oilGasProdDepComboBox->setEnabled(false);
  ui->layerLabel->setText(ui->layerLabel->text() + Style::GetInputAlert());
  ui->substanceLabel->setText(ui->substanceLabel->text() + Style::GetInputAlert());

  auto wlist = PassportComponentControl::tabOrderWidgets();
  for(int i = 0; i < wlist.size(); i++)
  {
    auto w = wlist[i];
    QWidget* prev = nullptr;
    if(i != 0) prev = wlist[i-1];
    setTabOrder(prev, w);
  }

  connect(ui->wellClusterComboBox, &QComboBox::currentIndexChanged, this, [this](int ind)
  {
    if(ind == -1)
    {
      ui->wellComboBox->setCurrentIndex(-1);
      return;
    }
    auto v = ui->wellClusterComboBox->itemData(ind, PassportModelsManager::childModelRole);
    if(!v.isValid() || v.isNull() || v.value<QStandardItemModel*>() == nullptr)
    {
      Notification::NotifyError("Failed to set well model for well cluster", "Combo models error");
    }
    else
    {
      ui->wellComboBox->setModel(v.value<QStandardItemModel*>());
//      if(!ui->wellComboBox->isEnabled())
//        ui->wellComboBox->setEnabled(true);
    }
  });

  connect(ui->substanceSpinBox, &QDoubleSpinBox::valueChanged, this, &PassportComponentControl::substanceChanged);
}


QWidgetList PassportComponentControl::tabOrderWidgets()
{
  return {ui->layerComboBox,
          ui->substanceSpinBox,
          ui->wellClusterComboBox,
          ui->wellComboBox,
          ui->depthSpinBox,
          ui->depthTVDSpinBox,
//          ui->depthTypeSwitcher,
          ui->dateEdit,
//          ui->sourceLineEdit,
          ui->altitudeComboBox,
          ui->perforationIntervalSpinBox,
          ui->stratumLineEdit,
          ui->fluidTypeLineEdit,
          ui->clusterComboBox,
          ui->boosterComboBox,
          ui->oilGasProdDepComboBox};
}
