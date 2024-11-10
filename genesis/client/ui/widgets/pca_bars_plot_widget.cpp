#include "pca_bars_plot_widget.h"

#include <QVBoxLayout>

#include "genesis_style/style.h"
#include "ui/item_models/bar_plot_table_model.h"
#include <graphicsplot/graphicsplot_extended.h>
#include <logic/models/analysis_entity_model.h>
#include <ui/plots/gp_items/gpshape_item.h>

// using namespace Model;
using namespace Models;
using namespace AnalysisEntity;

namespace Widgets
{
  PcaBarsPlotWidget::PcaBarsPlotWidget(QWidget* parent)
    : PcaPlotWidget(parent)
  {
    setupBarsPlot();
  }

  void PcaBarsPlotWidget::setModel(QPointer<AnalysisEntityModel> model)
  {
    if(mEntityModel)
    {
      disconnect(mEntityModel, nullptr, this, nullptr);
    }
    PcaPlotWidget::setModel(model);
    if(mEntityModel)
    {
      connect(mEntityModel, &AnalysisEntityModel::modelDataChanged, this, &PcaBarsPlotWidget::onModelDataChanged);
      connect(mEntityModel, &AnalysisEntityModel::modelReset, this, &PcaBarsPlotWidget::onModelReset);
      if (const auto table = findChild<QTableView*>())
      {
        const auto barModel = new BarPlotTableModel(this);
        barModel->setModel(mEntityModel);
        table->setModel(barModel);
      }
      onModelReset();
    }

  }

  bool PcaBarsPlotWidget::eventFilter(QObject* target, QEvent* event)
  {
    if(target == mPlot)
    {
      if (event->type() == QEvent::ToolTip)
      {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QVariant details;
        auto val = mBars->selectTest(helpEvent->pos(), false, &details);
        if (val > 0 && val < mPlot->selectionTolerance())
        {
          GPDataSelection selection = details.value<GPDataSelection>();
          auto data = mBars->data()->at(selection.dataRange().begin());

          mPlot->setToolTip(tr("PC") + QString("%1\t%2\%").arg((*data).sortKey()).arg((*data).mainValue() * 100));
        }
        else
        {
          mPlot->setToolTip("");
        }
      }
      // else if(event->type() == QEvent::Wheel)
      // {
      //   auto e = (QWheelEvent*)event;
      //   if(!e->modifiers().testFlag(Qt::ControlModifier))
      //   {
      //     e->ignore();
      //     return true;
      //   }
      //   else
      //   {
      //     return false;
      //   }
      // }
    }
    return PcaPlotWidget::eventFilter(target, event);
  }

  void PcaBarsPlotWidget::setupBarsPlot()
  {
    mPlot = new GraphicsPlotExtended(this);
    mPlot->setAttribute(Qt::WA_NoMousePropagation, false);
    mPlot->setZoomModifiers(Qt::ControlModifier);
    mPlot->setPlottingHint(GP::phImmediateRefresh, false);
    mPlot->installEventFilter(this);
    mPlot->setFlag(GP::AASettingsEdit, false);
    mPlot->setFlag(GP::AARulers, false);
    mPlot->xAxis->setLabelColor(QColor(0, 32, 51, 154));
    mPlot->yAxis->setLabelColor(QColor(0, 32, 51, 154));
    mPlot->xAxis->setTickLabelColor(QColor(0, 32, 51, 154));
    mPlot->yAxis->setTickLabelColor(QColor(0, 32, 51, 154));
    // mPlot->setAntialiasedElements(GP::aeAll);
    // mPlot->setNoAntialiasingOnDrag(true);
    // mPlot->setOpenGl(true, 16);

    mPlot->xAxis->setSubTicks(false);
    mPlot->xAxis->setTickLength(0, 4);
    mPlot->xAxis->setRange(0, 100);
    mPlot->xAxis->grid()->setVisible(false);

    mPlot->yAxis->setRange(0, 100);
    mPlot->yAxis->setPadding(5);
    mPlot->yAxis->setLabel(tr("Explained variance, %"));
    mPlot->yAxis->grid()->setSubGridVisible(true);

    mBars = new GPBars(mPlot->xAxis, mPlot->yAxis);
    mBars->setPen(QColor(86, 185, 242));
    mBars->setBrush(QColor(86, 185, 242));
    mBars->setWidth(mBars->width() / 2);

    mCurve = new GPCurve(mPlot->xAxis, mPlot->yAxis);
    QPen pen(GPShapeItem::ToColor(GPShapeItem::Blue));
    pen.setWidth(2);
    mCurve->setPen(pen);
    mCurve->setScatterStyle(GPScatterStyle(GPScatterStyle::ssCircle, pen.color(), Qt::white, 10));

    auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(mPlot);
    mainLayout->setStretchFactor(mPlot, 1);
    mPlot->installEventFilter(this);
    const auto tableCaption = new QLabel(tr("Primary components with % explained variance"));
    tableCaption->setStyleSheet(Style::Genesis::GetH2());
    mainLayout->addWidget(tableCaption);
    const auto table = new QTableView(this);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setVisible(false);
    table->setStyleSheet("QHeaderView::section { padding: 5;};");
    mainLayout->addWidget(table);
  }

  void PcaBarsPlotWidget::onModelDataChanged(const QHash<int, QVariant> &roleDataMap)
  {
    invalidateBars();
  }

  void PcaBarsPlotWidget::onModelReset()
  {
    invalidateBars();
  }

  void PcaBarsPlotWidget::invalidateBars()
  {
    if(!mEntityModel)
      return;
    mBars->data().data()->clear();
    mCurve->data().data()->clear();

    auto varianceMap = mEntityModel->modelData(ModelRoleComponentsExplVarianceMap)
                           .value<TComponentsExplVarianceMap>();

    const auto acceptableCount = varianceMap.size();

    QVector<double> ticks(acceptableCount);
    std::generate(ticks.begin(), ticks.end(), [n = 1]() mutable { return n++;
    });

    QVector<QString> labels;
    labels.reserve(acceptableCount);

    std::transform(ticks.begin(), ticks.end(), std::back_inserter(labels),
      [](double val) { return tr("PC %1").arg(val); });

    QVector<double> barsValues;
    barsValues.reserve(acceptableCount);
    std::transform(ticks.begin(), ticks.end(), std::back_inserter(barsValues),
                   [varianceMap](double tick) { return varianceMap.value((int)tick, -1); });
    mBars->addData(ticks, barsValues, true);

    QVector<double> curveValues;
    curveValues.reserve(acceptableCount);
    std::transform(barsValues.begin(), barsValues.end(),
    std::back_inserter(curveValues),
      [n = 0.0](double val) mutable { n += val; return n; });

    mCurve->addData(ticks, curveValues);

    GPRange xRange(0, acceptableCount + 1);
    GPRange yRange(-1, 110);
    mPlot->xAxis->setRange(xRange);
    mPlot->yAxis->setRange(yRange);

    QSharedPointer<GPAxisTickerText> textTicker(new GPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    mPlot->xAxis->setTicker(textTicker);
    mPlot->queuedReplot();
  }

  void PcaBarsPlotWidget::showEvent(QShowEvent *event)
  {
    if(mPlot)
      mPlot->queuedReplot();
  }
}
