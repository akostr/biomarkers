#include "web_dialog_pls_concentration_settings.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QComboBox>

#include <genesis_style/style.h>
#include <logic/notification.h>

WebDialogPLSConcentrationSettings::WebDialogPLSConcentrationSettings(QWidget *parent)
  : Dialogs::Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
{
  SetupUi();
}

WebDialogPLSConcentrationSettings::ConcentrationGatheringMethod WebDialogPLSConcentrationSettings::getConcentrationGatheringMethod()
{
  if(ManualRadio->isChecked())
    return manual;
  if(AutoFromLayers->isChecked())
    return autoFromLayers;
  if(AutoRandomly->isChecked())
    return autoRandomly;
  return none;
}

bool WebDialogPLSConcentrationSettings::isManual()
{
  return ManualRadio->isChecked();
}

void WebDialogPLSConcentrationSettings::FillLayers(const QStringList& layerNames)
{
  Layers->clear();
  Layers->addItems(layerNames);
}

QString WebDialogPLSConcentrationSettings::SelectedLayer() const
{
  return Layers->currentText();
}

void WebDialogPLSConcentrationSettings::SetupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Concentration data");
  applySettings(s);

  auto body = new QWidget(Content);
  getContent()->layout()->addWidget(body);

  auto bodyLayout = new QVBoxLayout();
  body->setLayout(bodyLayout);

  //// Content
  {
    //// Input
    {
      //// Title
      {
        QLabel* caption = new QLabel(tr("Pick data input method"), body);
        caption->setStyleSheet(Style::ApplySASS("QLabel {font: @defaultFontBold; color: @textColor;}"));

        bodyLayout->addWidget(caption);
        bodyLayout->addSpacing(Style::Scale(4));
      }

      //// Content
      {
        ManualRadio = new QRadioButton(tr("Manually"));

        AutoFromLayers   = new QRadioButton(tr("Auto by layer concentration"));
        Layers = new QComboBox(this);
        Layers->setVisible(false);
        connect(AutoFromLayers, &QRadioButton::toggled, this,
          [&](bool checked) { Layers->setVisible(checked); });
        AutoRandomly   = new QRadioButton(tr("Auto by randomly from 5 to 95"));
        ManualRadio->setChecked(true);

        bodyLayout->addWidget(ManualRadio);

        bodyLayout->addSpacing(Style::Scale(4));
        bodyLayout->addWidget(AutoFromLayers);
        bodyLayout->addWidget(Layers);

        bodyLayout->addSpacing(Style::Scale(4));
        bodyLayout->addWidget(AutoRandomly);
      }

      bodyLayout->addSpacing(Style::Scale(50));
    }
    auto okBtn = ButtonBox->button(QDialogButtonBox::Ok);
    okBtn->setText(tr("To PLS Model setup"));
  }
}
