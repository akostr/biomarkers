#include "view_root_layout_welcome_banners.h"

#include <genesis_style/style.h>

#include "../../../build_number.h"

#include <QLabel>
#include <QPixmap>

////////////////////////////////////////////////////
//// Root Layout Welcome Banners view class
namespace Views
{
ViewRootLayoutWelcomeBanners::ViewRootLayoutWelcomeBanners(QWidget* parent)
  : View(parent)
{
  SetupUi();
}

ViewRootLayoutWelcomeBanners::~ViewRootLayoutWelcomeBanners()
{
}

void ViewRootLayoutWelcomeBanners::SetupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(0, 0, 0, 0);
  setContentsMargins(0, 0, 0, 0);

  //// Root
  Root = new QWidget(this);
  Layout->addWidget(Root);

  Root->setContentsMargins(0, 0, 0, 0);
  Root->setStyleSheet("background: #ecf1f4;");

  //// Root layout
  RootLayout = new QVBoxLayout(Root);
  RootLayout->setContentsMargins(Style::Scale(44), Style::Scale(32), Style::Scale(64), Style::Scale(32));

  enum content
  {
    contentLogo,
    contentBanners
  };

  //// Logo
  {
    QLabel* logo = new QLabel(Root);
    logo->setPixmap(QPixmap("://resource/Logo_short@2x.png"));
    logo->setContentsMargins(0, 0, 0, 0);
    RootLayout->addWidget(logo);
    RootLayout->setStretch(contentLogo, 0);
  }

  //// Content
  Content = new QWidget(Root);
  RootLayout->addWidget(Content);
  RootLayout->setStretch(contentBanners, 1);

  Content->setContentsMargins(0, 0, 0, 0);

  //// Content layout
  ContentLayout = new QVBoxLayout(Content);
  Content->setContentsMargins(0, 0, 0, 0);
  
  //// Build info
  QString buildInformation = tr("Build %1")
    .arg(BUILD_NUMBER);
  QString datetime = QString("%1\n%2").arg(BUILD_DATE).arg(BUILD_TIME);
  QLabel* build = new QLabel(buildInformation, this);
  build->setToolTip(datetime);
  build->setStyleSheet(" QLabel { color: #59002033; } ");
  build->setEnabled(false);
  RootLayout->addWidget(build);
}
}//namespace Views
