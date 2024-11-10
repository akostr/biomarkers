#include "view_page_profile.h"
#include "ui_view_page_profile.h"
#include "genesis_style/style.h"
#include "logic/context_root.h"
#include "logic/notification.h"
#include "logic/known_context_tag_names.h"
#include "api/api_rest.h"

namespace Views
{
ViewPageProfile::ViewPageProfile(QWidget *parent)
  : View(parent),
    ui(new Ui::ProfilePage)
{
  SetupUi();
}

ViewPageProfile::~ViewPageProfile()
{
  delete ui;
}

void ViewPageProfile::SetupUi()
{
  ui->setupUi(this);
  ui->pageHeaderLabel->setStyleSheet(Style::Genesis::Fonts::H1());
  ui->pageSubHeaderLabel->setStyleSheet(Style::Genesis::Fonts::H2());
  ui->loginLabel->setStyleSheet(Style::Genesis::Fonts::RegularLight());
  ui->organizationLabel->setStyleSheet(Style::Genesis::Fonts::RegularLight());
  ui->availableGroupsLabel->setStyleSheet(Style::Genesis::Fonts::RegularLight());
  ui->loginDataLabel->setStyleSheet(Style::Genesis::Fonts::RegularText());
  ui->organizationDataLabel->setStyleSheet(Style::Genesis::Fonts::RegularText());
  ui->availableGroupsDataLabel->setStyleSheet(Style::Genesis::Fonts::RegularText());

  ui->loginDataLabel->setText(tr("Not loaded"));
  ui->organizationDataLabel->setText(tr("Not loaded"));
  ui->availableGroupsDataLabel->setText(tr("Not loaded"));

  ui->infoContainer->setStyleSheet(
    "QWidget {border: none;"
    "border-radius: 8px;"
    "background-color: white;}");
}

void ViewPageProfile::reloadData()
{
  ui->loginDataLabel->setText(tr("Loading"));
  ui->organizationDataLabel->setText(tr("Loading"));
  ui->availableGroupsDataLabel->setText(tr("Loading"));
  API::REST::Tables::GetUserInfo(
    [this](QNetworkReply* r, QVariantMap map)
    {
      auto children = map["children"].toList();
      auto user = children[0].toMap();
      auto userName = user["username"].toString();
      auto groupsList = user["groups"].toStringList();
      auto organization = user["organization"].toString();
      ui->loginDataLabel->setText(userName);
      ui->organizationDataLabel->setText(organization);
      ui->availableGroupsDataLabel->setText(groupsList.join("<p>"));
    },
    [](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      Notification::NotifyError(tr("Network error"), e);
    });
}

void ViewPageProfile::ApplyContextUser(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);

  if(isReset)
  {
    reloadData();
  }
  else if(dataId == Names::UserContextTags::kUserState)
  {
    if(data.toInt() == Names::UserContextTags::UserState::authorized)
      reloadData();
  }

}
}//namespace Views
