#include "chemical_passport_form.h"
#include "logic/models/passport_combo_models.h"
#include "ui_chemical_passport_form.h"
#include <QStandardItemModel>
#include <genesis_style/style.h>

ChemicalPassportForm::ChemicalPassportForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ChemicalPassportForm)
{
  ui->setupUi(this);
  setupUi();
}

ChemicalPassportForm::~ChemicalPassportForm()
{
  delete ui;
}

Passport::SampleChemical ChemicalPassportForm::data()
{
  Passport::SampleChemical data;
  data.detectorType = ui->detectorCombo->currentData().value<Passport::DetectorType>();
  data.sampleVolume = ui->sampleInjVolSpin->value();
  data.columnLength = ui->columnLengthSpin->value();
  data.columnInnerDiameter = ui->innerDiameterSpin->value();
  data.columnStaticPhaseThickness = ui->staticPhaseThicknessSpin->value();
  data.columnStaticPhaseComposition = ui->staticPhaseCompositionLineEdit->text();
  data.columnMark = ui->columnMarkLineEdit->text();
  data.evaporatorTemperature = ui->evaporatorTempSpin->value();
  data.detectorTemperature = ui->detectorTempSpin->value();
  data.initialColumnTemperature = ui->initialColumnTempSpin->value();
  data.columnTemperatureRiseRate = ui->columnTepmRiseRateSpin->value();
  data.сolumnHoldingTimeAtMaximumTemperature = ui->columnHoldingTimeAtMaxTempSpin->value();
  data.carrierGasFlow = ui->gasCarrierFlowSpin->value();
  data.flowSeparation = ui->flowSeparationLineEdit->text();
  data.innerStandart = ui->innerStandardLineEdit->text();
  data.carrierGasType = ui->gasCarrierTypeCombo->currentData().value<Passport::CarrierGasType>();
  data.samplePreparationType = ui->samplePreparationTypeCombo->currentData().value<Passport::SamplePreparationType>();
  data.date = ui->analysisDateEdit->dateTime();
  data.additionalInfo = ui->additionalInfoLineEdit->text();
  return data;
}

void ChemicalPassportForm::setData(const Passport::SampleChemical &data)
{
  auto findAndSet = [](QComboBox* combo, QVariant data)
  {
    auto m = combo->model();
    for(int row = 0; row < m->rowCount(); row++)
    {
      auto ind = m->index(row, 0);
      if(data.toInt() == m->data(ind, Qt::UserRole).toInt())
      {
        combo->setCurrentIndex(row);
        return;
      }
    }
    combo->setCurrentIndex(0);
  };
  findAndSet(ui->detectorCombo, data.detectorType);
  ui->sampleInjVolSpin->setValue(data.sampleVolume);
  ui->columnLengthSpin->setValue(data.columnLength);
  ui->innerDiameterSpin->setValue(data.columnInnerDiameter);
  ui->staticPhaseThicknessSpin->setValue(data.columnStaticPhaseThickness);
  ui->staticPhaseCompositionLineEdit->setText(data.columnStaticPhaseComposition);
  ui->columnMarkLineEdit->setText(data.columnMark);
  ui->evaporatorTempSpin->setValue(data.evaporatorTemperature);
  ui->detectorTempSpin->setValue(data.detectorTemperature);
  ui->initialColumnTempSpin->setValue(data.initialColumnTemperature);
  ui->columnTepmRiseRateSpin->setValue(data.columnTemperatureRiseRate);
  ui->columnHoldingTimeAtMaxTempSpin->setValue(data.сolumnHoldingTimeAtMaximumTemperature);
  ui->gasCarrierFlowSpin->setValue(data.carrierGasFlow);
  ui->flowSeparationLineEdit->setText(data.flowSeparation);
  ui->innerStandardLineEdit->setText(data.innerStandart);
  ui->analysisDateEdit->setDateTime(data.date);
  ui->additionalInfoLineEdit->setText(data.additionalInfo);
  findAndSet(ui->gasCarrierTypeCombo, data.carrierGasType);
  findAndSet(ui->samplePreparationTypeCombo, data.samplePreparationType);
}

void ChemicalPassportForm::SetModelsManager(QPointer<PassportModelsManager> manager)
{
  mModelsManager = manager;
  if(mModelsManager)
  {
    ui->detectorCombo->setModel(mModelsManager->getDetectorsModel());
    ui->gasCarrierTypeCombo->setModel(mModelsManager->getCarrierGasesModel());
    ui->samplePreparationTypeCombo->setModel(mModelsManager->getSamplePreparingsModel());

    ui->detectorCombo->setEnabled(true);
    ui->gasCarrierTypeCombo->setEnabled(true);
    ui->samplePreparationTypeCombo->setEnabled(true);
  }
}

void ChemicalPassportForm::setupUi()
{
  //// toggle buttons
  connect(ui->columnToggleBtn, &QPushButton::toggled, this, qOverload<>(&ChemicalPassportForm::onButtonCheckStateChanged));
  connect(ui->tempToggleBtn, &QPushButton::toggled, this, qOverload<>(&ChemicalPassportForm::onButtonCheckStateChanged));
  connect(ui->additionalToggleBtn, &QPushButton::toggled, this, qOverload<>(&ChemicalPassportForm::onButtonCheckStateChanged));
  ui->columnToggleBtn->setProperty("toggle_widget", QVariant::fromValue(ui->columnWidget));
  ui->tempToggleBtn->setProperty("toggle_widget", QVariant::fromValue(ui->temperatureWidget));
  ui->additionalToggleBtn->setProperty("toggle_widget", QVariant::fromValue(ui->additionalWidget));
  onButtonCheckStateChanged(ui->columnToggleBtn);
  onButtonCheckStateChanged(ui->tempToggleBtn);
  onButtonCheckStateChanged(ui->additionalToggleBtn);

  //// combos
  ui->detectorCombo->setEnabled(false);
  ui->gasCarrierTypeCombo->setEnabled(false);
  ui->samplePreparationTypeCombo->setEnabled(false);

  auto polish = [](QWidget* w){w->style()->polish(w);};
  QString headingButtonsStyle = Style::ApplySASS(
  "QPushButton"
  "{"
    "padding: 10px 10px 10px 14px;"
    "text-align: left;"
    "background-color:   @dialogBoxButtonSecondaryBg;"
    "border-color:       @dialogBoxButtonSecondaryBg;"
    "color:              @dialogBoxButtonSecondaryFg;"
  "}"
  "QPushButton:pressed "
  "{"
    "background-color:   @dialogBoxButtonSecondaryBg;"
    "border-color:       @dialogBoxButtonSecondaryBg;"
    "color:              @dialogBoxButtonSecondaryFg;"
  "}");
  ui->columnToggleBtn->setStyleSheet(headingButtonsStyle);
  polish(ui->columnToggleBtn);
  ui->tempToggleBtn->setStyleSheet(headingButtonsStyle);
  polish(ui->tempToggleBtn);
  ui->additionalToggleBtn->setStyleSheet(headingButtonsStyle);
  polish(ui->additionalToggleBtn);

}

void ChemicalPassportForm::onButtonCheckStateChanged()
{
  auto btn = qobject_cast<QPushButton*>(sender());
  if(!btn)
    return;
  onButtonCheckStateChanged(btn);
};

void ChemicalPassportForm::onButtonCheckStateChanged(QPushButton* btn)
{
  auto toggleWidget = btn->property("toggle_widget").value<QWidget*>();
  if(btn->isChecked())
  {
    btn->setIcon(QIcon(":/resource/controls/collpse_up.png"));
    if(toggleWidget)
      toggleWidget->show();
  }
  else
  {
    btn->setIcon(QIcon(":/resource/controls/collpse_dn.png"));
    if(toggleWidget)
      toggleWidget->hide();
  }
  updateGeometry();
}

void ChemicalPassportForm::onFluidTypeChanged(Passport::FluidType fluidType)
{
  switch(fluidType)
  {
  case Passport::InvalidFluid:
  case Passport::FWater:
    if(!isHidden())
      hide();
    break;
  case Passport::FGas:
    if(isHidden())
      show();
    ui->samplePreparationTypeCombo->setEnabled(false);
    break;
  case Passport::FOil:
  case Passport::FCondensate:
  case Passport::FMixed:
  case Passport::FEmpty:
  case Passport::FBitumoid:
    if(isHidden())
      show();
    ui->samplePreparationTypeCombo->setEnabled(true);
    break;
  }
};
