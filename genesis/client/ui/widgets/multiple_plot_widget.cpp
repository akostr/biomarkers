#include "multiple_plot_widget.h"

#include "pls_coefficient_plot_widget.h"
#include "pls_slope_offset_plot_widget.h"

namespace Widgets
{
  MultiplePlotWidget::MultiplePlotWidget(QWidget* parent)
    : PlotWidget(parent)
  {
    if (const auto mainLayout = qobject_cast<QVBoxLayout*>(layout()); mainLayout)
    {
      const auto scrollArea = new QScrollArea(this);

      scrollArea->setWidgetResizable(true);
      scrollArea->setStyleSheet("QWidget { background-color:white; }");
      scrollArea->setFrameShape(QFrame::NoFrame);
      scrollArea->setAutoFillBackground(false);
      scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

      ScrollLayout = new QVBoxLayout(scrollArea);
      ScrollLayout->setContentsMargins(0,0,0,0);
      const auto scrollWidget = new QWidget;
      scrollWidget->setContentsMargins(0,0,0,0);
      scrollWidget->setLayout(ScrollLayout);
      scrollArea->setWidget(scrollWidget);
      scrollArea->setContentsMargins(0,0,0,0);

      mainLayout->addWidget(scrollArea);
    }

    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this, [&]()
      {
        const auto plots = findChildren<PlotWidget*>();
        for (const auto& plot : plots)
          plot->SetModel(ObjectModel->GetModel());
      });
  }

  void MultiplePlotWidget::AppendPlots(const std::list<PlotWidget*>& list)
  {
    for (const auto& plot : list)
    {
      plot->setMaximumHeight(this->height());
      plot->setMinimumHeight(this->height());
      ScrollLayout->addWidget(plot);
      plot->show();
    }
  }
}

void Widgets::MultiplePlotWidget::resizeEvent(QResizeEvent *event)
{
  PlotWidget::resizeEvent(event);
  for(int i = 0; i < ScrollLayout->count(); i++)
  {
    if(auto w = ScrollLayout->itemAt(i)->widget())
    {
      w->setMaximumHeight(this->height());
      w->setMinimumHeight(this->height());
    }
  }
}
