#include "mcr_component_settings.h"

#include "genesis_style/style.h"
#include "ui_mcr_component_settings.h"

#include <QStringListModel>

namespace Widgets
{
  MCRComponentSettings::MCRComponentSettings(QWidget* parent)
    : QWidget(parent)
  {
    WidgetUi = new Ui::MCRComponentSettings();
    WidgetUi->setupUi(this);
    WidgetUi->SampleComboBox->setModel(new QStringListModel());
    WidgetUi->label_star->setStyleSheet(Style::ApplySASS("QLabel {color: @textColorInputAlert;}"));
  }

  MCRComponentSettings::~MCRComponentSettings()
  {
    delete WidgetUi;
  }

  void MCRComponentSettings::SetNonNegativeConcentration(Qt::CheckState state)
  {
    WidgetUi->NonNegativeConcentrationCheckBox->setCheckState(state);
  }

  void MCRComponentSettings::SetNonNegativeSpectral(Qt::CheckState state)
  {
    WidgetUi->NonNegativeSpectral->setCheckState(state);
  }

  void MCRComponentSettings::SetFixed(Qt::CheckState state)
  {
    WidgetUi->FixCheckBox->setCheckState(state);
  }

  void MCRComponentSettings::SetSampleList(const QStringList& list)
  {
    if (const auto model = qobject_cast<QStringListModel*>(WidgetUi->SampleComboBox->model()))
    {
      model->setStringList(list);
    }
  }

  void MCRComponentSettings::SetSampleListVisible(bool isVisible)
  {
    WidgetUi->SampleComboBox->setVisible(isVisible);
  }

  bool MCRComponentSettings::IsNonNegativeConcentration() const
  {
    return WidgetUi->NonNegativeConcentrationCheckBox->checkState() == Qt::Checked;
  }

  bool MCRComponentSettings::IsNonNegativeSpectral() const
  {
    return WidgetUi->NonNegativeSpectral->checkState() == Qt::Checked;
  }

  bool MCRComponentSettings::IsFixed() const
  {
    return WidgetUi->FixCheckBox->checkState() == Qt::Checked;
  }

  QString MCRComponentSettings::GetSelectedSample() const
  {
    return WidgetUi->SampleComboBox->currentText();
  }

  void MCRComponentSettings::SetSelectedSample(const QString& sampleName)
  {
    const auto index = WidgetUi->SampleComboBox->findText(sampleName);
    WidgetUi->SampleComboBox->setCurrentIndex(index);
  }

  void MCRComponentSettings::SetLabelText(const QString& componentName)
  {
    WidgetUi->ComponentNumbeLabel->setText(componentName);
  }

  void MCRComponentSettings::SetStarVisible(bool visible)
  {
    WidgetUi->label_star->setVisible(visible);
  }
}