#include "chromatogram_widget_settings_paramerets_dialog.h"
#include "ui_chromatogram_widget_settings_paramerets_dialog.h"

#include <QLayout>

#include "genesis_style/style.h"
#include "logic/markup/baseline_data_model.h"
#include "logic/markup/genesis_markup_enums.h"
#include "logic/notification.h"
#include "ui/dialogs/web_overlay.h"

ChromatogramWidgetSettingsParameretsDialog::ChromatogramWidgetSettingsParameretsDialog(GenesisMarkup::ChromatogrammModelPtr model,
  const GenesisMarkup::StepInfo& stepInfo, bool hideSmoothLevel, QPushButton* setDefaultSettings,
  QWidget* parent) :
  QWidget(parent),
  ui(new Ui::ChromatogramWidgetSettingsParameretsDialog)
  , mChromatogramModel(model)
  , mStepInfo(stepInfo)
  , mSetDefaultSettings(setDefaultSettings)
{
  ui->setupUi(this);
  setupUi();

  if (hideSmoothLevel)
  {
    ui->SmoothLevel->hide();
    ui->SmoothLevel_label->hide();
  }
}

ChromatogramWidgetSettingsParameretsDialog::~ChromatogramWidgetSettingsParameretsDialog()
{
  delete ui;
}

void ChromatogramWidgetSettingsParameretsDialog::setupUi()
{
  if (mChromatogramModel->isMaster())
  {
    if (mStepInfo.step < GenesisMarkup::Step7Complete)
    {
      mForbidden = false;
    }
  }
  else
  {
    mForbidden = false;
  }

  ui->tabWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/ui_qtabbar_buttons.qss"));
  ui->tabWidget->tabBar()->setDocumentMode(true);

  ui->SmoothLevel_label->setStyleSheet(Style::ApplySASS("QLabel {font: @defaultFontBold;}"));
  ui->Min_h_label->setStyleSheet(Style::ApplySASS("QLabel {font: @defaultFontBold;}"));
  ui->Chromatogram_type_label->setStyleSheet(Style::ApplySASS("QLabel {font: @defaultFontBold;}"));
  //ui->Med_bounds->setStyleSheet(Style::ApplySASS("QCheckBox {font: @defaultFont; color: @textColorPale;}"));
  ui->View_smooth->setStyleSheet(Style::ApplySASS("QCheckBox {font: @defaultFont;}"));

  ui->Min_h->setEnabled(!mForbidden);
  ui->Min_h->setRange(0, std::numeric_limits<double>::max());

  ui->Noisy->setEnabled(!mForbidden);
  ui->Not_noisy->setEnabled(!mForbidden);
  ui->View_smooth->setEnabled(!mForbidden);

  //ui->Window_size->setEnabled(!mForbidden);
  //ui->Window_size->setRange(1, std::numeric_limits<int>::max());
  //ui->Window_size->setSingleStep(2);

  ui->Noisy_window_size->setEnabled(!mForbidden);
  ui->Noisy_window_size->setRange(1, std::numeric_limits<int>::max());
  ui->Noisy_window_size->setSingleStep(2);

  ui->Sign_to_med->setEnabled(!mForbidden);
  ui->Sign_to_med->setRange(0, std::numeric_limits<double>::max());

  ui->Doug_peuck->setEnabled(!mForbidden);
  ui->Doug_peuck->setRange(0, 100);

  ui->Coel->setEnabled(!mForbidden);
  ui->Coel->setRange(0, 100);


  //// Smoothing
  {
    auto overlay = new WebOverlay("", ui->SmoothLevel);
    API::REST::BaselineData(mChromatogramModel->id(), true, {},
                            [this, overlay](QNetworkReply*, QJsonDocument data)
                            {
                              auto jroot = data.object();
                              if(jroot["error"].toBool())
                              {
                                Notification::NotifyError(tr("Failed to load baseline smooth lvls"), jroot["msg"].toString());
                                overlay->deleteLater();
                                return;
                              }
                              auto jlvls = jroot["baseline_smooth_levels"].toArray();
                              QList<int> lvls;
                              for(int i = 0; i < jlvls.size(); i++)
                                lvls << jlvls[i].toString().toInt();
                              using namespace GenesisMarkup;
                              auto blines = mChromatogramModel->getEntities(TypeBaseline);
                              if (!blines.empty())
                              {
                                mBLineDataModelPtr = blines.first()->getData(BaseLineData).value<TBaseLineDataModel>();
                                auto currentSmoothLvl = mBLineDataModelPtr->getSmoothFactor();
                                std::sort(lvls.begin(), lvls.end());

                                for (const auto& lvl : lvls)
                                {
                                  ui->SmoothLevel->addItem(QString::number(lvl), lvl);
                                  if (lvl == currentSmoothLvl)
                                    ui->SmoothLevel->setCurrentIndex(ui->SmoothLevel->count() - 1);
                                }
                                mOriginSmoothLvl = currentSmoothLvl;
                                connect(ui->SmoothLevel, &QComboBox::currentIndexChanged, this, [this](int index)
                                        {
                                          if (index == -1)
                                            return;
                                          mBLineDataModelPtr->setBaseLine(ui->SmoothLevel->currentData().toInt(), mChromatogramModel->id());
                                        });
                              }
                              else
                              {
                                ui->SmoothLevel->addItem(QString::number(Defaults::kChromatogrammSettingsBaseLineSmoothLvlDefault),
                                                         Defaults::kChromatogrammSettingsBaseLineSmoothLvlDefault);
                                ui->SmoothLevel->setCurrentIndex(ui->SmoothLevel->count() - 1);
                                ui->SmoothLevel->setEnabled(false);
                                mOriginSmoothLvl = Defaults::kChromatogrammSettingsBaseLineSmoothLvlDefault;
                              }
                              overlay->deleteLater();
                            },
                            [overlay](QNetworkReply*, QNetworkReply::NetworkError err)
                            {
                              Notification::NotifyError(tr("Failed to load baseline smooth lvls"), err);
                              overlay->deleteLater();
                            });
  }

  //// Connect
  //connect(ui->Med_bounds, &QCheckBox::stateChanged, [&] { update(); checkDefaultSettings(); });
  connect(ui->View_smooth, &QCheckBox::stateChanged, [&] { update(); checkDefaultSettings(); });
  connect(ui->SmoothLevel, &QComboBox::currentIndexChanged, [&] { checkDefaultSettings(); });
  connect(ui->Min_h, &QDoubleSpinBox::valueChanged, [&] { checkDefaultSettings(); });
  connect(ui->Coel, &QSpinBox::valueChanged, [&] { checkDefaultSettings(); });
  //connect(ui->Window_size, &QSpinBox::valueChanged, [&] { checkDefaultSettings(); });
  connect(ui->tabWidget, &QTabWidget::currentChanged, [&] { checkDefaultSettings(); });
  connect(ui->Doug_peuck, &QSpinBox::valueChanged, [&] { checkDefaultSettings(); });
  connect(ui->Sign_to_med, &QDoubleSpinBox::valueChanged, [&] { checkDefaultSettings(); });

  ///// Validation
  ///
  /*
  connect(ui->Window_size, &QSpinBox::valueChanged, [this]
    {
      int val = ui->Window_size->value();
      if (val % 2 == 0)
      {
        ui->Window_size->setValue(val - 1);
        return;
      }

      ui->Noisy_window_size->blockSignals(true);
      ui->Noisy_window_size->setValue(val);
      ui->Noisy_window_size->blockSignals(false);
    });
  */
  connect(ui->Noisy_window_size, &QSpinBox::valueChanged, [this]
    {
      int val = ui->Noisy_window_size->value();
      if (val % 2 == 0)
      {
        ui->Noisy_window_size->setValue(val - 1);
        return;
      }

      //ui->Window_size->blockSignals(true);
      //ui->Window_size->setValue(val);
      //ui->Window_size->blockSignals(false);
    });

  update();
  checkDefaultSettings();
}

void ChromatogramWidgetSettingsParameretsDialog::update()
{
   /*
  {
    auto checked = ui->Med_bounds->isChecked();
    ui->Window_size_label->setHidden(!checked);
    ui->Window_size->setHidden(!checked);
  }
  */
  {
    auto checked = ui->View_smooth->isChecked();
    ui->Noisy_window_size_label->setHidden(!checked);
    ui->Noisy_window_size->setHidden(!checked);
    ui->Sign_to_med_label->setHidden(!checked);
    ui->Sign_to_med->setHidden(!checked);
  }
}

void ChromatogramWidgetSettingsParameretsDialog::checkDefaultSettings()
{
  if (mSetDefaultSettings)
  {
    if (GenesisMarkup::ChromaSettings() != getSettings())
    {
      mSetDefaultSettings->setEnabled(true);
    }
    else
    {
      mSetDefaultSettings->setDisabled(true);
    }
  }
}

void ChromatogramWidgetSettingsParameretsDialog::setSettings(const GenesisMarkup::ChromaSettings& settings)
{
  mSettings = settings;
  ui->Min_h->setValue(settings.Min_h);
  ui->tabWidget->setCurrentWidget(settings.Noisy ? ui->Noisy : ui->Not_noisy);
  ui->View_smooth->setChecked(settings.View_smooth);
  //ui->Window_size->setValue(settings.Window_size);
  ui->Noisy_window_size->setValue(settings.Window_size);
  ui->Sign_to_med->setValue(settings.Sign_to_med);
  ui->Doug_peuck->setValue(settings.Doug_peuck);
  //ui->Med_bounds->setChecked(settings.Med_bounds);
  ui->Coel->setValue(settings.Coel);
}

GenesisMarkup::ChromaSettings ChromatogramWidgetSettingsParameretsDialog::getSettings()
{
  auto settings = mSettings;
  settings.Min_h = ui->Min_h->value();
  settings.Noisy = ui->tabWidget->currentWidget() == ui->Noisy;
  settings.View_smooth = ui->View_smooth->isChecked();
  //settings.Window_size = ui->Window_size->value();
  settings.Sign_to_med = ui->Sign_to_med->value();
  settings.Doug_peuck = ui->Doug_peuck->value();
  //settings.Med_bounds = ui->Med_bounds->isChecked();
  settings.Coel = ui->Coel->value();
  settings.BLineSmoothLvl = ui->SmoothLevel->currentData().toInt();
  mSettings = settings;
  return settings;
}

int ChromatogramWidgetSettingsParameretsDialog::getOriginSmoothLvl() const
{
  return mOriginSmoothLvl;
}

int ChromatogramWidgetSettingsParameretsDialog::getNewSmoothLvl() const
{
  return ui->SmoothLevel->currentData().toInt();
}

void ChromatogramWidgetSettingsParameretsDialog::setOriginBaseLine()
{
  if(!mBLineDataModelPtr)
    return;
  if (getNewSmoothLvl() != getOriginSmoothLvl())
    mBLineDataModelPtr->setBaseLine(getOriginSmoothLvl());
}
