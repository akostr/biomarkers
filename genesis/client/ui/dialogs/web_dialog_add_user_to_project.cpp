//#include "web_dialog_add_user_to_project.h"

//#include "../../api/api_rest.h"
//#include "../../logic/context_root.h"
//#include "../../logic/tree_model_presentation.h"
//#include "../../logic/notification.h"

//#include <genesis_style/style.h>
//#include <logic/context_root.h>
//#include <QComboBox>
//#include <QPushButton>
//#include <QWidget>
//#include <QLabel>
//#include <QLayout>
//#include <QAbstractItemView>
//#include <QStyledItemDelegate>
//#include <QDebug>

//using namespace Core;

///////////////////////////////////////////////////////
////// Web Dialog / add user to project
//WebDialogAddUserToProject::WebDialogAddUserToProject(QWidget* parent)
//  : WebDialog(parent, nullptr, QSizeF(0.4, 0), QDialogButtonBox::Apply | QDialogButtonBox::Cancel)
//{
//  SetupModels();
//  SetupUi();
//}

//WebDialogAddUserToProject::~WebDialogAddUserToProject()
//{
//}

//void WebDialogAddUserToProject::SetupModels()
//{
//  Access     = new TreeModelDynamicAccess(this);
//  OtherUsers = new TreeModelDynamicOtherUsers(this);
//  Roles      = new TreeModelDynamicRoles(this);
//}

//void WebDialogAddUserToProject::SetupUi()
//{
//  //// Body
//  Body = new QWidget(Content);
//  Content->layout()->addWidget(Body);
//  Body->setStyleSheet(Style::Genesis::GetUiStyle());

//  BodyLayout = new QVBoxLayout(Body);
//  BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
//  BodyLayout->setSpacing(0);

//  //// Content
//  {
//    //// Header
//    {
//      QLabel* caption = new QLabel(tr("Add users to project"), Body);
//      BodyLayout->addWidget(caption);

//      caption->setAlignment(Qt::AlignCenter);
//      caption->setStyleSheet(Style::Genesis::GetH2());
//    }

//    //// Space
//    BodyLayout->addSpacing(Style::Scale(26));

//    //// Input
//    {
//      //// Users
//      {
//        //// Caption
//        QLabel* caption = new QLabel(tr("User emails list").arg(Style::GetInputAlert()), Body);
//        BodyLayout->addWidget(caption);

//        //// Space
//        BodyLayout->addSpacing(Style::Scale(4));

//        //// Create editor
//        UsersView = new TextEditEmails(Body);
//        BodyLayout->addWidget(UsersView);

//        //// Create completer
//        QCompleter* completer = new QCompleter(UsersView);
//        completer->setModel(OtherUsers->GetSingleColumnProxyModel(TreeModelDynamicOtherUsers::ColumnEmail));
//        completer->setModelSorting(QCompleter::UnsortedModel);
//        completer->setCaseSensitivity(Qt::CaseInsensitive);
//        completer->setWrapAround(false);
//        completer->popup()->setObjectName("completerPopup");
//        completer->popup()->setItemDelegate(new QStyledItemDelegate(completer->popup()));
//        completer->popup()->setStyleSheet(Style::Genesis::GetUiStyle());
//        UsersView->SetCompleter(completer);
//        UsersView->setFocus();

//        //// Set display names for email list editor
//        QPointer<WebDialogAddUserToProject> instance = this;
//        auto updateUsers = [instance]
//        {
//          if (!instance)
//            return;

//          QMap<QString, QString> emailDisplayNames;
//          QMap<QString, int> emailIds;
//          for (int r = 0; r < instance->OtherUsers->rowCount(); r++)
//          {
//            QString key = instance->OtherUsers->data(instance->OtherUsers->index(r, TreeModelDynamicOtherUsers::ColumnEmail)).toString();
//            QString val = QString("%1 (%2)").arg(instance->OtherUsers->data(instance->OtherUsers->index(r, TreeModelDynamicOtherUsers::ColumnFullName)).toString())
//                                            .arg(instance->OtherUsers->data(instance->OtherUsers->index(r, TreeModelDynamicOtherUsers::ColumnEmail)).toString());
//            int id = instance->OtherUsers->data(instance->OtherUsers->index(r, TreeModelDynamicOtherUsers::ColumnId)).toInt();

//            if (!key.isEmpty() && !val.isEmpty())
//            {
//              emailDisplayNames[key] = val;
//            }
//            if (!key.isEmpty() && id)
//            {
//              emailIds[key] = id;
//            }
//          }
//          instance->UsersView->SetDisplayNames(emailDisplayNames);
//          instance->UsersView->SetIds(emailIds);
//        };
//        connect(OtherUsers, &QAbstractItemModel::modelReset, updateUsers);
//        updateUsers();
//      }

//      //// Space
//      BodyLayout->addSpacing(Style::Scale(16));

//      //// Access
//      {
//        //// Caption
//        QLabel* caption = new QLabel(tr("Access").arg(Style::GetInputAlert()), Body);
//        BodyLayout->addWidget(caption);

//        //// Space
//        BodyLayout->addSpacing(Style::Scale(4));

//        //// Input
//        AccessView = new QComboBox(Body);
//        BodyLayout->addWidget(AccessView);

//        //// Fill
//        QPointer<WebDialogAddUserToProject> instance = this;
//        auto fill = [instance]()
//        {
//          if (!instance)
//            return;

//          instance->AccessView->clear();
//          for (auto r = 0; r < instance->Access->rowCount(); r++)
//          {
//            QString access = instance->Access->data(instance->Access->index(r, TreeModelDynamicAccess::ColumnAccess)).toString();
//            int id = instance->Access->data(instance->Access->index(r, TreeModelDynamicAccess::ColumnId)).toInt();

//            instance->AccessView->addItem(access, id);
//          }
//        };
//        connect(Access, &QAbstractListModel::modelReset, fill);
//        fill();
//      }

//      //// Space
//      BodyLayout->addSpacing(Style::Scale(16));

//      //// Roles
//      {
//        //// Caption
//        QLabel* caption = new QLabel(tr("Roles").arg(Style::GetInputAlert()), Body);
//        BodyLayout->addWidget(caption);

//        //// Space
//        BodyLayout->addSpacing(Style::Scale(4));

//        //// Input
//        RolesView = new QComboBox(Body);
//        BodyLayout->addWidget(RolesView);

//        //// Fill
//        QPointer<WebDialogAddUserToProject> instance = this;
//        auto fill = [instance]()
//        {
//          if (!instance)
//            return;

//          instance->RolesView->clear();
//          for (auto r = 0; r < instance->Roles->rowCount(); r++)
//          {
//            QString access = instance->Roles->data(instance->Roles->index(r, TreeModelDynamicRoles::ColumnRole)).toString();
//            int id = instance->Roles->data(instance->Roles->index(r, TreeModelDynamicRoles::ColumnId)).toInt();

//            if (id != TreeModelDynamicRoles::ConstantRoleAuthor)
//            {
//              instance->RolesView->addItem(access, id);
//            }
//          }
//        };

//        connect(Roles, &QAbstractListModel::modelReset, fill);
//        fill();
//      }

//    }
//  }

//  //// Buttons
//  if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
//  {
//    apply->setText(tr("Add users"));
//    apply->setDefault(true);

//    connect(apply, &QPushButton::clicked, ButtonBox, &QDialogButtonBox::accepted);
//  }

//  //// Handle input
//  auto handleInput = [this]()
//  {
//    if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
//    {
//      apply->setEnabled(!UsersView->GetInputIds().empty());
//    }
//  };
//  connect(UsersView, &QTextEdit::textChanged, handleInput);
//  handleInput();
//}

//void WebDialogAddUserToProject::Accept()
//{
//  QList<int> userIds = UsersView->GetInputIds();
//  if (!userIds.empty())
//  {
//    if (auto root = GenesisContextRoot::Get())
//    {
//      if (auto project = root->GetContextProject())
//      {
//        int projectId = project->GetData("id").toInt();
//        int accessId  = AccessView->currentData().toInt();
//        int roleId    = RolesView->currentData().toInt();

//        if (projectId && accessId && roleId)
//        {
//          for (auto userId : userIds)
//          {
//            API::REST::Tables::AddUserToProject(projectId, userId, accessId, roleId,
//              [userId](QNetworkReply*, QVariantMap /*data*/)
//              {
//                TreeModel::ResetInstances("TreeModelDynamicProjectUsers");
//                Notification::NotifySuccess(tr("User added to project"));
//              },
//              [userId](QNetworkReply*, QNetworkReply::NetworkError error)
//              {
//                Notification::NotifyError(tr("Failed to add user to project"), error);
//              });
//          }
//        }
//      }
//    }
//  }

//  WebDialog::Accept();
//}
