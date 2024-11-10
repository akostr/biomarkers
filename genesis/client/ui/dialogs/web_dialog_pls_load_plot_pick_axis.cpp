#include "web_dialog_pls_load_plot_pick_axis.h"

#include "ui_web_dialog_pls_load_plot_pick_axis.h"

#include <genesis_style/style.h>

namespace Dialogs
{
  WebDialogPlsLoadPlotPickAxis::WebDialogPlsLoadPlotPickAxis(QWidget* parent)
    : WebDialog(parent, nullptr, QSizeF(0.2, 0.1), QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  {
    SetupUi();
    ConnectSignals();
    WidgetUi->Dimension2DViewRadioButton->setChecked(true);
  }

  void WebDialogPlsLoadPlotPickAxis::SetPCXAxisNames(const QStringList& axis)
  {
    WidgetUi->Dim2XAxisComboBox->addItems(axis);
  }

  void WebDialogPlsLoadPlotPickAxis::setPCXAxisIndex(int index)
  {
    WidgetUi->Dim2XAxisComboBox->setCurrentIndex(index);
  }

  void WebDialogPlsLoadPlotPickAxis::SetPCYAxisNames(const QStringList& axis)
  {
    WidgetUi->Dim2YAxisComboBox->addItems(axis);
    WidgetUi->VerticalAxisYComboBox->addItems(axis);
  }

  void WebDialogPlsLoadPlotPickAxis::setPCYAxisIndex(int index)
  {
    WidgetUi->Dim2YAxisComboBox->setCurrentIndex(index);
    WidgetUi->VerticalAxisYComboBox->setCurrentIndex(index);
  }

  int WebDialogPlsLoadPlotPickAxis::Dim2XAxisCheckBoxValue()
  {
    return WidgetUi->Dim2XAxisComboBox->currentIndex();
  }

  int WebDialogPlsLoadPlotPickAxis::Dim2YAxisCheckBoxValue()
  {
    return WidgetUi->Dim2YAxisComboBox->currentIndex();
  }

  int WebDialogPlsLoadPlotPickAxis::VerticalAxisYComboBoxValue()
  {
    return WidgetUi->VerticalAxisYComboBox->currentIndex();
  }

  void WebDialogPlsLoadPlotPickAxis::setSpectralTypeGraphic(SpectralGraphicType type)
  {
    WidgetUi->GraphicStyleSwitcher->setCurrentIndex(static_cast<int>(type));
  }

  SpectralGraphicType WebDialogPlsLoadPlotPickAxis::getSpectralTypeGraphic()
  {
    return static_cast<SpectralGraphicType>(WidgetUi->GraphicStyleSwitcher->currentIndex());
  }

  SelectedTypeView WebDialogPlsLoadPlotPickAxis::getSelectedTypeView()
  {
    return WidgetUi->Dimension2DViewRadioButton->isChecked()
      ? SelectedTypeView::DIMENSION2D
      : SelectedTypeView::SPECTRAL;
  }

  void WebDialogPlsLoadPlotPickAxis::setSelectedTypeView(SelectedTypeView type)
  {
    WidgetUi->Dimension2DViewRadioButton->setChecked(type == SelectedTypeView::DIMENSION2D);
    WidgetUi->SpectralViewRadioButton->setChecked(type == SelectedTypeView::SPECTRAL);
  }

  void WebDialogPlsLoadPlotPickAxis::SetupUi()
  {
    const auto body = new QWidget();
    body->setStyleSheet(Style::Genesis::GetUiStyle());
    WidgetUi = new Ui::WebDialogPlsLoadPlotPickAxis();
    WidgetUi->setupUi(body);
    WidgetUi->CaptionLabel->setStyleSheet(Style::Genesis::Fonts::ModalHead());
    WidgetUi->SelectGraphicViewLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());

    WidgetUi->SelectGraphicViewLabel->setStyleSheet(Style::Genesis::GetH2());

    if (const auto mainLayout = Content->layout(); mainLayout)
    {
      mainLayout->addWidget(body);
    }
    ButtonBox->button(QDialogButtonBox::Ok)->setText(tr("Apply"));
    WidgetUi->GraphicStyleSwitcher->addTab(tr("Dot"));
    WidgetUi->GraphicStyleSwitcher->addTab(tr("Line"));
  }

  void WebDialogPlsLoadPlotPickAxis::ConnectSignals()
  {
    connect(WidgetUi->Dimension2DViewRadioButton, &QRadioButton::toggled,
      this, &WebDialogPlsLoadPlotPickAxis::RadioButtonToggled);

    connect(WidgetUi->SpectralViewRadioButton, &QRadioButton::toggled,
      this, &WebDialogPlsLoadPlotPickAxis::RadioButtonToggled);

    connect(WidgetUi->closeButton, &QPushButton::clicked,
      this, &WebDialog::Reject);
  }

  void WebDialogPlsLoadPlotPickAxis::RadioButtonToggled(bool toggled)
  {
    const auto dim2Checked = WidgetUi->Dimension2DViewRadioButton->isChecked();
    WidgetUi->Dim2XAxisComboBox->setVisible(dim2Checked);
    WidgetUi->Dim2YAxisComboBox->setVisible(dim2Checked);
    WidgetUi->Dim2XAxisLabel->setVisible(dim2Checked);
    WidgetUi->Dim2YAxisLabel->setVisible(dim2Checked);

    const auto spectralViewChecked = WidgetUi->SpectralViewRadioButton->isChecked();
    WidgetUi->VerticalAxisYComboBox->setVisible(spectralViewChecked);
    WidgetUi->VerticalAxisYLabel->setVisible(spectralViewChecked);
    WidgetUi->GraphicStyleSwitcher->setVisible(spectralViewChecked);
    WidgetUi->GraphicTypeLable->setVisible(spectralViewChecked);
  }
}