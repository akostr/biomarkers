#include "web_dialog_pls_build_settings_refactor.h"
#include "ui_web_dialog_pls_build_settings_refactor.h"

#include <QPushButton>

#include "ui/item_models/checkable_string_list_model.h"
#include "ui/controls/multi_select_combobox.h"
#include "genesis_style/style.h"

using namespace Models;
using namespace Controls;
using namespace Structures;

WebDialogPLSBuildSettingsRefactor::WebDialogPLSBuildSettingsRefactor(QWidget* parent)
  : Dialogs::Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  , ui(new Ui::WebDialogPLSBuildSettingsRefactor)
  , mContent(new QWidget(nullptr))
{
  SetupUi();
}

WebDialogPLSBuildSettingsRefactor::~WebDialogPLSBuildSettingsRefactor()
{
  delete ui;
}

void WebDialogPLSBuildSettingsRefactor::SetMaxPCCount(size_t value)
{
  ui->spinBox_PCMaxCount->setMaximum(value);
}

void WebDialogPLSBuildSettingsRefactor::SetPCValue(size_t value)
{
  ui->spinBox_PCMaxCount->setValue(value);
}

void WebDialogPLSBuildSettingsRefactor::SetMaxSamplesPerSegmentCount(size_t value)
{
  ui->spinBox_samplePerSegment->setMaximum(value);
}

void WebDialogPLSBuildSettingsRefactor::SetSamplesPerSegmentCount(size_t value)
{
  ui->spinBox_samplePerSegment->setValue(value);
}

size_t WebDialogPLSBuildSettingsRefactor::MaxCountOfPC() const
{
  return static_cast<size_t>(ui->spinBox_PCMaxCount->value());
}

bool WebDialogPLSBuildSettingsRefactor::Autoscale() const
{
  return ui->checkBox_enableAutoscale->checkState() == Qt::Checked;
}

void WebDialogPLSBuildSettingsRefactor::SetAutoscale(bool autoscale)
{
  ui->checkBox_enableAutoscale->setChecked(autoscale);
}

bool WebDialogPLSBuildSettingsRefactor::Normalization() const
{
  return ui->checkBox_enableNormalization->checkState() == Qt::Checked;
}

void WebDialogPLSBuildSettingsRefactor::SetNormalization(bool normalization)
{
  ui->checkBox_enableNormalization->setChecked(normalization);
}

bool WebDialogPLSBuildSettingsRefactor::IsCross() const
{
  return ui->radioButton_crossMethod->isChecked();
}

void WebDialogPLSBuildSettingsRefactor::SetIsCross(bool isCross)
{
  emit ui->radioButton_crossMethod->toggled(isCross);
  emit ui->radioButton_testMethod->toggled(!isCross);
}

bool WebDialogPLSBuildSettingsRefactor::IsFull() const
{
  return ui->tabSwitcher_fullSegmented->currentIndex() == 0;
}

void WebDialogPLSBuildSettingsRefactor::SetIsFull(bool isFull)
{
  emit ui->tabSwitcher_fullSegmented->setCurrentIndex(isFull ? 0 : 1);
}

bool WebDialogPLSBuildSettingsRefactor::IsSegment() const
{
  return ui->tabSwitcher_fullSegmented->currentIndex() == 1;
}

void WebDialogPLSBuildSettingsRefactor::SetIsSegment(bool isSegment)
{
  emit ui->tabSwitcher_fullSegmented->setCurrentIndex(isSegment ? 1 : 0);
}

size_t WebDialogPLSBuildSettingsRefactor::SamplesPerSegment() const
{
  return static_cast<size_t>(ui->spinBox_samplePerSegment->value());
}

void WebDialogPLSBuildSettingsRefactor::SetEnableAdditionParameters(bool isEnable)
{
  ui->checkBox_enableAutoscale->setEnabled(isEnable);
  ui->checkBox_enableNormalization->setEnabled(isEnable);
}

void WebDialogPLSBuildSettingsRefactor::SetTestData(const std::map<int, QString>& list)
{
  if (const auto box = findChild<MultiSelectComboBox*>())
  {
    box->clear();
    box->AddTestData(list);
    box->clearEditText();
  }
}

void WebDialogPLSBuildSettingsRefactor::SetSelectedData(const std::vector<int>& selected)
{
  if (const auto box = findChild<MultiSelectComboBox*>())
  {
    box->SetSelectedItems(selected);
  }
}

QVector<int> WebDialogPLSBuildSettingsRefactor::GetSelectedData()
{
  if (const auto box = findChild<MultiSelectComboBox*>())
  {
    const auto vec = box->SelectedItems();
    return { vec.begin(), vec.end() };
  }
  return {};
}

void WebDialogPLSBuildSettingsRefactor::SetupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("PLS model building parameters");
  s.buttonsNames = { {QDialogButtonBox::Ok, tr("Build PLS model")},
                   {QDialogButtonBox::Cancel, tr("Cancel")} };
  applySettings(s);

  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);

  ui->spinBox_PCMaxCount->setMinimum(1);

  auto stubCombo = new MultiSelectComboBox();
  ui->horizontalLayout_testMethod->addWidget(stubCombo);
  const auto model = new CheckableStringListModel();
  connect(model, &QAbstractTableModel::dataChanged, this, &WebDialogPLSBuildSettingsRefactor::SelectedDataChanged);
  stubCombo->setModel(model);
  ui->label->setStyleSheet(Style::ApplySASS("QLabel {font: @RegularBoldFont;}"));
  ui->label_2->setStyleSheet(Style::ApplySASS("QLabel {font: @RegularBoldFont;}"));
  ui->label_4->setStyleSheet(Style::ApplySASS("QLabel {font: @RegularBoldFont;}"));

  /// set hiding logic for group
  connect(ui->radioButton_testMethod, &QRadioButton::toggled, this, [this, stubCombo](bool checked)
    {
      stubCombo->setVisible(checked);
    });

  ui->tabSwitcher_fullSegmented->addTab(tr("Full"));
  ui->tabSwitcher_fullSegmented->addTab(tr("Segmented"));

  connect(ui->tabSwitcher_fullSegmented, &TabSwitcher::currentChanged, this,
    [this](int index)
    {
      ui->label_samplePerSegment->setVisible(index && ui->radioButton_crossMethod->isChecked());
      ui->spinBox_samplePerSegment->setVisible(index && ui->radioButton_crossMethod->isChecked());
    });

  connect(ui->radioButton_crossMethod, &QRadioButton::toggled, this,
    [this, stubCombo](bool checked)
    {
      ui->tabSwitcher_fullSegmented->setVisible(checked);
      auto segmMethod = ui->tabSwitcher_fullSegmented->currentIndex() == 1;
      ui->label_samplePerSegment->setVisible(checked && segmMethod);
      ui->spinBox_samplePerSegment->setVisible(checked && segmMethod);
      stubCombo->setVisible(!checked);
    });

  ui->radioButton_crossMethod->setChecked(true);
  SetIsFull(true);
}

void WebDialogPLSBuildSettingsRefactor::SelectedDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles)
{
  if (!roles.contains(Qt::CheckStateRole))
    return;

  bool checked = true;
  if (ui->radioButton_testMethod->isChecked())
  {
    if (const auto cb = findChild<MultiSelectComboBox*>())
    {
      if (const auto model = qobject_cast<CheckableStringListModel*>(cb->model()))
      {
        checked = model->SelectedCount() > 1;
      }
    }
  }

  const auto okBtn = ButtonBox->button(QDialogButtonBox::Ok);
  okBtn->setEnabled(checked);
}
