#include "view.h"
#include "view_factory_registry.h"

#include "../logic/tree_model.h"
#include <logic/known_context_names.h>
#include <logic/known_context_tag_names.h>
#include <ui/known_view_names.h>
#include <ui/genesis_window.h>
#include <logic/context_root.h>
#include <QApplication>

using namespace Core;

/////////////////////////////////////////////////////
//// View base class
namespace Views
{
View::View(QWidget* parent)
  : QWidget(parent)
{
}

View::~View()
{
}

//// Create
View* View::Create(const QString& factoryId, QWidget* parent)
{
  if (auto registry = ViewFactoryRegistry::Get())
  {
    if (auto factory = registry->GetFactory(factoryId))
    {
      return factory->CreateView(parent);
    }
  }
  return nullptr;
}

View *View::Create(const std::string &factoryId, QWidget *parent)
{
  return Create(_Q(factoryId), parent);
}

bool View::isDataReset(const QString &dataId, const QVariant &data)
{
  return dataId.isEmpty() && !data.isValid();
}

////// Set context
//void View::SetContext(GenesisContextPtr context)
//{
//  //// Store if changed
//  if (Contexts.value(context->GetClass()) != context)
//  {
//    //// Store
//    Contexts[context->GetClass()] = context;

//    //// Notify data changes
//    const auto data = context->GetData();
//    for (auto k : data.keys())
//    {
//      ContextChanged(context.get(), k, data[k]);
//    }

//    //// Connect
////    connect(context.get(), &GenesisContext::Changed, this, &View::ContextChanged, Qt::UniqueConnection);
//    connect(context.get(), &GenesisContext::Changed, this, &View::ApplyContextChangesSlot, Qt::UniqueConnection);
//    connect(context.get(), &GenesisContext::Reset,   this, [this](Core::IStorage* context){ApplyContextChangesSlot(context, "", QVariant());}, Qt::UniqueConnection);
//  }

//  //// Call specific virtual callbacks
//  QMap<QString, std::function<void(GenesisContextPtr)>> callbacks =
//  {
//    { GenesisContextNames::Root,[this](GenesisContextPtr context) { ApplyContextRoot(context); }},
//    { GenesisContextNames::Ui,[this](GenesisContextPtr context) { ApplyContextUi(context); }},
//    { GenesisContextNames::User,[this](GenesisContextPtr context) { ApplyContextUser(context); }},
//    { GenesisContextNames::Project,[this](GenesisContextPtr context) { ApplyContextProject(context); }},
//    { GenesisContextNames::FileCDF,[this](GenesisContextPtr context) { ApplyContextFileCDF(context); } },
//    { GenesisContextNames::Markup,[this](GenesisContextPtr context) { ApplyContextMarkup(context); }},
//    { GenesisContextNames::MarkupStepBased,[this](GenesisContextPtr context) { ApplyContextStepBasedMarkup(context); } },
//    { GenesisContextNames::MarkupVersion,[this](GenesisContextPtr context) { ApplyContextMarkupVersion(context); } },
//    { GenesisContextNames::MarkupVersionAnalysisPCA,[this](GenesisContextPtr context) { ApplyContextMarkupVersionAnalysisPCA(context); } },
//    { GenesisContextNames::PCAPlots,[this](GenesisContextPtr context) { ApplyContextPCAPlots(context); }},
//    { GenesisContextNames::MarkupVersionAnalysisPLS,[this](GenesisContextPtr context) { ApplyContextMarkupVersionAnalysisPLS(context); }},
//    { GenesisContextNames::MarkupVersionAnalysisPLSPredict,[this](GenesisContextPtr context) { ApplyContextMarkupVersionAnalysisPLSPredict(context); }},
//    { GenesisContextNames::MarkupVersionAnalysisMCR,[this](GenesisContextPtr context) { ApplyContextMarkupVersionAnalysisMCR(context); }},
//  };
//  if (callbacks.contains(context->GetClass()))
//  {
//    callbacks[context->GetClass()](context);
//  }

//  //// Apply child contexts
//  for (auto c : context->GetChildContexts().values())
//  {
//    SetContext(c);
//  }
//}

void View::SetContext(GenesisContextPtr context)
{
  //connect if nessesary
  if(!Contexts.contains(context->GetClass()) ||
      Contexts[context->GetClass()] != context)
  {
    Contexts[context->GetClass()] = context;

    //apply context changes
    connect(context.get(), &GenesisContext::Changed, this, &View::ApplyContextChangesSlot, Qt::UniqueConnection);

    //reload context data
    connect(context.get(), &GenesisContext::Reset, this, &View::onContextReset, Qt::UniqueConnection);

    //connect newly created child contexts
    connect(context.get(), &GenesisContext::ChildContextAdded, this, &View::SetContext, Qt::UniqueConnection);
  }

  //reload context data
  ApplyContextChangesSlot(context, "", QVariant());
  //// set child contexts
  for (auto& c : context->GetChildContexts())
    SetContext(c);
}

void View::SwitchToPage(const QString &pageId)
{
  auto overlayId =
      GenesisWindow::Get()->
      ShowOverlay(
          tr("Loading")
          + " \""
          + QCoreApplication::translate(
              "View page names",
              ViewPageNames::ViewPageTranslatedName[pageId])
          + "\"");
  // qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
  auto uiContext = GenesisContextRoot::Get()->GetContextUi();
  uiContext->LockedSetData(Names::ContextTagNames::Page, pageId);
  GenesisWindow::Get()->RemoveOverlay(overlayId);
  emit pageSwitchProcessed(pageId, true);
}

void View::SwitchToModule(int module)
{
  auto moduleContext = GenesisContextRoot::Get()->GetContextModules();
  auto projectContext = GenesisContextRoot::Get()->GetContextProject();

  moduleContext->LockedSetData(Names::ModulesContextTags::kModule, (Names::ModulesContextTags::Module)module);
  emit moduleSwitchProcessed(module, true);
}

void View::ForseResetModule()
{
  auto moduleContext = GenesisContextRoot::Get()->GetContextModules();
  moduleContext->LockedSetData(Names::ModulesContextTags::kModule, Names::ModulesContextTags::Module::MNoModule);
}

void View::ForseSetModule(int module)
{
  auto moduleContext = GenesisContextRoot::Get()->GetContextModules();
  moduleContext->LockedSetData(Names::ModulesContextTags::kModule, (Names::ModulesContextTags::Module)module);
}

void View::RejectPageSwitching()
{
  emit pageSwitchProcessed(CurrentPageId(), false);
}

void View::RejectModuleSwitching()
{
  emit moduleSwitchProcessed(CurrentModule(), false);
}

void View::LogOut()
{
  //this function must emit this signal anyway, otherwise bad things will happens
  emit logOutAcceptStatus(false);
}

void View::RejectLoggingOut()
{
  emit logOutAcceptStatus(true);
}

void View::ApplyContextChangesSlot(IStorage* storage, const QString& dataId, const QVariant& data)
{
  if (auto context = (GenesisContext*)(storage))
  {
    //// Call specific virtual callbacks
    QMap<QString, std::function<void(const QString &, const QVariant &)>> callbacks =
        {
            { GenesisContextNames::Root,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextRoot(dataId, data);
             }},
            { GenesisContextNames::Ui,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextUi(dataId, data);
             }},
            { GenesisContextNames::User,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextUser(dataId, data);
             }},
            { GenesisContextNames::Modules,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextModules(dataId, data);
             }},
            { GenesisContextNames::Project,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextProject(dataId, data);
             }},
            { GenesisContextNames::FileCDF,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextFileCDF(dataId, data);
             }},
            { GenesisContextNames::Markup,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextMarkup(dataId, data);
             }},
            { GenesisContextNames::Library,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextLibrary(dataId, data);
             }},
            { GenesisContextNames::CreationPlot,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextCreationPlot(dataId, data);
             }},
            { GenesisContextNames::MarkupStepBased,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextStepBasedMarkup(dataId, data);
             }},
            { GenesisContextNames::MarkupVersion,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextMarkupVersion(dataId, data);
             }},
            { GenesisContextNames::MarkupVersionAnalysisPCA,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextMarkupVersionAnalysisPCA(dataId, data);
             }},
            { GenesisContextNames::PCAPlots,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextPCAPlots(dataId, data);
             }},
            { GenesisContextNames::MarkupVersionAnalysisPLS,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextMarkupVersionAnalysisPLS(dataId, data);
             }},
            { GenesisContextNames::MarkupVersionAnalysisPLSPredict,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextMarkupVersionAnalysisPLSPredict(dataId, data);
             }},
            { GenesisContextNames::MarkupVersionAnalysisMCR,[this](const QString &dataId, const QVariant &data)
             {
               ApplyContextMarkupVersionAnalysisMCR(dataId, data);
             }},
        };
    if (callbacks.contains(context->GetClass()))
    {
      callbacks[context->GetClass()](dataId, data);
    }
  }
}

void View::onContextReset(Core::IStorage *context)
{
  ApplyContextChangesSlot(context, "", QVariant());
}

//void View::ApplyContextChangesSlot(IStorage* storage)
//{
//  if (auto context = (GenesisContext*)(storage))
//  {
//    SetContext(context);
//  }
//}

QList<QPointer<TreeModel>> View::GetModels()
{
  QList<QPointer<TreeModel>> models;

  auto ms = findChildren<TreeModel*>();
  for (auto& m : ms)
  {
    models << m;
  }

  return models;
}

void View::ResetModels()
{
  auto models = GetModels();
  for (auto& m : models)
  {
    m->Reset();
  }
}

QString View::CurrentPageId()
{
  if(auto ctx = GenesisContextRoot::Get())
  {
    if(auto uiCtx = ctx->GetContextUi())
    {
      return uiCtx->GetData(Names::ContextTagNames::Page).toString();
    }
  }
  return QString();
}

int View::CurrentModule()
{
  if(auto ctx = GenesisContextRoot::Get())
  {
    if(auto moduleCtx = ctx->GetContextModules())
    {
      return moduleCtx->GetData(Names::ModulesContextTags::kModule).toInt();
    }
  }
  return Names::ModulesContextTags::Module::MNoModule;
}
}//namespace Views
