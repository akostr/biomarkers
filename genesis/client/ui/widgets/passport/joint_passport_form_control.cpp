#include "joint_passport_form_control.h"
#include <logic/models/passport_combo_models.h>
#include "ui_joint_passport_form_control.h"
#include <QStandardItemModel>
#include <logic/notification.h>

JointPassportFormControl::JointPassportFormControl(QWidget* parent)
  : iTabOrderWidget(parent)
  , ui(new Ui::JointPassportFormControl())
{
  ui->setupUi(this);
  SetupUi();
}

JointPassportFormControl::~JointPassportFormControl()
{
  delete ui;
}

void JointPassportFormControl::FillField(const QStringList& fields)
{
  ui->fieldComboBox->addItems(fields);
}

QString JointPassportFormControl::SelectedField() const
{
  return ui->fieldComboBox->currentText();
}

void JointPassportFormControl::FillCluster(const QStringList& clusters)
{
  ui->wellClusterComboBox->addItems(clusters);
}

QString JointPassportFormControl::SelectedCluster() const
{
  return ui->wellComboBox->currentText();
}

void JointPassportFormControl::FillWells(const QStringList& wells)
{
  ui->wellComboBox->addItems(wells);
}

QString JointPassportFormControl::SelectedWell() const
{
  return ui->wellComboBox->currentText();
}

QString JointPassportFormControl::SelectedDepthType()
{
//  return ui->depthTypeSwitcher->tabText(ui->depthTypeSwitcher->currentIndex());
  return QString();
}

QDateTime JointPassportFormControl::SelectedDate() const
{
  return ui->dateTimeEdit->dateTime();
}

Passport::SampleGeoMainJoint JointPassportFormControl::Data() const
{
  Passport::SampleGeoMainJoint data;
  data.field = ui->fieldComboBox->currentText();
  data.fieldId = ui->fieldComboBox->currentData(PassportModelsManager::idRole).toInt();
  data.wellCluster = ui->wellClusterComboBox->currentText();
  data.wellClusterId = ui->wellClusterComboBox->currentData(PassportModelsManager::idRole).toInt();
  data.well = ui->wellComboBox->currentText();
  data.wellId = ui->wellComboBox->currentData(PassportModelsManager::idRole).toInt();
  data.depth = ui->depthSpinBox->value();
  data.depth_tvd = ui->depthTVDSpinBox->value();
//  data.depthType = ui->depthTypeSwitcher->currentIndex() == 0 ? Passport::MD : Passport::TVD;
  data.sampleDate = ui->dateTimeEdit->dateTime();
  data.passportId = property("passportId").toInt();
  return data;
}

void JointPassportFormControl::SetData(const Passport::SampleGeoMainJoint &data)
{
  ui->fieldComboBox->setCurrentIndex(ui->fieldComboBox->findData(data.fieldId, PassportModelsManager::idRole));
  ui->wellClusterComboBox->setCurrentIndex(ui->wellClusterComboBox->findData(data.wellClusterId, PassportModelsManager::idRole));
  ui->wellComboBox->setCurrentIndex(ui->wellComboBox->findData(data.wellId, PassportModelsManager::idRole));

  ui->depthSpinBox->setValue(data.depth);
  ui->depthTVDSpinBox->setValue(data.depth_tvd);
//  ui->depthTypeSwitcher->setCurrentIndex(data.depthType == Passport::MD ? 0 : 1);
  ui->dateTimeEdit->setDateTime(data.sampleDate);
  setProperty("passportId", data.passportId);
}

void JointPassportFormControl::SetModelsManager(QPointer<PassportModelsManager> manager)
{
  mModelsManager = manager;
  if(mModelsManager)
  {
    ui->fieldComboBox->setModel(mModelsManager->getFieldsModel());
//    ui->fieldComboBox->setEnabled(true);
  }
}

void JointPassportFormControl::SetupUi()
{
//  ui->depthTypeSwitcher->addTab("MD");
//  ui->depthTypeSwitcher->addTab("TVD");

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
//    qDebug() << "field changed";
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


QWidgetList JointPassportFormControl::tabOrderWidgets()
{
  return {ui->fieldComboBox,
          ui->wellClusterComboBox,
          ui->wellComboBox,
          ui->dateTimeEdit,
          ui->depthSpinBox,
          ui->depthTVDSpinBox,
//          ui->depthTypeSwitcher
  };
}
