#include "mcr_calculation_plot.h"

#include "logic/structures/common_structures.h"
#include "logic/models/mcr_analysis_response_model.h"
#include "ui/plots/pls_plot_tiped.h"
#include "genesis_style/style.h"

namespace
{
  std::map<size_t, QColor> ColorByCurveId;
}

using namespace Objects;
using namespace Structures;

namespace
{
  const static QColor AxisColor(113, 130, 141);
  const static QColor GridColor(204, 217, 224);
}

namespace Widgets
{
  MCRCalculationPlot::MCRCalculationPlot(QWidget* parent)
    : PlotWidget(parent)
  {
    SetupUi();
    ConnectSignals();
  }

  std::map<size_t, QColor> MCRCalculationPlot::GetColors() const
  {
    return ColorByCurveId;
  }

  void MCRCalculationPlot::SetColors(const std::map<size_t, QColor>& colors)
  {
    ColorByCurveId = colors;
  }

  void MCRCalculationPlot::SetXAxisLabel(const QString& xAxisLabel)
  {
    XAxisLabel = xAxisLabel;
  }

  void MCRCalculationPlot::SetYAxisLabel(const QString& yAxisLabel)
  {
    YAxisLabel = yAxisLabel;
  }

  void MCRCalculationPlot::SetModelComponentName(const std::string& componentName)
  {
    ComponentName = componentName;
  }

  void MCRCalculationPlot::SetupUi()
  {
    const auto box = findChild<QVBoxLayout*>();
    const auto plot = new PlsPlotTiped(this);
    plot->setZoomModifiers(Qt::ControlModifier);
    plot->setAttribute(Qt::WA_NoMousePropagation, false);
    plot->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    plot->setFlag(GP::AAMagnifier, false);
    plot->setInteraction(GP::Interaction::iSelectItems, false);
    connect(plot, &GraphicsPlotExtended::plottableDoubleClick, this,
      [&](GPAbstractPlottable* plottable, int dataIndex, QMouseEvent* event)
      {
        mouseDoubleClickEvent(event);
      });
    plot->setMultiSelectModifier(Qt::NoModifier);
    plot->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    box->addWidget(plot);
    emit PlotChanged();
  }

  void MCRCalculationPlot::ConnectSignals()
  {
    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this, &MCRCalculationPlot::ModelChanged);
  }

  void MCRCalculationPlot::mouseDoubleClickEvent(QMouseEvent* event)
  {
    if (event->button() == Qt::LeftButton
      && event->modifiers() == Qt::ControlModifier)
    {
      if (const auto plot = findChild<PlsPlotTiped*>())
        plot->ResizeAxisToDefault();
    }
    QWidget::mouseDoubleClickEvent(event);
  }

  void MCRCalculationPlot::ModelChanged()
  {
    const auto plot = findChild<PlsPlotTiped*>();

    if (plot == nullptr)
      return;

    ColorByCurveId.clear();
    plot->clearItems();
    plot->clearCurves();

    const auto model = ObjectModel->GetModel();
    if (model == nullptr)
      return;

    const auto concentrations = model->Get<MCRCalculationList>(ComponentName);
    if (concentrations.empty())
      return;
    const auto analysisData = model->Get<AnalysisData>(McrAnalysisResponseModel::AnalysisData);
    double yMax = 0;
    double yMin = 0;

    if (ColorByCurveId.empty())
    {
      const auto red = 0;
      const auto orange = 1;
      const auto yellow = 2;
      const auto green = 3;
      const auto cyan = 4;
      const auto blue = 5;
      const auto violet = 6;

      const auto allColors = concentrations.size();
      std::transform(concentrations.begin(), concentrations.end(),
        std::inserter(ColorByCurveId, ColorByCurveId.end()),
        [&](const MCRCalculation& item)
        {
          const auto id = static_cast<int>(item.Id) == -1
            ? item.ComponentNumber
            : item.Id;

          float r = 0;
          float g = 0;
          float b = 0;

          auto x = (id + 1) * (violet - 1) / static_cast<double>(allColors);
          if ((red <= x && x <= orange) || (blue <= x && x <= violet))
            r = 1;
          else if (cyan <= x && x <= blue)
            r = x - cyan;
          else if (orange <= x && x <= yellow)
            r = 1 - x + orange;
          r = std::round(r * 255);

          if (orange <= x && x <= green)
            g = 1;
          else if (red <= x && x <= orange)
            g = x - red;
          else if (green <= x && x <= cyan)
            g = 1 - x + green;
          g = std::round(g * 255);

          if (green <= x && x <= blue)
            b = 1;
          else if (yellow <= x && x <= green)
            b = x - yellow;
          else if (blue <= x && x <= violet)
            b = 1 - x + blue;
          b = std::round(b * 255);

          const QColor color(r, g, b);
          return std::make_pair(id, color);
        });
    }
    DoubleVector xAxis(concentrations.front().Values.size());
    std::iota(xAxis.begin(), xAxis.end(), 1);
    const auto xMax = xAxis.size();
    const auto xMin = *std::min_element(xAxis.begin(), xAxis.end());

    for (const auto& curve : concentrations)
    {
      const auto isComponent = static_cast<int>(curve.Id) == -1;
      const auto id = isComponent
        ? curve.ComponentNumber
        : curve.Id;
      const auto legendItemName = isComponent
        ? tr("Component %1").arg(id) //if you need to fix numeration to id+1 in legend, backend sholuld also fix numeration for concentrations_column
        : analysisData.Data.T_Title[id].FileName.c_str();
      AppendCurve(plot, xAxis, curve.Values, ColorByCurveId[id]
        , 2, GPCurve::LineStyle::lsLine, legendItemName);

      yMax = std::max(yMax, *std::max_element(curve.Values.begin(), curve.Values.end()));
      yMin = std::min(yMin, *std::min_element(curve.Values.begin(), curve.Values.end()));
    }

    plot->xAxis->setLabel(XAxisLabel);
    plot->xAxis->setLabelColor(AxisColor);
    plot->xAxis->setLabelFont(Style::Genesis::Fonts::RegularText());
    plot->xAxis->setTickLabelColor(AxisColor);
    plot->xAxis->setTickLabelFont(Style::Genesis::Fonts::AxisTicks());

    plot->yAxis->setLabel(YAxisLabel);
    plot->yAxis->setLabelFont(Style::Genesis::Fonts::RegularText());
    plot->yAxis->setLabelColor(AxisColor);
    plot->yAxis->setTickLabelColor(AxisColor);
    plot->yAxis->setTickLabelFont(Style::Genesis::Fonts::AxisTicks());

    plot->SetDefaultAxisRanges(GPRange(xMin, xMax), GPRange(yMin, yMax));
  }
}
