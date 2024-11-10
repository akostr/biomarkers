#include "identification_widget.h"
#include "logic/markup/genesis_markup_enums.h"
#include <extern/common_gui/standard_widgets/tumbler.h>
#include "ui_identification_widget.h"
#include <logic/markup/item_models/identification_tree_model.h>
#include <logic/markup/item_models/identification_presentation_model.h>
#include <logic/notification.h>
#include <logic/tree_model_item.h>
#include <ui/widgets/markup/identification_legend_plate.h>
#include <ui/flow_layout.h>
#include <logic/markup/commands.h>
#include <logic/enums.h>
#include <genesis_style/style.h>

#include <QMouseEvent>
bool RescaleSpectrum = true;

IdentificationWidget::IdentificationWidget(QWidget *parent)
  : IMarkupTab(parent)
  , ui(new Ui::IdentificationWidget)
  , mIdentificationModel(new IdentificationTreeModel(this))
{
  ui->setupUi(this);
  mIdentificationModel->GetPresentationModel()->SetRejectedColumns({IdentificationTreeModel::LAST_COLUMN});
  ui->compoundsTable->setModel(mIdentificationModel->GetPresentationModel());
  ui->compoundsTable->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  ui->compoundsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  ui->compoundsTable->setStyleSheet(Style::ApplySASS("QTreeView::branch:selected {background-color: @brandColor;}"));
  ui->compoundsTable->setIndentation(3);
  ui->compoundsTable->sortByColumn(IdentificationTreeModel::METRICS, Qt::DescendingOrder);
  ui->spectrumSplitter->setSizes({0, 1});
  ui->chromatogrammPlot->show();
  // ui->chromatogrammPlot->setAttribute(Qt::WA_NoMousePropagation, false);
  ui->chromatogrammPlot->setZoomModifiers(Qt::ControlModifier);
  // ui->chromatogrammPlot->installEventFilter(this);

  auto chromaOverlayLayout = new QGridLayout(ui->chromatogrammPlot);
  ui->chromatogrammPlot->setLayout(chromaOverlayLayout);

  chromaOverlayLayout->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Ignored), 0,0);
  chromaOverlayLayout->addItem(new QSpacerItem(0,0,QSizePolicy::Ignored, QSizePolicy::Expanding), 1,1);

  auto helpLabel = new QLabel(ui->chromatogrammPlot);
  helpLabel->setText("<a href=\"#\"><img src=\":/resource/icons/icon_action_information_black.png\" width=\"6\" height=\"14\"/></a>");
  connect(helpLabel, &QLabel::linkActivated, this, [this, helpLabel]()
          {
            //блиллиантовый костыль, что бы после клика на "хелп", перемещение графика не залипало
            //потому что лэйбл перехватывает MouseButtonRelease но не перехватывает MouseButtonPress
            //и в итоге получается что плот получает сигнал buttonPress, и не получая сигнал buttonRelease
            //залипает в перетаскивании
            //поэтому мы имитируем плоту сигнал отпускания кнопки мышки:
            QPointF globalPos = QCursor::pos();
            QPointF localPos = mapFromGlobal(globalPos);
            QPointF scenePos = localPos;
            Qt::MouseButton btn = Qt::LeftButton;
            Qt::MouseButtons buttons = Qt::LeftButton;
            Qt::KeyboardModifiers modifiers = Qt::NoModifier;

            QMouseEvent event(QEvent::MouseButtonRelease, localPos, scenePos, globalPos, btn, buttons, modifiers);
            QApplication::sendEvent(ui->chromatogrammPlot, &event);
            //показываем тултип
            QToolTip::showText(QCursor::pos(), tr("Hold Shift and click on any point to move selector"), helpLabel);
          });
  helpLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  helpLabel->setMaximumSize(QSize(18,18));
  helpLabel->setMinimumSize(QSize(18,18));
  helpLabel->setStyleSheet("QLabel{background: transparent;}");

  chromaOverlayLayout->addWidget(helpLabel, 0, 1);

  ui->spectrumPlot->setInteractions(GP::Interaction::iRangeDrag | GP::Interaction::iRangeZoom);
  mLegendLayout = new FlowLayout(ui->legendWidget, 0,0,0);
  ui->spectrumSplitter->setStretchFactor(0, 1);
  ui->legendWidget->setLayout(mLegendLayout);
  {
    ui->specCaption->setStyleSheet(Style::Genesis::Fonts::RegularBold());
    QString text = tr("Mass spectre view");
    QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_information_black.png\" width=\"6\" height=\"14\"/></a></td></tr></table>").arg(text);
    ui->specCaption->setText(tmpl);
    connect(ui->specCaption, &QLabel::linkActivated, this,
            [this]()
            {
              QToolTip::showText(QCursor::pos(), tr("To view the mass spectra of compounds,"
                                                    "\nenable the checkbox in the list of compounds."
                                                    "\nTo view the mass spectrum of a reference slice,"
                                                    "\nplace the slice on the chromatogram"
                                                    "\n(available for GC-MS or GC-MS/MS)."), ui->tableCaption);
            });
    ui->tableCaption->setStyleSheet(Style::Genesis::Fonts::RegularBold());
    text = tr("Compounds table");
    tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_information_black.png\" width=\"6\" height=\"14\"/></a></td></tr></table>").arg(text);
    ui->tableCaption->setText(tmpl);
    connect(ui->tableCaption, &QLabel::linkActivated, this,
            [this]()
            {
              QToolTip::showText(QCursor::pos(), tr("\"Compliance with the cut, %\" - after setting"
                                                    "\nthe cut on the standard, the system will "
                                                    "\ndetermine the percentage of compliance of "
                                                    "\neach compound with the mass spectrum of the"
                                                    "\nstandard cut. \"Apply to peak\" — the peak"
                                                    "\nselected on the reference will receive the"
                                                    "\nname of the compound of the selected line. The "
                                                    "\ncheckbox at the compound level enables viewing"
                                                    "\n of the mass spectrum graph."), ui->tableCaption);
            });
  }
  ui->searchLineEdit->setStyleSheet("QLineEdit {placeholder-text-color: rgba(0, 32, 51, 35%);}");
  ui->searchLineEdit->setPlaceholderText(tr("Search by name"));
  mNameSearchAction = new QAction(QIcon(":/resource/icons/search.png"), "", ui->searchLineEdit);
  mNameSearchAction->setToolTip(tr("Search by name"));
  mNameSearchClearAction = new QAction(QIcon(":/resource/icons/icon_action_cross.png"), "", ui->searchLineEdit);
  mNameSearchClearAction->setToolTip(tr("Clear search"));
  ui->searchLineEdit->addAction(mNameSearchAction, QLineEdit::ActionPosition::LeadingPosition);
  ui->searchLineEdit->addAction(mNameSearchClearAction, QLineEdit::ActionPosition::TrailingPosition);
  mNameSearchClearAction->setVisible(false);

  ui->classifierTypeFilterCombo->addItem(tr("All"), static_cast<int>(LibraryFilter::Classifiers::All));
  ui->classifierTypeFilterCombo->addItem(tr("GC_PID"), static_cast<int>(LibraryFilter::Classifiers::GC_PID));
  ui->classifierTypeFilterCombo->addItem(tr("GC_MS"), static_cast<int>(LibraryFilter::Classifiers::GC_MS));
  // ui->classifierTypeFilterCombo->addItem(tr("Saturated"), static_cast<int>(LibraryFilter::Classifiers::Saturated));
  // ui->classifierTypeFilterCombo->addItem(tr("Aromatic"), static_cast<int>(LibraryFilter::Classifiers::Aromatic));
  ui->classifierTypeFilterCombo->addItem(tr("Pyrolysis"), static_cast<int>(LibraryFilter::Classifiers::Pyrolysis));
  ui->classifierTypeFilterCombo->addItem(tr("Gas composition"), static_cast<int>(LibraryFilter::Classifiers::GasComposition));
  ui->classifierTypeFilterCombo->addItem(tr("Isotopy"), static_cast<int>(LibraryFilter::Classifiers::Isotopy));
  ui->classifierTypeFilterCombo->addItem(tr("Water composition"), static_cast<int>(LibraryFilter::Classifiers::WaterComposition));

  ui->fileTypeFilterCombo->addItem(tr("All"), All);
  ui->fileTypeFilterCombo->addItem(tr("System"), System);
  ui->fileTypeFilterCombo->addItem(tr("Favorites"), Favorites);

  ui->applyBtn->setProperty("blue", true);
  ui->clearBtn->setProperty("secondary", true);
  auto menu = new QMenu(ui->customizeSpecPlotBtn);
  ui->customizeSpecPlotBtn->setMenu(menu);
  ui->customizeSpecPlotBtn->setProperty("menu_white", true);
  {
    auto wa = new QWidgetAction(menu);
    auto wgt = new QWidget;
    auto wgtL = new QVBoxLayout;
    wgt->setLayout(wgtL);
    auto showChromaIntensityCheckBox = new Tumbler(tr("Show m/z of GC-MS chromatogramm"), wgt);
    wgtL->addWidget(showChromaIntensityCheckBox);
    auto showCompoundIntensityCheckBox = new Tumbler(tr("Show m/z of compound"), wgt);
    wgtL->addWidget(showCompoundIntensityCheckBox);
    auto switchModeCheckBox = new Tumbler(tr("Switch view mode"), wgt);
    wgtL->addWidget(switchModeCheckBox);

    wa->setDefaultWidget(wgt);
    menu->addAction(wa);

    connect(showChromaIntensityCheckBox, &Tumbler::toggled, this, &IdentificationWidget::setChromaIntensityVisible);
    connect(showCompoundIntensityCheckBox, &Tumbler::toggled, this, &IdentificationWidget::setCompoundIntensityVisible);
    connect(switchModeCheckBox, &Tumbler::toggled, this, &IdentificationWidget::onSpectrumSwitchCheckStateChanged);
  }

  connect(mNameSearchAction, &QAction::triggered, this, &IdentificationWidget::applyNameFilter);
  connect(mNameSearchClearAction, &QAction::triggered, ui->searchLineEdit, &QLineEdit::clear);
  connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &IdentificationWidget::applyNameFilter);
  connect(ui->searchLineEdit, &QLineEdit::textChanged, this, [this](const QString& text)
          {
    if(text.isEmpty())
      mNameSearchClearAction->setVisible(false);
    else
      mNameSearchClearAction->setVisible(true);
  });
  connect(ui->classifierTypeFilterCombo, &QComboBox::currentIndexChanged, this, &IdentificationWidget::applyClassifierTypeFilter);
  connect(ui->fileTypeFilterCombo, &QComboBox::currentIndexChanged, this, &IdentificationWidget::applyFileTypeFilter);

  connect(mIdentificationModel, &IdentificationTreeModel::currentSpectreChanged, this, &IdentificationWidget::onCurrentSpectreChanged);
  connect(mIdentificationModel, &IdentificationTreeModel::additionalSpectreAdded, this, &IdentificationWidget::onAdditionalSpectreAdded);
  connect(mIdentificationModel, &IdentificationTreeModel::spectersCleared, this, &IdentificationWidget::onSpectersCleared);
  connect(mIdentificationModel, &IdentificationTreeModel::additionalSpectreRemoved, this, &IdentificationWidget::onAdditionalSpectreRemoved);
  connect(ui->spectrumPlot, &SpectrumPlot::barColorChanged, this, &IdentificationWidget::onCompoundColorChanged);
  connect(ui->applyBtn, &QPushButton::clicked, this, &IdentificationWidget::onApplyButtonClicked);
  connect(ui->clearBtn, &QPushButton::clicked, this, &IdentificationWidget::clearPeakCompound);
  connect(ui->compoundsTable, &QTreeView::doubleClicked, this, &IdentificationWidget::applyCompoundIntoPeak);
  // ui->chromaSplitter->setEnabled(false);
}

IdentificationWidget::~IdentificationWidget()
{
  delete ui;
}

void IdentificationWidget::setChromaHeight(int height)
{
  if(isVisible())
  {
    ui->chromaSplitter->setSizes({height, ui->chromaSplitter->height() - height});
    mPendingChromaHeight = -1;
  }
  else
  {
    mPendingChromaHeight = height;
  }
}

void IdentificationWidget::onCurrentSpectreChanged(const QVector<double> &mass, const QVector<double> &intensity)
{
  if(mLegendMainItem)
    mLegendMainItem->deleteLater();
  mLegendMainItem = new IdentificationLegendPlate(QColor(204, 217, 224, 255), tr("GC-MS chromatogramm"), ui->legendWidget);
  ui->legendWidget->layout()->addWidget(mLegendMainItem);
  ui->spectrumPlot->setReferenceSpec({mass, intensity});
  resortLegendItems();

  if(RescaleSpectrum)
  {
    ui->spectrumPlot->rescaleAxes();
    ui->spectrumPlot->replot();
  }
  if(!mass.isEmpty())
    RescaleSpectrum = false;
}

void IdentificationWidget::onAdditionalSpectreAdded(const QVector<double> &mass, const QVector<double> &intensity, int sourceRow)
{
  ui->spectrumPlot->addSpec(sourceRow, {mass, intensity});
}

void IdentificationWidget::onSpectersCleared()
{
  ui->spectrumPlot->clearSpecs();
}

void IdentificationWidget::onAdditionalSpectreRemoved(int sourceRow)
{
  ui->spectrumPlot->removeSpec(sourceRow);
}

void IdentificationWidget::onCompoundColorChanged(int sourceRow, QVariant color)
{
  if(color.isValid())
  {
    if(mLegendItems.contains(sourceRow))
    {
      qobject_cast<IdentificationLegendPlate*>(mLegendItems[sourceRow])->setColor(color);
    }
    else
    {
      auto ind = mIdentificationModel->index(sourceRow, IdentificationTreeModel::SHORT_TITLE);
      auto text = mIdentificationModel->data(ind, Qt::DisplayRole).toString();
      auto plate = new IdentificationLegendPlate(color, text, ui->legendWidget);
      ui->legendWidget->layout()->addWidget(plate);
      mLegendItems[sourceRow] = plate;
    }
  }
  else
  {
    if(mLegendItems.contains(sourceRow))
    {
      mLegendItems[sourceRow]->deleteLater();
      mLegendItems.remove(sourceRow);
    }
  }
  resortLegendItems();
}

void IdentificationWidget::onSpectrumSwitchCheckStateChanged(bool state)
{
  bool barsMode = !state;
  ui->spectrumPlot->SwitchMode(barsMode);
}

void IdentificationWidget::setChromaIntensityVisible(bool visible)
{
  ui->spectrumPlot->setChromaLabelsVisible(visible);
}

void IdentificationWidget::setCompoundIntensityVisible(bool visible)
{
  ui->spectrumPlot->setCompoundLabelsVisible(visible);
}

void IdentificationWidget::onApplyButtonClicked()
{
  applyCompoundIntoPeak(ui->compoundsTable->currentIndex());
}

void IdentificationWidget::resortLegendItems()
{
  QList<QLayoutItem*> items;
  while(mLegendLayout->count() > 0)
  {
    auto item = mLegendLayout->takeAt(0);
    if(!mLegendMainItem || item->widget() != mLegendMainItem)
      items << item;
  }
  std::sort(items.begin(), items.end(), [this](QLayoutItem* first, QLayoutItem* second)->bool
            {
    auto it = std::find(mLegendItems.begin(), mLegendItems.end(), first->widget());
    if(it == mLegendItems.end())
      return false;
    int firstRow = it.key();
    it = std::find(mLegendItems.begin(), mLegendItems.end(), second->widget());
    if(it == mLegendItems.end())
      return true;
    int secondRow = it.key();
    return firstRow > secondRow;
  });
  if(mLegendMainItem)
    mLegendLayout->addWidget(mLegendMainItem);
  for(auto& item : items)
    mLegendLayout->addItem(item);
}

void IdentificationWidget::applyCompoundIntoPeak(QModelIndex proxyIndex)
{
  if(!proxyIndex.isValid())
  {
    Notification::NotifyError(tr("Select compound in table"), tr("Compound applying error"));
    return;
  }
  auto selectorPos = mMasterChroma->getEntity(mSelectorUid)->getData(GenesisMarkup::KeySelectorKey).toDouble();
  auto peakUid = mMasterChroma->getPeakUidFromX(selectorPos);

  if(peakUid.isNull())
  {
    Notification::NotifyError(tr("Move selector to desired peak for applying compound"), tr("Compound applying error"));
    return;
  }

  auto sourceInd = mIdentificationModel->GetPresentationModel()->mapToSource(proxyIndex);
  auto item = mIdentificationModel->GetItem(sourceInd);
  auto libraryGroupId = item->GetData("library_group_id").toInt();

  if(mMasterChroma->getEntity(peakUid)->getData(GenesisMarkup::PeakLibraryGroupId).toInt() == libraryGroupId)
  {
    //this compound alredy assigned to this peak
    return;
  }

  auto compoundShortTitle = item->GetData("short_title").toString();

  QList<QUndoCommand*> sequence;

  QStringList dupeTitles;
  for(auto& ent : mMasterChroma->getEntities({{GenesisMarkup::PeakLibraryGroupId, libraryGroupId}}))
  {
    auto uid = ent->getUid();
    auto title = QString::number(abs(ent->getData(GenesisMarkup::PeakId).toInt()));
    sequence << new GenesisMarkup::ChangeEntityDataCommand(mMasterChroma, uid, GenesisMarkup::PeakLibraryGroupId, QVariant());
    sequence << new GenesisMarkup::ChangeEntityDataCommand(mMasterChroma, uid, GenesisMarkup::PeakCompoundTitle, QVariant());
    //peak title should be resolved in Model::setEntityData wich calls by ChangeEntityDataCommand
    dupeTitles << title;
  }

  sequence << new GenesisMarkup::ChangeEntityDataCommand(mMasterChroma, peakUid, GenesisMarkup::PeakLibraryGroupId, libraryGroupId);
  sequence << new GenesisMarkup::ChangeEntityDataCommand(mMasterChroma, peakUid, GenesisMarkup::PeakCompoundTitle, compoundShortTitle);
  //peak title should be resolved in Model::setEntityData wich calls by ChangeEntityDataCommand
  auto cmd = new GenesisMarkup::CommandSequence(sequence);

  emit newCommand(cmd);
  if(!dupeTitles.isEmpty())
    Notification::NotifyInfo(tr("The compound %1 is assigned to the selected peak "
                                "and canceled from the previously identified one %2. "
                                "To cancel press Ctrl + Z").arg(compoundShortTitle).arg(dupeTitles.join(",")), tr("Warning"));
}

void IdentificationWidget::clearPeakCompound()
{
  auto selectorPos = mMasterChroma->getEntity(mSelectorUid)->getData(GenesisMarkup::KeySelectorKey).toDouble();
  auto peakUid = mMasterChroma->getPeakUidFromX(selectorPos);
  if(peakUid.isNull())
  {
    Notification::NotifyError(tr("Move selector to desired peak for applying compound"), tr("Compound applying error"));
    return;
  }
  auto peak = mMasterChroma->getEntity(peakUid);
  auto peakTitle = QString::number(abs(peak->getData(GenesisMarkup::PeakId).toInt()));

  QList<QUndoCommand*> sequence;
  sequence << new GenesisMarkup::ChangeEntityDataCommand(mMasterChroma, peakUid, GenesisMarkup::PeakLibraryGroupId, QVariant());
  sequence << new GenesisMarkup::ChangeEntityDataCommand(mMasterChroma, peakUid, GenesisMarkup::PeakCompoundTitle, QVariant());
  //peak title should be resolved in Model::setEntityData wich calls by ChangeEntityDataCommand
  auto cmd = new GenesisMarkup::CommandSequence(sequence);
  emit newCommand(cmd);
}

void IdentificationWidget::applyNameFilter()
{
  auto filterStr = ui->searchLineEdit->text().toLower();
  auto filter = [filterStr](TreeModelItem* item)->bool
  {
    if(filterStr.isEmpty())
      return true;
    if(item->GetData("short_title").toString().toLower().contains(filterStr) ||
      item->GetData("full_title").toString().toLower().contains(filterStr))
      return true;
    return false;
  };
  mIdentificationModel->GetPresentationModel()->setCustomItemFilter(filter);
}

void IdentificationWidget::applyFileTypeFilter()
{
  auto filterType = (FileTypeFilter)ui->fileTypeFilterCombo->currentData(Qt::UserRole).toInt();
  auto proxy = mIdentificationModel->GetPresentationModel();
  switch(filterType)
  {
  case System:
    proxy->RemoveItemDataFilter("favourites");
    proxy->SetItemDataFilter("system", true);
    break;
  case Favorites:
    proxy->RemoveItemDataFilter("system");
    proxy->SetItemDataFilter("favourites", true);
    break;
  case All:
  case None:
  default:
    proxy->RemoveItemDataFilter("favourites");
    proxy->RemoveItemDataFilter("system");
    break;
  }
}

void IdentificationWidget::applyClassifierTypeFilter()
{
  auto vfilter = ui->classifierTypeFilterCombo->currentData();
  if(!vfilter.isValid() || vfilter.toInt() == (int)LibraryFilter::Classifiers::All)
    mIdentificationModel->GetPresentationModel()->RemoveItemDataFilter("classifier_id");
  else
    mIdentificationModel->GetPresentationModel()->SetItemDataFilter("classifier_id", vfilter.toInt());
}

QSet<int> IdentificationWidget::getUsedLibraryGroupIds()
{
  QSet<int> result;
  if(mMasterChroma.isNull())
    return result;
  for(auto& ent : mMasterChroma->getEntities(GenesisMarkup::TypePeak))
  {
    if(ent->hasDataAndItsValid(GenesisMarkup::PeakLibraryGroupId))
      result << ent->getData(GenesisMarkup::PeakLibraryGroupId).toInt();
  }
  return result;
}

void IdentificationWidget::setMarkupModel(GenesisMarkup::MarkupModelPtr markupModelPtr)
{
  if(!markupModelPtr)
  {
    mIdentificationModel->Reset();
    return;
  }

  if(markupModelPtr->controller()->getCurrentStepInfo().step != GenesisMarkup::Step6Identification)
    return;
  if(!markupModelPtr->data().contains(GenesisMarkup::MarkupIdentificationData))
    return;

  QJsonObject identificationTableData = markupModelPtr->getData(GenesisMarkup::MarkupIdentificationData).value<QJsonObject>();
  mIdentificationModel->Load(identificationTableData);
  ui->compoundsTable->header()->resizeSections(QHeaderView::ResizeToContents);

  mMasterChroma = markupModelPtr->getMaster();
  connect(mMasterChroma.get(), &GenesisMarkup::ChromatogramDataModel::entityChanged, this,
          [this](EntityType type,
             TEntityUid eId,
             DataRoleType role,
             const QVariant& value)
          {
    if(type == GenesisMarkup::TypeKeySelector && role == GenesisMarkup::KeySelectorKey)
      mIdentificationModel->setCurrentKey(value.toDouble());
    else if(type == GenesisMarkup::TypePeak && role == GenesisMarkup::PeakLibraryGroupId)
      mIdentificationModel->setAlredyUsedCompoundsIds(getUsedLibraryGroupIds());
  });
  mIdentificationModel->setAlredyUsedCompoundsIds(getUsedLibraryGroupIds());
  ui->chromatogrammPlot->setChromatogrammModel(mMasterChroma, markupModelPtr->controller()->getCurrentStepInfo());
  auto peaks = mMasterChroma->getOrderedPeaks();
  auto peak = mMasterChroma->getEntity(peaks.first());
  auto curve = mMasterChroma->getEntities(GenesisMarkup::TypeCurve).first();
  auto keys = curve->getData(GenesisMarkup::CurveData).value<GenesisMarkup::TCurveData>()->keys();
  auto retTime = peak->getData(GenesisMarkup::PeakRetentionTime).toDouble();
  auto it = std::lower_bound(keys.begin(), keys.end(), retTime);
  auto prev = it - 1;
  if(it == keys.begin())
    retTime = *it;
  else if(it == keys.end())
    retTime = keys.last();
  else if(fabs(*it - retTime) > fabs(*prev - retTime))
    retTime = *prev;
  else
    retTime = *it;

  mSelectorUid = mMasterChroma->addNewEntity(GenesisMarkup::TypeKeySelector);
  mMasterChroma->setEntityData(mSelectorUid, GenesisMarkup::KeySelectorKey, retTime);
}

bool IdentificationWidget::isVisibleOnStep(GenesisMarkup::StepInfo step)
{
  return step.guiInteractions.testFlag(GenesisMarkup::SIGIdentification);
}


void IdentificationWidget::showEvent(QShowEvent *event)
{
  ui->spectrumPlot->replot();
  ui->chromatogrammPlot->replot(GraphicsPlot::rpQueuedReplot);
  if(mPendingChromaHeight != -1)
  {
    ui->chromaSplitter->setSizes({mPendingChromaHeight, ui->chromaSplitter->height() - mPendingChromaHeight});
    mPendingChromaHeight = -1;
  }
}


// bool IdentificationWidget::eventFilter(QObject *watched, QEvent *event)
// {
//   if(watched == ui->chromatogrammPlot && event->type() == QEvent::Wheel)
//   {
//     auto e = (QWheelEvent*)event;
//     if(!e->modifiers().testFlag(Qt::ControlModifier))
//     {
//       e->ignore();
//       return true;
//     }
//     else
//     {
//       return false;
//     }
//   }
//   else
//     return false;
// }
