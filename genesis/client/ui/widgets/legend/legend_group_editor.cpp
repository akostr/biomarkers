#include "legend_group_editor.h"
#include "ui_legend_group_editor.h"

#include <ui/item_models/color_item_model.h>
#include <ui/item_models/shape_item_model.h>
#include <genesis_style/style.h>
#include "ui/dialogs/templates/confirm.h"
#include <logic/notification.h>

#include <QPushButton>

using BB = QDialogButtonBox::StandardButton;
using namespace AnalysisEntity;
namespace Dialogs
{
LegendGroupEditor::LegendGroupEditor(const TEntityUid &entityUid, QPointer<AnalysisEntityModel> model, QWidget *parent)
  : Dialog(parent, BB::Ok | BB::Cancel),
  mEntityUid(entityUid),
  mModel(model),
  ui(new Ui::LegendGroupEditor),
  mContent(new QWidget(nullptr))
{
  setupUi();
}

LegendGroupEditor::~LegendGroupEditor()
{
  delete ui;
}

void LegendGroupEditor::Accept()
{
  if(!mModel)
  {
    WebDialog::Reject();
    return;
  }
  auto setIfChanged = [this](const QVariant& value, int role)
  {
    if(mModel->getEntity(mEntityUid)->getData(role) != value)
      mModel->setEntityData(mEntityUid, role, value);
  };
  setIfChanged(ui->nameLineEditEx->text(), RoleTitle);
  setIfChanged(ui->colorCombo->currentData(), RoleColor);
  setIfChanged(ui->shapeCombo->currentData(), RoleShape);
  setIfChanged(ui->sizeSpin->value(), RoleShapePixelSize);
  Notification::NotifySuccess(tr("Changes in samples group \"%1\" successfully saved").arg(ui->nameLineEditEx->text()), tr("Changes successfully saved"));
  WebDialog::Accept();
}

void LegendGroupEditor::Reject()
{
  WebDialog::Reject();
}

void LegendGroupEditor::setupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  if(!mModel)
    s.dialogHeader = tr("No model");
  else
    s.dialogHeader = tr("Samples group editing");

  s.buttonsNames = {{QDialogButtonBox::Ok, tr("Save")},
                    {QDialogButtonBox::Cancel, tr("Cancel")}};
  s.buttonsProperties = {{QDialogButtonBox::Ok, {{"blue", true}}}};
  applySettings(s);
  if(!mModel)
    return;
  auto ent = mModel->getEntity(mEntityUid);
  auto groupName = ent->getData(RoleTitle).toString();
  ui->setupUi(mContent);
  auto leS = ui->nameLineEditEx->settings();
  leS.defaultText = groupName;
  leS.textHeader = tr("Group name");
  leS.textMaxLen = 150;
  leS.textPlaceholder = tr("Enter group name");
  leS.textRequired = true;
  leS.textTooltipsSet.empty = tr("Group name should be non-empty");
  leS.textTooltipsSet.forbidden = tr("Group with same name alredy exists");
  leS.textTooltipsSet.notChecked = tr("Not checked");
  leS.textTooltipsSet.valid = tr("Group name is valid");

  auto groupType = ent->getType();
  for(auto& gent : mModel->getEntities(groupType))
    leS.forbiddenStrings << gent->getData(RoleTitle).toString();

  leS.forbiddenStrings.remove(groupName);
  ui->nameLineEditEx->applySettings(leS);
  getContent()->layout()->addWidget(mContent);
  auto removeBtn = new QPushButton(tr("Remove group"), this);
  removeBtn->setStyleSheet(Style::Genesis::GetUiStyle());
  removeBtn->setProperty("red", true);
  if(!ent->getData(RoleGroupUserCreated).toBool())
    removeBtn->setVisible(false);
  ButtonLayout->insertWidget(0, removeBtn);
  connect(removeBtn, &QPushButton::clicked, this, &LegendGroupEditor::showConfirmDialog);
  auto colorModel = new ColorItemModel(ui->colorCombo);
  ui->colorCombo->setModel(colorModel);
  {
    auto currInd = colorModel->getIndex(ent->getData(RoleColor).value<TColor>());
    if(currInd == GPShapeItem::CustomColor)//@TODO: error case, no such color in pool
      currInd = 0;//just set first for stub
    ui->colorCombo->setCurrentIndex(currInd);
  }
  auto shapeModel = new ShapeItemModel(ui->shapeCombo);
  ui->shapeCombo->setModel(shapeModel);
  {
    auto currInd = shapeModel->getIndex(ent->getData(RoleShape).value<TShape>());
    if(currInd == GPShape::PathShape)//@TODO: error case, no such shape in pool
      currInd = 0;//just set first for stub
    ui->shapeCombo->setCurrentIndex(currInd);
  }
  ui->sizeSpin->setValue(ent->getData(RoleShapePixelSize).toInt());
  ui->subCaptionLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
}

void LegendGroupEditor::removeGroup()
{
  emit removeGroupAndRegroup(mEntityUid);
  WebDialog::Reject();
}

void LegendGroupEditor::showConfirmDialog()
{
  auto settings = Dialogs::Templates::Confirm::Settings();
  settings.dialogHeader = tr("Removing");
  settings.buttonsProperties = { { QDialogButtonBox::Ok, {{"red", true}} } };
  settings.buttonsNames = { { QDialogButtonBox::Ok , tr("Remove") } };
  auto label = new QLabel(tr("Are you sure you want to remove the sample group %1 from the legend? It will be impossible to cancel this action.").arg(ui->nameLineEditEx->text()));
  auto confirm = new Dialogs::Templates::Confirm(this, settings, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, label);
  connect(confirm, &Dialogs::Templates::Confirm::Accepted, this, &LegendGroupEditor::removeGroup);
  confirm->Open();
}

}//namespace Dialogs
