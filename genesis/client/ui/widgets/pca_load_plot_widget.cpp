#include "pca_load_plot_widget.h"

// #include "logic/models/pca_analysis_response_model.h"
// #include "logic/structures/analysis_model_structures.h"
// #include "ui/plots/gp_items/gpshape_with_label_item.h"
#include <ui/plots/analysis_plot_refactor.h>
#include "graphicsplot/graphicsplot.h"
#include <ui/plots/gp_items/gpshape_with_label_item.h>
#include <extern/common_gui/standard_widgets/tumbler.h>
// using namespace Structures;
using namespace Dialogs;
using namespace AnalysisEntity;

namespace
{
  const QColor kGrey(0, 32, 51, 154);
}

namespace Widgets
{
  PcaLoadPlotWidget::PcaLoadPlotWidget(QWidget* parent)
    : PcaPlotWidget(parent)
  {
    SetupLoadPlot();
    ConnectSignals();
  }

  void PcaLoadPlotWidget::setModel(QPointer<AnalysisEntityModel> model)
  {
    QScopeGuard modelReset([&]() { onEntityModelReset(); });
    if (model && model == mEntityModel)
    {
      return;
    }

    PcaPlotWidget::setModel(model);
    mPlot->setModel(mEntityModel);
    connect(mEntityModel, &AnalysisEntityModel::modelDataChanged, this, &PcaLoadPlotWidget::onEntityModelDataChanged);
    connect(mEntityModel, &AnalysisEntityModel::entityChanged, this, &PcaLoadPlotWidget::onEntityModelChanged);
    connect(mEntityModel, &AnalysisEntityModel::modelAboutToReset, this, &PcaLoadPlotWidget::onEntityModelAboutToReset);
    connect(mEntityModel, &AnalysisEntityModel::modelReset, this, &PcaLoadPlotWidget::onEntityModelReset);
    PlotToolBar->SwitchScale->setCheckState(Qt::Checked);
  }

  void PcaLoadPlotWidget::onPlotContextMenuRequested(QContextMenuEvent* event)
  {
    if (mContextMenu)
      mContextMenu->clear();
    else
      mContextMenu = new QMenu(this);

    auto selected = mPlot->selectedItems();
    auto item = dynamic_cast<GPShapeWithLabelItem*>(mPlot->itemAt(event->pos(), true));
    auto excludedEntityCount = mEntityModel->countEntities(
      [](AbstractEntityDataModel::ConstDataPtr ent)->bool
      {
        return ent->getData(RoleExcluded).toBool();
      });

    if (!item && selected.empty())
    {
      //default plot context menu
      mContextMenu->setHidden(true);
      auto nameAct = mContextMenu->addAction(tr("Count plot"));
      nameAct->setEnabled(false);

    }
    else
    {
      mContextMenu->setHidden(false);
      if ((item && !selected.contains(item)) ||
        (!item && selected.size() == 1) ||
        (selected.contains(item) && selected.size() == 1)
        )
      {
        if (!item && selected.size() == 1)
          item = dynamic_cast<GPShapeWithLabelItem*>(selected.first());
        //item menu
        auto entity = mEntityModel->getEntity(item->property(AnalysisPlotRefactor::kEntityUid).toUuid());
        auto nameAct = mContextMenu->addAction(QTextDocumentFragment::fromHtml(item->getLabelText()).toPlainText());
        nameAct->setEnabled(false);
        mContextMenu->addSeparator();
        if (excludedEntityCount == 0)
        {
          QAction* pickForModel = mContextMenu->addAction(tr("Pick for new model"));
          connect(pickForModel, &QAction::triggered, this,
            [this, entity]()
            {
              for (auto& peak : mEntityModel->getEntities(TypePeak))
              {
                if (peak != entity)
                  mEntityModel->setEntityData(peak->getUid(), RoleExcluded, true);
              }
            });
        }

        if (entity->getData(RoleExcluded).toBool() == false)
        {
          QAction* excludeFromModel = mContextMenu->addAction(tr("Pick for excluding from model"));
          connect(excludeFromModel, &QAction::triggered, this,
            [this, entity]()
            {
              mEntityModel->setEntityData(entity->getUid(), RoleExcluded, true);
            });
          excludeFromModel->setEnabled(true);
        }
        else
        {
          QAction* reincludeToModel = mContextMenu->addAction(tr("Reset excluding from model"));
          connect(reincludeToModel, &QAction::triggered, this,
            [this, entity]()
            {
              mEntityModel->setEntityData(entity->getUid(), RoleExcluded, false);
            });
          reincludeToModel->setEnabled(true);
        }

      }
      else
      {
        //whole selected items context menu

        auto nameAct = mContextMenu->addAction(tr("PICKED %n PEAKS", "", selected.size()));
        QSet<AbstractEntityDataModel::ConstDataPtr> entList;

        for (auto& item : selected)
          entList << mEntityModel->getEntity(item->property(AnalysisPlotRefactor::kEntityUid).toUuid());

        nameAct->setEnabled(false);
        mContextMenu->addSeparator();
        bool bHasNonExcludedItem = false;
        for (auto& ent : entList)
        {
          if (ent->getData(RoleExcluded).toBool() == false)
          {
            bHasNonExcludedItem = true;
            break;
          }
        }
        if (bHasNonExcludedItem)
        {
          QAction* excludeFromModel = mContextMenu->addAction(tr("Pick for excluding from model"));

          connect(excludeFromModel, &QAction::triggered, this,
            [this, entList]()
            {
              for (auto& ent : entList)
              {
                mEntityModel->setEntityData(ent->getUid(), RoleExcluded, true);
              }
            });
          excludeFromModel->setEnabled(true);
        }
        if (excludedEntityCount == 0)
        {
          QAction* pickForModel = mContextMenu->addAction(tr("Pick for new model"));

          connect(pickForModel, &QAction::triggered, this,
            [this, entList]()
            {
              for (auto& entity : mEntityModel->getEntities(TypePeak))
              {
                if (!entList.contains(entity))
                  mEntityModel->setEntityData(entity->getUid(), RoleExcluded, true);
              }
            });
        }
        bool bHasExcludedItems = false;
        for (auto& entity : mEntityModel->getEntities(TypePeak))
        {
          if (entity->getData(RoleExcluded).toBool())
          {
            bHasExcludedItems = true;
            break;
          }
        }
        if (bHasExcludedItems)
        {
          QAction* reincludeToModel = mContextMenu->addAction(tr("Reset excluding from model"));

          connect(reincludeToModel, &QAction::triggered, this,
            [this, entList]()
            {
              for (auto& ent : entList)
                mEntityModel->setEntityData(ent->getUid(), RoleExcluded, false);
            });
          reincludeToModel->setEnabled(true);
        }
      }
    }
    mContextMenu->popup(event->globalPos());
    event->accept();
  }

  void PcaLoadPlotWidget::SetupLoadPlot()
  {
    mPlot = new AnalysisPlotRefactor(this, { TypePeak });
    // mPlot->installEventFilter(this);
    mPlot->setAttribute(Qt::WA_NoMousePropagation, false);
    mPlot->setZoomModifiers(Qt::ControlModifier);
    // Plot->setFlag(GP::AASettingsEdit, false);
    mPlot->xAxis->setTickLength(0, 4);
    mPlot->xAxis->setRange(0, 5);
    mPlot->yAxis->setRange(0, 10);
    mPlot->yAxis->setPadding(5);
    mPlot->yAxis->grid()->setSubGridVisible(false);
    mPlot->xAxis->grid()->setSubGridVisible(false);
    mPlot->xAxis->setBasePen(kGrey);
    mPlot->yAxis->setBasePen(kGrey);
    mPlot->xAxis->setTickPen(kGrey);
    mPlot->yAxis->setTickPen(kGrey);
    mPlot->xAxis->setSubTickPen(kGrey);
    mPlot->yAxis->setSubTickPen(kGrey);
    mPlot->xAxis->setTickLabelColor(kGrey);
    mPlot->yAxis->setTickLabelColor(kGrey);
    mPlot->xAxis->setLabelColor(kGrey);
    mPlot->yAxis->setLabelColor(kGrey);
    auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    PlotToolBar = QSharedPointer<PlotToolbarWidget>::create(this);
    PlotToolBar->Caption->setVisible(false);
    mainLayout->addWidget(PlotToolBar.get());
    mainLayout->addWidget(mPlot);
    mainLayout->setStretchFactor(mPlot, 1);
    auto layout = PlotToolBar->layout();
    layout->removeWidget(PlotToolBar->DisplayMenuButton);
    delete PlotToolBar->DisplayMenuButton;
    mShowNamesCheckBox = new Tumbler(tr("Show peak names"));
    mShowNamesCheckBox->setChecked(true);
    layout->addWidget(mShowNamesCheckBox);
    PlotToolBar->CancelExcludingChoise->setVisible(false);
  }

  void PcaLoadPlotWidget::ConnectSignals()
  {
    connect(PlotToolBar->CancelExcludingChoise, &QPushButton::clicked, this,
      [&]()
      {
        if (!mEntityModel)
          return;
        mEntityModel->cancelExclude(TypePeak);
      });

    connect(mShowNamesCheckBox, &QCheckBox::toggled, this, [this](bool checked)
      {
        mPlot->showNames(checked);
      });

    connect(PlotToolBar->PCButton, &QPushButton::clicked, this,
      [&]()
      {
        if (!mEntityModel)
          return;
        mEntityModel->setModelData(ModelRoleIsSpectral, false);
        mEntityModel->setCurrentXYComponent(1, 2);
        mPlot->resize();
      });

    connect(PlotToolBar->AlterPCButton, &QPushButton::clicked, this,
      [&]()
      {
        if (!mEntityModel)
          return;
        mEntityModel->setModelData(ModelRoleIsSpectral, false);
        mEntityModel->setCurrentXYComponent(3, 4);
        mPlot->resize();
      });

    connect(PlotToolBar->PickOtherAxisButton, &QPushButton::clicked, this, &PcaLoadPlotWidget::ShowPickAxis);
    connect(mPlot, &AnalysisPlotRefactor::contextMenuRequested, this, &PcaLoadPlotWidget::onPlotContextMenuRequested);
    connect(mPlot->yAxis, qOverload<const GPRange&>(&GPAxis::rangeChanged), this, &PcaLoadPlotWidget::rescaleXAxisToYAxis);
    connect(mPlot->xAxis, qOverload<const GPRange&>(&GPAxis::rangeChanged), this, &PcaLoadPlotWidget::rescaleYAxisToXAxis);
    connect(PlotToolBar->SwitchScale, &QCheckBox::stateChanged, this, [&]()
      {
        mPlot->xAxis->setScaleRatio(mPlot->yAxis);
        mPlot->xAxis->rescale();
        mPlot->replot(GraphicsPlot::RefreshPriority::rpQueuedReplot);
      });
  }

  void PcaLoadPlotWidget::ShowPickAxis()
  {
    if (!mEntityModel)
      return;

    QList<double> compList;
    auto varianceMap = mEntityModel->modelData(ModelRoleComponentsExplVarianceMap)
      .value<TComponentsExplVarianceMap>();
    compList.resize(varianceMap.size(), -1);
    for (auto iter = varianceMap.constKeyValueBegin(); iter != varianceMap.constKeyValueEnd(); iter++)
      compList[iter->first - 1] = iter->second;

    QStringList pcAxisNames;
    for (auto index = 0; index < compList.size(); index++)
    {
      pcAxisNames.append(tr("PC%1 (%2\%)").arg(index + 1).arg(compList[index]));
    }
    auto dial = new WebDialogPlsLoadPlotPickAxis(this);
    dial->SetPCXAxisNames(pcAxisNames);
    dial->setPCXAxisIndex(mEntityModel->modelData(ModelRoleCurrentXComponentNum).toInt() - 1);
    dial->SetPCYAxisNames(pcAxisNames);
    dial->setPCYAxisIndex(mEntityModel->modelData(ModelRoleCurrentYComponentNum).toInt() - 1);
    dial->setSelectedTypeView(
      mEntityModel->modelData(ModelRoleIsSpectral).toBool()
      ? SelectedTypeView::SPECTRAL
      : SelectedTypeView::DIMENSION2D);
    dial->setSpectralTypeGraphic(
      mEntityModel->modelData(ModelRoleSpectralRenderType).value<TSpectralRenderType>() == Line
      ? SpectralGraphicType::LINE_STYLE
      : SpectralGraphicType::DOT_STYLE);
    connect(dial, &WebDialog::Accepted, this, [&, dial]()
      {
        mEntityModel->setModelData(ModelRoleIsSpectral, dial->getSelectedTypeView() == SelectedTypeView::SPECTRAL);
        mEntityModel->setModelData(ModelRoleSpectralRenderType, (dial->getSpectralTypeGraphic() == SpectralGraphicType::DOT_STYLE)
          ? RenderType::Dot
          : RenderType::Line);
        if (dial->getSelectedTypeView() == SelectedTypeView::SPECTRAL)
        {
          mEntityModel->setCurrentSpectralComponent(dial->VerticalAxisYComboBoxValue() + 1);
        }
        else
        {
          mEntityModel->setCurrentXYComponent(dial->Dim2XAxisCheckBoxValue() + 1,
            dial->Dim2YAxisCheckBoxValue() + 1);
        }
      });
    dial->Open();
  }

  void PcaLoadPlotWidget::onEntityModelDataChanged(const QHash<int, QVariant>&)
  {
    if (!mEntityModel)
      return;
    auto varianceMap = mEntityModel->modelData(ModelRoleComponentsExplVarianceMap).value<TComponentsExplVarianceMap>();

    const auto xPC = mEntityModel->currentXComponent();
    mPlot->setXAxisLabel(mEntityModel->modelData(ModelRoleIsSpectral).toBool()
      ? tr("Variables")
      : generateAxisLabel(xPC, varianceMap[xPC]));
    const auto yPC = mEntityModel->currentYComponent();
    mPlot->setYAxisLabel(generateAxisLabel(yPC, varianceMap[yPC]));
    PlotToolBar->SetNameForPCButtons(varianceMap.size());
  }

  void PcaLoadPlotWidget::onEntityModelChanged(EntityType type, TEntityUid, DataRoleType role, const QVariant&)
  {
    if (role != RoleExcluded && type != TypePeak)
      return;
    const auto excludedCount = mEntityModel->countEntities(
      [](AbstractEntityDataModel::ConstDataPtr ent)->bool
      {
        return ent->getData(RoleExcluded).toBool() && ent->getType() == TypePeak;
      });
    PlotToolBar->CancelExcludingChoise->setVisible(excludedCount > 0);
  }

  void PcaLoadPlotWidget::onEntityModelAboutToReset()
  {
    if (!mEntityModel)
      return;
    disconnect(mEntityModel, &AnalysisEntityModel::modelDataChanged, this, &PcaLoadPlotWidget::onEntityModelDataChanged);
  }

  void PcaLoadPlotWidget::onEntityModelReset()
  {
    if (!mEntityModel)
      return;
    connect(mEntityModel, &AnalysisEntityModel::modelDataChanged, this, &PcaLoadPlotWidget::onEntityModelDataChanged);
    onEntityModelDataChanged(mEntityModel->modelData());
    onEntityModelChanged(TypeSample, QUuid(), RoleExcluded, QVariant());
  }

  void PcaLoadPlotWidget::rescaleXAxisToYAxis()
  {
    if(PlotToolBar->SwitchScale->checkState() == Qt::Checked)
      mPlot->xAxis->setScaleRatio(mPlot->yAxis);
  }

  void PcaLoadPlotWidget::rescaleYAxisToXAxis()
  {
    if (PlotToolBar->SwitchScale->checkState() == Qt::Checked)
      mPlot->yAxis->setScaleRatio(mPlot->xAxis);
  }

  // Component PcaLoadPlotWidget::ReplacePtoIndex(const Component& component)
  // {
  //   QList<qreal> spectr(component.P.size());
  //   std::iota(spectr.begin(), spectr.end(), 1);
  //   return
  //   {
  //     component.ExplPCsVariance,
  //     QVector<double>::fromList(spectr),
  //     component.T
  //   };
  // }
}
