#include "pls_slope_offset_plot_widget.h"

#include "pls_statistic_widget.h"
#include "logic/structures/analysis_model_structures.h"
#include "logic/models/pls_fit_response_model.h"
#include "ui/dialogs/web_dialog_concentration_table.h"
#include "table_view_widget.h"
#include "ui/item_models/concentration_table_model.h"
#include "genesis_style/style.h"
#include "ui/contexts/slope_offset_context.h"
#include "ui/contexts/pls_setup_view_context.h"
#include <extern/common_gui/standard_widgets/tumbler.h>

namespace
{
  const std::string_view SampleId = "sample_id";
}

namespace Widgets
{
  using namespace Structures;
  using namespace Model;
  using namespace Dialogs;

  PlsSlopeOffsetPlotWidget::PlsSlopeOffsetPlotWidget(QWidget* parent)
    : PlsPlotWidget(parent)
  {
    SetupUi();
    ConnectSignals();
  }

  ConcetrantionTable PlsSlopeOffsetPlotWidget::GetConcentrationTable() const
  {
    ConcetrantionTable result;
    const auto plsModel = ObjectModel->GetModel();

    const auto titleGrad = plsModel->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleGrad);
    const auto components = plsModel->Get<PlsComponents>(PlsFitResponseModel::PlsComponents);
    const auto currentComponent = FindPlsComponentByIndex(CurrentComponent);
    if (!currentComponent.has_value())
      return {};

    if (currentComponent->Y_grad.size() == currentComponent->Y_prov.size())
    {
      for (int i = 0; i < titleGrad.size(); i++)
        result.push_back({ titleGrad[i].FileName, titleGrad[i].YConcentration, currentComponent->Y_grad[i],  currentComponent->Y_prov[i] });
    }
    else
    {
      assert(titleGrad.size() == currentComponent->Y_grad.size());

      for (int i = 0; i < titleGrad.size(); i++)
      {
        result.push_back({ titleGrad[i].FileName, titleGrad[i].YConcentration, currentComponent->Y_grad[i], 0 });
      }

      const auto titleProv = plsModel->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleProv);
      assert(titleProv.size() == currentComponent->Y_prov.size());

      for (int i = 0; i < titleProv.size(); i++)
      {
        result.push_back({ titleProv[i].FileName, titleProv[i].YConcentration, 0, currentComponent->Y_prov[i] });
      }
    }
    return result;
  }

  QList<GPAbstractItem*> PlsSlopeOffsetPlotWidget::GetItemsByFileId(const std::vector<int>& ids) const
  {
    QList<GPAbstractItem*> items;
    items.reserve(ids.size() * 2);
    for (const auto& item : ids)
    {
      const auto it = GradItems.find(item);
      if (it != GradItems.end())
        items.append(it->second);
    }
    auto model = ObjectModel->GetModel();
    const auto& predict = model->Get<std::string>(PlsFitResponseModel::Predict);
    if (predict != "test")
    {
      for (const auto& item : ids)
      {
        const auto it = ProvItems.find(item);
        if (it != ProvItems.end())
          items.append(it->second);
      }
    }
    return items;
  }

  void PlsSlopeOffsetPlotWidget::SetupUi()
  {
    if (const auto mainLayout = qobject_cast<QVBoxLayout*>(layout()); mainLayout)
    {
      PlotToolBar = QSharedPointer<PlotToolbarWidget>::create(this);
      PlotToolBar->SwitchScale->setVisible(false);
      mainLayout->addWidget(PlotToolBar.get());

      if (const auto ptLayout = PlotToolBar->layout(); ptLayout)
      {
        for (auto widget : QVector<QPushButton*>{
            PlotToolBar->PCButton ,
            PlotToolBar->AlterPCButton ,
            PlotToolBar->PickOtherAxisButton
          })
        {
          ptLayout->removeWidget(widget);
          delete widget;
        }
        const auto openConcentrationTable = new QPushButton(this);
        openConcentrationTable->setProperty("lightweight_brand", true);
        openConcentrationTable->setText(tr("Open concentration table"));
        connect(openConcentrationTable, &QPushButton::clicked, this, &PlsSlopeOffsetPlotWidget::onOpenConcentrationTable);
        ptLayout->addWidget(openConcentrationTable);
        auto showStatistics = new QPushButton(tr("Show statistics"));
        showStatistics->setProperty("lightweight_brand", true);
        ptLayout->addWidget(showStatistics);
        showStatistics->setCheckable(true);
        connect(showStatistics, &QPushButton::toggled, this, &PlsSlopeOffsetPlotWidget::onShowStatisticsStateChanged);
      }

      PlotToolBar->Caption->setText(tr("Slope-Offset plot"));
      PlotToolBar->Caption->setStyleSheet(Style::Genesis::Fonts::H2());

      mToolbarContext = new PlsSetupViewContext(PlotToolBar.data());
      // mToolbarContext->mResetAllStylesBtn->setVisible(false);
      // mToolbarContext->mResetAllStylesBtn->setVisible(false);
      mToolbarContext->mSelectUserStyleCheckBox->setVisible(false);

      mToolbarContext->mShowPassportInfoCheckBox->setEnabled(false);
      PlotToolBar->SetMenu(mToolbarContext);

      PlotToolBar->CancelExcludingChoise->setVisible(false);
      SlopeOffsetPlotWidget = new SlopeOffsetPlot(this);
      SlopeOffsetPlotWidget->setZoomModifiers(Qt::ControlModifier);
      SlopeOffsetPlotWidget->setAttribute(Qt::WA_NoMousePropagation, false);
      mainLayout->addWidget(SlopeOffsetPlotWidget);
      {
        auto plotOverlayVLayout = new QVBoxLayout();
        StatisticsOverlayLayout = new QHBoxLayout();

        //set desirable StatisticsOverlayLayout content margins here,
        //later in show event it will be added to axisRect content margins
        statisticWidgetMarginsFromAxisRect = { 0,0,20,20 };

        //plotOverlayLayout and StatisticsOverlayLayout content margins will not
        //updated in show event, recommended to leave it as zero margins
        StatisticsOverlayLayout->setContentsMargins(0, 0, 0, 0);
        plotOverlayVLayout->setContentsMargins(0, 0, 0, 0);
        plotOverlayVLayout->addStretch();
        plotOverlayVLayout->addLayout(StatisticsOverlayLayout);

        StatisticsOverlayLayout->addStretch();
        StatisticsWidget = new PlsStatisticWidget(SlopeOffsetPlotWidget);
        StatisticsOverlayLayout->addWidget(StatisticsWidget);

        SlopeOffsetPlotWidget->setLayout(plotOverlayVLayout);
        StatisticsWidget->hide();
      }
    }
  }

  void PlsSlopeOffsetPlotWidget::SetPCMaxCount(int maxCount)
  {
    CurrentComponent = maxCount;
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    SlopeOffsetPlotWidget->clearItems();
    SlopeOffsetPlotWidget->clearCurves();
    SlopeOffsetPlotWidget->ClearExcluded();
    GradItems.clear();
    ProvItems.clear();

    const auto currentComponent = FindPlsComponentByIndex(maxCount);
    if (!currentComponent.has_value())
      return;

    const auto yxGrad = model->Get<DoubleVector>(PlsFitResponseModel::YxGrad);
    assert(currentComponent->Y_grad.size() == yxGrad.size());
    const auto yxProv = model->Get<DoubleVector>(PlsFitResponseModel::YxProv);
    assert(currentComponent->Y_prov.size() == yxProv.size());
    const auto ttitleGrad = model->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleGrad);
    for (const auto& [sampleId, name] : PlotTitlesGrad)
    {
      const auto indexIt = std::find_if(ttitleGrad.begin(), ttitleGrad.end(),
        [&, sampleId = sampleId](const AnalysisMetaInfo& title) { return title.SampleId == static_cast<size_t>(sampleId); });
      const auto index = std::distance(ttitleGrad.begin(), indexIt);
      auto shape = CreateCircle( SlopeOffsetPlotWidget,
        { yxGrad[index], currentComponent->Y_grad[index] },
        name, GPShapeItem::DefaultColor::Blue);
      shape->setSelectable(true);
      shape->setProperty(SampleId.data(), sampleId);
      GradItems.emplace(sampleId, shape);
    }

    const auto ttitleProv = model->Get<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleProv);
    for (const auto& [sampleId, name] : PlotTitlesProv)
    {
      const auto indexIt = std::find_if(ttitleProv.begin(), ttitleProv.end(),
        [&, sampleId = sampleId](const AnalysisMetaInfo& title) { return title.SampleId == static_cast<size_t>(sampleId); });
      const auto index = std::distance(ttitleProv.begin(), indexIt);
      auto shape = CreateCircle(
        SlopeOffsetPlotWidget,
        { yxProv[index], currentComponent->Y_prov[index] },
        name, GPShapeItem::DefaultColor::Red);
      shape->setSelectable(true);
      shape->setProperty(SampleId.data(), sampleId);
      ProvItems.emplace(sampleId, shape);
    }

    {
      auto redLine = CreateLine(SlopeOffsetPlotWidget, 2,
        { 0, currentComponent->Statistic.Offset.Graduirovka },
        { 1, currentComponent->Statistic.Slope.Graduirovka
          + currentComponent->Statistic.Offset.Graduirovka }, GPShapeItem::DefaultColor::Blue);
      redLine->setSelectable(false);
    }

    {
      auto blueLine = CreateLine(SlopeOffsetPlotWidget, 2,
        { 0, currentComponent->Statistic.Offset.Proverka },
        { 1, currentComponent->Statistic.Slope.Proverka
          + currentComponent->Statistic.Offset.Proverka }, GPShapeItem::DefaultColor::Red);
      blueLine->setSelectable(false);
    }

    {
      auto grayLine = CreateLine(SlopeOffsetPlotWidget, 1, { 0, 0 }, { 1, 1 }, GPShapeItem::DefaultColor::Gray);
      grayLine->setSelectable(false);
    }

    QVector<double> yValues = { currentComponent->Y_grad.begin(), currentComponent->Y_grad.end() };
    yValues.append({ currentComponent->Y_prov.begin(), currentComponent->Y_prov.end() });

    SlopeOffsetPlotWidget->SetDefaultAxisRanges(CreateRangeFromVector(yxGrad + yxProv),
      CreateRangeFromVector(yValues));

    if(const auto stat = findChild<PlsStatisticWidget*>())
      stat->SetStatistic(&currentComponent->Statistic);
  }

  void PlsSlopeOffsetPlotWidget::excludeItems(const std::vector<int>& items) const
  {
    auto exclude = GetItemsByFileId(items);
    SlopeOffsetPlotWidget->excludeItems(exclude);
  }

  void PlsSlopeOffsetPlotWidget::resetExcludeItems(const std::vector<int>& items) const
  {
    auto reset = GetItemsByFileId(items);
    SlopeOffsetPlotWidget->resetExcludedItems(reset);
  }

  void PlsSlopeOffsetPlotWidget::onShowStatisticsStateChanged(bool showStatistics)
  {
    StatisticsWidget->setVisible(showStatistics);
  }

  void PlsSlopeOffsetPlotWidget::onOpenConcentrationTable(bool)
  {
    const auto dialog = new WebDialogConcentrationTable(this);
    dialog->SetConcentrationTable(GetConcentrationTable());
    dialog->Open();
  }

  void PlsSlopeOffsetPlotWidget::ConnectSignals()
  {
    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged,
      this, &PlsSlopeOffsetPlotWidget::SetupPlot);

    connect(mToolbarContext->mShowNamesCheckBox, &QCheckBox::stateChanged,
      this, [&](int state)
      {
        const auto gpItems = findChildren<GPShapeWithLabelItem*>();
        for (const auto& item : gpItems)
        {
          item->setLabelHidden(state == Qt::Unchecked);
        }
        SlopeOffsetPlotWidget->queuedReplot();
      });
    connect(PlotToolBar->CancelExcludingChoise, &QPushButton::clicked, this, [&]()
      {
        emit excludedItemsReset(SlopeOffsetPlotWidget->GetExcludedItemsIds());
        SlopeOffsetPlotWidget->resetExcludedItems();
      });

    connect(SlopeOffsetPlotWidget, &SlopeOffsetPlot::itemsExcluded, this, &PlsSlopeOffsetPlotWidget::itemsExcluded);
    connect(SlopeOffsetPlotWidget, &SlopeOffsetPlot::itemsExcluded, this, &PlsSlopeOffsetPlotWidget::excludeItems);

    connect(SlopeOffsetPlotWidget, &SlopeOffsetPlot::excludedItemsReset, this, &PlsSlopeOffsetPlotWidget::excludedItemsReset);
    connect(SlopeOffsetPlotWidget, &SlopeOffsetPlot::excludedItemsReset, this, &PlsSlopeOffsetPlotWidget::resetExcludeItems);

    connect(SlopeOffsetPlotWidget, &SlopeOffsetPlot::excludedStateChanged, this, [&](bool hasExcluded)
      {
        PlotToolBar->CancelExcludingChoise->setVisible(hasExcluded);
      });
  }

  void PlsSlopeOffsetPlotWidget::SetupPlot()
  {

    const auto model = ObjectModel->GetModel();
    assert(model);
    SlopeOffsetPlotWidget->yAxis->setLabel(tr("Prediction"));
    SlopeOffsetPlotWidget->xAxis->setLabel(tr("Layer concentration (Layer)"));
    PlotTitlesGrad.clear();
    PlotTitlesProv.clear();

    model->Exec<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleGrad,
      [&](const AnalysisMetaInfoList& infoList)
      {
        for (const auto& info : infoList)
        {
          const auto fileName = QString::fromStdString(info.FileName);
          const auto lof = fileName.lastIndexOf('/') + 1;
          PlotTitlesGrad.emplace(info.SampleId, fileName.mid(lof, fileName.count() - lof));
        }
      });

    model->Exec<AnalysisMetaInfoList>(PlsFitResponseModel::T_TitleProv,
      [&](const AnalysisMetaInfoList& infoList)
      {
        for (const auto& info : infoList)
        {
          const auto fileName = QString::fromStdString(info.FileName);
          const auto lof = fileName.lastIndexOf('/') + 1;
          PlotTitlesProv.emplace(info.SampleId, fileName.mid(lof, fileName.count() - lof));
        }
      });
  }
}


void Widgets::PlsSlopeOffsetPlotWidget::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  if (StatisticsOverlayLayout && SlopeOffsetPlotWidget)
  {
    auto margins = statisticWidgetMarginsFromAxisRect + SlopeOffsetPlotWidget->axisRect()->margins();
    StatisticsOverlayLayout->setContentsMargins(margins);
  }
}
