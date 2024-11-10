#include "icon_cache.h"

#include <QSharedPointer>

namespace
{
  QSharedPointer<IconCache> IconCache_Instance__;
}

IconCache::IconCache()
{
}

IconCache::~IconCache()
{
  for (const auto& icon : Cached)
    delete icon;
}

IconCache& IconCache::Get()
{
  if (IconCache_Instance__.isNull())
    IconCache_Instance__.reset(new IconCache);
  return *IconCache_Instance__;
}

QIcon IconCache::GetIcon(const QString& name)
{
  IconCache& instance = Get();
  if (instance.Cached.contains(name))
  {
    return *(instance.Cached.value(name));
  }

  QIcon* icon = new QIcon(name);
  instance.Cached.insert(name, icon);
  return *icon;
}

void IconCache::Clear()
{
  IconCache_Instance__.reset();
}
