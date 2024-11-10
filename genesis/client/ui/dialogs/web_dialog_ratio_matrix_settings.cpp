#include "web_dialog_ratio_matrix_settings.h"

#include "ui_web_dialog_ratio_matrix_settings.h"
#include <genesis_style/style.h>

#include <ui/controls/dialog_line_edit_ex.h>
#include <QPushButton>

namespace Dialogs
{
  WebDialogRatioMatrixSettings::WebDialogRatioMatrixSettings(QWidget* parent)
    :Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  {
    auto s = Templates::Dialog::Settings();
    s.dialogHeader = tr("Ratio heights matrix generation parameters");
    s.buttonsNames = {{QDialogButtonBox::Ok, tr("Generate matrix")}};
    s.buttonsProperties = {{QDialogButtonBox::Ok, {{"blue", true}}}};
    SetupUi();
    applySettings(s);
    Size = QSizeF(0.25, 0.3);
    UpdateGeometry();
  }

  WebDialogRatioMatrixSettings::~WebDialogRatioMatrixSettings()
  {
    delete WidgetUi;
  }

  QString WebDialogRatioMatrixSettings::GetTitle() const
  {
    return WidgetUi->matrixTitleEdit->text();
  }

  QString WebDialogRatioMatrixSettings::GetComment() const
  {
    return WidgetUi->matrixCommentEdit->text();
  }

  double WebDialogRatioMatrixSettings::GetWindowSize() const
  {
    return WidgetUi->WindowSizeSpinBox->value();
  }

  double WebDialogRatioMatrixSettings::GetMinimum() const
  {
    return WidgetUi->MinimumSpinBox->value();
  }

  double WebDialogRatioMatrixSettings::GetMaximum() const
  {
    return WidgetUi->MaximumSpinBox->value();
  }

  bool WebDialogRatioMatrixSettings::GetIsIncludeMarkers() const
  {
    return WidgetUi->IncludeMarkersCheckBox->isChecked();
  }

  void WebDialogRatioMatrixSettings::SetupUi()
  {
    WidgetUi = new Ui::WebDialogRatioMatrixSettings();
    auto& ui = WidgetUi;
    auto body = new QWidget(this);
    body->setStyleSheet(Style::Genesis::GetUiStyle());
    ui->setupUi(body);
    getContent()->layout()->addWidget(body);
    ui->WindowSizeLabel->setStyleSheet(Style::Genesis::GetH4());
    ui->MinimumLabel->setStyleSheet(Style::Genesis::GetH4());
    ui->MaximumLabel->setStyleSheet(Style::Genesis::GetH4());
    auto s = ui->matrixTitleEdit->settings();
    s.defaultText = QString("");
    s.textHeader = tr("Matrix title");
    s.textMaxLen = 50;
    s.textPlaceholder = tr("Enter matrix title");
    s.textRequired = true;
    s.textTooltipsSet.empty = tr("Title empty");
    s.textTooltipsSet.forbidden = tr("Title is alredy in use");
    s.textTooltipsSet.notChecked = tr("Not checked");
    s.textTooltipsSet.valid = tr("Title is valid");
    ui->matrixTitleEdit->applySettings(s);
    connect(ui->matrixTitleEdit, &DialogLineEditEx::validityChanged, this, [this](bool isValid)
    {
      ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
    });
  }
}
