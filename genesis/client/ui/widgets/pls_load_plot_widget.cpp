#include "pls_load_plot_widget.h"

#include <ui/plots/load_plot.h>
#include <logic/models/pls_fit_response_model.h>
#include <logic/structures/analysis_model_structures.h>
#include "ui/contexts/pls_setup_view_context.h"
#include <extern/common_gui/standard_widgets/tumbler.h>

namespace Widgets
{
  using namespace Model;
  using namespace Structures;

  PlsLoadPlotWidget::PlsLoadPlotWidget(QWidget* parent)
    : PlsPlotWidget(parent)
  {
    SetupLoadPlot();
    ConnectSignals();
  }

  void PlsLoadPlotWidget::SetAxisForPC(int xPC, int yPC)
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    const auto pcCount = model->Get<PlsComponents>(PlsFitResponseModel::PlsComponents).size();

    yPC = std::min(yPC, static_cast<int>(pcCount));
    xPC = std::min(xPC, static_cast<int>(pcCount));

    const static size_t minimumPC = 3;
    yPC = (yPC == minimumPC && xPC == minimumPC && pcCount == minimumPC) ? yPC - 1 : yPC;
    xAxis = xPC;
    yAxis = yPC;
    const auto xComp = FindComponentByIndex(xPC);
    const auto yComponent = FindComponentByIndex(yPC);
    if (!xComp.has_value() || !yComponent.has_value())
      return;

    const auto& xComponent = TypeView == SelectedTypeView::SPECTRAL
      ? ReplacePtoIndex(xComp.value())
      : xComp.value();

    Plot->SetYAxisLabel(GenerateAxisLabel(yPC, yComponent->ExplPCsVariance));
    Plot->SetXAxisLabel(TypeView == SelectedTypeView::SPECTRAL
      ? tr("Variables")
      : GenerateAxisLabel(xPC, xComponent.ExplPCsVariance));

    Plot->SetYComponent(yPC);
    Plot->SetXComponent(xPC);

    Plot->clearData();

    const auto ttitle = model->Get<StringList>(PlsFitResponseModel::P_Title);
    QStringList plotTitles;
    std::transform(ttitle.begin(), ttitle.end(), std::back_inserter(plotTitles),
      [](const std::string& info) { return QString::fromStdString(info); });

    QMap<QString, int> pIds;
    for (const auto& info : plotTitles)
    {
      const auto splitInfo = info.split(' ');
      pIds[info] = splitInfo.back().toInt();
    }

    Plot->setPoints(plotTitles, pIds, xComponent, yComponent.value(), {});
    Plot->SetDefaultAxisRanges(CreateRangeFromVector(xComponent.P, 0.1),
      CreateRangeFromVector(yComponent->P, 0.1));
  }

  void PlsLoadPlotWidget::SetPCMaxCount(int maxCount)
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    MaxCount = maxCount;
    QPointF conectrationPoint;
    model->Exec<AnalysisMetaInfoWithPeaksList>(PlsFitResponseModel::T_TitleGrad,
      [&](const AnalysisMetaInfoWithPeaksList& list)
      {
        assert(MaxCount <= list.size());
        if (list[MaxCount - 1].YConcentration.has_value())
        {
          conectrationPoint.setX(list[MaxCount - 1].YConcentration.value());
          conectrationPoint.setY(MaxCount);
        }
      });

    if (!ConcentrationPoint)
    {
      ConcentrationPoint = new GPShapeWithLabelItem(
        Plot, conectrationPoint, GPShape::Circle, { 4 , 4 }, tr("Concentration"));

      ConcentrationPoint->setColor(GPShapeItem::DefaultColor::Red);
      ConcentrationPoint->setShapePen(Qt::NoPen);
      ConcentrationPoint->setSelectable(false);
      ConcentrationPoint->setPlatePen(Qt::NoPen);
    }
    ConcentrationPoint->SetPos(conectrationPoint);
    Plot->queuedReplot();
  }

  QStringList PlsLoadPlotWidget::GetExcludedPeaks() const
  {
    QStringList peaks;
    const auto items = Plot->GetExcludedItems();
    peaks.reserve(items.size());
    std::transform(items.begin(), items.end(), std::back_inserter(peaks),
      [&](GPShapeWithLabelItem* item) { return item->getLabelText(); });
    return peaks;
  }

  bool PlsLoadPlotWidget::HasExcludedItems() const
  {
    return Plot->getHasExcludedItems();
  }

  void PlsLoadPlotWidget::ExcludePeaks(const IntVector& peakIds)
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    if (Plot)
    {
      QStringList names;
      names.reserve(peakIds.size());
      const auto ids = model->Get<IntVector>(PlsFitResponseModel::P_id);
      model->Exec<StringList>(PlsFitResponseModel::P_Title,
        [&](const StringList& pnames)
        {
          for (const auto& id : peakIds)
          {
            const auto index = ids.indexOf(id);
            if(index > -1)
              names.append(QString::fromStdString(pnames[index]));
          }
        });
      Plot->resetExcludedItems(false);
      Plot->ExcludePeaks(names);
      OnExcludedStateChanged(!names.isEmpty());
    }
  }

  void PlsLoadPlotWidget::CancelExclude()
  {
    Plot->resetExcludedItems(false);
    OnExcludedStateChanged(false);
  }

  void PlsLoadPlotWidget::SetupLoadPlot()
  {
    Plot = new LoadPlot(this);
    Plot->setZoomModifiers(Qt::ControlModifier);
    Plot->setAttribute(Qt::WA_NoMousePropagation, false);
    Plot->setFlag(GP::AASettingsEdit, false);
    Plot->xAxis->setTickLength(0, 4);
    Plot->xAxis->setRange(0, 5);
    Plot->xAxis->grid()->setVisible(true);
    Plot->yAxis->setRange(0, 10);
    Plot->yAxis->setPadding(5);
    Plot->yAxis->grid()->setSubGridVisible(true);

    if (const auto mainLayout = qobject_cast<QVBoxLayout*>(layout()); mainLayout)
    {
      PlotToolBar = QSharedPointer<PlotToolbarWidget>::create(this);
      mainLayout->addWidget(PlotToolBar.get());
      mainLayout->addWidget(Plot);
      mainLayout->setStretchFactor(Plot, 1);

      auto layout = PlotToolBar->layout();
      layout->removeWidget(PlotToolBar->DisplayMenuButton);
      delete PlotToolBar->DisplayMenuButton;
      mShowNamesCheckBox = new Tumbler(tr("Show peak names"));
      mShowNamesCheckBox->setChecked(true);
      layout->addWidget(mShowNamesCheckBox);
      PlotToolBar->CancelExcludingChoise->setVisible(false);
      PlotToolBar->Caption->setVisible(false);
    }
    PlotToolBar->SwitchScale->setCheckState(Qt::Checked);
  }

  void PlsLoadPlotWidget::ConnectSignals()
  {
    connect(ObjectModel.get(), &ObjectWithModel::ModelChanged, this, [&]()
      {
        const auto model = ObjectModel->GetModel();
        if (model)
        {
          const auto pcCount = model->Get<PlsComponents>(PlsFitResponseModel::PlsComponents).size();
          PlotToolBar->SetNameForPCButtons(pcCount);
        }
      });

    connect(this, &PlsLoadPlotWidget::excludedStateChanged, this, &PlsLoadPlotWidget::OnExcludedStateChanged);

    connect(mShowNamesCheckBox, &QCheckBox::toggled, this,
      [this](bool checked) { Plot->showNames(checked); });

    connect(PlotToolBar->PCButton, &QPushButton::clicked, this, [&]()
      {
        SetAxisForPC(1, 2);
        emit AxisChanged(1, 2);
      });

    connect(PlotToolBar->AlterPCButton, &QPushButton::clicked, this, [&]()
      {
        SetAxisForPC(3, 4);
        emit AxisChanged(3, 4);
      });

    connect(PlotToolBar->PickOtherAxisButton, &QPushButton::clicked, this, &PlsLoadPlotWidget::ShowPickAxis);
    connect(PlotToolBar->CancelExcludingChoise, &QPushButton::clicked, this,
      [&]() { Plot->resetExcludedItems(); });

    connect(Plot, &LoadPlot::excludedStateChanged, this, &PlsLoadPlotWidget::OnExcludedStateChanged);
    connect(Plot, &LoadPlot::excludedStateChanged, this, &PlsLoadPlotWidget::excludedStateChanged);
    connect(Plot, &LoadPlot::AllItemsExcluded, this, &PlsLoadPlotWidget::AllItemsExcluded);
    connect(Plot, &LoadPlot::ResetExcludedPeaks, this, &PlsLoadPlotWidget::ResetExcludedPeaks);
    connect(Plot->yAxis, qOverload<const GPRange&>(&GPAxis::rangeChanged), this, &PlsLoadPlotWidget::rescaleXAxisToYAxis);
    connect(Plot->xAxis, qOverload<const GPRange&>(&GPAxis::rangeChanged), this, &PlsLoadPlotWidget::rescaleYAxisToXAxis);
    connect(PlotToolBar->SwitchScale, &QCheckBox::stateChanged, this, [&]()
      {
        Plot->xAxis->setScaleRatio(Plot->yAxis);
        Plot->xAxis->rescale();
        Plot->replot(GraphicsPlot::RefreshPriority::rpQueuedReplot);
      });
  }

  void PlsLoadPlotWidget::OnExcludedStateChanged(bool hasExcluded)
  {
    PlotToolBar->CancelExcludingChoise->setVisible(hasExcluded);
  }

  void PlsLoadPlotWidget::ShowPickAxis()
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    const auto varianceX = model->Get<DoubleVector>(PlsFitResponseModel::VarianceX);
    QList<Component> compList;
    model->Exec<PlsComponents>(PlsFitResponseModel::PlsComponents,
      [&](const PlsComponents& components)
      {
        for (const auto& item : components)
        {
          compList.append(
            {
              varianceX[item.Number - 1],
              { item.P.begin(), item.P.end() },
              { item.T.begin(), item.T.end() }
            });
        }
      });

    QStringList pcAxisNames;
    for (auto index = 0; index < compList.size(); index++)
    {
      pcAxisNames.append(tr("PC%1\ (%2\%)").arg(index + 1).arg(varianceX[index]));
    }

    auto dial = new WebDialogPlsLoadPlotPickAxis(this);
    dial->SetPCXAxisNames(pcAxisNames);
    dial->SetPCYAxisNames(pcAxisNames);

    dial->setPCXAxisIndex(xAxis - 1);
    dial->setPCYAxisIndex(yAxis - 1);
    connect(dial, &WebDialog::Accepted, this, [&, dial]()
      {
        TypeView = dial->getSelectedTypeView();
        if (TypeView == SelectedTypeView::SPECTRAL)
        {
          GraphicLineType = dial->getSpectralTypeGraphic();
          SetAxisForPC(dial->VerticalAxisYComboBoxValue() + 1,
            dial->VerticalAxisYComboBoxValue() + 1);
          emit AxisChanged(dial->VerticalAxisYComboBoxValue() + 1,
            dial->VerticalAxisYComboBoxValue() + 1);
        }
        else
        {
          SetAxisForPC(dial->Dim2XAxisCheckBoxValue() + 1,
            dial->Dim2YAxisCheckBoxValue() + 1);
          emit AxisChanged(dial->Dim2XAxisCheckBoxValue() + 1,
            dial->Dim2YAxisCheckBoxValue() + 1);
        }
      });
    dial->Open();
  }

  void PlsLoadPlotWidget::ResetExcludedPeaks(QStringList names)
  {
    const auto model = ObjectModel->GetModel();
    if (!model)
      return;

    std::vector<int> peaksNumbers;
    peaksNumbers.reserve(names.size());
    model->Exec<StringList>(PlsFitResponseModel::P_Title,
      [&](const StringList& pnames)
      {
        for (const auto& name : names)
        {
          const auto findIt = std::find(pnames.begin(), pnames.end(), name.toStdString());
          if (findIt != pnames.end())
            peaksNumbers.push_back(std::distance(pnames.begin(), findIt));
        }
      });
    emit ResetExcludedPeaksNumbers(peaksNumbers);
  }

  Component PlsLoadPlotWidget::ReplacePtoIndex(const Component& component)
  {
    QList<qreal> spectr(component.P.size());
    std::iota(spectr.begin(), spectr.end(), 1);
    return
    {
      component.ExplPCsVariance,
      QVector<double>::fromList(spectr),
      component.T
    };
  }

  void PlsLoadPlotWidget::rescaleXAxisToYAxis()
  {
    if (PlotToolBar->SwitchScale->checkState() == Qt::Checked)
      Plot->xAxis->setScaleRatio(Plot->yAxis);
  }

  void PlsLoadPlotWidget::rescaleYAxisToXAxis()
  {
    if (PlotToolBar->SwitchScale->checkState() == Qt::Checked)
      Plot->yAxis->setScaleRatio(Plot->xAxis);
  }
}
