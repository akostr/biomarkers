#include "view_root_toolbar_account_menu.h"

#include "../api/network_manager.h"
#include "../logic/notification.h"
#include "../logic/context_root.h"
#include <logic/known_context_tag_names.h>
#include "ui/known_view_names.h"
#include "ui/genesis_window.h"
#include <genesis_style/style.h>

//#include <QTextDocument>
//#include <QPainter>

#include <QHBoxLayout>
//#include <QComboBox>
#include <QPushButton>
//#include <QLabel>
#include <QMenu>
#include <QEvent>

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

////////////////////////////////////////////////////
//// Root toolbar account menu view class
namespace Views
{
ViewRootToolbarAccountMenu::ViewRootToolbarAccountMenu(QWidget* parent)
  : View(parent)
{
  SetupUi();
}

ViewRootToolbarAccountMenu::~ViewRootToolbarAccountMenu()
{
}

void ViewRootToolbarAccountMenu::SetupUi()
{
  //// Layout
  Layout = new QHBoxLayout(this);
  Layout->setContentsMargins(0,0,0,0);

  MenuButton = new QPushButton(this);
  Layout->addWidget(MenuButton);

  QString AccountMenuButtonStyleSheet =
"QPushButton {"
" border: none;"
" border-radius: 8px;"
" background-color: transparent;"
" font: @RegularBoldFont;"
" color: @fontColorPrimary;"
" text-align: left;"
"}\n"
"QPushButton:pressed {"
" background-color: rgba(0, 66, 105, 7%);"
"}\n"
"QPushButton:hover {"
" background-color: rgba(0, 66, 105, 7%);"
"}\n"
"QPushButton:open { /* when the button has its menu open */"
" background-color: rgba(0, 66, 105, 7%);"
"}\n"
"QPushButton::menu-indicator {"
" image: url(:/resource/controls/collpse_rt.png);"
" subcontrol-origin: padding;"
" subcontrol-position: right;"
"}\n"
"QPushButton::menu-indicator:pressed, QPushButton::menu-indicator:open {"
" position: relative;"
" top: 0px; left: -18px; /* shift the arrow by 2 px (original left = -20px) */"
"}";
  AccountMenuButtonStyleSheet = Style::ApplySASS(AccountMenuButtonStyleSheet);
  MenuButton->setStyleSheet(AccountMenuButtonStyleSheet);
  Menu = new QMenu(MenuButton);
  Menu->installEventFilter(this);
  MenuButton->setMenu(Menu);
  auto profileAction = new QAction(tr("Profile"), Menu);
  Menu->addAction(profileAction);
  initStatisticButtonForRootUser();
  Menu->addSeparator();

  auto logOutAction = new QAction(tr("Log out"), Menu);
  Menu->addAction(logOutAction);

  connect(logOutAction, &QAction::triggered, this,
          []()
          {
            if(auto page = GenesisWindow::Get()->CurrentPage())
              page->LogOut();
          });
  connect(profileAction, &QAction::triggered, this,
          []()
          {
            GenesisWindow::Get()->ShowPage(ViewPageProfileName);
          });
}

void ViewRootToolbarAccountMenu::initStatisticButtonForRootUser()
{
  if (const auto root = GenesisContextRoot::Get())
  {
    if (const auto context = root->GetContextUser())
    {
      auto statisticAction = new QAction(tr("Statistics"), Menu);
      Menu->addAction(statisticAction);

      connect(statisticAction, &QAction::triggered, this,
        []()
        {
          GenesisWindow::Get()->ShowPage(ViewPageStatisticName);
        });
    }
  }
}

void ViewRootToolbarAccountMenu::ApplyContextUser(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
    MenuButton->setText("  " + GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kUsername).toString()); // we need empty string here for left padding because QPushButton text-align: left; is insensitive to padding
  else if(dataId == Names::UserContextTags::kUsername)
    MenuButton->setText(data.toString());
}

bool ViewRootToolbarAccountMenu::eventFilter(QObject *watched, QEvent *event)
{
  if(watched == Menu && event->type() == QEvent::Show)
  {
    auto g = Menu->geometry();
    g.moveBottomLeft(MenuButton->mapToGlobal(MenuButton->rect().bottomRight()));
    Menu->setGeometry(g);
  }
  return false;
}
}//namespace Views
