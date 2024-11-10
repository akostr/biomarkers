#include "markup_save_as_dialog.h"
#include <genesis_style/style.h>
#include "logic/notification.h"

#include "ui/controls/dialog_line_edit_ex.h"
#include <QLayout>
#include <QPushButton>

MarkupSaveAsDialog::MarkupSaveAsDialog(QWidget* parent)
  : Dialogs::Templates::Dialog(parent, Btns::Ok | Btns::Cancel)
{
  setupUi();
}

MarkupSaveAsDialog::~MarkupSaveAsDialog()
{
}

void MarkupSaveAsDialog::Accept()
{
  if(mMarkupNameEdit->text().isEmpty())
  {
    Notification::NotifyError(tr("Markup name can't be empty"), tr("Save as dialog"));
    return;
  }
  WebDialog::Accept();
}

QString MarkupSaveAsDialog::getTitle()
{
  return mMarkupNameEdit->text();
}

void MarkupSaveAsDialog::setupUi()
{
  Dialogs::Templates::Dialog::Settings sd;
  sd.dialogHeader = tr("Create a copy of the markup");
  applySettings(sd);

  ButtonBox->button(QDialogButtonBox::Ok)->setText(tr("Create"));

  auto s = DialogLineEditEx::Settings();
  s.defaultText = tr("New markup");
  s.textHeader = tr("Markup name");
  s.textMaxLen = 50;
  s.textPlaceholder = tr("Enter markup name");
  s.textRequired = true;
  s.controlSymbols = true;
  s.textTooltipsSet = {
    tr("Name is empty"),
    tr("Name alredy in use"),
    tr("Not checked"),
    tr("Name is valid"),
    tr("Name must not contain the characters '\', '/', '\"', '*', '<', '|', '>'")
  };

  mMarkupNameEdit = new DialogLineEditEx(this, s);
  getContent()->layout()->addWidget(mMarkupNameEdit);

  connect(mMarkupNameEdit, &DialogLineEditEx::validityChanged, this, [&](bool isValid)
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
  });
}
