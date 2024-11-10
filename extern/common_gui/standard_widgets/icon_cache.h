#pragma once

#include <QIcon>
#include <QMap>

class IconCache
{
public:
  explicit IconCache();
  ~IconCache();

  static IconCache& Get();
  static QIcon GetIcon(const QString& name);
  static void Clear();

private:
  mutable QMap<QString, QIcon*> Cached;
};
