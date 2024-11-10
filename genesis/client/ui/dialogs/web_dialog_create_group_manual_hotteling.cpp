#include "web_dialog_create_group_manual_hotteling.h"
#include "ui/controls/dialog_line_edit_ex.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QVariant>

WebDialogCreateGroupManualHotteling::WebDialogCreateGroupManualHotteling(QWidget* parent, const QSet<QString>& occupiedNames)
    : Dialogs::Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
{
  mForbiddenStrings = occupiedNames;
  setupUi();

  Dialogs::Templates::Dialog::Settings sd;
  sd.dialogHeader = tr("Create a new group");
  sd.buttonsNames = {{QDialogButtonBox::Ok, tr("Create group")}};
  sd.buttonsProperties = {{QDialogButtonBox::Ok, {{"blue", true}}}};
  applySettings(sd);
}

void WebDialogCreateGroupManualHotteling::setGroupAddingFunc(std::function<void (WebDialogCreateGroupManualHotteling* dialog)> func)
{
    mGroupAddingFunc = func;
}

void WebDialogCreateGroupManualHotteling::Accept()
{
    mGroupAddingFunc(this);
//    Done(QDialog::Accepted);

}

const QString WebDialogCreateGroupManualHotteling::getNameGroup() const
{
  return mGroupNameEdit->text();
}

void WebDialogCreateGroupManualHotteling::setupUi()
{
//  ButtonBox->button(QDialogButtonBox::Ok)->setText(tr("Create group"));

  auto s = DialogLineEditEx::Settings();
  s.defaultText = tr("New group");
  s.textHeader = tr("Group name");
  s.textMaxLen = 20;
  s.textPlaceholder = tr("Enter group name");
  s.textRequired = true;
  s.controlSymbols = true;
  s.forbiddenStrings = mForbiddenStrings;
  s.textTooltipsSet = {
    tr("Name is empty"),
    tr("Name alredy in use"),
    tr("Not checked"),
    tr("Name is valid"),
    tr("Name must not contain the characters '\', '/', '\"', '*', '<', '|', '>'")
  };

  mGroupNameEdit = new DialogLineEditEx(this);
  mGroupNameEdit->applySettings(s);
  getContent()->layout()->addWidget(mGroupNameEdit);

  connect(mGroupNameEdit, &DialogLineEditEx::validityChanged, this, [&](bool isValid)
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
  });
}
