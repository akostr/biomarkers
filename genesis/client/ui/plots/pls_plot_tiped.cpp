#include "pls_plot_tiped.h"
#include <ui/plots/gp_items/gp_curve_tracer_item.h>
#include <algorithm>

#include "genesis_style/style.h"
#include "ui/widgets/curve_label_widget.h"

namespace
{
  const static double kSize = 10;
  const static int MinimumLength = 100;
}

PlsPlotTiped::PlsPlotTiped(QWidget* parent, const unsigned& flags)
  : GraphicsPlotExtended(parent, flags)
{
  m_tipWidget = new QWidget(this);
  m_tipWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_tipWidget->setContentsMargins(9, 9, 9, 9);
  m_tipWidget->setStyleSheet(Style::ApplySASS(
    "QWidget {background-color: rgb(45, 50, 54); border-radius:2px;}"
    "QLabel{ font: @SecondaryTextFont; color: #FFFFFF; }"
  ));
  auto layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  m_tipWidget->setLayout(layout);
  m_tipCaptionLabel = new QLabel(tr("Please select curve from difference plot"));
  layout->addWidget(m_tipCaptionLabel);
  m_tipCurvesForm = new QFormLayout();
  m_tipCurvesForm->setVerticalSpacing(0);
  m_tipCurvesForm->setHorizontalSpacing(0);
  m_tipCurvesForm->setSpacing(0);
  m_tipCurvesForm->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
  layout->addLayout(m_tipCurvesForm);
  m_tipWidget->hide();
}

PlsPlotTiped::~PlsPlotTiped()
{
  for (auto& conn : m_connections)
    disconnect(conn);
  m_connections.clear();
  m_tracers.clear();//not owned this objects
  m_legendItems.clear();//not owned this objects
}

void PlsPlotTiped::SetDefaultAxisRanges(const GPRange& xRange, const GPRange& yRange)
{
  xRangeDefault = xRange;
  yRangeDefault = yRange;
  ResizeAxisToDefault();
}

void PlsPlotTiped::ResizeAxisToDefault()
{
  xAxis->setRange(xRangeDefault);
  yAxis->setRange(yRangeDefault);
  queuedReplot();
}

void PlsPlotTiped::mouseMoveEvent(QMouseEvent* event)
{
  auto labels = xAxis->tickVectorLabels();
  if (labels.isEmpty())
    return;

  auto pos = event->pos();

  for (const auto& curve : mCurves)
  {
    auto tracer = m_tracers[curve];
    tracer->setCurveKey(xAxis->pixelToCoord(pos.x()));
    m_legendItems[curve].second->setText(QString::number(tracer->position->value()));
    m_legendItems[curve].first->setText(curve->name());
    const auto color = curve->pen().color();
    tracer->setBrush(color);
    m_legendItems[curve].first->setColor(color);
    if (!tracer->visible())
      tracer->setVisible(true);
  }

  m_tipWidget->move(pos);

  if (!m_tipWidget->isVisible())
  {
    m_tipWidget->show();
    m_tipWidget->adjustSize();
  }
  if (!m_tracers.isEmpty())
    m_tipCaptionLabel->setText(xAxis->tickLabel(m_tracers.first()->position->key()));
  queuedReplot();
  GraphicsPlotExtended::mouseMoveEvent(event);
}

void PlsPlotTiped::leaveEvent(QEvent* event)
{
  GraphicsPlotExtended::leaveEvent(event);
  for (auto& tracer : m_tracers)
    tracer->setVisible(false);
  m_tipWidget->hide();
}

void PlsPlotTiped::onCurveRemoved(QObject* obj)
{
  auto curvePtr = static_cast<GPCurve*>(obj);
  //this connection, actually, have to disconnect automatically while not in the destructor
  auto connection = m_connections.take(curvePtr);
  auto tracer = m_tracers.take(curvePtr);
  auto legendItem = m_legendItems.take(curvePtr);
  if (hasItem(tracer))
    removeItem(tracer);
  m_tipCurvesForm->removeRow(legendItem.second);
}

void PlsPlotTiped::addLegendItem(GPCurve* curvePtr)
{
  m_legendItems[curvePtr].first = new CurveLabelWidget(curvePtr->name(), curvePtr->pen().color());
  m_legendItems[curvePtr].second = new QLabel("0");
  m_tipCurvesForm->addRow(m_legendItems[curvePtr].first, m_legendItems[curvePtr].second);
}

GPCurveTracerItem* PlsPlotTiped::createTracerForCurve(GPCurve* curvePtr)
{
  auto tracer = new GPCurveTracerItem(this);
  m_tracers[curvePtr] = tracer;
  tracer->setCurve(curvePtr);

  tracer->setSize(kSize);
  tracer->setPen(QPen(Qt::white, 2));
  tracer->setBrush(curvePtr->pen().color());
  tracer->setVisible(false);
  tracer->setStyle(GPCurveTracerItem::TracerStyle::tsCircle);

  return tracer;
}

GPCurve* PlsPlotTiped::addCurve(GPAxis* keyAxis, GPAxis* valueAxis)
{
  auto curvePtr = GraphicsPlotExtended::addCurve(keyAxis, valueAxis);
  createTracerForCurve(curvePtr);
  addLegendItem(curvePtr);
  auto connection = connect(curvePtr, &GPCurve::destroyed, this, &PlsPlotTiped::onCurveRemoved);
  m_connections[curvePtr] = connection;
  return curvePtr;
}

void PlsPlotTiped::mouseDoubleClickEvent(QMouseEvent* event)
{
  ResizeAxisToDefault();
  GraphicsPlotExtended::mouseDoubleClickEvent(event);
}
