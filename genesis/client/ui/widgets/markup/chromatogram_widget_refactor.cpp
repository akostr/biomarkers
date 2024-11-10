#include "chromatogram_widget_refactor.h"

#include <QVBoxLayout>
#include <QScrollBar>
#include <QWheelEvent>

#include <ui/plots/chromatogram_plot.h>
#include "logic/markup/commands.h"
#include "logic/markup/markup_step_controller.h"
#include "logic/markup/markup_data_model.h"
#include "logic/markup/genesis_markup_enums.h"
#include "logic/markup/baseline_data_model.h"
#include "genesis_style/style.h"
#include "ui/dialogs/web_dialog.h"
#include "ui/dialogs/chromatogram_widget_settings_dialog_refactor.h"
#include <ui/widgets/markup/gp_stackedwidget.h>
#include <QGraphicsColorizeEffect>
#include "ui_chromatogram_widget_refactor.h"

namespace GenesisMarkup
{
bool ChromatogramWidgetRefactor::mIconsInitialized = false;
QIcon ChromatogramWidgetRefactor::mActiveMasterIcon = QIcon();
QIcon ChromatogramWidgetRefactor::mInactiveMasterIcon = QIcon();
QIcon ChromatogramWidgetRefactor::mUnlockedActionsIcon = QIcon();
QIcon ChromatogramWidgetRefactor::mLockedActionsIcon = QIcon();
QIcon ChromatogramWidgetRefactor::mSettingsIcon = QIcon();
QIcon ChromatogramWidgetRefactor::mInfoIcon = QIcon();
QIcon ChromatogramWidgetRefactor::mRemoveIcon = QIcon();

ChromatogramWidgetRefactor::ChromatogramWidgetRefactor(ChromaId id, int pixelHeight, QWidget *parent)
  : QWidget(parent)
  , mId(id)
  , ui(new Ui::ChromatogrammUi)
{
  setFocusPolicy(Qt::WheelFocus);
  if(!mIconsInitialized)
  {
    mActiveMasterIcon = QIcon(":/resource/icons/icon_star_active.png");
    mInactiveMasterIcon = QIcon(":/resource/icons/icon_star_inactive.png");
    mUnlockedActionsIcon = QIcon(":/resource/icons/icon_action_unlock.png");
    mLockedActionsIcon = QIcon(":/resource/icons/icon_action_lock.png");
    mSettingsIcon = QIcon(":/resource/icons/icon_action_settings.png");
    mInfoIcon = QIcon(":/resource/icons/icon_action_passport.png");
    mRemoveIcon = QIcon(":/resource/icons/icon_action_delete.png");
    mIconsInitialized = true;
  }
  setStyleSheet("QPushButton {background: none;} QLabel {background: none;}");

  ui->setupUi(this);

  ui->captionLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  setAttribute(Qt::WA_DeleteOnClose);
  setupToolBar();

  //menu style:

  // plot
  Plot = new ChromatogramPlot(this);
  Plot->setAttribute(Qt::WA_NoMousePropagation, false);
  // Plot->installEventFilter(this);
  Plot->setZoomModifiers(Qt::ControlModifier);
  connect(Plot, &ChromatogramPlot::afterReplot, this, [this]()
  {
    auto newLeftMargin = Plot->yAxis->axisRect()->left();
    if(ui->leftSpacer->sizeHint().width() != newLeftMargin)
    {
      ui->leftSpacer->changeSize(newLeftMargin, 0);
      ui->headerLayout->invalidate();
    }
  });

  connect(Plot.get(), &ChromatogramPlot::newCommand, this, &ChromatogramWidgetRefactor::newCommand);
  connect(Plot.get(), &ChromatogramPlot::intervalsApplyingRequested, this, &ChromatogramWidgetRefactor::intervalsApplyingRequested);

  setFixedHeight(pixelHeight);
  ui->stackedWidget->setPlot(Plot);

  connect(Plot->xAxis, qOverload<const GPRange &>(&GPAxis::rangeChanged), this, &ChromatogramWidgetRefactor::xAxisRangeChanged);
  connect(Plot->yAxis, qOverload<const GPRange &>(&GPAxis::rangeChanged), this, &ChromatogramWidgetRefactor::yAxisRangeChanged);

  auto plotLayout = new QVBoxLayout();
  Plot->setLayout(plotLayout);
  PlotInternalToolbarLayout = new QHBoxLayout();
  plotLayout->addLayout(PlotInternalToolbarLayout);
  plotLayout->addStretch();
  PlotInternalToolbarLayout->addStretch();
  auto cancelIntervalsBtn = new QPushButton(tr("exit intervals"));
  auto applyIntervalsBtn = new QPushButton(tr("apply intervals"));
  PlotInternalToolbarLayout->addWidget(applyIntervalsBtn);
  PlotInternalToolbarLayout->addWidget(cancelIntervalsBtn);
  applyIntervalsBtn->setStyleSheet(Style::ApplySASS(
                                     "QPushButton"
                                     "{"
                                     "  background-color:   @dialogBoxButtonPrimaryBg;"
                                     "  border-color:       @dialogBoxButtonPrimaryBg;"
                                     "  color:              @dialogBoxButtonPrimaryFg;"
                                     "  border-radius:      4px;"
                                     "}"
                                     ));
  cancelIntervalsBtn->setStyleSheet(Style::ApplySASS(
                                     "QPushButton"
                                     "{"
                                     "color:              @dialogBoxButtonSecondaryFg;"
                                     "background-color:   @dialogBoxButtonSecondaryBg;"
                                     "border-color:       @uiInputElementColorFrameOpaque;"
                                     "border-radius:      4px;"
                                     "}"
                                     ));

  connect(cancelIntervalsBtn, &QPushButton::clicked, this, &ChromatogramWidgetRefactor::onExitIntervals);
  connect(applyIntervalsBtn, &QPushButton::clicked, this, &ChromatogramWidgetRefactor::onApplyIntervals);

  connect(this, &ChromatogramWidgetRefactor::BackgroundColorChanged, this, qOverload<>(&QWidget::update));
}

ChromatogramWidgetRefactor::~ChromatogramWidgetRefactor()
{
  delete ui;
}

void ChromatogramWidgetRefactor::setMarkupModel(MarkupModelPtr markup, const StepInfo& modelStepInteractions)
{
  if(connectionToMarkupModel)
    disconnect(connectionToMarkupModel);
  if(connectionToChromaModel)
    disconnect(connectionToChromaModel);
  mMarkupModel = markup;//should be exactly here
  connectionToMarkupModel = connect(markup.get(), &MarkupDataModel::dataChanged, this, &ChromatogramWidgetRefactor::onMarkupDataChanged);
  handleStepInteractions(mMarkupModel, modelStepInteractions);
  if(markup->getMaster())
    onMarkupDataChanged(MarkupMasterIdRole, markup->getMaster()->id());

  auto chroma = markup->getChromatogramm(mId);
  connectionToChromaModel = connect(chroma.get(), &ChromatogramDataModel::chromatogramDataChanged, this,
  [this](int dataRole, QVariant newValue)
  {
    if(dataRole == ChromatogrammMode)
      onChromatogramModeChanged((ChromatogrammModes)newValue.toInt());
  });
  onChromatogramModeChanged((ChromatogrammModes)chroma->getChromatogramValue(ChromatogrammMode).toInt());

  Plot->setChromatogrammModel(markup->getChromatogramm(mId), modelStepInteractions);
  ui->leftSpacer->changeSize(Plot->yAxis->calculateMargin(), 0);
  ui->headerLayout->invalidate();
}

void ChromatogramWidgetRefactor::swapChromatogramm(ChromatogrammModelPtr newPtr)
{
  if(connectionToChromaModel)
    disconnect(connectionToChromaModel);
  connectionToChromaModel = connect(newPtr.get(), &ChromatogramDataModel::chromatogramDataChanged, this,
  [this](int dataRole, QVariant newValue)
  {
    if(dataRole == ChromatogrammMode)
      onChromatogramModeChanged((ChromatogrammModes)newValue.toInt());
  });
  onChromatogramModeChanged((ChromatogrammModes)newPtr->getChromatogramValue(ChromatogrammMode).toInt());
  Plot->setChromatogrammModel(newPtr, mStepInteractions);
}

GPRange ChromatogramWidgetRefactor::dataRangeX()
{
  return Plot->xDataRange();
}

GPRange ChromatogramWidgetRefactor::dataRangeY()
{
  return Plot->yDataRange();
}

GPRange ChromatogramWidgetRefactor::xAxisRange()
{
  return Plot->xAxis->range();
}

QPair<GPRange, GPRange> ChromatogramWidgetRefactor::getBoundings()
{
  return Plot->boundingRange();
}

void ChromatogramWidgetRefactor::setXAxisRange(const GPRange &newRange)
{
  Plot->xAxis->setRange(newRange);
  Plot->replotOnVisibleViewPort(GraphicsPlot::rpQueuedReplot);
}

void ChromatogramWidgetRefactor::setYAxisRange(const GPRange &newRange)
{
  Plot->yAxis->setRange(newRange);
  Plot->replotOnVisibleViewPort(GraphicsPlot::rpQueuedReplot);
}

void ChromatogramWidgetRefactor::setName(const QString &name)
{
  ui->captionLabel->setText(name + QString(" (%1)").arg(mId));
}

void ChromatogramWidgetRefactor::setInteractionDisabled(bool disabled)
{
  if(disabled)
  {
    Plot->setBackground(QColor(242, 244, 245));
    setBackgroundColor(QColor(242, 244, 245));
  }
  else
  {
    Plot->setBackground(Qt::white);
    //recolorize widget regarding its interactivity flags
    handleStepInteractions(mMarkupModel);
  }
  ui->menuButton->setDisabled(disabled);
  setLocked(disabled);
}

void ChromatogramWidgetRefactor::setLocked(bool locked)
{
  Plot->setInteractionsLocked(locked);
}

void ChromatogramWidgetRefactor::setupToolBar()
{
  {
    ui->masterActionButton->setCheckable(false);

    connect(ui->masterActionButton, &QPushButton::clicked, this, [this]() {
      if(mMarkupModel && mStepInteractions.guiInteractions.testFlag(SIGPickMaster))
      {
        if(auto master = mMarkupModel->getMaster())
        {
          if(master->id() == mId)
          {
            emit newCommand(new SetMasterCommand(-1, mMarkupModel));
            return;
          }
        }
        emit newCommand(new SetMasterCommand(mId, mMarkupModel));
      }
    });
  }

  /*
  auto lockChangesAction = new QAction(tr("Lock changes (unlocked)"), this);
  lockChangesAction->setIcon(mUnlockedActionsIcon);
  lockChangesAction->setCheckable(true);
  connect(lockChangesAction, &QAction::toggled, this, [this, lockChangesAction](bool isChecked)
  {
    if(isChecked)
    {
      lockChangesAction->setText(tr("Unock changes (locked)"));
      lockChangesAction->setIcon(mLockedActionsIcon);
      setLocked(true);
    }
    else
    {
      lockChangesAction->setText(tr("Lock changes (unlocked)"));
      lockChangesAction->setIcon(mUnlockedActionsIcon);
      setLocked(false);
    }
  });
   */
  auto settingsAction = new QAction(mSettingsIcon, tr("Plot settings"), this);
  {
    connect(settingsAction, &QAction::triggered, this, [this]()
    {
      auto chromaModel = mMarkupModel->getChromatogramm(mId);
      auto dial = new ChromatogramWidgetSettingsDialogRefactor(chromaModel, mStepInteractions, 0, this);

      connect(dial, &WebDialog::Accepted, this, [this, dial, chromaModel]()
      {
        if(mStepInteractions.step == Step0DataLoad)
        {
          if(chromaModel->isMaster())
          {
            auto cmd = new SetMasterChromatogramSettingsCommand(mMarkupModel, dial->getSettings());
            emit newCommand(cmd);
          }
          else
          {
            auto cmd = new SetChromatogramSettingsCommand(chromaModel, dial->getSettings());
            emit newCommand(cmd);
          }
        }
        else
        {
          if(!chromaModel->isMaster())
          {
            ChromaSettings settings = {dial->getSettings()};
            emit remarkupCalled(chromaModel->id(), settings);
          }
          else
          {
            Q_ASSERT(chromaModel->markupModel()->hasInternalMaster());
            auto markupModel = chromaModel->markupModel();
            emit intervalsApplyingRequested(markupModel->getChromatogrammsIdList(), dial->getIntervalsSettings(), dial->getDeprecationIntervals(), dial->getSettings(), Step1PeakDetection);
          }
        }
      }, Qt::DirectConnection);
      dial->Open();
    });
  }
  auto infoAction = new QAction(mInfoIcon, tr("Plot info"), this);
  {
      connect(infoAction, &QAction::triggered, this, [this]()
      {
        //// TODO Ions Dialog
        {
//          if(!AllIons.isEmpty())
//          {
//              ChromatogramWidgetIonsDialog *dialog = new ChromatogramWidgetIonsDialog(this, Plot);
//              dialog->Open();
//          }
//          else
//          {
//              Notification::Notify("view/project/markup", Notification::StatusWarning, QString(tr("Empty ions list")));
//          }
        }
      });
  }
  infoAction->setDisabled(true);

  RemoveAction = new QAction(mRemoveIcon, tr("Remove plot"), this);
  connect(RemoveAction, &QAction::triggered, this, [this]()
  {
    auto cmd = new RemoveChromatogramComand(mMarkupModel, id());
    emit newCommand(cmd);
  });
  auto menu = new QMenu(ui->menuButton);
  menu->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qmenu.qss"));
  //menu->addAction(lockChangesAction);
  menu->addAction(settingsAction);
  menu->addAction(infoAction);
  menu->addAction(RemoveAction);
  ui->menuButton->setMenu(menu);
  ui->menuButton->setStyleSheet("QPushButton::menu-indicator { image: none; width: 0px; }");
  connect(menu, &QMenu::aboutToShow, this, &ChromatogramWidgetRefactor::onBurgerMenuCalled);

  auto toolBar = new QToolBar(this);
  ui->headerLayout->addWidget(toolBar);

  auto baseLineEditAction = new QAction(tr("BL"), this);
  baseLineEditAction->setShortcut(QKeySequence(QKeyCombination(Qt::Key_B)));
  baseLineEditAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  baseLineEditAction->setCheckable(true);
  this->addAction(baseLineEditAction);
  toolBar->addAction(baseLineEditAction);
  toolBar->setStyleSheet(Style::ApplySASS(
"QToolButton {"
    "padding: 2px;"
    "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255,255,255,0), stop:0.1 rgba(255,255,255,0), stop:0.5 lightgray);"
    "color: gray; "
    "border: none; "
    "border-top-left-radius: 0px; "
    "border-top-right-radius: 0px;}\n"
"QToolButton:checked {"
    "padding: 2px;"
    "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255,255,255,0), stop:0.1 rgba(255,255,255,0), stop:0.5 @brandColor);"
    "color: white;"
    "border: none; "
    "border-top-left-radius: 0px; "
    "border-top-right-radius: 0px;}\n"
"QToolBar {"
    "background: transparent;}"));
  auto actionButton = toolBar->widgetForAction(baseLineEditAction);
  QGraphicsColorizeEffect *eEffect= new QGraphicsColorizeEffect(actionButton);
  eEffect->setColor(QColor(Qt::red));
  auto baseStrength = eEffect->strength();
  eEffect->setStrength(0);
  actionButton->setGraphicsEffect(eEffect);
  auto blinkAnimationGroup = new QSequentialAnimationGroup(actionButton);
  QPropertyAnimation *blinkForward = new QPropertyAnimation(eEffect,"strength", blinkAnimationGroup);
  blinkForward->setStartValue(0);
  blinkForward->setEndValue(baseStrength);
  blinkForward->setDuration(200);
  QPropertyAnimation *blinkBackward = new QPropertyAnimation(eEffect,"strength", blinkAnimationGroup);
  blinkBackward->setStartValue(baseStrength);
  blinkBackward->setEndValue(0);
  blinkBackward->setDuration(200);
  blinkAnimationGroup->setLoopCount(3);

  connect(baseLineEditAction, &QAction::toggled, this,
          [this, baseLineEditAction, blinkAnimationGroup, eEffect](bool checked)
          {
            if(checked)
            {
              if(!Plot->setBLSelectionMode(checked))
              {
                baseLineEditAction->setChecked(false);
                blinkAnimationGroup->start();
              }
            }
            else
            {
              Plot->setBLSelectionMode(false);
            }
          });
}

void ChromatogramWidgetRefactor::onMarkupDataChanged(uint role, QVariant newValue)
{
  switch((MarkupDataRoles)role)
  {
  case MarkupMasterIdRole:
  {
    ChromaId masterId = newValue.toInt();
    if(mId == masterId)
    {
      ui->masterActionButton->setIcon(mActiveMasterIcon);
      ui->masterActionButton->setToolTip(tr("Unmake master"));
      handleStepInteractions(mMarkupModel);
    }
    else
    {
      ui->masterActionButton->setIcon(mInactiveMasterIcon);
      ui->masterActionButton->setToolTip(tr("Make master"));
      handleStepInteractions(mMarkupModel);
    }
    break;
  }
  default:
    break;
  }
}

void ChromatogramWidgetRefactor::paintEvent(QPaintEvent *event)
{
  QPainter p(this);
  p.setPen(Qt::NoPen);
  p.setBrush(mBackgroundColor);
  p.drawRect(rect());
}

void ChromatogramWidgetRefactor::setPlotInternalToolbarHidded(bool hidden)
{
  for(int i = 0; i < PlotInternalToolbarLayout->count(); i++)
  {
    auto item = PlotInternalToolbarLayout->itemAt(i);
    if(auto wgt = item->widget())
    {
      wgt->setHidden(hidden);
    }
  }
}

void ChromatogramWidgetRefactor::onExitIntervals()
{
  auto intervalIsValid = [](AbstractEntityDataModel::ConstDataPtr intervalEntity)->bool
  {
    return intervalEntity->hasDataAndItsValid(IntervalIsApplied) && intervalEntity->getData(IntervalIsApplied).toBool();
  };
  auto chroma = mMarkupModel->getChromatogramm(id());
  Q_ASSERT(chroma);
  if(!chroma)
    return;
  QList<AbstractEntityDataModel::ConstDataPtr> toRemove;

  for(auto& interval : chroma->getEntities(TypeInterval))
    if(!intervalIsValid(interval))
      toRemove << interval;

  auto redoSequence = [toRemove, chroma]()
  {
    for(auto& i : toRemove)
      chroma->removeEntity(i->getUid());
    chroma->setChromatogramValue(ChromatogrammMode, CMMarkup);
  };
  auto undoSequence = [toRemove, chroma]()
  {
    for(auto& i : toRemove)
      chroma->addNewEntity(i->getType(),
                           *i,
                           i->getUid());
    chroma->setChromatogramValue(ChromatogrammMode, CMIntervals);
  };

  auto cmd = new LambdaSequenceCommand(redoSequence, undoSequence);
  emit newCommand(cmd);
}

void ChromatogramWidgetRefactor::onApplyIntervals()
{
  auto chroma = mMarkupModel->getChromatogramm(mId);

  auto dial = new ChromatogramWidgetSettingsDialogRefactor(chroma, mStepInteractions, 1, this);
  connect(dial, &WebDialog::Accepted, this, [this, chroma, dial]()
  {
    if(chroma->isMaster())
    {
      Q_ASSERT(chroma->markupModel()->hasInternalMaster());
      auto markupModel = chroma->markupModel();
      emit intervalsApplyingRequested(markupModel->getChromatogrammsIdList(), dial->getIntervalsSettings(), dial->getDeprecationIntervals(), dial->getSettings(), mStepInteractions.step);
    }
    else
    {
      emit intervalsApplyingRequested({chroma->id()}, dial->getIntervalsSettings(), dial->getDeprecationIntervals(), dial->getSettings(), mStepInteractions.step);
    }
  }, Qt::DirectConnection);
  dial->Open();
}

void ChromatogramWidgetRefactor::onChromatogramModeChanged(ChromatogrammModes newMode)
{
  switch(newMode)
  {
  case CMMarkup:
    setPlotInternalToolbarHidded(true);
    emit intervalsModeStateChanged();
    break;
  case CMIntervals:
    setPlotInternalToolbarHidded(false);
    emit intervalsModeStateChanged();
    break;
  case CMNone:
  default:
    break;
  }
}

void ChromatogramWidgetRefactor::onBurgerMenuCalled()
{
  if(mMarkupModel && mMarkupModel->getMaster() && mMarkupModel->getMaster()->id() == mId)
  {
    if(mStepInteractions.guiInteractions.testFlag(SIGRemoveMasterChromatogramm))
      RemoveAction->setEnabled(true);
    else
      RemoveAction->setEnabled(false);
  }
  else
  {
    if(mStepInteractions.guiInteractions.testFlag(SIGRemoveSlaveChromatogramm))
      RemoveAction->setEnabled(true);
    else
      RemoveAction->setEnabled(false);
  }
}

void ChromatogramWidgetRefactor::handleStepInteractions(MarkupModelPtr markup, const StepInfo &newModelStepInteractions)
{
  mStepInteractions = newModelStepInteractions;
  handleStepInteractions(markup);
}

void ChromatogramWidgetRefactor::handleStepInteractions(MarkupModelPtr markup)
{
  bool enabledStyle = markup->getChromatogramm(mId)->isMaster() ?
                        mStepInteractions.masterInteractions.testFlag(SIMEnabledStyle) :
                        mStepInteractions.slaveInteractions.testFlag(SISEnabledStyle);
  if(enabledStyle)
    setBackgroundColor(Qt::white);
  else
    setBackgroundColor(QColor(242, 244, 245));
}

QColor ChromatogramWidgetRefactor::BackgroundColor() const
{
  return mBackgroundColor;
}

void ChromatogramWidgetRefactor::setBackgroundColor(const QColor &newBackgroundColor)
{
  if (mBackgroundColor == newBackgroundColor)
    return;
  mBackgroundColor = newBackgroundColor;
  emit BackgroundColorChanged();
}

QPointer<GPStackedWidget> ChromatogramWidgetRefactor::getStackedPlotWidget() const
{
  return ui->stackedWidget;
}

QPointer<ChromatogramPlot> ChromatogramWidgetRefactor::getPlot() const
{
  return Plot;
}

int ChromatogramWidgetRefactor::id() const
{
  return mId;
}

// bool ChromatogramWidgetRefactor::eventFilter(QObject *watched, QEvent *event)
// {
//   if(watched == Plot && event->type() == QEvent::Wheel)
//   {
//     auto e = (QWheelEvent*)event;
//     if(!e->modifiers().testFlag(Qt::ControlModifier))
//     {
//       e->ignore();
//       auto p = parent();
//       while(p)
//       {
//         if(auto sa = qobject_cast<QScrollArea*>(p))
//         {
//           auto ne = new QWheelEvent(e->position(), e->globalPosition(), e->pixelDelta(), e->angleDelta(), e->buttons(), e->modifiers(), e->phase(), e->inverted(), e->source(), e->pointingDevice());
//           QCoreApplication::sendEvent(sa->verticalScrollBar(), ne);
//           break;
//         }
//         p = p->parent();
//       }
//       return true;
//     }
//     else
//     {
//       return false;
//     }
//   }
//   else
//      return false;
// }

void GenesisMarkup::ChromatogramWidgetRefactor::enterEvent(QEnterEvent *event)
{
  setFocus(Qt::MouseFocusReason);
  QWidget::enterEvent(event);
}

void GenesisMarkup::ChromatogramWidgetRefactor::leaveEvent(QEvent *event)
{
  QWidget::leaveEvent(event);
}
}//GenesisMarkup


