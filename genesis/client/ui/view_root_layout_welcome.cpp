#include "view_root_layout_welcome.h"
#include <logic/known_context_tag_names.h>
#include "ui/known_view_names.h"

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

////////////////////////////////////////////////////
//// Root Layout Welcome view class
namespace Views
{
ViewRootLayoutWelcome::ViewRootLayoutWelcome(QWidget* parent)
  : View(parent)
{
  SetupUi();
}

ViewRootLayoutWelcome::~ViewRootLayoutWelcome()
{
}

void ViewRootLayoutWelcome::SetupUi()
{
  //// Root
  RootLayout = new QHBoxLayout(this);
  RootLayout->setSpacing(0);
  RootLayout->setContentsMargins(0, 0, 0, 0);

  //// Content
  enum content
  {
    contentAreaBanners,
    contentAreaRegistration
  };

  //// Banners area
  {
    auto banners = View::Create(ViewRootLayoutWelcomeBannersPageName, this);
    RootLayout->addWidget(banners);
    RootLayout->setStretch(contentAreaBanners, 1);
  }

  //// Registration area
  {
    //// Shall display either login or create login view
    Layout = new QVBoxLayout;
    RootLayout->addLayout(Layout);
    RootLayout->setStretch(contentAreaRegistration, 2);

    Layout->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);

    //// Layout stack
    LayoutStack = new QStackedWidget(this);
    LayoutStack->setStyleSheet("background: #ffffff;");
    Layout->addWidget(LayoutStack);

    LayoutStack->setContentsMargins(0, 0, 0, 0);

    {
      //// Login
      Login = View::Create(ViewLoginPageName, LayoutStack);
      LayoutStack->addWidget(Login);
    }

    {
      //// Login create
      LoginCreate = View::Create(ViewLoginCreatePageName, LayoutStack);
      LayoutStack->addWidget(LoginCreate);
    }
  }
}

//// Handle ui context
void ViewRootLayoutWelcome::ApplyContextUi(const QString& dataId, const QVariant& data)
{
  if(dataId != Names::ContextTagNames::Page)
    return;
  QMap<QString, QPointer<View>> mapping =
  {
    { ViewLoginPageName,        Login },
    { ViewLoginCreatePageName,  LoginCreate },
  };
  if (auto page = mapping.value(data.toString()))
  {
    LayoutStack->setCurrentWidget(page);
  }
}
}//namespace Views
