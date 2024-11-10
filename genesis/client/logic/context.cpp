#include "context.h"

#include "known_context_names.h"
#include "genesis_context_step_based_markup.h"

///////////////////////////////////////////////////////////////////
//// Genesis context

GenesisContext::GenesisContext(GenesisContext* parent)
  : INotifyObject(parent)
{
}

GenesisContext::~GenesisContext()
{
}

//// Data access
QVariantMap GenesisContext::GetData() const
{
  return Data;
}

void GenesisContext::SetData(const QVariantMap& data)
{
  if(IsLocked)
  {
    Q_ASSERT(false);//changes locked;
    return;
  }
  //// Dependecies
  ClearChildrenData();

  //// Self
  Data = data;
  NotifyReset();
}

QVariant GenesisContext::GetData(const QString& dataId) const
{
  return Data.value(dataId);
}

void GenesisContext::SetData(const QString& dataId, const QVariant& data)
{
  if(IsLocked)
  {
    Q_ASSERT(false);//changes locked;
    return;
  }
  //// Dependencies
//  if (dataId ==  Names::ContextTagNames::ProjectId && (Data.value("id") != data))
//  {
//    ClearChildrenData();
//  }

  //// Self
  if (!Data.contains(dataId)
    || Data.value(dataId) != data)
  {
    Data[dataId] = data;
    NotifyChanged(dataId, data);
  }
}

void GenesisContext::ReloadData()
{
  NotifyReset();
}

void GenesisContext::ClearData()
{
  //// Already empty?

  //// Dependencies
  ClearChildrenData();
  if (!Data.isEmpty())
  {
    Data.clear();
    NotifyReset();
  }
  //// Self
}

void GenesisContext::ClearData(const QString& dataId)
{
  if (Data.isEmpty())
    return;

  Data.remove(dataId);
  NotifyChanged(dataId, QVariant());
}

bool GenesisContext::HasData(const QString &dataId)
{
  return Data.contains(dataId);
}

void GenesisContext::ClearChildrenData()
{
  auto children = ChildContexts.values();
  for (auto& c : children)
  {
    c->ClearData();
  }
}

//// Notifications
void GenesisContext::NotifyChanged(const QString& dataId, const QVariant& data)
{
  emit Changed(this, dataId, data);
}

void GenesisContext::NotifyReset()
{
  emit Reset(this);
}

void GenesisContext::NotifyChildContextAdded(QPointer<GenesisContext> ctx)
{
  emit ChildContextAdded(ctx);
}

//// Class id
QString GenesisContext::GetClass()
{
  return ClassId;
}

void GenesisContext::SetClass(const QString& classId)
{
  ClassId = classId;
}

//// Composite
QPointer<GenesisContext> GenesisContext::GetParentContext()
{
  return qobject_cast<GenesisContext*>(parent());
}

void GenesisContext::SetChildContext(const QString& id, QPointer<GenesisContext> ctx)
{
  ChildContexts[id] = ctx;
}

QPointer<GenesisContext> GenesisContext::GetChildContext(const QString& id)
{
  if (auto ctx = FindChildContext(id))
    return ctx;

  auto ctx = new GenesisContext(this);
  ChildContexts[id] = ctx;
  ctx->SetClass(id);
  NotifyChildContextAdded(ctx);
  return ctx;
}

QPointer<GenesisContext> GenesisContext::FindChildContext(const QString& id)
{
  return ChildContexts.value(id);
}

QPointer<GenesisContext> GenesisContext::GetChildContextUi()
{
  auto context = GetChildContext(GenesisContextNames::Ui);
  context->lockChanges();
  return context;
}

QPointer<GenesisContext> GenesisContext::GetChildContextModules()
{
  auto context = GetChildContext(GenesisContextNames::Modules);
  context->lockChanges();
  return context;
}

QPointer<GenesisContextStepBasedMarkup> GenesisContext::GetChildContextStepBasedMarkup()
{
  auto id = GenesisContextNames::MarkupStepBased;
  if (auto ctx = FindChildContext(id))
  return static_cast<GenesisContextStepBasedMarkup*>(ctx.data());

  auto ctx = new GenesisContextStepBasedMarkup(this);
  ChildContexts[id] = ctx;
  ctx->SetClass(id);
  NotifyChildContextAdded(ctx);
  return ctx;
}

QMap<QString, GenesisContextPtr> GenesisContext::GetChildContexts()
{
  return ChildContexts;
}

void GenesisContext::LockedSetData(const QVariantMap &data)
{
  ClearChildrenData();

  //// Self
  Data = data;
  NotifyReset();
}

void GenesisContext::LockedSetData(const QString &dataId, const QVariant &data)
{
  if (!Data.contains(dataId)
      || Data.value(dataId) != data)
  {
    Data[dataId] = data;
    NotifyChanged(dataId, data);
  }
}

void GenesisContext::lockChanges()
{
  IsLocked = true;
}

void GenesisContext::unlockChanges()
{
  IsLocked = false;
}
