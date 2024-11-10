#include "web_dialog_analysis_model_settings.h"

#include <genesis_style/style.h>

#include <QPushButton>
#include <QStyle>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>

using Buttons = QDialogButtonBox::StandardButton;

WebDialogAnalysisModelSettings::WebDialogAnalysisModelSettings(QWidget* parent)
  : Dialogs::Templates::Dialog(parent, Buttons::Ok | Buttons::Cancel)
{
  SetupUi();
}

void WebDialogAnalysisModelSettings::Accept()
{
  emit OptionsAccepted(GetOptions());
  WebDialog::Accept();
}

void WebDialogAnalysisModelSettings::AddEnabledOptions(const std::map<Option, bool>& opt)
{
  auto layout = getContent()->layout();

  auto checkNeccessaryParameters = new QLabel(tr("Check necessary parameters"));
  checkNeccessaryParameters->setStyleSheet(Style::Genesis::GetH3());
  layout->addWidget(checkNeccessaryParameters);
  for (auto& [option, checked] : opt)
  {
    if (option == Autoscale)
    {
      AutoScaleStatus = new QCheckBox(tr("Autoscale"), this);
      AutoScaleStatus->setChecked(checked);
      layout->addWidget(AutoScaleStatus);
    }

    if (option == FilterPKEnabled)
    {
      FilterCheckBox = new QCheckBox(tr("Show only PK with 95\%-99\% explained dispersion"));
      FilterCheckBox->setChecked(checked);
      layout->addWidget(FilterCheckBox);
    }

    if (option == Normalization)
    {
      NormStatus = new QCheckBox(tr("Normalilzation"), this);
      NormStatus->setChecked(checked);
      layout->addWidget(NormStatus);
    }
  }
}

void WebDialogAnalysisModelSettings::AddDisabledOptions(const std::map<Option, bool>& opt)
{
  auto layout = getContent()->layout();

  auto modelParameters = new QLabel(tr("Model parameters during setup"));
  modelParameters->setStyleSheet(Style::Genesis::GetH3());
  layout->addWidget(modelParameters);

  for (auto& [option, status] : opt)
  {
    switch (option)
    {
    case Autoscale:
    {
      AutoScaleStatus = new QCheckBox(tr("Autoscale"), this);
      AutoScaleStatus->setChecked(status);
      AutoScaleStatus->setDisabled(true);
      layout->addWidget(AutoScaleStatus);
      break;
    }
    case FilterPKEnabled:
    {
      FilterCheckBox = new QCheckBox(tr("Show only PK with 95\%-99\% explained dispersion"));
      FilterCheckBox->setChecked(status);
      FilterCheckBox->setDisabled(true);
      layout->addWidget(FilterCheckBox);
      break;
    }
    case Normalization:
    {
      NormStatus = new QCheckBox(tr("Normalilzation"), this);
      NormStatus->setChecked(status);
      NormStatus->setDisabled(true);
      layout->addWidget(NormStatus);
      break;
    }
    default:
      break;
    }
  }
}

WebDialogAnalysisModelSettings::Options WebDialogAnalysisModelSettings::GetOptions()
{
  Options opt;
  if (FilterCheckBox && FilterCheckBox->isChecked())
    opt.setFlag(FilterPKEnabled);

  if (AutoScaleStatus && AutoScaleStatus->isChecked())
    opt.setFlag(Autoscale);

  if (NormStatus && NormStatus->isChecked())
    opt.setFlag(Normalization);

  return opt;
}

void WebDialogAnalysisModelSettings::SetupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Parameters for constructing the PCA model");
  applySettings(s);

  ButtonBox->button(QDialogButtonBox::Ok)->setText(tr("Build a PCA model"));
}
