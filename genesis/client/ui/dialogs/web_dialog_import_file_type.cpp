#include "web_dialog_import_file_type.h"
#include "ui_web_dialog_import_file_type.h"

#include <genesis_style/style.h>
#include <QPushButton>
namespace Dialogs
{

WebDialogImportFileType::WebDialogImportFileType(QWidget *parent) :
  Templates::Dialog(parent, Btns::Ok | Btns::Cancel),
  ui(new Ui::WebDialogImportFileType)
{
  auto s = Templates::Dialog::Settings();
  s.dialogHeader = tr("Files import");
  s.buttonsNames = {{Btns::Ok, tr("Continue import")}};
  s.buttonsProperties = {{Btns::Ok, {{"blue", true}}}};
  setupUi();
  applySettings(s);
}

WebDialogImportFileType::~WebDialogImportFileType()
{
  delete ui;
}

Import::ImportedFilesType WebDialogImportFileType::fileType()
{
  if(ui->chromatogrammsRadio->isChecked())
    return Import::Chromatogramms;
  else if(ui->fragmentsRadio->isChecked())
    return Import::Fragments;
  else
    return Import::FilesTypeLast;
}

void WebDialogImportFileType::setupUi()
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
}//namespace Dialogs
