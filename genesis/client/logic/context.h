#pragma once

#include <QMap>
#include <QVariantMap>
#include <QUuid>

#include "istorage.h"
#include "iclass.h"
#include "inotify_object.h"

class GenesisContextStepBasedMarkup;
namespace Views
{
class View;
}
///////////////////////////////////////////////////////////////////
//// Genesis context
class GenesisContext : public Core::INotifyObject, public Core::IStorage, public Core::IClass
{
  Q_OBJECT
public:
  explicit GenesisContext(GenesisContext* parent);
  virtual ~GenesisContext();

  //// Data access
  QVariantMap GetData() const override;
  void        SetData(const QVariantMap& data) override;
  QVariant    GetData(const QString& dataId) const override;
  void        SetData(const QString& dataId, const QVariant& data) override;
  void        ReloadData();
  void        ClearData() override;
  void        ClearChildrenData();
  void        ClearData(const QString& dataId) override;
  bool        HasData(const QString& dataId) override;

  //// Notifications
  void      NotifyChanged(const QString& dataId, const QVariant& data) override;
  void      NotifyReset() override;
  void      NotifyChildContextAdded(QPointer<GenesisContext> ctx);

signals:
  void ChildContextAdded(QPointer<GenesisContext> ctx);

public:
  //// Class id
  QString   GetClass() override;
  void      SetClass(const QString& classId) override;

  //// Composite
  QPointer<GenesisContext> GetParentContext();
  void SetChildContext(const QString &id, QPointer<GenesisContext> ctx);
  QPointer<GenesisContext> GetChildContext(const QString& id);
  QPointer<GenesisContext> FindChildContext(const QString& id);
  QPointer<GenesisContext> GetChildContextUi();
  QPointer<GenesisContext> GetChildContextModules();
  QPointer<GenesisContextStepBasedMarkup> GetChildContextStepBasedMarkup();

  QMap<QString, QPointer<GenesisContext>> GetChildContexts();

protected:
  //// Stored data
  QVariantMap Data;

  //// Class id
  QString     ClassId;

  QMap<QString, QPointer<GenesisContext>> ChildContexts;
  bool IsLocked = false;


  void LockedSetData(const QVariantMap& data);
  void LockedSetData(const QString& dataId, const QVariant& data);
  void lockChanges();
  void unlockChanges();

  friend class Views::View;
};

using GenesisContextPtr = QPointer<GenesisContext>;
