#include "pls_count_plot_widget.h"

#include "ui/plots/counts_plot.h"
#include "logic/models/pls_fit_response_model.h"
#include "logic/structures/analysis_model_structures.h"
#include "genesis_style/style.h"
#include "extern/common_gui/standard_widgets/tumbler.h"
#include "ui/dialogs/web_dialog_pick_axis_components.h"
#include "ui/contexts/pls_setup_view_context.h"

namespace
{
  const std::string_view SampleId = "sample_id";
}

namespace Widgets
{
  using namespace Structures;

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

  PlsCountPlotWidget::PlsCountPlotWidget(QWidget* parent)
    : PlsPlotWidget(parent)
  {
    SetupCountPlot();
    ConnectSignals();
  }

  void PlsCountPlotWidget::SetAxisForPC(int xPC, int yPC)
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    const auto pcCount = model->Get<PlsComponents>(PlsFitResponseModel::PlsComponents).size();

    xPC = std::min(xPC, static_cast<int>(pcCount));
    yPC = std::min(yPC, static_cast<int>(pcCount));

    const static size_t minimumPC = 3;
    xPC = (xPC == minimumPC && yPC == minimumPC && pcCount == minimumPC) ? xPC - 1 : xPC;

    xAxis = xPC;
    yAxis = yPC;

    const auto xComp = FindComponentByIndex(xPC);
    const auto yComp = FindComponentByIndex(yPC);
    if (!xComp.has_value() || !yComp.has_value())
      return;

    Plot->clearData();

    const auto ttitle = model->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleGrad);
    QList<T_Title> plotTitles;
    std::transform(ttitle.begin(), ttitle.end(), std::back_inserter(plotTitles),
      [](const MetaInfo& info)
      {
        const auto fileName = _Q(info.FileName);
        const auto lof = fileName.lastIndexOf('/') + 1;
        return T_Title
        {
          _Q(info.Date),
          _Q(info.Field),
          fileName.mid(lof, fileName.count() - lof),
          fileName,
          _Q(info.Layer),
          _Q(info.Well),
          _Q(info.Depth),
          _Q(info.WellCluster),
          static_cast<int>(info.FileId),
          static_cast<int>(info.SampleId),
          false
        };
      });

    Plot->SetYAxisLabel(GenerateAxisLabel(yPC, yComp->ExplPCsVariance));
    Plot->SetXAxisLabel(GenerateAxisLabel(xPC, xComp->ExplPCsVariance));
    Plot->clearStyles();
    Plot->setPoints(plotTitles, xComp.value(), yComp.value());
    Plot->SetDefaultAxisRanges(CreateRangeFromVector(xComp->T), CreateRangeFromVector(yComp->T));
  }

  void PlsCountPlotWidget::SetPCMaxCount(int maxCount)
  {
    MaxCount = maxCount;
  }

  std::vector<int> PlsCountPlotWidget::GetExcludedSampleIds() const
  {
    const auto excluded = Plot->GetExcludedItems();
    std::vector<int> labels;
    labels.reserve(excluded.size());
    std::transform(excluded.begin(), excluded.end(), std::back_inserter(labels),
      [&](const QPair<GPShapeWithLabelItem*, SampleInfoPlate*>& item)
      { return item.first->property(SampleId.data()).toInt(); });

    return labels;
  }

  bool PlsCountPlotWidget::HasExcludedItems() const
  {
    return Plot->getHasExcludedItems();
  }

  void PlsCountPlotWidget::excludeItems(const std::vector<int>& items) const
  {
    std::map<int, GPShapeWithLabelItem*> labelByItems;
    for (int i = 0; i < Plot->itemCount(); i++)
    {
      if (const auto item = qobject_cast<GPShapeWithLabelItem*>(Plot->item(i)))
        labelByItems.emplace(item->property(SampleId.data()).toInt(), item);
    }
    QList<GPShapeWithLabelItem*> exclude;
    exclude.reserve(items.size());
    for (const auto& item : items)
    {
      const auto it = labelByItems.find(item);
      if (it != labelByItems.end())
        exclude.append(it->second);
    }
    Plot->excludeItems(exclude);
  }

  void PlsCountPlotWidget::resetExcludeItems(const std::vector<int>& items) const
  {
    std::map<int, GPShapeWithLabelItem*> labelByItems;
    for (int i = 0; i < Plot->itemCount(); i++)
    {
      if (const auto item = qobject_cast<GPShapeWithLabelItem*>(Plot->item(i)))
        labelByItems.emplace(item->property(SampleId.data()).toInt(), item);
    }
    QList<GPShapeWithLabelItem*> resetItems;
    resetItems.reserve(items.size());
    for (const auto& [text, shape] : labelByItems)
      resetItems.append(shape);
    Plot->resetExcludedItems(resetItems);
  }

  void PlsCountPlotWidget::SetupCountPlot()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());

    Plot = new CountsPlot(this);
    Plot->setZoomModifiers(Qt::ControlModifier);
    Plot->setAttribute(Qt::WA_NoMousePropagation, false);

    Plot->xAxis->setTickLength(0, 4);
    Plot->xAxis->setRange(0, 11);
    Plot->xAxis->grid()->setVisible(true);

    Plot->yAxis->setRange(0, 11);
    Plot->yAxis->setPadding(5);
    Plot->yAxis->grid()->setSubGridVisible(true);
    PlotLegend = new PlotLegendWidget(Plot, this);
    if (const auto mainLayout = qobject_cast<QVBoxLayout*>(layout()); mainLayout)
    {
      PlotToolBar = QSharedPointer<PlotToolbarWidget>::create(this);
      PlotToolBar->Caption->setVisible(false);
      PlotToolBar->CancelExcludingChoise->setVisible(false);
      mToolbarContext = new PlsSetupViewContext(PlotToolBar.data());
      PlotToolBar->SetMenu(mToolbarContext);
      mainLayout->addWidget(PlotToolBar.get());
      mainLayout->addWidget(Plot);
      mainLayout->addWidget(PlotLegend);
      mainLayout->setStretchFactor(Plot, 1);
    }
    PlotToolBar->SwitchScale->setCheckState(Qt::Checked);
  }

  void PlsCountPlotWidget::ConnectSignals()
  {
    connect(PlotToolBar->CancelExcludingChoise, &QPushButton::clicked, this,
      [&]()
      {
        const auto items = Plot->GetExcludedItems();
        std::vector<int> ids;
        ids.reserve(items.size());
        for (const auto& [label, plate] : items)
          ids.push_back(label->property(SampleId.data()).toInt());

        emit excludedItemsReset(ids);
        Plot->resetExcludedItems();
      });

    connect(mToolbarContext->mShowNamesCheckBox, &QCheckBox::toggled, this,
      [&](bool checked) { Plot->ShowNames(checked); });

    connect(mToolbarContext->mShowPassportInfoCheckBox, &QCheckBox::toggled, this,
      [&](bool checked) { Plot->ShowPassport(checked); });

    // connect(mToolbarContext->mResetUserStyleBtn, &QPushButton::clicked, this,
    //   [&]() { Plot->resetCustomStyles(); });

    // connect(mToolbarContext->mResetAllStylesBtn, &QPushButton::clicked, this,
    //   [&]()
    //   {
    //       Plot->resetCustomStyles();
    //       Plot->SetupDefaultStyle();
    //   });

    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this,
      [&]()
      {
        const auto model = ObjectModel->GetModel();
        const auto pcCount = model->Get<PlsComponents>(PlsFitResponseModel::PlsComponents).size();
        PlotToolBar->SetNameForPCButtons(pcCount);
      });

    connect(Plot, &CountsPlot::excludedStateChanged, this, &PlsCountPlotWidget::OnExcludedStateChanged);
    connect(Plot, &CountsPlot::excludedStateChanged, this, &PlsCountPlotWidget::excludedStateChanged);

    connect(Plot, &CountsPlot::setSelectedSamplesForExcludingFromModel, this, &PlsCountPlotWidget::itemsExcluded);
    connect(Plot, &CountsPlot::excludedItemsReset, this, &PlsCountPlotWidget::excludedItemsReset);

    connect(PlotToolBar->PCButton, &QPushButton::clicked, this,
      [&]()
      {
        SetAxisForPC(1, 2);
        emit AxisChanged(1, 2);
      });

    connect(PlotToolBar->AlterPCButton, &QPushButton::clicked, this,
      [&]()
      {
        SetAxisForPC(3, 4);
        emit AxisChanged(3, 4);
      });

    connect(PlotToolBar->PickOtherAxisButton, &QPushButton::clicked, this, &PlsCountPlotWidget::ShowPickAxis);

    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this, &PlsCountPlotWidget::SetupCustomItemStyle);
    connect(Plot->yAxis, qOverload<const GPRange&>(&GPAxis::rangeChanged), this, &PlsCountPlotWidget::rescaleXAxisToYAxis);
    connect(Plot->xAxis, qOverload<const GPRange&>(&GPAxis::rangeChanged), this, &PlsCountPlotWidget::rescaleYAxisToXAxis);
    connect(PlotToolBar->SwitchScale, &QCheckBox::stateChanged, this, [&]()
      {
        Plot->xAxis->setScaleRatio(Plot->yAxis);
        Plot->xAxis->rescale();
        Plot->replot(GraphicsPlot::RefreshPriority::rpQueuedReplot);
      });
  }

  void PlsCountPlotWidget::OnExcludedStateChanged()
  {
    bool hasExcluded = Plot->getHasExcludedItems();
    PlotToolBar->CancelExcludingChoise->setVisible(hasExcluded);
  }

  void PlsCountPlotWidget::ShowPickAxis()
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    const auto varianceX = model->Get<DoubleVector>(PlsFitResponseModel::VarianceX);

    auto dial = new WebDialogPickAxisComponents(this, varianceX);
    dial->setCurrentXComponent(xAxis - 1);
    dial->setCurrentYComponent(yAxis - 1);
    connect(dial, &WebDialog::Accepted, this, [&, dial]()
      {
        auto comp = dial->GetComponents();
        SetAxisForPC(comp.first + 1, comp.second + 1);
        emit AxisChanged(comp.first + 1, comp.second + 1);
      });

    dial->Open();
  }

  void PlsCountPlotWidget::SetupCustomItemStyle()
  {
    const auto model = ObjectModel->GetModel();
    assert(model);

    CustomItemStyleMap map;
    model->Exec<MetaInfoList>(PlsFitResponseModel::T_TitleGrad,
      [&map](const MetaInfoList& list)
      {
        for (const auto& item : list)
        {
          map.insert(static_cast<int>(item.SampleId),
            CustomItemStyle
            {
                GPShape::ShapeType::Circle,
                GPShapeItem::DefaultColor::Green,
                QSizeF{ 7, 7 }
            });
        }
      });

    Plot->SetCustomItemStyle(map);
  }

  void PlsCountPlotWidget::rescaleXAxisToYAxis()
  {
    if (PlotToolBar->SwitchScale->checkState() == Qt::Checked)
      Plot->xAxis->setScaleRatio(Plot->yAxis);
  }

  void PlsCountPlotWidget::rescaleYAxisToXAxis()
  {
    if (PlotToolBar->SwitchScale->checkState() == Qt::Checked)
      Plot->yAxis->setScaleRatio(Plot->xAxis);
  }
}
