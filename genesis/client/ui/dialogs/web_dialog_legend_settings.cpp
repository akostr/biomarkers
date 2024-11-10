#include "web_dialog_legend_settings.h"

#include "ui_web_dialog_legend_settings.h"

#include "genesis_style/style.h"

#include <QLayout>

namespace Dialogs
{
  WebDialogLegendSettings::WebDialogLegendSettings(QWidget* parent)
    : Templates::Info(parent, QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel)
  {
    setupUi();
    connectSignals();
  }

  WebDialogLegendSettings::~WebDialogLegendSettings()
  {
    delete mUi;
  }

  void WebDialogLegendSettings::setupUi()
  {
    mUi = new Ui::LegendSettingsWidget();
    auto body = new QWidget(this);
    body->setStyleSheet(Style::Genesis::GetUiStyle());
    mUi->setupUi(body);
    getContent()->layout()->addWidget(body);
    mUi->legendPositionComboBox->addItem(tr("Bottom"), static_cast<int>(QInternal::BottomDock));
    mUi->legendPositionComboBox->addItem(tr("Top"), static_cast<int>(QInternal::TopDock));
    mUi->legendPositionComboBox->addItem(tr("Left"), static_cast<int>(QInternal::LeftDock));
    mUi->legendPositionComboBox->addItem(tr("Right"), static_cast<int>(QInternal::RightDock));
    updateAlignComboBox(0);
    Settings settings;
    settings.buttonsNames = {
      { QDialogButtonBox::StandardButton::Ok, tr("Save") },
      { QDialogButtonBox::StandardButton::Cancel, tr("Cancel") },
    };
    settings.dialogHeader = tr("Legend position");
    settings.contentHeader = "";
    applySettings(settings);
  }

  void WebDialogLegendSettings::connectSignals()
  {
    connect(mUi->legendPositionComboBox, &QComboBox::currentIndexChanged, this, &WebDialogLegendSettings::updateAlignComboBox);
  }

  void WebDialogLegendSettings::updateAlignComboBox(int)
  {
    const auto data = mUi->legendPositionComboBox->currentData();
    if (data.isValid() && !data.isNull())
    {
      mUi->legendAlignComboBox->clear();
      const auto legendPosition = static_cast<QInternal::DockPosition>(data.toInt());
      if (legendPosition == QInternal::BottomDock
        || legendPosition == QInternal::TopDock)
      {
        mUi->legendAlignComboBox->addItem(tr("Left"), static_cast<int>(Qt::AlignLeft));
        mUi->legendAlignComboBox->addItem(tr("Center"), static_cast<int>(Qt::AlignHCenter));
        mUi->legendAlignComboBox->addItem(tr("Right"), static_cast<int>(Qt::AlignRight));
      }
      else
      {
        mUi->legendAlignComboBox->addItem(tr("Top"), static_cast<int>(Qt::AlignTop));
        mUi->legendAlignComboBox->addItem(tr("Center"), static_cast<int>(Qt::AlignVCenter));
        mUi->legendAlignComboBox->addItem(tr("Bottom"), static_cast<int>(Qt::AlignBottom));
      }
    }
  }

  void WebDialogLegendSettings::setApplyToAll(bool apply)
  {
    mUi->applyToAllCheckBox->setChecked(apply);
  }

  bool WebDialogLegendSettings::applyToAll() const
  {
    return mUi->applyToAllCheckBox->isChecked();
  }

  QInternal::DockPosition WebDialogLegendSettings::legendPosition() const
  {
    return static_cast<QInternal::DockPosition>(mUi->legendPositionComboBox->currentData().toInt());
  }

  void WebDialogLegendSettings::setLegendPosition(QInternal::DockPosition position)
  {
    const auto index = mUi->legendPositionComboBox->findData(static_cast<int>(position));
    mUi->legendPositionComboBox->setCurrentIndex(index);
  }

  Qt::Alignment WebDialogLegendSettings::legendAlign() const
  {
    return static_cast<Qt::Alignment>(mUi->legendAlignComboBox->currentData().toInt());
  }

  void WebDialogLegendSettings::setLegendAlign(Qt::Alignment align)
  {
    const auto index = mUi->legendAlignComboBox->findData(static_cast<int>(align));
    mUi->legendAlignComboBox->setCurrentIndex(index);
  }
}