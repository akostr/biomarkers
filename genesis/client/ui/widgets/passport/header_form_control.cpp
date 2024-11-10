#include "header_form_control.h"
#include <logic/models/passport_combo_models.h>
#include "ui_header_form_control.h"
#include <QStandardItemModel>

PassportHeaderFormControl::PassportHeaderFormControl(QWidget* parent)
  : iTabOrderWidget(parent),
    ui(new Ui::PassportHeaderFormControl())
{
  ui->setupUi(this);
  SetupUi();
  ConnectSignals();
}

PassportHeaderFormControl::~PassportHeaderFormControl()
{
  delete ui;
}

void PassportHeaderFormControl::SetComment(const QString& comment)
{
  ui->commentLineEdit->setText(comment);
}

QString PassportHeaderFormControl::Comment() const
{
  return ui->commentLineEdit->text();
}

void PassportHeaderFormControl::FillSamples(const QStringList& samples)
{
  ui->sampleComboBox->addItems(samples);
}

QString PassportHeaderFormControl::SelectedSample() const
{
  return ui->sampleComboBox->currentText();
}

void PassportHeaderFormControl::FillFluids(const QStringList& fluids)
{
  ui->fluidComboBox->addItems(fluids);
}

QString PassportHeaderFormControl::SelectedFluid() const
{
  return ui->fluidComboBox->currentText();
}

void PassportHeaderFormControl::SetData(const Passport::SampleHeaderData &data)
{
  auto findAndSet = [](QComboBox* combo, int data)
  {
    for(int i = 0; i < combo->count(); i++)
    {
      if(combo->itemData(i) == data)
      {
        combo->setCurrentIndex(i);
        return;
      }
    }
    combo->setCurrentIndex(-1);
  };
  if(ui->sampleComboBox->currentData() == data.sampleType)
    emit SampleTypeChanged((Passport::SampleType)ui->sampleComboBox->currentData().toInt());
  else
    findAndSet(ui->sampleComboBox, data.sampleType);
  if(ui->fluidComboBox->currentData() == data.fluidType)
    emit FluidChanged((Passport::FluidType)ui->fluidComboBox->currentData().toInt());
  else
    findAndSet(ui->fluidComboBox, data.fluidType);
  ui->commentLineEdit->setText(data.comment);
}

Passport::SampleHeaderData PassportHeaderFormControl::Data()
{
  Passport::SampleHeaderData data;
  data.sampleType = (Passport::SampleType) ui->sampleComboBox->currentData().toInt();
  data.fluidType = (Passport::FluidType) ui->fluidComboBox->currentData().toInt();
  data.comment = ui->commentLineEdit->text();
  return data;
}

void PassportHeaderFormControl::SetModelsManager(QPointer<PassportModelsManager> manager)
{
  mModelsManager = manager;
  if(mModelsManager)
  {
    ui->fluidComboBox->setModel(mModelsManager->getFluidTypesModel());
    ui->sampleComboBox->setModel(mModelsManager->getSampleTypesModel());

    ui->fluidComboBox->setEnabled(true);
    ui->sampleComboBox->setEnabled(true);
  }
}

void PassportHeaderFormControl::SetupUi()
{
  ui->fluidComboBox->setEnabled(false);
  ui->sampleComboBox->setEnabled(false);
}

void PassportHeaderFormControl::ConnectSignals()
{
  connect(ui->fluidComboBox, &QComboBox::currentIndexChanged, this, [this](int index)
  {
    if(index == -1)
    {
      if(!ui->fluidComboBox->isEnabled())
      {
        emit FluidChanged(Passport::FEmpty);
      }
      else
      {
        emit FluidChanged(Passport::InvalidFluid);
      }
    }
    else
      emit FluidChanged((Passport::FluidType)ui->fluidComboBox->currentData().toInt());
  });
  connect(ui->sampleComboBox, &QComboBox::currentIndexChanged, this, [this](int index)
  {
//    qDebug() << (Passport::SampleType)ui->sampleComboBox->currentData().toInt();
    if(index == -1)
      emit SampleTypeChanged(Passport::InvalidSampleType);
    else
    {
      if(index == 3)
      {
        ui->fluidComboBox->setDisabled(true);
        ui->fluidComboBox->setToolTip(tr("There is no fluid for a blank sample"));
        ui->fluidComboBox->setCurrentIndex(-1);
      }
      else
      {
        if(!ui->fluidComboBox->isEnabled())
        {
          ui->fluidComboBox->setDisabled(false);
          ui->fluidComboBox->setToolTip(tr(""));
          ui->fluidComboBox->setCurrentIndex(0);
        }
      }
      emit SampleTypeChanged((Passport::SampleType)ui->sampleComboBox->currentData().toInt());
    }
  });
}


QWidgetList PassportHeaderFormControl::tabOrderWidgets()
{
  return {ui->sampleComboBox, ui->fluidComboBox, ui->commentLineEdit};
}
