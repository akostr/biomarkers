#include "form_control.h"
#include <logic/models/passport_combo_models.h>
#include "ui_form_control.h"
#include <QStandardItemModel>
#include <logic/notification.h>

PassportFormControl::PassportFormControl(QWidget* parent)
  : iTabOrderWidget(parent),
    ui(new Ui::PassportFormControl())
{
  ui->setupUi(this);
  SetupUi();
}

PassportFormControl::~PassportFormControl()
{
  delete ui;
}

void PassportFormControl::FillField(const QStringList& fields)
{
  ui->fieldComboBox->addItems(fields);
}

QString PassportFormControl::SelectedField() const
{
  return ui->fieldComboBox->currentText();
}

void PassportFormControl::FillCluster(const QStringList& clusters)
{
  ui->wellClusterComboBox->addItems(clusters);
}

QString PassportFormControl::SelectedCluster() const
{
  return ui->wellComboBox->currentText();
}

void PassportFormControl::FillWells(const QStringList& wells)
{
  ui->wellComboBox->addItems(wells);
}

QString PassportFormControl::SelectedWell() const
{
  return ui->wellComboBox->currentText();
}

void PassportFormControl::FillLayers(const QStringList& layers)
{
  ui->layerComboBox->addItems(layers);
}

QString PassportFormControl::SelectedLayer() const
{
  return ui->layerComboBox->currentText();
}

QString PassportFormControl::SelectedDepthType()
{
  return QString();
//  return ui->depthTypeSwitcher->tabText(ui->depthTypeSwitcher->currentIndex());
}

QDateTime PassportFormControl::SelectedDate() const
{
  return ui->dateEdit->dateTime();
}

Passport::SampleGeoMainNonMixed PassportFormControl::Data()
{
  Passport::SampleGeoMainNonMixed data;
  data.field = ui->fieldComboBox->currentText();
  data.fieldId = ui->fieldComboBox->currentData(PassportModelsManager::idRole).toInt();
  data.wellCluster = ui->wellClusterComboBox->currentText();
  data.wellClusterId = ui->wellClusterComboBox->currentData(PassportModelsManager::idRole).toInt();
  data.well = ui->wellComboBox->currentText();
  data.wellId = ui->wellComboBox->currentData(PassportModelsManager::idRole).toInt();
  data.depth = ui->depthSpinBox->value();
  data.depth_tvd = ui->depthTVDSpinBox->value();
//  data.depthType = Passport::MD;
  data.sampleDate = ui->dateEdit->dateTime();
  data.layer = ui->layerComboBox->currentText();
  data.layerId = ui->layerComboBox->currentData(PassportModelsManager::idRole).toInt();
  data.passportId = property("passportId").toInt();
  return data;
}

void PassportFormControl::SetData(const Passport::SampleGeoMainNonMixed &data)
{
  ui->fieldComboBox->setCurrentIndex(ui->fieldComboBox->findData(data.fieldId, PassportModelsManager::idRole));
  ui->wellClusterComboBox->setCurrentIndex(ui->wellClusterComboBox->findData(data.wellClusterId, PassportModelsManager::idRole));
  ui->wellComboBox->setCurrentIndex(ui->wellComboBox->findData(data.wellId, PassportModelsManager::idRole));
  ui->depthSpinBox->setValue(data.depth);
  ui->depthTVDSpinBox->setValue(data.depth_tvd);
//  ui->depthTypeSwitcher->setCurrentIndex(data.depthType == Passport::MD ? 0 : 1);
  ui->dateEdit->setDateTime(data.sampleDate);
  ui->layerComboBox->setCurrentIndex(ui->layerComboBox->findData(data.layerId, PassportModelsManager::idRole));
  setProperty("passportId", data.passportId);
}

void PassportFormControl::Clear()
{
  ui->fieldComboBox->setCurrentIndex(-1);
//  ui->wellClusterComboBox->clear();
//  ui->wellComboBox->clear();
  ui->depthSpinBox->setValue(0);
  ui->depthTVDSpinBox->setValue(0);
//  ui->depthTypeSwitcher->setCurrentIndex(0);
  ui->dateEdit->setDateTime(QDateTime::currentDateTime());
}

void PassportFormControl::SetModelsManager(QPointer<PassportModelsManager> manager)
{
  mModelsManager = manager;
  if(mModelsManager)
  {
    ui->layerComboBox->setModel(mModelsManager->getLayersModel());
    ui->fieldComboBox->setModel(mModelsManager->getFieldsModel());
//    ui->layerComboBox->setEnabled(true);
//    ui->fieldComboBox->setEnabled(true);
  }
}

void PassportFormControl::SetupUi()
{
//  ui->depthTypeSwitcher->addTab(tr("MD"));
//  ui->depthTypeSwitcher->addTab(tr("TVD"));
//  ui->layerComboBox->setEnabled(false);
//  ui->fieldComboBox->setEnabled(false);
//  ui->wellClusterComboBox->setEnabled(false);
//  ui->wellComboBox->setEnabled(false);

  connect(ui->fieldComboBox, &QComboBox::currentIndexChanged, this, [this](int ind)
  {
    if(ind == -1)
    {
      ui->wellClusterComboBox->setCurrentIndex(-1);
      ui->wellComboBox->setCurrentIndex(-1);
      return;
    }
    auto v = ui->fieldComboBox->itemData(ind, PassportModelsManager::childModelRole);
    if(!v.isValid() || v.isNull() || v.value<QStandardItemModel*>() == nullptr)
    {
      Notification::NotifyError("Failed to set well model for well cluster", "Combo models error");
    }
    else
    {
      ui->wellClusterComboBox->setModel(v.value<QStandardItemModel*>());
      //      if(!ui->wellClusterComboBox->isEnabled())
      //        ui->wellClusterComboBox->setEnabled(true);
    }
  });
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
      Notification::NotifyError("Failed to set well model for well", "Combo models error");
    }
    else
    {
      ui->wellComboBox->setModel(v.value<QStandardItemModel*>());
//      if(!ui->wellComboBox->isEnabled())
//        ui->wellComboBox->setEnabled(true);
    }
  });
}


QWidgetList PassportFormControl::tabOrderWidgets()
{
  return {ui->fieldComboBox,
          ui->wellClusterComboBox,
          ui->wellComboBox,
          ui->layerComboBox,
          ui->depthSpinBox,
          ui->depthTVDSpinBox,
//          ui->depthTypeSwitcher,
          ui->dateEdit};
}
