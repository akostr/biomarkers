#include "markup_save_table_dialog.h"
#include "logic/notification.h"
#include "ui/controls/dialog_line_edit_ex.h"
#include "ui/controls/dialog_text_edit_ex.h"
#include <genesis_style/style.h>
#include <logic/context_root.h>
#include <logic/known_context_tag_names.h>

#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

using namespace Names;

using Btns = QDialogButtonBox::StandardButton;

MarkupSaveTableDialog::MarkupSaveTableDialog(QWidget *parent, const QStringList& occupiedNames)
  : Dialogs::Templates::Dialog(parent, Btns::Ok | Btns::Cancel),
    mOccupiedNames(occupiedNames)
{
  setupUi();
  Size = QSizeF(0.25, 0.3);
  UpdateGeometry();
}

void MarkupSaveTableDialog::Accept()
{
  if (mMarkupNameEdit->text().isEmpty())
  {
    Notification::NotifyError(tr("Table name must be unique and non-empty"));
    return;
  }
  WebDialog::Accept();
}

QString MarkupSaveTableDialog::name()
{
  return mMarkupNameEdit->text();
}

QString MarkupSaveTableDialog::comment()
{
  return mMarkupCommentEdit->text();
}

bool MarkupSaveTableDialog::generateMatrix()
{
  return mGenerateMatrixCheckBox->isChecked();
}

void MarkupSaveTableDialog::setupUi()
{
  Dialogs::Templates::Dialog::Settings sd;
  sd.dialogHeader = tr("Create a data table");
  sd.buttonsNames[QDialogButtonBox::Ok] = tr("Create");
  applySettings(sd);

  auto l = getContent()->layout();
  auto infoLabel = new QLabel(tr("The completed markup table will be saved in the Numeric Data section in the Data Tables tab."));
  infoLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  infoLabel->setWordWrap(true);
  l->addWidget(infoLabel);

  {
    auto s = DialogLineEditEx::Settings();
    s.forbiddenStrings = QSet(mOccupiedNames.begin(), mOccupiedNames.end());
    s.defaultText = Core::GenesisContextRoot::Get()->GetContextMarkup()->GetData()[Names::MarkupContextTags::kMarkupTitle].toString();
    s.textHeader = tr("Table name");
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

    mMarkupNameEdit = new DialogLineEditEx(this, s);
    l->addWidget(mMarkupNameEdit);
  }

  {
    auto s = DialogTextEditEx::Settings();
    s.textHeader = tr("Table comment");
    s.textMaxLen = 70;
    s.textPlaceholder = tr("Enter table comment");
    s.textRequired = false;

    mMarkupCommentEdit = new DialogTextEditEx(this);
    mMarkupCommentEdit->applySettings(s);
    l->addWidget(mMarkupCommentEdit);
  }

  mGenerateMatrixCheckBox = new QCheckBox(tr("Calculate the height ratio matrix"));
  mGenerateMatrixCheckBox->setTristate(false);
  if(Core::GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).toInt() == ModulesContextTags::MReservoir)
  {
    l->addWidget(mGenerateMatrixCheckBox);
  }


  connect(mMarkupNameEdit, &DialogLineEditEx::validityChanged, this, [&](bool isValid)
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
  });
}

MarkupSaveTableDialog::~MarkupSaveTableDialog()
{
}
