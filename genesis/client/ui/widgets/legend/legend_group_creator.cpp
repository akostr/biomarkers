#include "legend_group_creator.h"
#include "ui_legend_group_creator.h"
#include <ui/item_models/color_item_model.h>
#include <ui/item_models/shape_item_model.h>
#include <genesis_style/style.h>
#include <ui/flow_layout.h>
#include <logic/notification.h>

using BB = QDialogButtonBox::StandardButton;
using namespace AnalysisEntity;

// namespace
// {
// class FinishAcceptedLineEdit : public QLineEdit
// {
// public:
//   FinishAcceptedLineEdit(QWidget *parent = nullptr) : QLineEdit(parent)
//     {
//     connect(this, &QLineEdit::editingFinished, this, &QLineEdit::returnPressed);
//     auto acceptAction = new QAction();
//     acceptAction->setIcon(QIcon("://resource/icons/icon_action_add.png"));
//     acceptAction->setToolTip(tr("Name should be non-empty"));
//     addAction(acceptAction, QLineEdit::TrailingPosition);
//     acceptAction->setEnabled(false);
//     connect(this, &QLineEdit::returnPressed, this, [this, acceptAction]()
//             {
//               if(!text().isEmpty())
//               {
//                 acceptAction->setIcon(QIcon("://resource/icons/icon_success_on.png"));
//                 acceptAction->setToolTip(tr("Group picked"));
//               }
//             });
//     connect(acceptAction, &QAction::triggered, this, [this]()
//             {
//               emit editingFinished();
//             });
//     connect(this, &QLineEdit::textEdited, this, [acceptAction](const QString& str)
//             {
//               acceptAction->setEnabled(!str.isEmpty());
//               acceptAction->setToolTip(str.isEmpty() ? tr("Name should be non-empty") : tr("Can be picked"));
//               acceptAction->setIcon(QIcon("://resource/icons/icon_action_add.png"));
//             });
//     }
// };
// }

namespace Dialogs
{
LegendGroupCreator::LegendGroupCreator(const QList<TEntityUid> &entityUids, QPointer<AnalysisEntityModel> model, bool manualHotteling, QWidget *parent)
  : Dialog(parent, BB::Ok | BB::Cancel),
  mEntityUids(entityUids),
  mModel(model),
  ui(new Ui::LegendGroupCreator),
  mContent(new QWidget(nullptr)),
  mInManualHotteling(manualHotteling)
{
  setupUi();
}

LegendGroupCreator::~LegendGroupCreator()
{
  delete ui;
}

void LegendGroupCreator::Accept()
{
  if(!mModel)
  {
    Notification::NotifyError(tr("No Model"));
    return;
  }

  if(ui->groupPickerCombo->currentIndex() == -1 && ui->groupPickerCombo->lineEdit()->text().isEmpty())
  {
    Notification::NotifyError(tr("Pick or create group with non-empty title"));
    return;
  }

  QUuid groupUid;
  if(ui->groupPickerCombo->currentIndex() != -1
      && ui->groupPickerCombo->currentData(Qt::DisplayRole).toString() == ui->groupPickerCombo->currentText())
  {
    //existed or newly inserted, groupUid could be null, and could be not
    groupUid = ui->groupPickerCombo->currentData().toUuid();
  }//in other cases groupUid will be null

  auto setIfChanged = [this](int role, const TEntityUid& uid, const QVariant& value)
  {
    if(uid.isNull() || !mModel->getEntity(uid))
      return;
    if(value != mModel->getEntity(uid)->getData(role))
      mModel->setEntityData(uid, role, value);
  };

  auto groupType = mInManualHotteling ? TypeHottelingGroup : TypeGroup;
  if(groupUid.isNull())
  {//newly created
    groupUid = mModel->addNewEntity(groupType);
    mModel->setEntityData(groupUid, RoleColor, ui->colorCombo->currentData());
    mModel->setEntityData(groupUid, RoleShape, ui->shapeCombo->currentData());
    mModel->setEntityData(groupUid, RoleShapePixelSize, ui->sizeSpin->value());
    mModel->setEntityData(groupUid, RoleTitle, ui->groupPickerCombo->currentText());//text from line edit
    mModel->setEntityData(groupUid, RoleGroupUserCreated, true);
  }
  else
  {//existed
    setIfChanged(RoleColor, groupUid, ui->colorCombo->currentData());
    setIfChanged(RoleShape, groupUid, ui->shapeCombo->currentData());
    setIfChanged(RoleShapePixelSize, groupUid, ui->sizeSpin->value());
  }
  for(auto& uid : mEntityUids)
  {
    mModel->resetEntityGroup(uid, groupType);
    mModel->setEntityGroup(uid, groupUid);
  }
  Notification::NotifySuccess(tr("The selected samples have been successfully added to the group \"%1\"").arg(ui->groupPickerCombo->currentText()),
                              tr("Samples have been added"));
  WebDialog::Accept();
}

void LegendGroupCreator::Reject()
{
  WebDialog::Reject();
}

void LegendGroupCreator::setupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  if(!mModel)
    s.dialogHeader = tr("No model");
  else
    s.dialogHeader = tr("Including samples into group");

  s.buttonsNames = {{QDialogButtonBox::Ok, tr("Save")},
                    {QDialogButtonBox::Cancel, tr("Cancel")}};
  s.buttonsProperties = {{QDialogButtonBox::Ok, {{"blue", true}}}};
  applySettings(s);
  if(!mModel)
    return;

  ui->setupUi(mContent);

  auto flowLayout = new FlowLayout(ui->pickedSamplesAggregationWidget);
  ui->pickedSamplesAggregationWidget->setLayout(flowLayout);
  for(auto& uid : mEntityUids)
  {
    auto ent = mModel->getEntity(uid);
    if(!ent)
      continue;
    auto passport = ent->getData(RolePassport).value<TPassport>();
    auto title = ent->getData(RoleTitle).toString();
    // auto title = passport.filetitle;
    // if(title.isEmpty())
    // {
    //   title = passport.filename.section('/', -1);
    //   if(title.contains('\\'))
    //     title = title.section('\\', -1);
    // }
    // else
    // {
    //   title = passport.filetitle;
    // }
    auto plate = new QWidget(ui->pickedSamplesAggregationWidget);
    plate->setProperty("uid", uid);
    auto layout = new QHBoxLayout(plate);
    plate->setLayout(layout);
    auto label = new QLabel(title, plate);
    layout->addWidget(label);
    auto removeBtn = new QPushButton(plate);
    removeBtn->setIcon(QIcon("://resource/icons/icon_action_cross.png"));
    removeBtn->setMinimumSize(removeBtn->iconSize());
    removeBtn->setMaximumSize(removeBtn->iconSize());
    removeBtn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    removeBtn->setFlat(true);
    layout->addWidget(removeBtn);
    connect(removeBtn, &QPushButton::clicked, this,
            [this, plate]()
            {
              if(mEntityUids.size() == 1)
                return;
              mEntityUids.removeAll(plate->property("uid").toUuid());
              ui->pickedSamplesSubCaption->setText(tr("Picked sample(s) (%n)", "", mEntityUids.size()));
              plate->deleteLater();
            });
    flowLayout->addWidget(plate);
  }
  ui->pickedSamplesSubCaption->setText(tr("Picked sample(s) (%n)", "", mEntityUids.size()));

  ui->displayingParametersSubCaption->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  ui->groupSubCaption->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  ui->pickedSamplesSubCaption->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  getContent()->layout()->addWidget(mContent);
  TEntityType groupType = mInManualHotteling ? TypeHottelingGroup : TypeGroup;
  auto groupsList = mModel->getEntities(groupType);
  // ui->groupPickerCombo->setLineEdit(new FinishAcceptedLineEdit(ui->groupPickerCombo));
  for(int i = 0; i < groupsList.size(); i++)
  {
    auto groupEnt = groupsList[i];
    if(!groupEnt->getData(RoleGroupUserCreated).toBool())
      continue;
    auto groupName = groupEnt->getData(RoleTitle).toString();
    ui->groupPickerCombo->addItem(groupName, groupEnt->getUid());
  }
  ui->groupPickerCombo->setCurrentIndex(-1);
  ui->groupPickerCombo->setPlaceholderText(tr("Enter new group name or pick group from list"));
  ui->groupPickerCombo->setEditable(true);
  ui->groupPickerCombo->setInsertPolicy(QComboBox::InsertAtTop);
  mColorModel = new ColorItemModel(ui->colorCombo);
  mShapeModel = new ShapeItemModel(ui->shapeCombo);
  connect(ui->groupPickerCombo, &QComboBox::currentIndexChanged, this, &LegendGroupCreator::onGroupComboIndexChanged);
  connect(ui->groupPickerCombo->lineEdit(), &QLineEdit::textChanged, this, [this](const QString& text)
          {
            if(text.isEmpty())
              ButtonBox->button(BB::Ok)->setEnabled(false);
            else
              ButtonBox->button(BB::Ok)->setEnabled(true);
          });

  ui->colorCombo->setModel(mColorModel);
  ui->shapeCombo->setModel(mShapeModel);
  setCurrentItem(QUuid());
  ButtonBox->button(BB::Ok)->setEnabled(false);
}
void LegendGroupCreator::onGroupComboIndexChanged(int ind)
{
  if(ind == -1)
  {
    setCurrentItem(QUuid());
    ButtonBox->button(BB::Ok)->setEnabled(false);
    return;
  }
  auto uid = ui->groupPickerCombo->currentData().toUuid();
  setCurrentItem(uid);
  ButtonBox->button(BB::Ok)->setEnabled(true);
}

void LegendGroupCreator::setCurrentItem(const TEntityUid &uid)
{
  int currShapeInd = 0;
  int currColorInd = 0;
  int currSize = 8;
  if(!uid.isNull())
  {
    auto ent = mModel->getEntity(uid);
    if(ent)
    {
      currColorInd = mColorModel->getIndex(ent->getData(RoleColor).value<TColor>());
      currShapeInd = mShapeModel->getIndex(ent->getData(RoleShape).value<TShape>());
      currSize = ent->getData(RoleShapePixelSize).toInt();
      if(currColorInd == GPShapeItem::CustomColor)//@TODO: error case, no such color in pool
        currColorInd = 0;//just set first for stub
      if(currShapeInd == GPShape::PathShape)//@TODO: error case, no such color in pool
        currShapeInd = 0;//just set first for stub
    }
  }
  ui->colorCombo->setCurrentIndex(currColorInd);
  ui->shapeCombo->setCurrentIndex(currShapeInd);
  ui->sizeSpin->setValue(currSize);
}

}//namespace Dialogs
