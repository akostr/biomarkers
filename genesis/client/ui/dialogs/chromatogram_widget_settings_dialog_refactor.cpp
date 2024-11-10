#include "chromatogram_widget_settings_dialog_refactor.h"
#include "chromatogram_widget_settings_paramerets_dialog.h"
#include "ui_chromatogram_widget_settings_dialog_refactor.h"
#include "ui_chromatogram_widget_settings_paramerets_dialog.h"

#include <QLayout>
#include <QPushButton>
#include <QScrollArea>

#include "genesis_style/style.h"
#include "logic/markup/baseline_data_model.h"
#include "logic/markup/genesis_markup_enums.h"
#include "ui/dialogs/templates/confirm.h"

ChromatogramWidgetSettingsDialogRefactor::ChromatogramWidgetSettingsDialogRefactor(GenesisMarkup::ChromatogrammModelPtr model,
  const GenesisMarkup::StepInfo& stepInfo, int startTab, QWidget* parent) :
  Dialogs::Templates::Dialog(parent, QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  , ui(new Ui::ChromatogramWidgetSettingsDialogRefactor)
  , mContent(new QWidget(nullptr))
  , mChromatogramModel(model)
  , mStepInfo(stepInfo)
{
  setupUi();
  if(startTab < ui->tabWidget->count())
    ui->tabWidget->setCurrentIndex(startTab);

  bool enabled = (mChromatogramModel->isMaster() && mStepInfo.masterInteractions.testFlag(GenesisMarkup::SIMSetupParameters)) ||
                  (!mChromatogramModel->isMaster() && mStepInfo.slaveInteractions.testFlag(GenesisMarkup::SISSetupParameters));

  for(int i = 0; i < ui->tabWidget->count(); i++)
    ui->tabWidget->widget(i)->setEnabled(enabled);
  ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

ChromatogramWidgetSettingsDialogRefactor::~ChromatogramWidgetSettingsDialogRefactor()
{
  delete ui;
}

void ChromatogramWidgetSettingsDialogRefactor::setupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Chromatogram parameters");
  s.buttonsNames = { {QDialogButtonBox::RestoreDefaults, tr("Set the default settings")},
                   {QDialogButtonBox::Ok, tr("Apply")},
                   {QDialogButtonBox::Cancel, tr("Cancel")} };
  s.buttonsProperties = { {QDialogButtonBox::RestoreDefaults, {{"gray", true}}},
                        {QDialogButtonBox::Ok, {{"gray", true}}},
                        {QDialogButtonBox::Cancel, {{"gray", true}}} };
  applySettings(s);

  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);

  mSetDefaultSettings = ButtonBox->button(QDialogButtonBox::StandardButton::RestoreDefaults);

  mMainWaidget = new ChromatogramWidgetSettingsParameretsDialog(mChromatogramModel, mStepInfo, false,
    mSetDefaultSettings, this);
  ui->main_tab->layout()->addWidget(mMainWaidget);

  GenesisMarkup::ChromaSettings main_settings = GenesisMarkup::ChromaSettings();

  {
    using namespace GenesisMarkup;
    main_settings.Min_h = (mChromatogramModel->getChromatogramValue(ChromatogrammSettingsMinimalPeakHeight).toDouble());
    main_settings.Noisy = (mChromatogramModel->getChromatogramValue(ChromatogrammSettingsNoisy).toBool());
    main_settings.View_smooth = (mChromatogramModel->getChromatogramValue(ChromatogrammSettingsViewSmooth).toBool());
    main_settings.Window_size = (mChromatogramModel->getChromatogramValue(ChromatogrammSettingsMedianFilterWindowSize).toInt());
    main_settings.Sign_to_med = (mChromatogramModel->getChromatogramValue(ChromatogrammSettingsSignalToMedianRatio).toDouble());
    main_settings.Doug_peuck = (mChromatogramModel->getChromatogramValue(ChromatogrammSettingsDegreeOfApproximation).toInt());
    main_settings.Med_bounds = (mChromatogramModel->getChromatogramValue(ChromatogrammSettingsBoundsByMedian).toBool());
    main_settings.Coel = (mChromatogramModel->getChromatogramValue(ChromatogrammSettingsCoelution).toInt());
  }

  setSettings(main_settings);

  ///@todo intervals
  {
    QVBoxLayout* scrollAreaContentLayoutCustomWidget = nullptr;

    {
      auto scrollArea = new QScrollArea(this);
      ui->custom_widget->layout()->addWidget(scrollArea);
      auto scrollAreaContent = new QWidget(scrollArea);

      scrollArea->setFrameShape(QFrame::NoFrame);
      scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      scrollArea->setAutoFillBackground(false);
      scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
      scrollArea->setWidget(scrollAreaContent);
      scrollArea->setWidgetResizable(true);

      scrollAreaContentLayoutCustomWidget = new QVBoxLayout(scrollAreaContent);
      scrollAreaContentLayoutCustomWidget->setContentsMargins(0, 0, 0, 0);
      scrollAreaContentLayoutCustomWidget->setSpacing(0);
    }

    QVBoxLayout* scrollAreaContentLayoutLockedWidget = nullptr;

    {
      auto scrollArea = new QScrollArea(this);
      ui->locked_widget->layout()->addWidget(scrollArea);
      auto scrollAreaContent = new QWidget(scrollArea);

      scrollArea->setFrameShape(QFrame::NoFrame);
      scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      scrollArea->setAutoFillBackground(false);
      scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
      scrollArea->setWidget(scrollAreaContent);
      scrollArea->setWidgetResizable(true);

      scrollAreaContentLayoutLockedWidget = new QVBoxLayout(scrollAreaContent);
      scrollAreaContentLayoutLockedWidget->setContentsMargins(0, 0, 0, 0);
      scrollAreaContentLayoutLockedWidget->setSpacing(0);
    }

    auto uids = mChromatogramModel->getUIdListOfEntities(GenesisMarkup::TypeInterval);
    if (!uids.isEmpty())
    {
      ui->custom_widget->setEnabled(true);
      ui->locked_widget->setEnabled(true);

      std::sort(uids.begin(), uids.end(), [this](const QUuid& a, const QUuid& b) -> bool
      {
        auto bData = mChromatogramModel->getEntity(b);
        auto aData = mChromatogramModel->getEntity(a);
        if (aData->hasDataAndItsValid(GenesisMarkup::IntervalLeft))
        {
          return false;  // a >= b
        }
        else
        {
          if (bData->hasDataAndItsValid(GenesisMarkup::IntervalLeft))
            return true;  // a < b
          else
          {
            auto left = aData->getData(GenesisMarkup::IntervalLeft).value<GenesisMarkup::TIntervalLeft>();
            auto right = aData->getData(GenesisMarkup::IntervalRight).value<GenesisMarkup::TIntervalRight>();
            return left < right;
          }
        }
      });

      GenesisMarkup::IntervalTypes interval_type = GenesisMarkup::ITNone;
      int custom_number = 1;
      int locked_number = 1;

      for (int i = 0; i < uids.size(); i++)
      {
        auto& uid = uids[i];
        auto entityPtr = mChromatogramModel->getEntity(uid);
        if (!entityPtr)
          continue;
        else
        {
          auto x_start = entityPtr->value(GenesisMarkup::IntervalLeft).value<GenesisMarkup::TIntervalLeft>();
          auto x_end = entityPtr->value(GenesisMarkup::IntervalRight).value<GenesisMarkup::TIntervalRight>();

          GenesisMarkup::IntervalTypes interval_type = (GenesisMarkup::IntervalTypes)entityPtr->getData(GenesisMarkup::IntervalType).toInt();;
          switch (interval_type) {
          case GenesisMarkup::ITNone:
            break;
          case GenesisMarkup::ITCustomParameters:
          {
            QString lock_interval = QString(tr("Interval %1 from %2 to %3").arg(custom_number).arg(x_start).arg(x_end));
            GenesisMarkup::ChromaSettings interval_settings;
            {
              using namespace GenesisMarkup;
              interval_settings.Min_h = entityPtr->getData(MinimalPeakHeight).toDouble();
              interval_settings.Noisy = entityPtr->getData(Noisy).toBool();
              interval_settings.View_smooth = entityPtr->getData(ViewSmooth).toBool();
              interval_settings.Window_size = entityPtr->getData(MedianFilterWindowSize).toInt();
              interval_settings.Sign_to_med = entityPtr->getData(SignalToMedianRatio).toDouble();
              interval_settings.Doug_peuck = entityPtr->getData(DegreeOfApproximation).toInt();
              interval_settings.Med_bounds = entityPtr->getData(BoundsByMedian).toBool();
              interval_settings.Coel = entityPtr->getData(Coelution).toInt();
            }

            auto getLabelText = [lock_interval](bool collapseDown = true)
            {
              return QString("<table><tr><td>&nbsp;<a href=\"showHide\"><img src=\":/resource/controls/%1.png\"/></a></td>\
                <td valign=\"middle\">&nbsp;&nbsp;&nbsp;%2</td></tr></table>").
                arg(collapseDown ? "collpse_dn" : "collpse_up", lock_interval);
            };
            auto warningText = QString(tr("Are you sure you want to delete the individual\nparameter interval set between %1 and %2?")).
              arg(x_start).arg(x_end);

            auto widget = new QWidget(this);
            auto label = new QLabel(getLabelText());
            widget->setStyleSheet(Style::ApplySASS("QWidget {background-color: @uiInputElementColorBgDisabled;}"));
            label->setStyleSheet(Style::ApplySASS("QLabel {background-color: @uiInputElementColorBgDisabled;}"));
            auto hLayout = new QHBoxLayout(widget);
            hLayout->addWidget(label);

            auto removeBtn = new QPushButton(QIcon(":/resource/icons/icon_cross.png"), "");
            removeBtn->setStyleSheet(Style::ApplySASS("QPushButton {border-color: @uiInputElementColorBgDisabled;}"));
            removeBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            removeBtn->setIconSize({ 11, 11 });
            removeBtn->setFixedSize({ 11, 11 });
            hLayout->addWidget(removeBtn);

            scrollAreaContentLayoutCustomWidget->addWidget(widget);

            auto settingsWidget = new ChromatogramWidgetSettingsParameretsDialog(mChromatogramModel, mStepInfo, true, nullptr, this);
            settingsWidget->setSettings(interval_settings);
            scrollAreaContentLayoutCustomWidget->addWidget(settingsWidget);
            settingsWidget->hide();
            mCustomWidgetsMap[uid] = settingsWidget;

            connect(removeBtn, &QPushButton::clicked, [&, settingsWidget, widget, uid, warningText, &deletedIntervalUid = mDeletedIntervalUid]
            {
              auto dial = Dialogs::Templates::Confirm::warning(this, tr("Warning"), warningText,
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
              connect(dial, &WebDialog::Accepted, [settingsWidget, widget, uid, &deletedIntervalUid]()
              {
                widget->hide();
                settingsWidget->hide();
                deletedIntervalUid.push_back(uid);
              });
              dial->Open();
            });

            connect(label, &QLabel::linkActivated, [settingsWidget, label, getLabelText](const QString& str)
            {
              if (str == "showHide")
              {
                if (settingsWidget->isHidden())
                {
                  settingsWidget->show();
                  label->setText(getLabelText(false));
                }
                else
                {
                  settingsWidget->hide();
                  label->setText(getLabelText());
                }
              }
            });

            custom_number++;
            break;
          }
          case GenesisMarkup::ITMarkupDeprecation:
          {
            mDeprecationIntervals[uid] = { x_start, x_end };
            QString labelText = QString(tr("Interval boundaries %1 from %2 to %3").arg(locked_number).arg(x_start).arg(x_end));
            auto warningText = QString(tr("Are you sure you want to delete the markup\ndeprecation interval set between %1 and %2?")).
              arg(x_start).arg(x_end);

            auto widget = new QWidget(this);
            auto label = new QLabel(labelText);
            widget->setStyleSheet(Style::ApplySASS("QWidget {background-color: @uiInputElementColorBgDisabled;}"));
            label->setStyleSheet(Style::ApplySASS("QLabel {background-color: @uiInputElementColorBgDisabled;}"));
            auto hLayout = new QHBoxLayout(widget);
            hLayout->addWidget(label);

            auto removeBtn = new QPushButton(QIcon(":/resource/icons/icon_cross.png"), "");
            removeBtn->setStyleSheet(Style::ApplySASS("QPushButton {border-color: @uiInputElementColorBgDisabled;}"));
            removeBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            removeBtn->setIconSize({ 11, 11 });
            removeBtn->setFixedSize({ 11, 11 });
            hLayout->addWidget(removeBtn);

            scrollAreaContentLayoutLockedWidget->addWidget(widget);

            connect(removeBtn, &QPushButton::clicked, [&, widget, uid, warningText,
              &deprecationIntervals = mDeprecationIntervals]
            {
              auto dial = Dialogs::Templates::Confirm::warning(this, tr("Warning"), warningText,
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
              connect(dial, &WebDialog::Accepted, [widget, uid, &deprecationIntervals]()
                {
                  widget->hide();
                  deprecationIntervals.remove(uid);
                });
              dial->Open();
            });

            locked_number++;
            break;
          }
          }
        }
      }
    }

    if (mCustomWidgetsMap.empty())
      scrollAreaContentLayoutCustomWidget->addWidget(new QLabel(tr("No interval with individual parameters has been added")));

    if (mDeprecationIntervals.empty())
      scrollAreaContentLayoutLockedWidget->addWidget(new QLabel(tr("No markup locked interval has been added")));

    scrollAreaContentLayoutCustomWidget->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
    scrollAreaContentLayoutLockedWidget->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
  }

    //// Connect
  connect(ui->tabWidget, &QTabWidget::currentChanged, [&]
  {
    if (ui->tabWidget->currentWidget() == ui->main_tab)
      mSetDefaultSettings->setVisible(true);
    else
      mSetDefaultSettings->setVisible(false);
  });

  connect(mSetDefaultSettings, &QPushButton::clicked, [&]
  {
    setSettings(GenesisMarkup::ChromaSettings());
    mSetDefaultSettings->setDisabled(true);
  });
}

void ChromatogramWidgetSettingsDialogRefactor::Accept()
{
  QString warningText;
  if(mChromatogramModel->isMaster())
    warningText = tr("Are you sure you want to apply the new settings for the reference\nchromatogram? The chromatogram markup will be reset.");
  else
    warningText = tr("Are you sure you want to apply the new settings for the chromatogram?\nThe chromatogram markup will be reset.");

  auto dial = Dialogs::Templates::Confirm::warning(this, tr("Warning"), warningText,
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(dial, &WebDialog::Accepted, [&]()
    {
      mMainWaidget->setOriginBaseLine();
      Dialog::Accept();
    });
  dial->Open();
}

void ChromatogramWidgetSettingsDialogRefactor::Reject()
{
  mMainWaidget->setOriginBaseLine();
  Dialog::Reject();
}

void ChromatogramWidgetSettingsDialogRefactor::setSettings(const GenesisMarkup::ChromaSettings& settings)
{
  mMainWaidget->setSettings(settings);
}

GenesisMarkup::ChromaSettings ChromatogramWidgetSettingsDialogRefactor::getSettings()
{
  return mMainWaidget->getSettings();
}

QMap<QUuid, GenesisMarkup::ChromaSettings> ChromatogramWidgetSettingsDialogRefactor::getIntervalsSettings()
{
  QMap <QUuid, GenesisMarkup::ChromaSettings> intervals;
  foreach(auto uid, mCustomWidgetsMap.keys())
  {
    if(mDeletedIntervalUid.contains(uid))
      continue;
    intervals[uid] = mCustomWidgetsMap[uid]->getSettings();
  }
  return intervals;
}

QMap<QUuid, QPair<double, double>> ChromatogramWidgetSettingsDialogRefactor::getDeprecationIntervals() const
{
  return mDeprecationIntervals;
}

int ChromatogramWidgetSettingsDialogRefactor::getOriginSmoothLvl() const
{
  return mMainWaidget->getOriginSmoothLvl();
}

int ChromatogramWidgetSettingsDialogRefactor::getNewSmoothLvl() const
{
  return mMainWaidget->getNewSmoothLvl();
}
