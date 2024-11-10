#include "pls_coefficient_plot_widget.h"

#include <genesis_style/style.h>
#include <ui/controls/tab_switcher.h>
#include "ui/plots/pls_coefficient_plot.h"
#include "logic/models/pls_fit_response_model.h"

namespace
{
  const static QColor LineColor("#0071B2");
  const static QColor SelectionColor(86, 185, 242, 60);
}

namespace Widgets
{
  PlsCoefficientPlotWidget::PlsCoefficientPlotWidget(QWidget* parent)
    : PlsPlotWidget(parent)
  {
    SetupUi();
    ConnectSignals();
  }

  void PlsCoefficientPlotWidget::SetupUi()
  {
    if (const auto mainLayout = qobject_cast<QVBoxLayout*>(layout()); mainLayout)
    {
      {//toolbar
        auto toolbarLayout = new QHBoxLayout();
        mainLayout->addLayout(toolbarLayout);
        QLabel* caption1 = new QLabel(tr("Regression Coefficient plot"));
        caption1->setStyleSheet(Style::Genesis::Fonts::H2());
        toolbarLayout->addWidget(caption1);
        toolbarLayout->addStretch();
        CancelExcludeButton = new QPushButton(tr("Cancel exclude"), this);
        CancelExcludeButton->setVisible(false);
        toolbarLayout->addWidget(CancelExcludeButton);
        Switcher = new TabSwitcher();
        toolbarLayout->addWidget(Switcher);
        Switcher->addTab(tr("Linear view"));
        Switcher->addTab(tr("Point view"));

        Plot = new PlsCoefficientPlot(this);
        Plot->setZoomModifiers(Qt::ControlModifier);
        Plot->setAttribute(Qt::WA_NoMousePropagation, false);
        mainLayout->addWidget(Plot);
        mainLayout->setStretchFactor(Plot, 1);
      }
    }
  }

  void PlsCoefficientPlotWidget::ConnectSignals()
  {
    connect(CancelExcludeButton, &QPushButton::clicked, this, &PlsCoefficientPlotWidget::ClearExlcudePeaks);
    connect(CancelExcludeButton, &QPushButton::clicked, this, &PlsCoefficientPlotWidget::OnPeaksExclude);
    connect(Plot, &PlsCoefficientPlot::ExcludePrepeared, this, &PlsCoefficientPlotWidget::OnPeaksExclude);
    connect(Plot, &PlsCoefficientPlot::ExcludePrepeared, this, &PlsCoefficientPlotWidget::UpdateCancelButtonVisible);
    connect(Switcher, &TabSwitcher::currentChanged, this,
      [&](int tabInd)
      {
        SwitchCoefficientView(static_cast<GPCurve::LineStyle>((tabInd + 1) % 2));
      });
  }

  void PlsCoefficientPlotWidget::SwitchCoefficientView(GPCurve::LineStyle type)
  {
    Curve->setScatterStyle(type == GPCurve::lsLine
      ? GPScatterStyle(GPScatterStyle::ssNone)
      : GPScatterStyle(GPScatterStyle::ssDisc, LineColor, 7));
    Curve->setLineStyle(type);
    Curve->parentPlot()->replot(GraphicsPlotExtended::rpQueuedReplot);
  }

  void PlsCoefficientPlotWidget::mouseDoubleClickEvent(QMouseEvent* event)
  {
    if (event->button() == Qt::LeftButton
      && event->modifiers() == Qt::ControlModifier)
    {
      if (Plot)
        Plot->ResizeAxisToDefault();
    }
    QWidget::mouseDoubleClickEvent(event);
  }

  void PlsCoefficientPlotWidget::UpdateCancelButtonVisible()
  {
    CancelExcludeButton->setVisible(Plot->ExcludedExists());
  }

  void PlsCoefficientPlotWidget::SetPCMaxCount(int maxCount)
  {
    const auto model = ObjectModel->GetModel();
    if (!model || !Plot)
      return;

    const auto currentComponent = FindPlsComponentByIndex(maxCount);
    if (!currentComponent.has_value())
      return;
    Plot->clearCurves();
    QVector<double> xCoords(currentComponent->Coefficients.size());
    std::iota(xCoords.begin(), xCoords.end(), 1);

    AppendCurve(Plot, xCoords, currentComponent->Coefficients,
      LineColor, 2);
    Curve = Plot->curve(0);
    SwitchCoefficientView(static_cast<GPCurve::LineStyle>((Switcher->currentIndex() + 1) % 2));
    Plot->SetDefaultAxisRanges(CreateRangeFromVector(xCoords, 0), CreateRangeFromVector(currentComponent->Coefficients, 0));
  }

  void PlsCoefficientPlotWidget::AddExcludedPeaks(const std::vector<int>& peaks)
  {
    if (Plot)
    {
      Plot->AddExcludedPeaks(peaks);
      Plot->HideBuildForNewModel();
      UpdateCancelButtonVisible();
    }
  }

  void PlsCoefficientPlotWidget::CancelExcludePeaks(const std::vector<int>& peaks)
  {
    if (Plot)
    {
      Plot->CancelExcludePeaks(peaks);
      Plot->HideBuildForNewModel();
    }
  }

  void PlsCoefficientPlotWidget::ClearExlcudePeaks()
  {
    if (Plot)
    {
      Plot->ClearExcludedPeaks();
      Plot->HideBuildForNewModel();
      UpdateCancelButtonVisible();
    }
  }

  IntVector PlsCoefficientPlotWidget::GetExcludedPeaks() const
  {
    IntVector excluded;
    if (Plot)
    {
      const auto model = ObjectModel->GetModel();
      const auto pids = model->Get<IntVector>(PlsFitResponseModel::P_id);
      const auto excludedIndexes = Plot->GetSelectedExcludedPeaks();
      excluded.reserve(excludedIndexes.size());
      for (const auto& index : excludedIndexes)
        excluded.push_back(pids[index]);
    }
    return excluded;
  }
}
