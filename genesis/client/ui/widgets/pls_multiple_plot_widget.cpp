#include "pls_multiple_plot_widget.h"

#include <genesis_style/style.h>

#include <logic/models/pls_fit_response_model.h>

#include "pls_plot_widget.h"

namespace Widgets
{
  PlsMultiplePlotWidget::PlsMultiplePlotWidget(QWidget* parent)
    : PlsPlotWidget(parent)
  {
    SetupUi();
    ConnectActions();
  }

  void PlsMultiplePlotWidget::SetLabelText(const QString& labelText)
  {
    if (const auto label = findChild<QLabel*>(); label)
    {
      label->setText(labelText);
    }
  }

  void PlsMultiplePlotWidget::SetPCMaxCountValue(int value)
  {
    if (const auto spinBox = findChild<QSpinBox*>(); spinBox)
    {
      spinBox->setValue(value);
    }
  }

  void PlsMultiplePlotWidget::SetupUi()
  {
    if (const auto mainLayout = qobject_cast<QVBoxLayout*>(layout()); mainLayout)
    {
      const auto spinbox = new QSpinBox(this);
      spinbox->setMinimum(1);
      const auto label = new QLabel(tr("PC count on plots"));
      label->setStyleSheet(Style::Genesis::GetVividLabel());
      const auto hBoxLayout = new QHBoxLayout(this);
      hBoxLayout->addWidget(label);
      hBoxLayout->addWidget(spinbox);
      hBoxLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
      mainLayout->insertLayout(0, hBoxLayout);
      MultiPlotWidget = new MultiplePlotWidget(this);
      mainLayout->addWidget(MultiPlotWidget);
    }
  }

  void PlsMultiplePlotWidget::ConnectActions()
  {
    if (const auto spinbox = findChild<QSpinBox*>(); spinbox)
    {
      connect(spinbox, &QSpinBox::valueChanged, this, &PlsMultiplePlotWidget::PCMaxCountValueChanged);
      connect(spinbox, &QSpinBox::valueChanged, this, &PlsMultiplePlotWidget::SetPCMaxCount);
      connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this, &PlsMultiplePlotWidget::ModelChanged);
    }
  }

  void PlsMultiplePlotWidget::ModelChanged()
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    const auto plots = findChildren<PlsPlotWidget*>();
    for (const auto& plot : plots)
      plot->SetModel(ObjectModel->GetModel());

    if (const auto spinBox = findChild<QSpinBox*>(); spinBox)
    {
      const auto plsComponents = model->Get<PlsComponents>(PlsFitResponseModel::PlsComponents);
      spinBox->setMaximum(plsComponents.size());
      const auto defaultValue = static_cast<int>(model->Get<size_t>(PlsFitResponseModel::DefaultNumberPC));
      spinBox->setValue(defaultValue);
    }
  }

  void PlsMultiplePlotWidget::SetAxisForPC(int yPC, int xPC)
  {
    const auto plots = findChildren<PlsPlotWidget*>();
    for (const auto& plot : plots)
      plot->SetAxisForPC(yPC, xPC);
  }

  void PlsMultiplePlotWidget::SetPCMaxCount(int maxCount)
  {
    const auto plots = findChildren<PlsPlotWidget*>();
    for (const auto& plot : plots)
      plot->SetPCMaxCount(maxCount);
  }

  void PlsMultiplePlotWidget::PlsAppendPlots(const std::list<PlotWidget*>& list)
  {
    MultiPlotWidget->AppendPlots(list);
  }
}
