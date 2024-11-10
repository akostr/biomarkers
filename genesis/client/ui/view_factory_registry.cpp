#include "view_factory_registry.h"
#include "view_factory_impls.h"

namespace Details
{
  static ViewFactoryRegistry* ViewFactoryRegistry_Instance__ = nullptr;
}

/////////////////////////////////////////////////////////////
//// View factory registry
ViewFactoryRegistry::ViewFactoryRegistry(QObject* parent)
  : QObject(parent)
{
  Details::ViewFactoryRegistry_Instance__ = this;
}

ViewFactoryRegistry::~ViewFactoryRegistry()
{
  Details::ViewFactoryRegistry_Instance__ = nullptr;
}

ViewFactoryRegistry* ViewFactoryRegistry::Get()
{
  return Details::ViewFactoryRegistry_Instance__;
}

void ViewFactoryRegistry::RegisterFactory(const QString& type, ViewFactory* factory)
{
  Factories[type] = factory;
}

ViewFactory* ViewFactoryRegistry::GetFactory(const QString& type)
{
  return Factories.value(type).data();
}
