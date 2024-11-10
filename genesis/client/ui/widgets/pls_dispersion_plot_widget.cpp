#include "pls_dispersion_plot_widget.h"

#include <ui/plots/pls_plot_tiped.h>
#include <logic/models/pls_fit_response_model.h>
#include <genesis_style/style.h>

namespace Widgets
{
  using namespace Model;

  PlsDispersionPlotWidget::PlsDispersionPlotWidget(QWidget* parent)
    : PlsPlotWidget(parent)
  {
    SetupUi();
    ConnectSignals();
  }

  void PlsDispersionPlotWidget::SetupUi()
  {
    if (const auto mainLayout = qobject_cast<QVBoxLayout*>(layout()); mainLayout)
    {
      auto caption = new QLabel(tr("Dispersion plot"));
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
        createAndPutLegendWidget(GPShapeItem::ToColor(GPShapeItem::DefaultColor::Orange),
                                 tr("Matrix X"), legendLayout);
        createAndPutLegendWidget(GPShapeItem::ToColor(GPShapeItem::DefaultColor::Green),
                                 tr("Matrix Y graduirovka"), legendLayout);
        createAndPutLegendWidget(GPShapeItem::ToColor(GPShapeItem::DefaultColor::Violet),
                                 tr("Matrix Y proverka"), legendLayout);
        legendLayout->addStretch();
      }

      const auto plot = new PlsPlotTiped(this);
      plot->setZoomModifiers(Qt::ControlModifier);
      plot->setAttribute(Qt::WA_NoMousePropagation, false);
      plot->setFlag(GP::AAMagnifier, false);
      plot->setInteraction(GP::Interaction::iSelectItems, false);
      plot->setMultiSelectModifier(Qt::NoModifier);
      plot->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

      plot->yAxis->setLabel(tr("Explane dispersion"));
      plot->xAxis->setLabel(tr("PC number"));

      mainLayout->addWidget(plot);
      mainLayout->setStretchFactor(plot, 1);
    }
  }

  void PlsDispersionPlotWidget::ConnectSignals()
  {
    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this, &PlsDispersionPlotWidget::UpdateDataFromModel);
  }

  void PlsDispersionPlotWidget::UpdateDataFromModel()
  {
    const auto model = ObjectModel->GetModel();
    assert(model);

    QVector<double> varianceYGrad;
    model->Exec<DoubleVector>(PlsFitResponseModel::VarianceYGrad,
      [&varianceYGrad](const DoubleVector& rmse)
      { varianceYGrad = { rmse.begin(), rmse.end() }; });

    if (varianceYGrad.isEmpty())
      return;

    QVector<double> varianceYProv;
    model->Exec<DoubleVector>(PlsFitResponseModel::VarianceYProv,
      [&varianceYProv](const DoubleVector& rmse)
      { varianceYProv = { rmse.begin(), rmse.end() }; });

    QVector<double> varianceX;
    model->Exec<DoubleVector>(PlsFitResponseModel::VarianceX,
      [&varianceX](const DoubleVector& rmse)
      { varianceX = { rmse.begin(), rmse.end() }; });

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

    AppendCurve(plot, xCoords, varianceYGrad, GPShapeItem::DefaultColor::Green, 2, GPCurve::lsLine, tr("Matrix Y graduirovka"));
    AppendCurve(plot, xCoords, varianceYProv, GPShapeItem::DefaultColor::Violet, 2, GPCurve::lsLine, tr("Matrix Y proverka"));
    AppendCurve(plot, xCoords, varianceX, GPShapeItem::DefaultColor::Orange, 2, GPCurve::lsLine, tr("Matrix X"));

    plot->xAxis->setTicker(CreateTicker());
    plot->SetDefaultAxisRanges(CreateRangeFromVector(xCoords, 0),
      CreateRangeFromVector(varianceYGrad + varianceYProv + varianceYGrad, 0));
  }

  QSharedPointer<GPAxisTickerText> PlsDispersionPlotWidget::CreateTicker()
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
