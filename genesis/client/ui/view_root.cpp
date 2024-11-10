#include "genesis_window.h"
#include "view_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_context_names.h"
#include "ui/known_view_names.h"
#include "logic/context_root.h"
#include "ui/view_root_layout_pages.h"

#include <QSet>

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

////////////////////////////////////////////////////
//// Root view class
namespace Views
{
ViewRoot::ViewRoot(QWidget* parent)
  : View(parent)
{
  SetupUi();
}

ViewRoot::~ViewRoot()
{
}

QPointer<View> ViewRoot::GetCurrentPage()
{
  if(auto lp = qobject_cast<ViewRootLayoutPages*>(LayoutPages))
    return lp->GetCurrentPage();

  return nullptr;
}

QString ViewRoot::GetCurrentPageId()
{
  if(auto lp = qobject_cast<ViewRootLayoutPages*>(LayoutPages))
    return lp->GetCurrentPageId();

  return QString();
}

void ViewRoot::SetupUi()
{
  //// Shall display either welcome layout or pages layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(0, 0, 0, 0);
  setContentsMargins(0, 0, 0, 0);

  //// Layout stack
  LayoutStack = new QStackedWidget(this);
  Layout->addWidget(LayoutStack);

  LayoutStack->setContentsMargins(0, 0, 0, 0);

  //// Welcome layout
   LayoutWelcome = View::Create(ViewRootLayoutWelcomePageName, LayoutStack);
  LayoutStack->addWidget(LayoutWelcome);

  //// Pages layout
  LayoutPages = View::Create(ViewRootLayoutPagesName, LayoutStack);
  LayoutStack->addWidget(LayoutPages);

  //// Modules layout
  LayoutModulesPage = View::Create(ViewPageModuleSelection, LayoutStack);
  LayoutStack->addWidget(LayoutModulesPage);
}

//// Handle user context
void ViewRoot::ApplyContextUser(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    auto context = GenesisContextRoot::Get()->GetContextUser();
    auto modulesContext = GenesisContextRoot::Get()->GetContextModules();
    auto login = context->GetData(Names::UserContextTags::kLogin).toString();
    if(login != LastUserLogin)
      GenesisContextRoot::Get()->GetContextProject()->ClearData();

    //// Logged in
    if (context->GetData(Names::UserContextTags::kUserState).toInt() == Names::UserContextTags::authorized)
    {
      if(modulesContext->GetData(Names::ModulesContextTags::kModule).toInt() == Names::ModulesContextTags::MNoModule)
      {
      	context->SetData(Names::UserContextTags::kInitialPageId, QVariant());
      	LayoutStack->setCurrentWidget(LayoutModulesPage);
      }
      else if (LayoutStack->currentWidget() != LayoutPages)
      {
        LayoutStack->setCurrentWidget(LayoutPages);
        // QString initialPageId = ViewProjectsPageName;

        // if(context->HasData(Names::UserContextTags::kInitialPageId))
        // {
        //   auto pageId = context->GetData(Names::UserContextTags::kInitialPageId);
        //   if(pageId.isValid())
        //     initialPageId = pageId.toString();
        // }
        // //// Initially, show list of projects
        // GenesisWindow::Get()->ShowPage(initialPageId);
      }
    }
    else
    {
      if (LayoutStack->currentWidget() != LayoutWelcome)
        LayoutStack->setCurrentWidget(LayoutWelcome);
    }
  }
  else if(dataId == Names::UserContextTags::kUserState)
  {
    auto userState = (Names::UserContextTags::UserState)dataId.toInt();
    auto context = GenesisContextRoot::Get()->GetContextUser();
    switch(userState)
    {
    case UserContextTags::notAuthorized:
    {
      if (LayoutStack->currentWidget() != LayoutWelcome)
      {
        LayoutStack->setCurrentWidget(LayoutWelcome);
        LastUserLogin = context->GetData(Names::UserContextTags::kLogin).toString();
      }
      break;
    }
    case UserContextTags::authorized:
    {
      auto login = context->GetData(Names::UserContextTags::kLogin).toString();
      if(login != LastUserLogin)
        GenesisContextRoot::Get()->GetContextProject()->ClearData();

      if (LayoutStack->currentWidget() != LayoutPages)
      {
        LayoutStack->setCurrentWidget(LayoutPages);
        QString initialPageId = ViewProjectsPageName;

        if(context->HasData(Names::UserContextTags::kInitialPageId))
        {
          auto pageId = context->GetData(Names::UserContextTags::kInitialPageId);
          if(pageId.isValid())
            initialPageId = pageId.toString();
        }
        //// Initially, show list of projects
        GenesisWindow::Get()->ShowPage(initialPageId);
      }
      break;
    }
    }
  }
}

void ViewRoot::ApplyContextModules(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  auto currentModule = ModulesContextTags::MNoModule;

  if(isReset)
  {
    auto context = GenesisContextRoot::Get()->GetContextModules();
    currentModule = (ModulesContextTags::Module)context->GetData(Names::ModulesContextTags::kModule).toInt();
    applyModule(currentModule);
  }
  else if(dataId == ModulesContextTags::kModule)
  {
    currentModule = (ModulesContextTags::Module)data.toInt();
    applyModule(currentModule);
  }

}

void ViewRoot::applyModule(int module)
{
  auto userContext = GenesisContextRoot::Get()->GetContextUser();
  bool isAuthorized = userContext->GetData(UserContextTags::kUserState).toInt() == UserContextTags::authorized;
  if(module == ModulesContextTags::MNoModule)
  {
    if (isAuthorized)
    {
      LayoutStack->setCurrentWidget(LayoutModulesPage);
    }
  }
  else
  {
    if (isAuthorized)
    {
      LayoutStack->setCurrentWidget(LayoutPages);
      QString initialPageId = ViewProjectsPageName;

      if(userContext->HasData(Names::UserContextTags::kInitialPageId))
      {
        auto pageId = userContext->GetData(Names::UserContextTags::kInitialPageId);
        if(pageId.isValid())
          initialPageId = pageId.toString();
      }
      //// Instantly switch page without confirmation because
      /// while switching modules we alredy have confirmation
      View::SwitchToPage(initialPageId);
      GenesisContextRoot::Get()->GetContextProject()->SetData(ContextTagNames::ProjectId, 0);
    }
  }
}

}//namespace Views
