//#pragma once

//#include "web_dialog.h"

//#include "../controls/text_edit_emails.h"

//#include "../../logic/tree_model_dynamic_access.h"
//#include "../../logic/tree_model_dynamic_roles.h"
//#include "../../logic/tree_model_dynamic_other_users.h"

//#include <QLineEdit>
//#include <QCheckBox>
//#include <QComboBox>
//#include <QTextEdit>

///////////////////////////////////////////////////////
////// Web Dialog / add user to project
//class WebDialogAddUserToProject : public WebDialog
//{
//  Q_OBJECT

//public:
//  WebDialogAddUserToProject(QWidget* parent);
//  ~WebDialogAddUserToProject();

//  virtual void Accept() override;

//private:
//  void SetupModels();
//  void SetupUi();

//private:
//  //// Models
//  QPointer<TreeModelDynamicAccess>      Access;
//  QPointer<TreeModelDynamicOtherUsers>  OtherUsers;
//  QPointer<TreeModelDynamicRoles>       Roles;

//  //// Ui
//  QPointer<QWidget>         Body;
//  QPointer<QVBoxLayout>     BodyLayout;

//  //// Input
//  QPointer<TextEditEmails>  UsersView;
//  QPointer<QComboBox>       AccessView;
//  QPointer<QComboBox>       RolesView;
//};
