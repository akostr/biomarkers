#include "genesis_window.h"
#include "view_login.h"

#include "../api/api_rest.h"
#include "../logic/context_root.h"
#include "../logic/notification.h"
#include "logic/known_context_tag_names.h"
#include "ui/known_view_names.h"
#include "api/network_manager.h"
#include "genesis_style/style.h"
#include "logic/app.h"

#include <QLabel>
#include <QToolButton>
#include <QDebug>
#include <QMenu>



using namespace Core;
using namespace Names;
using namespace ViewPageNames;

namespace
{
  //static QString defaultGroup = "appname";
  static QString APPLICATION_NAME = "AppName";
  static QString APP_URL = "WebAppUrl";
  static QString KEYCLOAK_URL = "KeycloakUrl";
  static QString CLIENT_ID = "client_id";
  static QString TEST_U = "test_u";
  static QString TEST_P = "test_p";
}

////////////////////////////////////////////////////
//// Login view class
namespace Views
{
ViewLogin::ViewLogin(QWidget* parent)
  : View(parent), AppSettings("config.cfg", QSettings::IniFormat)
{
  SetupUi();
}

ViewLogin::~ViewLogin()
{
}

void ViewLogin::SetupUi()
{
  qDebug() << "Setup ui view root login";
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Root
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(Style::Scale(44), Style::Scale(20), Style::Scale(44), Style::Scale(20));
  setContentsMargins(0, 0, 0, 0);

  //// Grid
  {
    LayoutGrid = new QGridLayout;
    Layout->addLayout(LayoutGrid);

    enum column
    {
      columnLeft,
      columnContent,
      columnRight
    };

    enum row
    {
      rowTop,
      rowContent,
      rowBottom,
    };

    LayoutGrid->setColumnStretch(columnLeft,    1);
    LayoutGrid->setColumnStretch(columnContent, 0);
    LayoutGrid->setColumnStretch(columnRight,   1);

    LayoutGrid->setRowStretch(rowTop,     1);
    LayoutGrid->setRowStretch(rowContent, 0);
    LayoutGrid->setRowStretch(rowBottom,  1);

    //// Inputs
    {
      Login = new QWidget(this);
      LayoutGrid->addWidget(Login, rowContent, columnContent);
      Login->setContentsMargins(0, 0, 0, 0);

      LoginLayout = new QVBoxLayout(Login);
      LoginLayout->setContentsMargins(0, 0, 0, 0);
      LoginLayout->setSpacing(0);

      //// Content
      {
        //// Caption
        {
          QHBoxLayout* box = new QHBoxLayout();
          box->setContentsMargins(0, 0, 0, 0);
          QLabel* caption = new QLabel(tr("Authorize"), Login);
          ActionSettings = new QPushButton(this);
          ActionSettings->setProperty("login", true);
          ActionSettings->setToolTip(tr("Choose config file for backend."));
          auto menu = new QMenu(ActionSettings);
          ConfigSelector = new QActionGroup(ActionSettings);
          auto defaultAction = ConfigSelector->addAction(AppSettings.value(APPLICATION_NAME).toString());
          defaultAction->setCheckable(true);
          defaultAction->setChecked(true);
          menu->addAction(defaultAction);
          for (const auto groupName : AppSettings.childGroups())
          {
            const auto appName = AppSettings.value(QString("%1/%2").arg(groupName).arg(APPLICATION_NAME)).toString();
            auto group = ConfigSelector->addAction(appName);
            group->setCheckable(true);
            menu->addAction(group);
          }
          ActionSettings->setStyleSheet("border-color: transparent;");
          ActionSettings->setMenu(menu);
          box->addWidget(caption);
          box->addWidget(ActionSettings);
          box->addSpacing(Style::Scale(268));
          LoginLayout->addLayout(box);
          caption->setStyleSheet(Style::Genesis::Fonts::H1());
        }

        //// Space
        LoginLayout->addSpacing(Style::Scale(20));

        //// Email
        {
          QLabel* caption = new QLabel(tr("Login %1").arg(Style::GetInputAlert()), Login);
          caption->setStyleSheet(Style::Genesis::Fonts::RegularText() + Style::ApplySASS("QLabel {color:@fontColorSecondary}"));
          LoginLayout->addWidget(caption);
          LoginLayout->addSpacing(Style::Scale(4));

          Email = new QLineEdit(Login);
          LoginLayout->addWidget(Email);
        }

        //// Space
        LoginLayout->addSpacing(Style::Scale(16));

        //// Password
        {
          QLabel* caption = new QLabel(tr("Password %1").arg(Style::GetInputAlert()), Login);
          caption->setStyleSheet(Style::Genesis::Fonts::RegularText() + Style::ApplySASS("QLabel {color:@fontColorSecondary}"));
          LoginLayout->addWidget(caption);

          LoginLayout->addSpacing(Style::Scale(4));

          Password = new QLineEdit(Login);
          Password->setEchoMode(QLineEdit::Password);
          LoginLayout->addWidget(Password);
        }

        //// Space
        LoginLayout->addSpacing(Style::Scale(32));

        //// Accept
        {
          Accept = new QDialogButtonBox(QDialogButtonBox::Ok, Login);
          LoginLayout->addWidget(Accept);

          if (auto ok = Accept->button(QDialogButtonBox::Ok))
          {
            ok->setText(tr("Log in"));
            ok->setStyleSheet(Style::Genesis::GetUiStyle());
          }

          if (auto layout = qobject_cast<QHBoxLayout*>(Accept->layout()))
          {
            if (auto space = layout->itemAt(0))
            {
              layout->removeItem(space);
            }
          }

          connect(Email,    &QLineEdit::textChanged, this, &ViewLogin::handleInput);
          connect(Password, &QLineEdit::textChanged, this, &ViewLogin::handleInput);

          connect(Email, &QLineEdit::returnPressed, this, &ViewLogin::returnPressed);
          connect(Password, &QLineEdit::returnPressed, this, &ViewLogin::returnPressed);
          connect(ConfigSelector, &QActionGroup::triggered, this, &ViewLogin::reconfigure);
          handleInput();

          //// Handle login
          connect(Accept, &QDialogButtonBox::accepted, this, &ViewLogin::TryToLogin);
        }
      }
    }
  }
}

void ViewLogin::ApplyContextUser(const QString& dataId, const QVariant& data)
{
  //// Created?
  if (Layout)
  {
    bool isReset = isDataReset(dataId, data);
    {
      QSet<QString> nessesaryTags = {Names::UserContextTags::kLogin, Names::UserContextTags::kPassword};
      if(!isReset && !nessesaryTags.contains(dataId))
        return;
    }
    if(isReset)
    {
      auto userContext = GenesisContextRoot::Get()->GetContextUser();
      Email->setText(userContext->GetData(Names::UserContextTags::kLogin).toString());
      Password->setText(userContext->GetData(Names::UserContextTags::kPassword).toString());
    }
    else
    {
      if(dataId == Names::UserContextTags::kLogin)
        Email->setText(data.toString());
      else if(dataId == Names::UserContextTags::kPassword)
        Password->setText(data.toString());
    }
  }
}

void ViewLogin::reconfigure(QAction* action)
{
  const auto name = action->text();
  const auto generalName = AppSettings.value(APPLICATION_NAME).toString();
  QString selectedGroup;
  if (name != generalName)
  {
    const auto groups = AppSettings.childGroups();
    for (const auto group : groups)
    {
      const auto groupName = QString("%1/%2").arg(group).arg(APPLICATION_NAME);
      if (AppSettings.value(groupName).toString() == name)
      {
        selectedGroup = group;
        break;
      }
    }
  }
  const QString app_url = AppSettings.value(name == generalName ? APP_URL : QString("%1/%2").arg(selectedGroup).arg(APP_URL)).toString();
  const QString keycloak_url = AppSettings.value(name == generalName ? KEYCLOAK_URL : QString("%1/%2").arg(selectedGroup).arg(KEYCLOAK_URL)).toString();
  const QString client_id = AppSettings.value(name == generalName ? CLIENT_ID : QString("%1/%2").arg(selectedGroup).arg(CLIENT_ID)).toString();

  const QString loginString = AppSettings.value(name == generalName ? TEST_U: QString("%1/%2").arg(selectedGroup).arg(TEST_U)).toString();
  Email->setText(loginString);
  const QString passwordString = AppSettings.value(name == generalName ? TEST_P: QString("%1/%2").arg(selectedGroup).arg(TEST_P)).toString();
  Password->setText(passwordString);

  const auto appName = AppSettings.value(name == generalName ? APPLICATION_NAME : QString("%1/%2").arg(selectedGroup).arg(APPLICATION_NAME)).toString();
  GenesisApp::activeWindow()->setWindowTitle(appName);
  auto netManager = NetworkManager::Get();
  netManager->SetHost(app_url);
  netManager->SetKeyCloak(keycloak_url, client_id);
  auto rootContext = GenesisContextRoot::Get();

#ifndef PLATFORM_WASM
#ifndef PUBLIC
  if (!AppSettings.value(TEST_U).toString().isEmpty())
  {
    rootContext.get()->GetContextUser()->SetData(Names::UserContextTags::kLogin, AppSettings.value(TEST_U).toString());
  }
  if (!AppSettings.value(TEST_P).toString().isEmpty())
  {
    rootContext.get()->GetContextUser()->SetData(Names::UserContextTags::kPassword, AppSettings.value(TEST_P).toString());
  }
#endif
#endif
}

void ViewLogin::handleInput()
{
  Accept->setEnabled(!Email->text().isEmpty() && !Password->text().isEmpty());
}

void ViewLogin::returnPressed()
{
  if (Accept->isEnabled())
    TryToLogin();
}

//#define AVOID_AUTHORIZATION
void ViewLogin::TryToLogin()
{
#ifndef PLATFORM_WASM
  qDebug() <<"OpenSSL status:" << QSslSocket::supportsSsl() << QSslSocket::sslLibraryVersionString();
#endif

  //// Store login data in context
  QString login    = Email->text().simplified();
  QString password = Password->text();
  auto userContext = GenesisContextRoot::Get()->GetContextUser();
  userContext->SetData(Names::UserContextTags::kLogin, login);
  userContext->SetData(Names::UserContextTags::kPassword, password);
//  auto userData = userContext->GetData();
//  userData[Names::UserContextTags::kLogin] = login;
//  userData[Names::UserContextTags::kPassword] = password;

//REMOVEME: YOU MUST(!) REMOVE THIS SECTION ON RELEASE!!! IT'S DANGEROUS!!!
//#define AVOID_AUTHORIZATION
#ifdef AVOID_AUTHORIZATION
  QVariantMap userData = QJsonDocument::fromJson(
"{\n"
"  \"group\": \"StrataSolution\",\n"
"  \"id\": -1,\n"
"  \"login\": \"fake_login\",\n"
"  \"kUsername\": \"fake_login\",\n"
"  \"password\": \"fake_password\",\n"
"  \"licenses_int_arr\": [1],\n"
"  \"state\": 1\n"
"}").object().toVariantMap();
  userContext->SetData(userData);
  return;
#else
//  userContext->SetData(userData);
  //// Perform login
  API::REST::Authorize(login, password,
  [login](QNetworkReply*, QVariantMap result)
  {
#ifdef DEBUG
    qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(result)).toJson();
#endif
    //// Model layout, one row expected
    QVariantList rows = result["children"].toList();
    if (rows.size())
    {
      //// All new data
      QVariantMap row = rows.first().toMap();

      //// Target
      auto user = GenesisContextRoot::Get()->GetContextUser();

      //// Preserve password
      auto userData = user->GetData();
      QString password = userData.value(Names::UserContextTags::kPassword).toString();
      QString username = row[Names::UserContextTags::kUsername].toString();
      QJsonArray jgroups = row[Names::UserContextTags::kJSonArrGroups].toJsonArray();
      QJsonArray jlicenses = row[Names::UserContextTags::kJSonArrLicenses].toJsonArray();
      QStringList groups;
      QList<int> availableModules;

//      //TODO: convert groups and licenses by right way
//      auto jgroupToGroup = [](const QString& jgroupName)->int{return 0;};
//      auto jlicenseToLicense = [](const QString& jlicenseName)->int{return 0;};
//      //TODO END;

      using namespace Names::UserContextTags;
      using namespace Names::ModulesContextTags;
      for(int i = 0; i < jgroups.size(); i++)
      {
        auto jgroup = jgroups[i].toString();
        groups.append(jgroup);
      }
      for(int i = 0; i < jlicenses.size(); i++)
      {
        auto jlicense = jlicenses[i].toString();
        if(kLicenseToModule.contains(jlicense))
          availableModules.append(kLicenseToModule[jlicense]);
      }
      row[Names::UserContextTags::kGroups] = QVariant::fromValue(groups);
      row[Names::UserContextTags::kCurrentGroup] = QString();
      row[Names::UserContextTags::kAvailableModules] = QVariant::fromValue(availableModules);

      row[Names::UserContextTags::kPassword] = password;
      row[Names::UserContextTags::kLogin] = login;
      row[Names::UserContextTags::kUserState] = Names::UserContextTags::authorized;
      userData.insert(row);
//      qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(userData)).toJson();

      //// Set
      user->SetData(userData);

      //// Notify
      Notification::NotifyHeaderless(tr("Logged in"));
    }
  },
  [](QNetworkReply*, QNetworkReply::NetworkError err)
  {
    //// Failed, clear context
    auto user = GenesisContextRoot::Get()->GetContextUser();
    user->SetData(Names::UserContextTags::kUserState, Names::UserContextTags::notAuthorized);
    //// Notify
    Notification::NotifyError(tr("Login failed"), tr("Login error"), err);
  });
#endif
}
}
