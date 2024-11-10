#include "plot_legend_widget.h"

#include "ui/plots/gp_items/gpshape_with_label_item.h"
#include "ui_plot_legend_widget.h"

#include "genesis_style/style.h"
#include "ui/dialogs/web_dialog_pca_legend_settings.h"
#include "ui/flow_layout.h"
#include "ui/chromotogram_page/legend_widget.h"

namespace
{
  const static int ContentMargin = 5;
}

PlotLegendWidget::PlotLegendWidget(CountsPlot* plot, QWidget* parent) : QWidget(parent)
{
  Plot = plot;
  SetupUi();
  ConnectSignals();
}

PlotLegendWidget::~PlotLegendWidget()
{
  delete WidgetUi;
}

void PlotLegendWidget::SetupUi()
{
  WidgetUi = new Ui::PlotLegendWidget();
  WidgetUi->setupUi(this);
  WidgetUi->ExceptionsLegendLayoutCaption->setHidden(true);
  WidgetUi->ExceptionsLegendLayoutCaption->setStyleSheet(Style::Genesis::GetH3());
  WidgetUi->EditLegendButton->setStyleSheet(Style::Genesis::GetUiStyle());
  WidgetUi->EditLegendButton->setFlat(true);
  WidgetUi->LegendCaption->setStyleSheet(Style::Genesis::GetH3());
  auto l = qobject_cast<QVBoxLayout*>(layout());
  ShapeLegendLayout = new FlowLayout(20, 20, 20);
  ColorLegendLayout = new FlowLayout(20, 20, 20);
  ExceptionsLegendLayout = new FlowLayout(20, 20, 20);
  l->insertLayout(1, ShapeLegendLayout);
  l->insertLayout(2, ColorLegendLayout);
  l->addLayout(ExceptionsLegendLayout);

}

void PlotLegendWidget::ConnectSignals()
{
  connect(WidgetUi->EditLegendButton, &QPushButton::clicked, this, &PlotLegendWidget::ShowEditDialog);
  connect(Plot, &CountsPlot::legendChanged, this, &PlotLegendWidget::OnLegendChanged);
}

void PlotLegendWidget::ShowEditDialog()
{
  auto dial = new Dialogs::WebDialogPCALegendSettings(this, Plot);
  dial->Open();
}

void PlotLegendWidget::OnLegendChanged()
{
  RemoveWidgetsFrom(ShapeLegendLayout);
  RemoveWidgetsFrom(ColorLegendLayout);
  RemoveWidgetsFrom(ExceptionsLegendLayout);

  if (Plot->GetShapeSortParam() != ParameterType::LastType)
  {
//    ShapeLegendType sl;
//    for(int i = 0; i < 100; i++)
//    {
//      auto key = QString::number(rand());
//      while(sl.contains(key))
//        key = QString::number(rand());
//      sl[key] = (GPShape::ShapeType)(i%32);
//    }
    auto shapes = Plot->getShapeLegend();
//    auto shapes = sl;
    for (auto key : shapes.keys())
    {
      auto item = new LegendWidget();
      item->setContentsMargins(ContentMargin, ContentMargin, ContentMargin, ContentMargin);
      item->setShape(shapes[key]);
      item->setShapeBrush(Qt::black);
      item->setShapePen(Qt::NoPen);
      item->setSpacing(ContentMargin);
      item->setText(key);
      ShapeLegendLayout->addWidget(item);
    }
  }
  else
  {
    auto item = new QLabel(tr("No grouping by shape"));
    item->setProperty("gray", true);
    ShapeLegendLayout->addWidget(item);
  }
  if (Plot->GetColorSortParam() != ParameterType::LastType)
  {
//    ColorLegendType cl;
//    for(int i = 0; i < 100; i++)
//    {
//      auto key = QString::number(rand());
//      while(cl.contains(key))
//        key = QString::number(rand());
//      cl[key] = (GPShapeItem::DefaultColor)(i%21);
//    }
    auto colors = Plot->getColorLegend();
//    auto colors = cl;
    for (auto key : colors.keys())
    {
      auto item = new LegendWidget();
      item->setContentsMargins(ContentMargin, ContentMargin, ContentMargin, ContentMargin);
      item->setShape(GPShape::Square);
      item->setShapeBrush(GPShapeItem::ToColor(colors[key]));
      item->setShapePen(Qt::NoPen);
      item->setSpacing(ContentMargin);
      item->setText(key);
      ColorLegendLayout->addWidget(item);
    }
  }
  else
  {
    auto item = new QLabel(tr("No grouping by color"));
    item->setProperty("gray", true);
    ColorLegendLayout->addWidget(item);
  }

  CustomItemStyleMap customStyle = Plot->getCustomItemStyle();

  WidgetUi->ExceptionsLegendLayoutCaption->setHidden(customStyle.isEmpty());
  for (auto key : customStyle.keys())
  {
    auto item = new LegendWidget();
    item->setContentsMargins(ContentMargin, ContentMargin, ContentMargin, ContentMargin);
    item->setShape(customStyle[key].shape);
    item->setShapeBrush(GPShapeItem::ToColor(customStyle[key].color));
    item->setShapePen(Qt::NoPen);
    item->setSpacing(ContentMargin);
    auto plotItem = Plot->findItemByFileId(key);
    if (plotItem)
      item->setText(plotItem->getLabelText());

    ExceptionsLegendLayout->addWidget(item);
  }
}

void PlotLegendWidget::RemoveWidgetsFrom(QLayout* layout)
{
  while (layout->count() > 0)
  {
    auto l = layout->takeAt(0);
    auto wgt = l->widget();
    wgt->setParent(nullptr);
    delete wgt;
  }
}
