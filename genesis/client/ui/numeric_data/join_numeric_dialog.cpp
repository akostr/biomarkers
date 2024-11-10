#include "join_numeric_dialog.h"
#include <genesis_style/style.h>
#include <QComboBox>
#include <QLabel>
#include <ui/flow_layout.h>
#include <QPushButton>
#include <QLineEdit>
#include "../../api/api_rest.h"
#include <logic/context_root.h>
#include "../../logic/notification.h"
#include <ui/controls/dialog_line_edit_ex.h>
#include <ui/controls/dialog_text_edit_ex.h>
#include <logic/known_context_tag_names.h>

#include <QTimer>

#include <QMetaEnum>
using namespace Core;

using Btns = QDialogButtonBox::StandardButton;

namespace Dialogs
{
JoinNumericDialog::JoinNumericDialog(const QList<QPair<int, QString> > &joinIdsAndName, QWidget *parent)
  : Templates::Dialog(parent, Btns::Ok | Btns::Cancel)
{
  auto s = Templates::Dialog::Settings();
  s.dialogHeader = tr("Joining tables");
  s.buttonsNames = { {Btns::Ok, tr("Join")} };
  applySettings(s);

  for (auto &[id, name]: joinIdsAndName){
    mData[id] = name;
    mSelectedIds.insert(id);
  }

  auto content = getContent();
  mBody = new QWidget(content);
  content->layout()->addWidget(mBody);

  mBodyLayout = new QVBoxLayout(mBody);
  mBodyLayout->setContentsMargins(0,0,0,0);

  auto ls = DialogLineEditEx::Settings();
  ls.textPlaceholder = tr("Input name of the joined table");
  ls.textHeader = tr("Name of the table");
  ls.textMaxLen = 50;
  ls.textRequired = true;
  ls.textTooltipsSet.empty = tr("Table name is empty");
  ls.textTooltipsSet.forbidden = tr("This name alredy occupied");
  ls.textTooltipsSet.notChecked = tr("Not checked");
  ls.textTooltipsSet.valid = tr("Table name is valid");
  mLineEditJoinTableName = new DialogLineEditEx(this, ls);
  mBodyLayout->addWidget(mLineEditJoinTableName);

  auto ts = DialogTextEditEx::Settings();
  ts.textPlaceholder = tr("Input comment of the joined table");
  ts.textHeader = tr("Comment of the table");
  ts.textMaxLen = 70;
  mTextEditJoinTableComment = new DialogTextEditEx(this);
  mTextEditJoinTableComment->applySettings(ts);
  mBodyLayout->addWidget(mTextEditJoinTableComment);

  mInteractiveLayout = new QVBoxLayout();
  mInteractiveLayout->setSpacing(4);
  mInteractiveLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Fixed, QSizePolicy::Expanding));

  connect(mLineEditJoinTableName, &DialogLineEditEx::validityChanged, this, [this](bool isValid)
  {
    if(!isValid || mSelectedIds.size() < 2)
      ButtonBox->button(Btns::Ok)->setEnabled(false);
    else
      ButtonBox->button(Btns::Ok)->setEnabled(true);
  });

  mBodyLayout->addStretch();
  mBodyLayout->addLayout(mInteractiveLayout);

  updateView();
}

void JoinNumericDialog::updateView()
{
  for (auto &plate: mplates){
    plate->deleteLater();
  }
  if(mSelectedIds.size() < 2 || !mLineEditJoinTableName->checkValidity())
    ButtonBox->button(Btns::Ok)->setEnabled(false);
  else
    ButtonBox->button(Btns::Ok)->setEnabled(true);

  for(auto& id : mSelectedIds)
  {
    auto plate = new QWidget(mBody);
    plate->setContentsMargins(0,0,0,0);
    mplates << plate;
    plate->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    plate->setProperty("id", id);
    auto l = new QHBoxLayout();
    l->setContentsMargins(0,0,0,0);
    plate->setLayout(l);
    auto label = new QLabel(mData.value(id) + QString(" (%1)").arg(id));
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    l->addWidget(label);
    auto removeBtn = new QPushButton(QIcon(":/resource/icons/icon_action_cross.png"),"");
    removeBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QSize btnSize = removeBtn->iconSize();
    btnSize.setWidth(btnSize.width() + 4);
    btnSize.setHeight(btnSize.height() + 4);
    removeBtn->setMinimumSize(btnSize);
    removeBtn->setMaximumSize(btnSize);
    l->addWidget(removeBtn);
    l->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Fixed));

    connect(removeBtn, &QPushButton::clicked, [this, plate](){
      mSelectedIds.remove(plate->property("id").toInt());
      updateView();

    });
    mInteractiveLayout->insertWidget(0, plate);
  }
}

QList<int> JoinNumericDialog::selectedIds() const
{
  return mSelectedIds.values();
}

void JoinNumericDialog::Accept()
{
  auto joinedTableName = mLineEditJoinTableName->text();
  auto joinedTableComment = mTextEditJoinTableComment->text();
  auto projectId = GenesisContextRoot::Get()->ProjectId();
  API::REST::Tables::JoinDataTables(projectId,
    mSelectedIds.values(),
    joinedTableName,
    joinedTableComment,
    [](QNetworkReply*, QJsonDocument doc){
        auto obj = doc.object();
        if(obj["error"].toBool())
        {
          Notification::NotifyError(obj["msg"].toString(), tr("Tables join error"));
        }
        else
        {
          Notification::NotifySuccess(tr("Successfully joined tables"));
        }
    },
    [](QNetworkReply*, QNetworkReply::NetworkError error){
      Notification::NotifyError(tr("Error joined tables: ") + QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(error), tr("Join error"));
    });
  WebDialog::Accept();
}
}//namespace Dialogs
