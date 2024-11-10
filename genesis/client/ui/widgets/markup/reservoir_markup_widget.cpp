#include "reservoir_markup_widget.h"
// #include "ui/widgets/markup/chromatogramms_tab_widget.h"

#include <genesis_style/style.h>
#include <ui/widgets/markup/chromatogram_widget_refactor.h>
#include <ui/widgets/markup/gp_stackedwidget.h>
#include <logic/markup/i_markup_tab.h>
#include <logic/markup/markup_step_controller.h>
#include <graphicsplot/graphicsplot.h>
#include <ui/plots/chromatogram_plot.h>

#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QScrollArea>
#include <QTabWidget>
#include <QGraphicsDropShadowEffect>

////you can change only kDesirableScrollBarResolution and kDataMarginInPercents for your purpose.
/// DO NOT change the others, if you don't know, how it used in code!
const double kDesirableScrollBarResolution = 10000;
const double kDataMarginInPercents = 0.02;//the margin for displaying curve with margin from left and right viewport borders. 0.02 (2%) - is the default value.
  //affects both of scrollBar and plot x axis
const double kActualScrollBarMargin = kDesirableScrollBarResolution * kDataMarginInPercents;//precalculated margin value for scroll bar range
const double kActualScrollBarResolution = kDesirableScrollBarResolution + kActualScrollBarMargin * 2;//precalculated expanded by margin actual scroll bar range

class ShowHideSignalizableScrollBarPrivate : public QScrollBar
{
  Q_OBJECT
public:
  ShowHideSignalizableScrollBarPrivate(Qt::Orientation orientation, QWidget* parent)
    : QScrollBar(orientation, parent){};

signals:
  void visibilityChanged(bool isVisible);
  // QWidget interface
protected:
  void showEvent(QShowEvent *event) override{
    QScrollBar::showEvent(event);
    emit visibilityChanged(true);
  }
  void hideEvent(QHideEvent *event) override{
    QScrollBar::hideEvent(event);
    emit visibilityChanged(false);
  }
};

namespace GenesisMarkup
{

ReservoirMarkupWidget::ReservoirMarkupWidget(QWidget *parent) :
  QWidget(parent),
  mVScrollBarTimer(new QTimer(this)),
  mXAxisUptateTimer(new QTimer(this)),
  mYAxisUptateTimer(new QTimer(this)),
  mMasterRightSpacer(nullptr),
  mWholeDataRangeX(new GPRange()),
  mWholeDataRangeY(new GPRange()),
  mXAxisViewportRange(new GPRange()),
  mYAxisViewportRange(new GPRange()),
  mPlotPixelHeight(280)
{
  setupUi();
  mXAxisUptateTimer->setSingleShot(true);
  mXAxisUptateTimer->setInterval(500);
  mXAxisUptateTimer->callOnTimeout(this, &ReservoirMarkupWidget::onXAxisRangeChangedTimeout);
  mYAxisUptateTimer->setSingleShot(true);
  mYAxisUptateTimer->setInterval(500);
  mYAxisUptateTimer->callOnTimeout(this, &ReservoirMarkupWidget::onYAxisRangeChangedTimeout);
  mVScrollBarTimer->setSingleShot(true);
  connect(mVScrollBarTimer, &QTimer::timeout,
          this, &ReservoirMarkupWidget::onOpenGlUpdateRequested,
          Qt::ConnectionType::QueuedConnection);
}

ReservoirMarkupWidget::~ReservoirMarkupWidget()
{
}

void ReservoirMarkupWidget::addTab(IMarkupTab *tab, const QString& title)
{
  mAdditionalTabs << tab;
  mTabWidget->addTab(tab, title);
  connect(tab, &IMarkupTab::newCommand, this, &ReservoirMarkupWidget::newCommand);
  if(mSteps.contains(mCurrentMarkupModel) && !tab->isVisibleOnStep(mSteps[mCurrentMarkupModel]))
    mTabWidget->setTabVisible(mTabWidget->count() - 1, false);
  tab->setMarkupModel(mCurrentMarkupModel);
}

void ReservoirMarkupWidget::removeTab(int index)
{
  auto tab = mAdditionalTabs.takeAt(index);
  tab->deleteLater();
}

void ReservoirMarkupWidget::removeTab(IMarkupTab *tab)
{
  for(auto& wgt : mAdditionalTabs)
  {
    if(wgt == tab)
    {
      mAdditionalTabs.removeOne(wgt);
      tab->deleteLater();
      return;
    }
  }
}

void ReservoirMarkupWidget::setMainMarkupModel(MarkupModelPtr model, const StepInfo &stepInfo)
{
  mCurrentMarkupModel = model;
  setMarkupModel(mMainMarkupModel, model, stepInfo);
}

void ReservoirMarkupWidget::setSubMarkupModel(MarkupModelPtr model, const StepInfo &stepInfo)
{
  if(!model)
  {
    mCurrentMarkupModel = mMainMarkupModel;
    setMarkupModel(mSubMarkupModel, model, stepInfo);
    for(auto& wgt : mAdditionalTabs)
      wgt->setMarkupModel(mCurrentMarkupModel);
    for(auto& wgt : mChromatogrammsWidgets[mMainMarkupModel])
      wgt->setInteractionDisabled(false);
    return;
  }
  else
  {
    mCurrentMarkupModel = model;
    setMarkupModel(mSubMarkupModel, model, stepInfo);
    for(auto& wgt : mChromatogrammsWidgets[mMainMarkupModel])
      wgt->setInteractionDisabled(true);
  }
}

void ReservoirMarkupWidget::setPlotRanges(const QPair<GPRange, GPRange> &ranges)
{
  *mXAxisViewportRange = ranges.first;
  *mYAxisViewportRange = ranges.second;
  for(auto& group : mChromatogrammsWidgets)
  {
    for(auto& wgt : group)
    {
      QSignalBlocker sb(wgt);
      wgt->setXAxisRange(*mXAxisViewportRange);
      wgt->setYAxisRange(*mYAxisViewportRange);
    }
  }
  updateCommonScrollbar();
}

void ReservoirMarkupWidget::setModule(Names::ModulesContextTags::Module module)
{
  for(auto& wgt : mAdditionalTabs)
    wgt->setModule(module);
}

void ReservoirMarkupWidget::setChromaHeight(int height)
{
  mPlotPixelHeight = height;
  for(auto& chromaGroup : mChromatogrammsWidgets)
  {
    for(auto& chroma : chromaGroup)
    {
      chroma->setFixedHeight(mPlotPixelHeight);
    }
  }
  mVScrollBarTimer->start(0);
}

int ReservoirMarkupWidget::chromaHeight()
{
  return mPlotPixelHeight;
}

QPair<GPRange, GPRange> ReservoirMarkupWidget::wholeDataRange()
{
  return {*mWholeDataRangeX, *mWholeDataRangeY};
}

void ReservoirMarkupWidget::clear()
{
  for(auto& modelWgts : mChromatogrammsWidgets)
  {
    for(auto& wgt : modelWgts)
      wgt->deleteLater();
  }
  mChromatogrammsWidgets.clear();
  mMainMarkupModel = nullptr;
  mSubMarkupModel = nullptr;
  mCurrentMarkupModel = nullptr;
  mSteps.clear();
  for(auto& tab : mAdditionalTabs)
    tab->setMarkupModel(nullptr);
  for(int i = 1; i < mTabWidget->count(); i++)
    mTabWidget->setTabVisible(i, false);
}

// ChromatogramWidgetRefactor *ReservoirMarkupWidget::getFirstChroma()
// {
//   return qobject_cast<ChromatogramWidgetRefactor*>(mPlotsWidgetLayout->itemAt(0)->widget());
// }

void ReservoirMarkupWidget::onActionsStateChanged(bool enabled)
{
  mUndoAction->setEnabled(enabled);
  mRedoAction->setEnabled(enabled);
}

void ReservoirMarkupWidget::onUndoActionStateChanged(bool enabled)
{
  mUndoAction->setEnabled(enabled);
}

void ReservoirMarkupWidget::onRedoActionStateChanged(bool enabled)
{
  mRedoAction->setEnabled(enabled);
}

void ReservoirMarkupWidget::setupUi()
{
  setStyleSheet(Style::Genesis::GetUiStyle());
  auto mainLayout = new QVBoxLayout(this);
  setLayout(mainLayout);
  mTabWidget = new QTabWidget(this);
  mTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  mainLayout->addWidget(mTabWidget);
  ///Plots
  {
    mPlotsAreaWidget = new QWidget(mTabWidget);
    mPlotsAreaWidgetLayout = new QVBoxLayout();
    mPlotsAreaWidgetLayout->setContentsMargins(0,0,0,0);
    mPlotsAreaWidgetLayout->setSpacing(0);
    mMasterChromaLayout = new QHBoxLayout();
    mMasterChromaLayout->setContentsMargins(0,0,0,0);//to align widget with in-scroll-area widgets
    mMasterChromaLayout->setSpacing(0);
    mMasterRightSpacer = new QSpacerItem(0,0);
    mMasterChromaLayout->addSpacerItem(mMasterRightSpacer);
    mPlotsAreaWidgetLayout->addLayout(mMasterChromaLayout);
    mPlotsAreaWidget->setLayout(mPlotsAreaWidgetLayout);
    mTabWidget->addTab(mPlotsAreaWidget, tr("Chromatogramm"));
    connect(mTabWidget, &QTabWidget::currentChanged, this,
            [this](int current)
            {
              //костыль, потому что не придумал пока как сделать
              //обновление графика эталона при возвращении с таба
              //идентификации
              if(current == 0 && mCurrentMarkupModel &&
                  mChromatogrammsWidgets.contains(mCurrentMarkupModel) &&
                  mCurrentMarkupModel->hasMaster() &&
                  mChromatogrammsWidgets[mCurrentMarkupModel].contains(mCurrentMarkupModel->getMaster()->id()))
                mChromatogrammsWidgets[mCurrentMarkupModel][mCurrentMarkupModel->getMaster()->id()]->getPlot()->replotOnVisibleViewPort();
            });

    mPlotsScrollArea = new QScrollArea(mPlotsAreaWidget);
    mPlotsScrollArea->setFrameShape(QFrame::Box);
    mPlotsScrollArea->setStyleSheet("QWidget{background-color: white; border: none;}");
    mPlotsScrollArea->setWidgetResizable(true);

    auto vScroll = new ShowHideSignalizableScrollBarPrivate(Qt::Vertical, mPlotsScrollArea);
    mPlotsScrollArea->setVerticalScrollBar(vScroll);
    connect(vScroll, &ShowHideSignalizableScrollBarPrivate::visibilityChanged,
            this, &ReservoirMarkupWidget::onChromaVScrollBarVisibilityChanged, Qt::QueuedConnection);
    auto widget = new QWidget(mPlotsScrollArea);
    widget->setAccessibleName("scrollAreaWgt");
    widget->setObjectName("scrollAreaWgt");
    QString scrollAreaWgtStyle =
      "\nQWidget#scrollAreaWgt\n"
      "{\n"
      "  background-color: white;\n"
      "  border: none;\n"
      "}\n"
      "\nQPushButton\n"
      "{\n"
      "  background-color: white\n"
      "}\n";
    widget->setStyleSheet(scrollAreaWgtStyle);

    //// Stack
    mPlotsScrollAreaWidgetLayout = new QVBoxLayout(widget);
    widget->setLayout(mPlotsScrollAreaWidgetLayout);
    mPlotsScrollAreaWidgetLayout->setContentsMargins(0,0,0,0);
    mPlotsScrollAreaWidgetLayout->setSpacing(0);

    //// Plots
    {
      mPlotsWidget = new QWidget(widget);
      auto plotsWidgetsWithSpacingLayout = new QVBoxLayout(mPlotsWidget);
      plotsWidgetsWithSpacingLayout->setContentsMargins(0,0,0,0);
      mPlotsWidget->setLayout(plotsWidgetsWithSpacingLayout);
      mPlotsWidgetLayout = new QVBoxLayout();
      mPlotsWidgetLayout->setContentsMargins(0,0,0,0);
      mPlotsWidgetLayout->setSpacing(0);
      plotsWidgetsWithSpacingLayout->addLayout(mPlotsWidgetLayout);
      plotsWidgetsWithSpacingLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
      plotsWidgetsWithSpacingLayout->addSpacing(0);
      mPlotsScrollAreaWidgetLayout->addWidget(mPlotsWidget);
      mPlotsScrollArea->setWidget(widget);
    }
    mPlotsAreaWidgetLayout->addWidget(mPlotsScrollArea);

    mCommonScrollBar = new QScrollBar(Qt::Horizontal, this);
    mPlotsAreaWidgetLayout->addWidget(mCommonScrollBar);

    auto tabWidgetOverlayLayout = new QGridLayout();
    mTabWidget->setLayout(tabWidgetOverlayLayout);
    tabWidgetOverlayLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
    tabWidgetOverlayLayout->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0);
    mPlotsToolBarOverlay = new QWidget(nullptr);
    mPlotsToolBarOverlay->setStyleSheet(".QWidget{border-radius: 4px; border: none; background-color: white;}"
                                        "QToolButton {"
                                        "outline:            transparent;"
                                        "font:               @RegularTextFont;"
                                        "padding:            0px;"
                                        "color:              @dialogBoxButtonSecondaryFg;"
                                        "background-color:   white;"
                                        "border-color:       transparent;"
                                        "border-width:       0px;"
                                        "border-radius:      @borderRadius;"
                                        "border-style:       none;"
                                        "}");
    mPlotsToolBarOverlay->setLayout(new QHBoxLayout());
    mPlotsToolBarOverlay->layout()->setContentsMargins(10, 12, 10, 12);
    mPlotsToolBarOverlay->layout()->setSpacing(0);
    auto shadow = new QGraphicsDropShadowEffect(mPlotsToolBarOverlay);
    shadow->setBlurRadius(24);
    shadow->setOffset(0,8);
    shadow->setColor(QColor(0, 32, 51, 31));
    mPlotsToolBarOverlay->setGraphicsEffect(shadow);
    mPlotsPageToolBar = new QToolBar(mPlotsToolBarOverlay);
    mPlotsToolBarOverlay->layout()->addWidget(mPlotsPageToolBar);
    mPlotsPageToolBar->setIconSize({23, 20});
    //actions setup:
    mUndoAction = new QAction(mPlotsPageToolBar);
    mUndoAction->setIcon(QIcon("://resource/icons/icon_revert.png"));
    mUndoAction->setToolTip(tr("Undo"));
    mUndoAction->setShortcut(QKeySequence(QKeyCombination(Qt::ControlModifier, Qt::Key_Z)));
    connect(mUndoAction, &QAction::triggered, this, &ReservoirMarkupWidget::onUndo);
    mPlotsPageToolBar->addAction(mUndoAction);

    mRedoAction = new QAction(mPlotsPageToolBar);
    mRedoAction->setIcon(QIcon("://resource/icons/icon_repeat.png"));
    mRedoAction->setToolTip(tr("Redo"));
    mRedoAction->setShortcut(QKeySequence(QKeyCombination(Qt::ControlModifier, Qt::Key_Y)));
    connect(mRedoAction, &QAction::triggered, this, &ReservoirMarkupWidget::onRedo);
    mPlotsPageToolBar->addAction(mRedoAction);

    mPlotsToolBarOverlay->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    tabWidgetOverlayLayout->addWidget(mPlotsToolBarOverlay, 1, 1);
  }
}

ChromatogramWidgetRefactor *ReservoirMarkupWidget::makeChomatogramWidget(ChromaId id)
{
  auto newWidget = new ChromatogramWidgetRefactor(id, mPlotPixelHeight, this);
  newWidget->setXAxisRange(*mXAxisViewportRange);
  newWidget->setYAxisRange(*mYAxisViewportRange);
  {
    auto passportData = mCurrentMarkupModel->controller()->passportsModel().getPassport(id);
    if(!passportData)
    {
      newWidget->setName(tr("#$%@!NAME ERROR! NO PASSPORT DATA!@%$#"));
    }
    else
    {
      auto nameIter = passportData->constFind(PassportDataRoles::PassportFilename);
      if(nameIter != passportData->end())
        newWidget->setName(nameIter->toString());
    }
  }

  connect(newWidget, &ChromatogramWidgetRefactor::xAxisRangeChanged, this, &ReservoirMarkupWidget::onXAxisRangeChanged);
  connect(newWidget, &ChromatogramWidgetRefactor::yAxisRangeChanged, this, &ReservoirMarkupWidget::onYAxisRangeChanged);

  connect(mPlotsScrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, &ReservoirMarkupWidget::onScrollBarMoved);

  connect(newWidget, &ChromatogramWidgetRefactor::newCommand, this, &ReservoirMarkupWidget::newCommand);
  connect(newWidget, &ChromatogramWidgetRefactor::remarkupCalled, this, &ReservoirMarkupWidget::remarkupCalled);
  connect(mCommonScrollBar, &QScrollBar::valueChanged, newWidget, [this, newWidget](int value)
          {
            double leftMargin = mWholeDataRangeX->size() * kDataMarginInPercents;
            GPRange newRange(newWidget->xAxisRange());
            double actualValue = (double)value * mScrollNormalizeCoefficient - leftMargin + mWholeDataRangeX->lower;
            double delta = actualValue - newRange.lower;
            newRange.lower = actualValue;
            newRange.upper += delta;// move upper border to save window size
            QSignalBlocker sb(newWidget);
            newWidget->setXAxisRange(newRange);
          });
  connect(newWidget, &ChromatogramWidgetRefactor::intervalsApplyingRequested, this,
          [this](QList<GenesisMarkup::ChromaId> chromaIds,
                 QMap<QUuid, GenesisMarkup::ChromaSettings> intervalsSettings,
                 QMap<QUuid, QPair<double, double>> deprecationIntervals,
                 GenesisMarkup::ChromaSettings chromaSettings,
                 Steps desiredStep)
          {
            mCurrentMarkupModel->controller()->applyIntervals(chromaIds,
                                                      intervalsSettings,
                                                      deprecationIntervals,
                                                      chromaSettings,
                                                      desiredStep);
          });
  connect(newWidget, &ChromatogramWidgetRefactor::intervalsModeStateChanged, this, &GenesisMarkup::ReservoirMarkupWidget::updateIntervalsMode);

  mPlotsWidgetLayout->addWidget(newWidget);
  return newWidget;
}

void ReservoirMarkupWidget::setMarkupModel(MarkupModelPtr& dst, MarkupModelPtr src, const StepInfo &stepInfo)
{
  for(auto& tab : mAdditionalTabs)
  {
    tab->setMarkupModel(src);
    auto tabIndex = mTabWidget->indexOf(tab);
    bool visible = tab->isVisibleOnStep(stepInfo);
    mTabWidget->setTabVisible(tabIndex, visible);
  }

  QList<ChromaId> existedIds;
  QList<ChromaId> desiredIds;
  if(dst)
  {
    existedIds = dst->getChromatogrammsIdList();
    disconnect(dst.get(), nullptr, this, nullptr);
  }
  if(src)
    desiredIds = src->getChromatogrammsIdList();

  auto chromaArray = mChromatogrammsWidgets.take(dst);
  mSteps.remove(dst);
  dst = src;
  mSteps[dst] = stepInfo;

  if (!src){
    for(auto& wgt : chromaArray)
      delete wgt;
    chromaArray.clear();
    return;
  }

  if (chromaArray.isEmpty())
  {// case, when first widgets creation
    for (auto& id : desiredIds)
    {
      auto newChroma = makeChomatogramWidget(id);
      if(chromaArray.value(id, nullptr))
        chromaArray[id]->deleteLater();
      chromaArray[id] = newChroma;
      newChroma->setMarkupModel(dst, stepInfo);
    }
    updateIntervalsMode();
    onStartupLoadPlots();
  }
  else
  {
    QSet<ChromaId> existedSet(existedIds.begin(), existedIds.end());
    QSet<ChromaId> desiredSet(desiredIds.begin(), desiredIds.end());
    auto toDelete = existedSet.subtract(desiredSet);
    auto toCreate = desiredSet.subtract(existedSet);

    for(auto& id : toDelete)
    {
      chromaArray[id]->deleteLater();
      chromaArray.remove(id);
    }

    for(auto& id : toCreate)
    {
      if(chromaArray.value(id, nullptr))
        chromaArray[id]->deleteLater();
      chromaArray[id] = makeChomatogramWidget(id);
    }

    for(auto& wgt : chromaArray)
      wgt->setMarkupModel(dst, stepInfo);
    updateIntervalsMode();
    onStartupLoadPlots();
  }

  for(auto it = chromaArray.begin(); it != chromaArray.end(); it++)
  {
    if(it == chromaArray.begin())
      *mWholeDataRangeX = (*it)->dataRangeX();
    else
      mWholeDataRangeX->expand((*it)->dataRangeX());

    if(it == chromaArray.begin())
      *mWholeDataRangeY = (*it)->dataRangeY();
    else
      mWholeDataRangeY->expand((*it)->dataRangeY());
  }
  mChromatogrammsWidgets[dst] = chromaArray;

  connect(dst.get(), &MarkupDataModel::dataChanged, this, [this](uint role, const QVariant& data)
          {
            switch((MarkupDataRoles)role)
            {
            case MarkupMasterIdRole:
              onMasterChanged();
            default:
              break;
            }
          });
  connect(dst.get(), &MarkupDataModel::chromatogrammSwapped, this, [this, dst](ChromaId id, ChromatogrammModelPtr newPtr)
          {
            if(mChromatogrammsWidgets[dst].contains(id))
            {
              mChromatogrammsWidgets[dst][id]->swapChromatogramm(newPtr);
              updateIntervalsMode();
              onStartupLoadPlots();
            }
            else
            {
              Q_ASSERT(false); // can't find proper choma widget while swapping
            }
          });
  connect(dst.get(), &MarkupDataModel::chromatogrammRemoved, this, [this, dst](ChromaId id)
          {
            if(mChromatogrammsWidgets[dst].contains(id))
            {
              mChromatogrammsWidgets[dst][id]->deleteLater();
              mChromatogrammsWidgets[dst].remove(id);
              onStartupLoadPlots();
            }
          });
  connect(dst.get(), &MarkupDataModel::chromatogrammAdded, this, [this, dst](ChromaId id, ChromatogrammModelPtr newPtr)
          {
            Q_ASSERT(!mChromatogrammsWidgets[dst].contains(id));
            if(mChromatogrammsWidgets[dst].contains(id))
              return;

            auto newChroma = makeChomatogramWidget(id);
            newChroma->setMarkupModel(dst, mSteps[dst]);

            if(mChromatogrammsWidgets[dst].value(id, nullptr))
              mChromatogrammsWidgets[dst][id]->deleteLater();
            mChromatogrammsWidgets[dst][id] = newChroma;
            sortWidgets();
            onStartupLoadPlots();
          });
  sortWidgets();
  onStartupLoadPlots();
}

void ReservoirMarkupWidget::onOpenGlUpdateRequested()
{
  for(auto& wgtList : mChromatogrammsWidgets)
  {
    for(auto& wgt : wgtList)
    {
      // if(mCurrentMarkupModel->hasInternalMaster() &&
      //    wgt->id() == mCurrentMarkupModel->getMaster()->id())
      //   continue;
      auto stackedPlotW  = wgt->getStackedPlotWidget();
      if (!stackedPlotW)
        continue;

      if(mMainMarkupModel && mMainMarkupModel->getMaster() && wgt->id() == mMainMarkupModel->getMaster()->id())
      {
        if(stackedPlotW)
          stackedPlotW->showPlot();
        continue;
      }
      auto viewport = mPlotsScrollArea->viewport();

      auto rect = stackedPlotW->geometry();
      rect.moveTopLeft(stackedPlotW->mapTo(viewport, stackedPlotW->geometry().topLeft()));

      bool visible = viewport->geometry().intersects(rect);
      if (visible){
        if (stackedPlotW->hiddenPlot()){
          stackedPlotW->showPlot();
        }
        continue;
      }
      stackedPlotW->hidePlot();
    }
  }
}

void ReservoirMarkupWidget::onXAxisRangeChanged(const GPRange &newRange)
{
  mRangeUpdater = qobject_cast<ChromatogramWidgetRefactor*>(sender());
  if(!mRangeUpdater)
    return;
  *mXAxisViewportRange = newRange;
  mXAxisUptateTimer->start();
}

void ReservoirMarkupWidget::onXAxisRangeChangedTimeout()
{
  for(auto& wgtSet : mChromatogrammsWidgets)
  {
    for(auto& wgt : wgtSet)
    {
      if(wgt == mRangeUpdater)
        continue;
      QSignalBlocker blocker(wgt.get());
      wgt->setXAxisRange(*mXAxisViewportRange);
    }
  }
  updateCommonScrollbar();
}

void ReservoirMarkupWidget::onYAxisRangeChanged(const GPRange &newRange)
{
  mRangeUpdater = qobject_cast<ChromatogramWidgetRefactor*>(sender());
  if(!mRangeUpdater)
    return;
  *mYAxisViewportRange = newRange;
  mYAxisUptateTimer->start();
}

void ReservoirMarkupWidget::onYAxisRangeChangedTimeout()
{
  for(auto& wgtSet : mChromatogrammsWidgets)
  {
    for(auto& wgt : wgtSet)
    {
      if(wgt == mRangeUpdater)
        continue;
      QSignalBlocker blocker(wgt.get());
      wgt->setYAxisRange(*mYAxisViewportRange);
    }
  }
}

void ReservoirMarkupWidget::onChromaVScrollBarVisibilityChanged(bool isVisible)
{
  if(isVisible && mMasterRightSpacer->sizeHint().width() == 0)
  {
    //qDebug() << "show spacer:" << ((ShowHideSignalizableScrollBar*)sender())->width();
    mMasterRightSpacer->changeSize(((ShowHideSignalizableScrollBarPrivate*)sender())->width(),0);
    mMasterChromaLayout->invalidate();
  }
  else if(!isVisible && mMasterRightSpacer->sizeHint().width() != 0)
  {
    //        qDebug() << "hide spacer";
    mMasterRightSpacer->changeSize(0,0);
    mMasterChromaLayout->invalidate();
  }
}

void ReservoirMarkupWidget::onUndo()
{
  if(!mCurrentMarkupModel)
    return;
  auto undoStack = mCurrentMarkupModel->controller()->getUndoStack();
  Q_ASSERT(undoStack);
  if(undoStack->canUndo())
    undoStack->undo();
}

void ReservoirMarkupWidget::onRedo()
{
  if(!mCurrentMarkupModel)
    return;
  auto undoStack = mCurrentMarkupModel->controller()->getUndoStack();
  Q_ASSERT(undoStack);
  if(undoStack->canRedo())
    undoStack->redo();
}

void ReservoirMarkupWidget::onMasterChanged()
{
  sortWidgets();
}

void ReservoirMarkupWidget::updateIntervalsMode()
{
  if(!mCurrentMarkupModel)
    return;
  bool isIntervalsModeActive = false;

  for(auto& chroma : mCurrentMarkupModel->getChromatogrammsListModels())
  {
    if(chroma->getChromatogramValue(ChromatogrammMode).toInt() == CMIntervals)
    {
      isIntervalsModeActive = true;
      break;
    }
  }

  emit intervalsModeChanged(isIntervalsModeActive);
}

void ReservoirMarkupWidget::onStartupLoadPlots()
{
  qApp->processEvents(QEventLoop::AllEvents);
  mVScrollBarTimer->start(0);
}

void ReservoirMarkupWidget::sortWidgets()
{
  QList<QPointer<ChromatogramWidgetRefactor>> ptrs{ mChromatogrammsWidgets[mCurrentMarkupModel].begin(),
                                                   mChromatogrammsWidgets[mCurrentMarkupModel].end()};
  for(int i = 0; i < mPlotsWidgetLayout->count(); i++)
  {
    auto item = mPlotsWidgetLayout->itemAt(i);
    if(item->widget() && ptrs.contains(item->widget()))
      mPlotsWidgetLayout->removeItem(item);
  }
  std::sort(ptrs.begin(), ptrs.end(), [](const ChromatogramWidgetRefactor* a, const ChromatogramWidgetRefactor* b)->bool
            {
              return a->id() < b->id();
            });
  if(mCurrentMarkupModel->hasInternalMaster())
  {
    auto masterIter = std::lower_bound(ptrs.constBegin(), ptrs.constEnd(), mCurrentMarkupModel->getMaster()->id(),
                                       [](const QPointer<ChromatogramWidgetRefactor> ptr, int val)->bool
                                       {
                                         return ptr->id() < val;
                                       });
    if(mCurrentMarkupModel->getMaster() && masterIter != ptrs.end() && (masterIter->get()->id() == mCurrentMarkupModel->getMaster()->id()))
    {
      auto master = *masterIter;
      ptrs.erase(masterIter);
      auto item = mMasterChromaLayout->itemAt(0);
      if(!item->spacerItem())
      {
        mMasterChromaLayout->removeItem(item);
      }

      mMasterChromaLayout->insertWidget(0, master);
      mMasterRightSpacer->changeSize(mPlotsScrollArea->verticalScrollBar()->isVisible() ? mPlotsScrollArea->verticalScrollBar()->width() : 0, 0);
      mMasterChromaLayout->invalidate();
    }
  }

  int ind = 0;
  for(auto& wgt : ptrs)
  {
    mPlotsWidgetLayout->insertWidget(ind++,wgt);
  }
}

void ReservoirMarkupWidget::onScrollBarMoved()
{
  mVScrollBarTimer->start(100);
}

void ReservoirMarkupWidget::updateCommonScrollbar()
{
  QSignalBlocker sb(mCommonScrollBar);
  double leftMargin = mWholeDataRangeX->size() * kDataMarginInPercents;

  mScrollNormalizeCoefficient = ((mWholeDataRangeX->size() + leftMargin * 2) / kActualScrollBarResolution);
  double currentWindow = mXAxisViewportRange->size() / mScrollNormalizeCoefficient;
  if(currentWindow >= kActualScrollBarResolution)
  {
    mCommonScrollBar->setRange(0, 0);
    mCommonScrollBar->hide();
    return;
  }
  else
  {
    if(mCommonScrollBar->isHidden())
      mCommonScrollBar->show();
  }
  mCommonScrollBar->setRange(0, std::round(kActualScrollBarResolution - currentWindow));
  mCommonScrollBar->setPageStep(std::round(currentWindow));
  mCommonScrollBar->setValue(std::round(((mXAxisViewportRange->lower + leftMargin) - (mWholeDataRangeX->lower)) / mScrollNormalizeCoefficient));
}
}//namespace GenesisMarkup
#include "reservoir_markup_widget.moc"


void GenesisMarkup::ReservoirMarkupWidget::resizeEvent(QResizeEvent *event)
{
  onScrollBarMoved();
  QWidget::resizeEvent(event);
}
