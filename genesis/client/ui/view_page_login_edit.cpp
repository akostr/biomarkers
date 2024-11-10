#include "view_page_login_edit.h"

#include "../logic/context_root.h"
#include "../api/api_rest.h"
#include "../logic/notification.h"
#include "logic/known_context_tag_names.h"

#include <genesis_style/style.h>

#include <QLabel>

using namespace Core;

////////////////////////////////////////////////////
//// LoginEdit
namespace Views
{
ViewPageLoginEdit::ViewPageLoginEdit(QWidget* parent)
  : View(parent)
{
  SetupModels();
  SetupUi();
}

ViewPageLoginEdit::~ViewPageLoginEdit()
{
}

void ViewPageLoginEdit::SetupModels()
{
  JobFunctions = new TreeModelDynamicJobFunctions(this);
}

void ViewPageLoginEdit::SetupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
  Layout->setSpacing(0);

  //// Caption
  {
    QLabel* catpion = new QLabel(tr("Profile settings"), this);
    Layout->addWidget(catpion);

    catpion->setStyleSheet(Style::Genesis::GetH1());
  }

  //// Space
  Layout->addSpacing(Style::Scale(24));

  //// Inner
  {
    InnerLayout = new QHBoxLayout;
    Layout->addLayout(InnerLayout, 1);

    InnerLayout->setContentsMargins(0, 0, 0, 0);
    InnerLayout->setSpacing(Style::Scale(14));

    //// Content
    {
      Content = new QFrame(this);
      InnerLayout->addWidget(Content, 1);

      Content->setFrameShape(QFrame::Box);
      Content->setObjectName("rounded");
      Content->setStyleSheet("QLabel, QFrame, QStackedWidget { background-color: white } ");

      //// Content layout
      {
        ContentLayout = new QVBoxLayout(Content);
        ContentLayout->setContentsMargins(Style::Scale(32), Style::Scale(32), Style::Scale(32), Style::Scale(32));
        ContentLayout->setSpacing(0);

        //// Stacked pages
        {
          ContentStack = new QStackedWidget(Content);
          ContentLayout->addWidget(ContentStack);

          ContentStack->setStyleSheet(".QWidget { background-color: white; }");
          ContentStack->setMinimumWidth(500);
          ContentStack->setContentsMargins(0, 0, 0, 0);

          //// Personal Data
          {
            PersonalData = new QWidget(ContentStack);
            ContentStack->addWidget(PersonalData);

            PersonalDataLayout = new QVBoxLayout(PersonalData);
            PersonalDataLayout->setContentsMargins(0, 0, 0, 0);
            PersonalDataLayout->setSpacing(0);

            //// Personal Data form
            {
              PersonalDataForm = new QHBoxLayout;
              PersonalDataLayout->addLayout(PersonalDataForm);

              PersonalDataForm->setContentsMargins(0, 0, 0, 0);
              PersonalDataForm->setSpacing(Style::Scale(40));

              //// Left
              {
                PersonalDataFormLeft = new QVBoxLayout;
                PersonalDataForm->addLayout(PersonalDataFormLeft, 1);

                PersonalDataFormLeft->setContentsMargins(0, 0, 0, 0);
                PersonalDataFormLeft->setSpacing(0);

                //// Input
                {
                  //// Caption
                  QLabel* caption = new QLabel(tr("Personal data"), PersonalData);
                  PersonalDataFormLeft->addWidget(caption);

                  caption->setStyleSheet(Style::Genesis::GetH2());

                  //// Space
                  PersonalDataFormLeft->addSpacing(Style::Scale(16));

                  //// Fields
                  {
                    //// First name
                    {
                      QLabel* caption = new QLabel(tr("First name %1").arg(Style::GetInputAlert()), PersonalData);
                      PersonalDataFormLeft->addWidget(caption);
                      PersonalDataFormLeft->addSpacing(Style::Scale(4));

                      PersonalDataFirstName = new QLineEdit(PersonalData);
                      PersonalDataFormLeft->addWidget(PersonalDataFirstName);
                    }

                    PersonalDataFormLeft->addSpacing(Style::Scale(16));

                    //// Last name
                    {
                      QLabel* caption = new QLabel(tr("Last name %1").arg(Style::GetInputAlert()), PersonalData);
                      PersonalDataFormLeft->addWidget(caption);
                      PersonalDataFormLeft->addSpacing(Style::Scale(4));

                      PersonalDataLastName = new QLineEdit(PersonalData);
                      PersonalDataFormLeft->addWidget(PersonalDataLastName);
                    }

                    PersonalDataFormLeft->addSpacing(Style::Scale(16));

                    //// Phone
                    {
                      QLabel* caption = new QLabel(tr("Phone"), PersonalData);
                      PersonalDataFormLeft->addWidget(caption);
                      PersonalDataFormLeft->addSpacing(Style::Scale(4));

                      PersonalDataPhone = new QLineEdit(PersonalData);
                      PersonalDataFormLeft->addWidget(PersonalDataPhone);
                    }

                    PersonalDataFormLeft->addSpacing(Style::Scale(16));

                    //// Function
                    {
                      QLabel* caption = new QLabel(tr("Position"), PersonalData);
                      PersonalDataFormLeft->addWidget(caption);
                      PersonalDataFormLeft->addSpacing(Style::Scale(4));

                      PersonalDataFunction = new QComboBox(PersonalData);
                      PersonalDataFormLeft->addWidget(PersonalDataFunction);

                      //// Fill]
                      auto fill = [this]()
                      {
                        PersonalDataFunction->clear();
                        for (int r = 0; r < JobFunctions->rowCount(); r++)
                        {
                          int id     = JobFunctions->data(JobFunctions->index(r, TreeModelDynamicJobFunctions::ColumnId)).toInt();
                          QString jf = JobFunctions->data(JobFunctions->index(r, TreeModelDynamicJobFunctions::ColumnJobFunction)).toString();
                          PersonalDataFunction->addItem(jf, id);
                        }

                        //// Function
                        int functionIndex = PersonalDataFunction->findText(GenesisContextRoot::Get()->GetContextUser() ->GetData("function").toString());
                        if (functionIndex != -1)
                        {
                          PersonalDataFunction->setCurrentIndex(functionIndex);
                        }
                      };
                      connect(JobFunctions, &QAbstractListModel::modelReset, fill);
                      fill();
                    }
                  }
                }
              }

              //// Right
              {
                PersonalDataFormRight = new QVBoxLayout;
                PersonalDataForm->addLayout(PersonalDataFormRight, 1);

                PersonalDataFormRight->setContentsMargins(0, 0, 0, 0);
                PersonalDataFormRight->setSpacing(0);

                //// Photo
                {
                  //// Caption
                  QLabel* caption = new QLabel(PersonalData);
                  PersonalDataFormRight->addWidget(caption);

                  caption->setStyleSheet(Style::Genesis::GetH2());

                  //// Space
                  PersonalDataFormRight->addSpacing(Style::Scale(20));

                  //// Photo
                  {
                    //// ....
                  }
                }
              }
            }

            //// Hline
            {
              PersonalDataLayout->addSpacing(Style::Scale(20));

              QFrame* hline = new QFrame(PersonalData);
              hline->setFrameShape(QFrame::HLine);
              hline->setObjectName("hline");
              hline->setAutoFillBackground(true);
              PersonalDataLayout->addWidget(hline);

              PersonalDataLayout->addSpacing(Style::Scale(12));
            }

            //// Email
            {
              //// Caption
              {
                QLabel* caption = new QLabel(tr("Email address"), PersonalData);
                PersonalDataLayout->addWidget(caption);

                caption->setStyleSheet(Style::Genesis::GetH2());
              }

              PersonalDataLayout->addSpacing(Style::Scale(14));

              //// Value
              {
                PersonalDataEmail = new QLabel("email@domain.com", PersonalData);
                PersonalDataLayout->addWidget(PersonalDataEmail);
              }
            }

            //// Hline
            {
              PersonalDataLayout->addSpacing(Style::Scale(12));

              QFrame* hline = new QFrame(PersonalData);
              hline->setFrameShape(QFrame::HLine);
              hline->setObjectName("hline");
              hline->setAutoFillBackground(true);
              PersonalDataLayout->addWidget(hline);

              PersonalDataLayout->addSpacing(Style::Scale(16));
            }

            //// Accept
            {
              PersonalDataAccept = new QDialogButtonBox(QDialogButtonBox::Ok, Content);
              PersonalDataLayout->addWidget(PersonalDataAccept, 0, Qt::AlignLeft);

              if (auto ok = PersonalDataAccept->button(QDialogButtonBox::Ok))
              {
                ok->setText(tr("Save personal data"));
              }

              if (auto layout = qobject_cast<QHBoxLayout*>(PersonalDataAccept->layout()))
              {
                if (auto space = layout->itemAt(0))
                {
                  layout->removeItem(space);
                }
              }
            }
          }

          //// Notification Settings
          {
            NotificationSettings = new QWidget(ContentStack);
            ContentStack->addWidget(NotificationSettings);

            NotificationSettingsLayout = new QVBoxLayout(NotificationSettings);
            NotificationSettingsLayout->setContentsMargins(0, 0, 0, 0);
            NotificationSettingsLayout->setSpacing(0);

            //// Caption
            {
              QLabel* caption = new QLabel(tr("Notification settings"), PersonalData);
              NotificationSettingsLayout->addWidget(caption);

              caption->setStyleSheet(Style::Genesis::GetH2());

              //// Space
              NotificationSettingsLayout->addSpacing(Style::Scale(20));
            }

            //// Input
            {
              //// ....
            }

            //// Stretch
            NotificationSettingsLayout->addStretch();
          }
        }

        ///// Stretch content
        ContentLayout->addStretch(1);
      }
    }

    //// Menu area
    {
      MenuAreaLayout = new QVBoxLayout;
      InnerLayout->addLayout(MenuAreaLayout, 0);

      MenuAreaLayout->setContentsMargins(0, 0, 0, 0);
      MenuAreaLayout->setSpacing(0);

      //// Menu
      {
        Menu = new QFrame(this);
        MenuAreaLayout->addWidget(Menu);

        Menu->setFrameShape(QFrame::NoFrame);
        Menu->setObjectName("menu");
        Menu->setStyleSheet(Style::Genesis::GetPageNavigationStyle());

        //// Menu
        {
          MenuLayout = new QVBoxLayout(Menu);
          MenuLayout->setContentsMargins(0, Style::Scale(8), Style::Scale(64), Style::Scale(8));
          MenuLayout->setSpacing(0);

          //// Menu items
          {
            //// Personal data
            {
              MenuButtonPersonalData = new QRadioButton(tr("Personal data"), Menu);
              MenuButtonPersonalData->setStyleSheet(".QRadioButton::indicator           { width: 24px; height: 24px; }"
                                                    ".QRadioButton                      { background-color: white; }"
                                                    ".QRadioButton                      { border-left: 2px solid white; }"
                                                    ".QRadioButton:checked              { border-left: 2px solid #0071B2; }"
                                                    ".QRadioButton::indicator:unchecked { image: url(:/resource/icons/icon_profile_user.png); }"
                                                    ".QRadioButton::indicator:checked   { image: url(:/resource/icons/icon_profile_user_active.png); }");
              MenuButtonPersonalData->setChecked(true);
              MenuLayout->addWidget(MenuButtonPersonalData);
            }

            //// Notification settings
            {
              MenuButtonNotificationSettings = new QRadioButton(tr("Notification settings"), Menu);
              MenuButtonNotificationSettings->setStyleSheet(".QRadioButton::indicator           { width: 24px; height: 24px; }"
                                                            ".QRadioButton                      { background-color: white; }"
                                                            ".QRadioButton                      { border-left: 2px solid white; }"
                                                            ".QRadioButton:checked              { border-left: 2px solid #0071B2; }"
                                                            ".QRadioButton::indicator:unchecked { image: url(:/resource/icons/icon_profile_notifications.png); }"
                                                            ".QRadioButton::indicator:checked   { image: url(:/resource/icons/icon_profile_notifications_active.png); }");
              MenuLayout->addWidget(MenuButtonNotificationSettings);
            }
          }
        }
      }

      //// Space
      MenuAreaLayout->addStretch();
    }
  }

  //// Handle input
  auto handleInput = [this]()
  {
    if (auto contextRoot = GenesisContextRoot::Get())
    {
      if (auto contextUser = contextRoot->GetContextUser())
      {
        if (auto ok = PersonalDataAccept->button(QDialogButtonBox::Ok))
        {
          const bool dirty = PersonalDataFirstName->text()        != contextUser->GetData("first_name").toString()
                             || PersonalDataLastName->text()         != contextUser->GetData("last_name").toString()
                             || PersonalDataPhone->text()            != contextUser->GetData("phone").toString()
                             || PersonalDataFunction->currentText()  != contextUser->GetData("function").toString();
          const bool valid = !PersonalDataFirstName->text().isEmpty()
                             && !PersonalDataLastName->text().isEmpty();
          ok->setEnabled(dirty && valid);
        }
      }
    }
  };
  handleInput();

  connect(PersonalDataFirstName,  &QLineEdit::textChanged, handleInput);
  connect(PersonalDataLastName,   &QLineEdit::textChanged, handleInput);
  connect(PersonalDataPhone,      &QLineEdit::textChanged, handleInput);
  connect(PersonalDataFunction,   QOverload<int>::of(&QComboBox::currentIndexChanged), handleInput);

  //// Handle accept
  if (auto ok = PersonalDataAccept->button(QDialogButtonBox::Ok))
  {
    connect(ok, &QAbstractButton::clicked, []()
            {
              //// @todo apply user properties through API
              //// API::DB:: ...

              //// Update user info
              API::REST::Tables::GetUserInfo
                (
                  [](QNetworkReply*, QVariantMap result)
                  {
                    //// Model layout, one row expected
                    QVariantList rows = result["children"].toList();
                    if (rows.size())
                    {
                      //// All new data
                      QVariantMap row = rows.first().toMap();

                      //// Target
                      auto user = GenesisContextRoot::Get()->GetContextUser();

                      //// Preserve password
//                      QString password = user->GetData(Names::UserContextTags::kPassword).toString();
//                      row[Names::UserContextTags::kPassword] = password;
//                      row[Names::UserContextTags::kUserState] = Names::UserContextTags::UserState::authorized;

                      //// Set
//                      user->SetData(Names::UserContextTags::kPassword, password);
                      user->SetData(Names::UserContextTags::kUserState, Names::UserContextTags::UserState::authorized);
                    }
                  },
                  [](QNetworkReply*, QNetworkReply::NetworkError err)
                  {
                    Notification::NotifyError(tr("Failed to get user data from database"), err);
                  });
            });
  }

  //// Handle page buttons
  connect(MenuButtonPersonalData,         &QAbstractButton::clicked, [this]() { ContentStack->setCurrentWidget(PersonalData); });
  connect(MenuButtonNotificationSettings, &QAbstractButton::clicked, [this]() { ContentStack->setCurrentWidget(NotificationSettings); });
}

//// Handle user context
void ViewPageLoginEdit::ApplyContextUser(const QString& dataId, const QVariant& data)
{
  //// Apply data
  bool isReset = isDataReset(dataId, data);
  {
    QSet<QString> nessesaryTags = {"first_name",
                                   "last_name",
                                   "phone",
                                   "function",
                                   Names::UserContextTags::kEmail};
    if(!isReset && !nessesaryTags.contains(dataId))
      return;
  }

  if(isReset)
  {
    auto context = GenesisContextRoot::Get()->GetContextUser();
    PersonalDataFirstName->setText(context->GetData("first_name").toString());
    PersonalDataLastName->setText(context->GetData("last_name").toString());
    PersonalDataPhone->setText(context->GetData("phone").toString());

    //// Function
    int functionIndex = PersonalDataFunction->findText(context->GetData("function").toString());
    if (functionIndex != -1)
    {
      PersonalDataFunction->setCurrentIndex(functionIndex);
    }

    PersonalDataEmail->setText(context->GetData(Names::UserContextTags::kEmail).toString());
  }
  else
  {
    if(dataId == "first_name")
      PersonalDataFirstName->setText(data.toString());
    else if(dataId == "last_name")
      PersonalDataLastName->setText(data.toString());
    else if(dataId == "phone")
      PersonalDataPhone->setText(data.toString());
    else if(dataId == "function")
    {
      int functionIndex = PersonalDataFunction->findText(data.toString());
      if (functionIndex != -1)
      {
        PersonalDataFunction->setCurrentIndex(functionIndex);
      }
    }
    else if(dataId == Names::UserContextTags::kEmail)
      PersonalDataEmail->setText(data.toString());

  }


}
}//namespace Views
