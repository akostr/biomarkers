#include "web_dialog_xls_file_import_type.h"
#include "web_dialog_xls_file_import_type.h"
#include "ui_web_dialog_xls_file_import_type.h"

#include <genesis_style/style.h>

using Btns = QDialogButtonBox::StandardButton;

namespace Dialogs
{
WebDialogXlsFileImportType::WebDialogXlsFileImportType(QWidget* parent) :
  Templates::Dialog(parent, Btns::Ok | Btns::Cancel),
  ui(new Ui::WebDialogXlsFileImportType)
{
  auto s = Templates::Dialog::Settings();
  s.dialogHeader = tr("Table import");
  s.buttonsNames = { {Btns::Ok, tr("Continue import")} };
  s.buttonsProperties = { {Btns::Ok, {{"blue", true}}} };
  setupUi();
  applySettings(s);
}

WebDialogXlsFileImportType::~WebDialogXlsFileImportType()
{
  delete ui;
}

Import::XlsFileImportedType WebDialogXlsFileImportType::importType()
{
  if (ui->simpleRadio->isChecked())
    return Import::XlsFileImportedType::Simple;
  else
    return Import::XlsFileImportedType::Custom;
}

void WebDialogXlsFileImportType::setupUi()
{
  auto body = new QWidget(this);
  ui->setupUi(body);
  getContent()->layout()->addWidget(body);
  QString groupBoxH3Style =
    "QGroupBox::title{"
    "font:  @h3;"
    "color: @textColor;"
    "}"
    "QGroupBox{"
    "border: none;"
    "}";
  ui->subCaptionLabel->setStyleSheet(Style::Genesis::GetH4());
}
}