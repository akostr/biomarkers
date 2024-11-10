#include "pls_rmse_plot_widget.h"

#include <logic/models/pls_fit_response_model.h>
#include <ui/plots/pls_plot_tiped.h>
#include <genesis_style/style.h>

#include <QPainter>

namespace Widgets
{
  using namespace Model;

  PlsRMSEPlotWidget::PlsRMSEPlotWidget(QWidget* parent)
    : PlsPlotWidget(parent)
  {
    SetupUi();
    ConnectSignals();
  }

  void PlsRMSEPlotWidget::SetupUi()
  {
    if (const auto mainLayout = qobject_cast<QVBoxLayout*>(layout()); mainLayout)
    {
      auto caption = new QLabel(tr("RMSE plot"));
      caption->setStyleSheet(Style::Genesis::GetH2());
      mainLayout->addWidget(caption);
      auto createAndPutLegendWidget = [](QColor color, const QString& text, QHBoxLayout* layout)
      {
        auto previewLabel = new QLabel();
        layout->addWidget(previewLabel);

        auto caption = new QLabel(text);
        layout->addWidget(caption);

        previewLabel->setScaledContents(true);
        QPixmap linePreview(20,20);
        linePreview.fill(QColor(0,0,0,0));
        QPainter p(&linePreview);
        p.setPen(QPen(color, 2));
        p.drawLine(QPoint(0, linePreview.size().height()/2),QPoint(linePreview.size().width(), linePreview.size().height()/2));
        previewLabel->setPixmap(linePreview);
        layout->addSpacing(15);
      };
      {
        auto legendLayout = new QHBoxLayout();
        mainLayout->addLayout(legendLayout);
        createAndPutLegendWidget(GPShapeItem::ToColor(GPShapeItem::DefaultColor::Blue),
                                 tr("Graduirovka"), legendLayout);
        createAndPutLegendWidget(GPShapeItem::ToColor(GPShapeItem::DefaultColor::Red),
                                 tr("Proverka"), legendLayout);
        legendLayout->addStretch();
      }
      const auto plot = new PlsPlotTiped(this);
      plot->setZoomModifiers(Qt::ControlModifier);
      plot->setAttribute(Qt::WA_NoMousePropagation, false);
      plot->setFlag(GP::AAMagnifier, false);
      plot->setInteraction(GP::Interaction::iSelectItems, false);
      plot->setMultiSelectModifier(Qt::NoModifier);
      plot->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

      plot->yAxis->setLabel(tr("RMSE"));
      plot->xAxis->setLabel(tr("PC number"));

      mainLayout->addWidget(plot);
      mainLayout->setStretchFactor(plot, 1);
    }
  }

  void PlsRMSEPlotWidget::ConnectSignals()
  {
    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this, &PlsRMSEPlotWidget::UpdateDataFromModel);
  }

  void PlsRMSEPlotWidget::UpdateDataFromModel()
  {
    const auto model = ObjectModel->GetModel();
    assert(model);

    QVector<double> rmseGradAll;

    model->Exec<DoubleVector>(PlsFitResponseModel::RMSEGradAll,
      [&rmseGradAll](const DoubleVector& rmse)
      { rmseGradAll = { rmse.begin(), rmse.end() }; });

    if (rmseGradAll.isEmpty())
      return;

    QVector<double> rmseProvAll;
    model->Exec<DoubleVector>(PlsFitResponseModel::RMSEProvAll,
      [&rmseProvAll](const DoubleVector& rmse)
      { rmseProvAll = { rmse.begin(), rmse.end() }; });

    QVector<double> xCoords;
    model->Exec<PlsComponents>(PlsFitResponseModel::PlsComponents,
      [&xCoords](const PlsComponents& comps)
      {
        std::transform(comps.begin(), comps.end(), std::back_inserter(xCoords),
          [](const PlsComponent& comp) { return static_cast<double>(comp.Number); });
      });

    const auto plot = findChild<PlsPlotTiped*>();
    assert(plot);
    plot->clearItems();
    plot->clearCurves();

    AppendCurve(plot, xCoords, rmseGradAll, GPShapeItem::DefaultColor::Blue, 2, GPCurve::lsLine, tr("Graduirovka"));
    AppendCurve(plot, xCoords, rmseProvAll, GPShapeItem::DefaultColor::Red, 2, GPCurve::lsLine, tr("Proverka"));

    plot->xAxis->setTicker(CreateTicker());
    plot->SetDefaultAxisRanges(CreateRangeFromVector(xCoords, 0),
      CreateRangeFromVector(rmseGradAll + rmseProvAll, 0));
  }

  QSharedPointer<GPAxisTickerText> PlsRMSEPlotWidget::CreateTicker()
  {
    const auto textTicker = QSharedPointer<GPAxisTickerText>::create();
    const auto model = ObjectModel->GetModel();
    assert(model);
    model->Exec<PlsComponents>(PlsFitResponseModel::PlsComponents,
      [&textTicker](const PlsComponents& comps)
      {
        for (const auto& comp : comps)
          textTicker->addTick(comp.Number, tr("PC %1").arg(comp.Number));
      });

    return textTicker;
  }
}

