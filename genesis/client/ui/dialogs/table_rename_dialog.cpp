#include "table_rename_dialog.h"
#include <genesis_style/style.h>
#include "logic/notification.h"

#include "ui/controls/dialog_line_edit_ex.h"
#include <QLayout>
#include <QPushButton>

TableRenameDialog::TableRenameDialog(QWidget *parent, QString title)
: Dialogs::Templates::Dialog(parent, Btns::Ok | Btns::Cancel)
, defaultTitle(title)
{
  setupUi();
}

TableRenameDialog::~TableRenameDialog()
{

}

void TableRenameDialog::Accept()
{
  if(mEdit->text().isEmpty())
  {
    Notification::NotifyError(tr("Markup name can't be empty"), tr("Save as dialog"));
    return;
  }
  WebDialog::Accept();
}

QString TableRenameDialog::getTitle()
{
  return mEdit->text();
}

void TableRenameDialog::SetLineEditCaption(const QString& caption)
{
  mEdit->SetHeaderLabel(caption);
}

void TableRenameDialog::setupUi()
{
  Dialogs::Templates::Dialog::Settings sd;
  sd.dialogHeader = tr("Edit");
  applySettings(sd);

  ButtonBox->button(QDialogButtonBox::Ok)->setText(tr("Rename"));

  auto s = DialogLineEditEx::Settings();
  s.defaultText = defaultTitle;
  s.textHeader = tr("Table rename");
  s.textMaxLen = 50;
  s.textPlaceholder = tr("Enter table name");
  s.textRequired = true;
  s.controlSymbols = true;
  s.textTooltipsSet = {
    tr("Name is empty"),
    tr("Name alredy in use"),
    tr("Not checked"),
    tr("Name is valid"),
    tr("Name must not contain the characters '\', '/', '\"', '*', '<', '|', '>'")
  };

  mEdit = new DialogLineEditEx(this, s);
  getContent()->layout()->addWidget(mEdit);

  connect(mEdit, &DialogLineEditEx::validityChanged, this, [&](bool isValid)
  {
      ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
  });
}
