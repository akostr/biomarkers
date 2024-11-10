#include "tree_model_dynamic.h"
#include "tree_model_item.h"

#include "context_root.h"

#include "known_context_names.h"

using namespace Core;

////////////////////////////////////////////////////
//// Tree model dynamic
TreeModelDynamic::TreeModelDynamic(QObject* parent)
  : TreeModel(parent)
{
  SetContext(GenesisContextRoot::Get().get());
}

TreeModelDynamic::~TreeModelDynamic()
{
}

//// Dynamic
void TreeModelDynamic::Reset()
{
  //// Shall clear
  Root->SetFetched(false);
  Root->setProperty("error", 0);
  TreeModel::Reset();

//  if(canFetchMore(QModelIndex()))
//    FetchMore(Root);
}

//// Set context
void TreeModelDynamic::SetContext(GenesisContextPtr context)
{
  //connect if nessesary
  if(!Contexts.contains(context->GetClass()) ||
      Contexts[context->GetClass()] != context)
  {
    Contexts[context->GetClass()] = context;

    //apply context changes
    connect(context.get(), &GenesisContext::Changed, this, &TreeModelDynamic::ApplyContextChangesSlot, Qt::UniqueConnection);

    //reload context data
    connect(context.get(), &GenesisContext::Reset, this, &TreeModelDynamic::onContextReset, Qt::UniqueConnection);

    //connect newly created child contexts
    connect(context.get(), &GenesisContext::ChildContextAdded, this, &TreeModelDynamic::SetContext, Qt::UniqueConnection);
  }

  ApplyContextChangesSlot(context, "", QVariant());

  //// Apply child contexts
  for (auto c : context->GetChildContexts().values())
    SetContext(c);
}

void TreeModelDynamic::ApplyContextChangesSlot(IStorage *storage, const QString &dataId, const QVariant &data)
{

  if (auto context = (GenesisContext*)(storage))
  {
    QMap<QString, std::function<void(const QString&, const QVariant&)>> callbacks =
      {
       { GenesisContextNames::Root,[this](const QString& dataId, const QVariant& data) { ApplyContextRoot(dataId, data); }},
       { GenesisContextNames::Ui,[this](const QString& dataId, const QVariant& data) { ApplyContextUi(dataId, data); }},
       { GenesisContextNames::User,[this](const QString& dataId, const QVariant& data) { ApplyContextUser(dataId, data); }},
       { GenesisContextNames::Project,[this](const QString& dataId, const QVariant& data) { ApplyContextProject(dataId, data); } },
       { GenesisContextNames::FileCDF,[this](const QString& dataId, const QVariant& data) { ApplyContextFileCDF(dataId, data); } },
       { GenesisContextNames::Markup,[this](const QString& dataId, const QVariant& data) { ApplyContextMarkup(dataId, data); } },
       { GenesisContextNames::Library,[this](const QString& dataId, const QVariant& data) { ApplyContextLibrary(dataId, data); } },
       { GenesisContextNames::MarkupVersion,[this](const QString& dataId, const QVariant& data) { ApplyContextMarkupVersion(dataId, data); } },
       { GenesisContextNames::MarkupVersionAnalysisPCA,[this](const QString& dataId, const QVariant& data) { ApplyContextMarkupVersionAnalysisPCA(dataId, data); } },
       };
    if (callbacks.contains(context->GetClass()))
    {
      callbacks[context->GetClass()](dataId, data);
    }
  }
}

//// Dynamic features
void TreeModelDynamic::PeekMore()
{
  if (Root->property("error").toInt() != 0)
  {
    Root->setProperty("error", 0);
    if (canFetchMore(QModelIndex()))
    {
      FetchMore(Root);
    }
  }
}

void TreeModelDynamic::onContextReset(Core::IStorage *context)
{
  ApplyContextChangesSlot(context, "", QVariant());
}

bool TreeModelDynamic::isDataReset(const QString &dataId, const QVariant &data)
{
  return dataId.isEmpty() && !data.isValid();
}
