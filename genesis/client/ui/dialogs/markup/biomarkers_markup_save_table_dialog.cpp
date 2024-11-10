#include "biomarkers_markup_save_table_dialog.h"
#include "ui_biomarkers_markup_save_table_dialog.h"
#include <logic/tree_model_dynamic_numeric_tables.h>
#include <logic/tree_model_item.h>
#include <logic/known_context_tag_names.h>
#include <logic/known_json_tag_names.h>
#include <logic/context_root.h>
#include <genesis_style/style.h>
#include <ui/dialogs/web_overlay.h>

#include <QStandardItemModel>
#include <QPushButton>
#include <QLineEdit>

enum RadioButton
{
  IntoCommonGroup,
  IntoSpecificGroup
};

BiomarkersMarkupSaveTableDialog::BiomarkersMarkupSaveTableDialog(QWidget *parent, const QStringList &occupiedNames)
  : Dialogs::Templates::Info(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel),
  ui(new Ui::BiomarkersMarkupSaveTableDialog),
  mContent(new QWidget(nullptr)),
  mTablesModel(new TreeModelDynamicNumericTables(Names::Group::DataTableIdentified, this)),
  mOccupiedNames(occupiedNames)
{
  setupUi();
  mLoadingOverlay = new WebOverlay(tr("Loading tables groups list"), this);
  connect(mTablesModel, &TreeModelDynamicNumericTables::modelReset,
          this, &BiomarkersMarkupSaveTableDialog::onTablesModelReset);
}

BiomarkersMarkupSaveTableDialog::~BiomarkersMarkupSaveTableDialog()
{
  delete ui;
}

bool BiomarkersMarkupSaveTableDialog::isExistedTableGroup()
{
  if(ui->specificGroupCombo->currentData(Qt::UserRole).isValid()
      && (ui->specificGroupCombo->currentText()
          == ui->specificGroupCombo->currentData(Qt::DisplayRole).toString()))
    return true;
  return false;
}

bool BiomarkersMarkupSaveTableDialog::isCommonGroup()
{
  return ui->commonGroupRadioBtn->isChecked();
}

int BiomarkersMarkupSaveTableDialog::tableGroupId()
{
  return ui->specificGroupCombo->currentData(Qt::UserRole).toInt();
}

QString BiomarkersMarkupSaveTableDialog::tableGroupTitle()
{
  return ui->specificGroupCombo->currentText();
}

QString BiomarkersMarkupSaveTableDialog::tableTitle()
{
  return ui->titleEdit->text();
}

QString BiomarkersMarkupSaveTableDialog::tableComment()
{
  return ui->commentEdit->text();
}

void BiomarkersMarkupSaveTableDialog::Accept()
{
  if(ui->stackedWidget->currentIndex() == 0)
  {
    ui->stackedWidget->setCurrentIndex(1);
  }
  else
  {
    WebDialog::Accept();
  }
}

void BiomarkersMarkupSaveTableDialog::setupUi()
{
  ui->setupUi(mContent);

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
    ui->titleEdit->applySettings(s);
  }
  {
    auto s = DialogTextEditEx::Settings();
    s.textHeader = tr("Table comment");
    s.textMaxLen = 70;
    s.textPlaceholder = tr("Enter table comment");
    s.textRequired = false;
    ui->commentEdit->applySettings(s);
  }

  mComboModel = qobject_cast<QStandardItemModel*>(ui->specificGroupCombo->model());
  ui->specificGroupCombo->lineEdit()->setPlaceholderText(tr("Type new group name or pick from list"));
  Q_ASSERT(mComboModel);
  mStepBackBtn = new QPushButton(QIcon("://resource/icons/icon_button_lt.png"),
                                     tr("Previous step"), this);
  mStepBackBtn->setProperty("secondary", true);
  mStepBackBtn->setStyleSheet(Style::Genesis::GetUiStyle());
  connect(mStepBackBtn, &QPushButton::clicked, this,
          [this]()
          {
            ui->stackedWidget->setCurrentIndex(0);
          });
  ButtonLayout->insertWidget(0, mStepBackBtn);

  connect(ui->stackedWidget, &QStackedWidget::currentChanged,
          this, &BiomarkersMarkupSaveTableDialog::onCurrentPageIndexChanged);

  if(ui->stackedWidget->currentIndex() != 0)
    ui->stackedWidget->setCurrentIndex(0);
  else
    onCurrentPageIndexChanged(ui->stackedWidget->currentIndex());

  auto bgroup = ui->tableGroupButtonsGroup;
  bgroup->setId(ui->commonGroupRadioBtn, IntoCommonGroup);
  bgroup->setId(ui->specificGroupRadioBtn, IntoSpecificGroup);
  connect(ui->tableGroupButtonsGroup, &QButtonGroup::idToggled,
          this, &BiomarkersMarkupSaveTableDialog::onRadioGroupIdToggled);
  onRadioGroupIdToggled(bgroup->checkedId(), true);

  connect(ui->titleEdit, &DialogLineEditEx::validityChanged, this,
          [this](bool isValid)
          {
            ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
          });
  connect(ui->specificGroupCombo, &QComboBox::currentTextChanged, this,
          [this](const QString& text)
          {
            ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
          });

  getContent()->layout()->addWidget(mContent);
}

void BiomarkersMarkupSaveTableDialog::onTablesModelReset()
{
  if(!mComboModel)
    return;
  mComboModel->clear();
  auto tablesRoot = mTablesModel->GetRoot();
  for(auto& child : tablesRoot->GetChildren())
  {
    qDebug() << child->GetData();
    if(child->GetData("group").toBool())
    {
      auto id = child->GetData("_id").toInt();
      auto item = new QStandardItem(child->GetData(JsonTagNames::_title).toString());
      item->setData(id, Qt::UserRole);
      mComboModel->appendRow(item);
    }
  }
  // if(mComboModel->rowCount() > 0)
  ui->specificGroupCombo->setCurrentIndex(-1);
  mLoadingOverlay->deleteLater();
  //mLoadingOverlay = nullptr;//not neccesary because it's QPointer
}

void BiomarkersMarkupSaveTableDialog::onCurrentPageIndexChanged(int current)
{
  if(current == 0)
  {
    Dialogs::Templates::Info::Settings s;
    s.dialogHeader = tr("Markup completion");
    s.contentHeader = tr("Step 1 of 2: assigning a name and comment");
    s.buttonsNames = {{QDialogButtonBox::Ok, tr("Next step")},
                      {QDialogButtonBox::Cancel, tr("Cancel")}};
    s.buttonsProperties = {{QDialogButtonBox::Ok, {{"Blue", true}}},
                           /*{QDialogButtonBox::Cancel, {{"blue", true}}}*/};
    applySettings(s);
    mStepBackBtn->hide();
  }
  else if(current == 1)
  {
    Dialogs::Templates::Info::Settings s;
    s.dialogHeader = tr("Markup completion");
    s.contentHeader = tr("Step 2 of 2: Selecting a Save Location");
    s.buttonsNames = {{QDialogButtonBox::Ok, tr("Complete markup")},
                      {QDialogButtonBox::Cancel, tr("Cancel")}};
    s.buttonsProperties = {{QDialogButtonBox::Ok, {{"Blue", true}}},
                           /*{QDialogButtonBox::Cancel, {{"blue", true}}}*/};
    applySettings(s);
    mStepBackBtn->show();
  }
}

void BiomarkersMarkupSaveTableDialog::onRadioGroupIdToggled(int id, bool state)
{
  if(!state)
    return;
  switch(id)
  {
  case IntoCommonGroup:
    ui->specificGroupCombo->hide();
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    break;
  case IntoSpecificGroup:
    ui->specificGroupCombo->show();
    if(ui->specificGroupCombo->currentText().isEmpty())
      ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    break;
  default:
    break;
  }
}
