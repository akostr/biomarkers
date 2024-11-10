#include "notification.h"

#include <QTimer>
#include <QApplication>
#include <QUuid>

namespace Details
{
  QPointer<NotificationManager> NotificationManager_Instance__;
  unsigned long long            Order__ = 0;
}

///////////////////////////////////////////////////////////////////
//// Notification data
Notification::Notification(Notification* parent)
  : QObject(parent)
  , Order(Details::Order__ ++)
{
}

Notification::~Notification()
{
}

void Notification::NotifyHeaderless(const QString &text)
{
  Notify(QUuid::createUuid().toString(), Status::StatusHeaderlessInformation, text, "", QNetworkReply::NoError);
}

void Notification::NotifyInfo(const QString& text, const QString& header)
{
  Notify(QUuid::createUuid().toString(), Status::StatusInformation, text, header, QNetworkReply::NoError);
}

void Notification::NotifyError(const QString& text, const QString& header, QNetworkReply::NetworkError error)
{
  Notify(QUuid::createUuid().toString(), Status::StatusError, text, header, error);
}

void Notification::NotifyError(const QString &text, QNetworkReply::NetworkError error)
{
  NotifyError(text, tr("Network error"), error);
}

void Notification::NotifyWarning(const QString &text, const QString &header)
{
  Notify(QUuid::createUuid().toString(), Status::StatusWarning, text, header, QNetworkReply::NoError);
}

void Notification::NotifySuccess(const QString &text, const QString &header)
{
  Notify(QUuid::createUuid().toString(), Status::StatusSuccess, text, header, QNetworkReply::NoError);
}

void Notification::NotifyMulti(const QString &id, const QString &text)
{
  Notify(id, Status::StatusViewChildren, text, "", QNetworkReply::NoError);
}

void Notification::NotifySubMultiProgress(const QString &id, const QString &text)
{
  Notify(id, Status::StatusProgress, text, "", QNetworkReply::NoError);
}

void Notification::NotifySubMultiSuccess(const QString &id, const QString &text)
{
  Notify(id, Status::StatusSuccess, text, "", QNetworkReply::NoError);
}

void Notification::NotifySubMultiError(const QString &id, const QString &text, QNetworkReply::NetworkError error)
{
  Notify(id, Status::StatusError, text, "", error);
}

void Notification::NotifyMultiClearSuccess(const QString &id)
{
  Notify(id, Status::StatusClearSuccess, "", "", QNetworkReply::NoError);
}

void Notification::NotifyMultiClear(const QString &id)
{
  Notify(id, Status::StatusClear, "", "", QNetworkReply::NoError);
}

//// Unique property name
const char* Notification::GetUniquePropertyName() const
{
  return "id";
}

//// Parent access
Notification* Notification::GetParent() const
{
  return qobject_cast<Notification*>(parent());
}

//// Chhildren access
QList<Notification*> Notification::GetChildren() const
{
  QList<Notification*> childItems;
  const QObjectList& cn = children();
  childItems.reserve(cn.size());
  for (const auto& child : cn)
  {
    if (auto ci = qobject_cast<Notification*>(child))
      childItems.push_back(ci);
  }
  return childItems;
}

QList<Notification*> Notification::GetChildrenRecursive() const
{
  QList<Notification*> childItems = GetChildren();
  QList<Notification*> childItemsRecursive;
  for (int i = 0; i < childItems.size(); ++i)
  {
    childItemsRecursive.append(childItems[i]->GetChildrenRecursive());
  }
  childItems << childItemsRecursive;
  return childItems;
}

QList<Notification*> Notification::GetChildrenRecursiveOrdered() const
{
  QList<Notification*> childItems = GetChildrenRecursive();
  std::sort(childItems.begin(), childItems.end(), [](Notification* lt, Notification* rt) -> bool { return lt->Order < rt->Order; });
  return childItems;
}

QList<Notification*> Notification::GetChildrenRecursiveOrderedViewable() const
{
  QList<Notification*> childItems = GetChildrenRecursiveOrdered();
  QList<Notification*> viewable;
  for (auto c : childItems)
  {
    if (c->IsViewable())
    {
      viewable << c;
    }
  }
  return viewable;
}

Notification* Notification::GetChild(const QString& id)
{
  QStringList pathNodes = id.split("/");

  //// By value
  if (pathNodes.size())
  {
    if (pathNodes.size() == 1)
    {
      //// Existing
      QList<Notification*> children = GetChildren();
      QList<Notification*>::iterator c = children.begin();
      while (c != children.end())
      {
        if ((*c)->GetData(GetUniquePropertyName()).toString() == pathNodes.first())
          return *c;
        ++c;
      }

      //// Create
      return nullptr;
    }
    else
    {
      //// Existing
      QList<Notification*> children = GetChildren();
      QList<Notification*>::iterator c = children.begin();
      Notification* child = nullptr;
      while (c != children.end())
      {
        if ((*c)->GetData(GetUniquePropertyName()).toString() == pathNodes.first())
        {
          child = *c;
          break;
        }
        ++c;
      }

      if (!child)
      {
        return nullptr;
      }

      pathNodes.removeFirst();
      QString path = pathNodes.join("/");
      return child->GetChild(path);
    }
  }
  return nullptr;
}

Notification *Notification::CreateChild(const QString &id)
{
  QStringList pathNodes = id.split("/");

  //// By value
  if (pathNodes.size())
  {
    if (pathNodes.size() == 1)
    {
      //// Existing
      QList<Notification*> children = GetChildren();
      QList<Notification*>::iterator c = children.begin();
      while (c != children.end())
      {
        if ((*c)->GetData(GetUniquePropertyName()).toString() == pathNodes.first())
          return nullptr;
        ++c;
      }

      //// Create
      Notification* created = new Notification(this);
      created->Data[GetUniquePropertyName()] = pathNodes.first();
      return created;
    }
    else
    {
      //// Existing
      QList<Notification*> children = GetChildren();
      QList<Notification*>::iterator c = children.begin();
      Notification* child = nullptr;
      while (c != children.end())
      {
        if ((*c)->GetData(GetUniquePropertyName()).toString() == pathNodes.first())
        {
          child = *c;
          break;
        }
        ++c;
      }

      if (!child)
      {
        child = new Notification(this);
        child->Data[GetUniquePropertyName()] = pathNodes.first();
      }

      pathNodes.removeFirst();
      QString path = pathNodes.join("/");
      return child->CreateChild(path);
    }
  }
  return nullptr;
}

//// Clear
void Notification::Clear()
{
  if (IsEmpty())
    return;

  QList<Notification*> children = GetChildren();
  QList<Notification*>::iterator c = children.begin();
  while (c != children.end())
  {
    delete *c;
    ++c;
  }

  Data.clear();
  NotifyChanged();
}

void Notification::ClearSuccessOnly()
{
  if(GetStatus() == StatusSuccess)
  {
    Clear();
    return;
  }
  auto children = GetChildren();
  for(auto& child : children)
    child->ClearSuccessOnly();
}

//// Id
QString Notification::GetNodeId() const
{
  if (IsRoot())
    return "root";

  return Data[GetUniquePropertyName()].toString();
}

bool Notification::IsRoot() const
{
  return GetParent() == nullptr;
}

bool Notification::IsEmpty() const
{
  return findChild<Notification*>() == nullptr;
}

bool Notification::IsViewable() const
{
  switch (GetStatus())
  {
  case StatusViewChildren:
    return true;

  case StatusProgress:
  case StatusSuccess:
  case StatusInformation:
  case StatusWarning:
  case StatusError:
    {
      if (auto parent = GetParent())
      {
        if (parent->IsViewable())
        {
          return false;
        }
      }
      return true;
    }
    break;
  default:
    break;
  }
  return false;
}

//// Get slash separated path
QString Notification::GetPath() const
{
  Notification* parent = GetParent();
  QString path = GetNodeId();
  if (parent)
  {
    path.prepend("/");
    path.prepend(parent->GetPath());
  }
  return path;
}

Notification* Notification::FindChild(const QString& path) const
{
  QStringList pathNodes = path.split('/');
  if (pathNodes.empty())
    return nullptr;

  auto children = GetChildren();
  auto c = children.cbegin();
  while (c != children.cend())
  {
    if ((*c)->GetNodeId() == pathNodes.first())
    {
      if (pathNodes.size() == 1)
      {
        return *c;
      }
      else
      {
        pathNodes.removeFirst();
        return (*c)->FindChild(pathNodes.join('/'));
      }
    }
    ++c;
  }
  return nullptr;
}

Notification* Notification::FindChildAbs(const QString& path) const
{
  QString p = GetPath();
  if (path.startsWith(p))
  {
    QString childPath = path;
    childPath.remove(QString("%1/").arg(p));
    return FindChild(childPath);
  }
  return nullptr;
}

//// Data
QVariant Notification::GetData(const QString& key) const
{
  return Data.value(key);
}

void Notification::SetData(const QString& key, const QVariant& data) 
{
  if (Data[key] != data)
  {
    Data[key] = data;
    NotifyChanged();
  }
}

Notification::Status Notification::GetStatus() const
{
  return (Notification::Status)GetData("status").toInt();
}

void Notification::SetStatus(Status status) 
{
  if (GetStatus() != status)
  {
    SetData("status", status);
  }
}

QString Notification::GetText() const
{
  return GetData("text").toString();
}

void Notification::SetText(const QString& text)
{
  if (GetText() != text)
  {
    SetData("text", text);
  }
}

QString Notification::GetHeader() const
{
  return GetData("header").toString();
}

void Notification::SetHeader(const QString &header)
{
  if(GetHeader() != header)
  {
    SetData("header", header);
  }
}

void Notification::SetNotification(Status status, const QString& text, const QString& header, QNetworkReply::NetworkError error)
{
  if (status == StatusDelete)
  {
    Notification* parent = GetParent();
    delete this;
    if (parent)
    {
      parent->NotifyChanged();
    }
  }
  else if (status == StatusClear)
  {
    Clear();
  }
  else if (status == StatusClearSuccess)
  {
    ClearSuccessOnly();
  }
  else
  {
    SetData(status, text, header, error);
  }
}

void Notification::SetError(QNetworkReply::NetworkError networkError)
{
  {
    QSignalBlocker sb(this);
    SetData("error_type",    "network");
    SetData("network_error", networkError);
  }
  NotifyChanged();
}

void Notification::StartDestroyTimer()
{
  if(destroyTimer)
  {
    destroyTimer->stop();
    destroyTimer->deleteLater();
    destroyTimer = nullptr;
  }
  auto delayMs = GetDecoyDelay();
  if(delayMs > 0)
  {
    destroyTimer = new QTimer(this);
    destroyTimer->setSingleShot(true);
    destroyTimer->setInterval(delayMs);
    connect(destroyTimer, &QTimer::timeout, this, [this]()
    {
      emit Close();
    }, Qt::QueuedConnection);
    destroyTimer->start();
  }
}

void Notification::StopDestroyTimer()
{
  if(destroyTimer)
    destroyTimer->stop();
}

int Notification::DestroyDelay()
{
  return GetDestroyDelayMs(GetStatus());
}

void Notification::SetData(Status status, const QString &text, const QString &header, QNetworkReply::NetworkError error)
{
  Data["status"] = status;
  Data["text"] = text;
  Data["header"] = header;
  Data["error_type"] = "network";
  Data["network_error"] = error;
  NotifyChanged();
}

int Notification::GetDecoyDelay()
{
  return 5000;
}

void Notification::NotifyChanged()
{
  emit Changed();
  if (auto parent = GetParent())
  {
    parent->NotifyChanged();
  }
}

int Notification::GetDestroyDelayMs(Status status)
{
  switch(status)
  {
  case StatusNone:
  case StatusDelete:
  case StatusClear:
  case StatusClearSuccess:
  case StatusViewChildren:
  case StatusWarning:
  case StatusLast:
    return -1;
  case StatusError:
    return 20000;
  case StatusSuccess:
  case StatusHeaderlessInformation:
    return 5000;
  case StatusProgress:
  case StatusInformation:
    return 10000;
  default:
    return -1;
  }
}

void Notification::Notify(const QString &id, Status status, const QString &text, const QString &header, QNetworkReply::NetworkError error)
{
  auto instance = NotificationManager::Get()->GetNotification(id);
  if(!instance)
  {
    switch(status)
    {
    case StatusNone:
    case StatusDelete:
    case StatusClear:
    case StatusClearSuccess:
    case StatusLast:
      return;
    default:
      break;
    }

    NotificationManager::Get()->AddNotification(id, status, text, header, error);
    return;
  }
  else
  {
    instance->SetNotification(status, text, header, error);
  }
}

///////////////////////////////////////////////////////////////////
//// Notification Manager
NotificationManager::NotificationManager(QObject* parent)
  : QObject(parent)
  , Root(nullptr)
{
  Details::NotificationManager_Instance__ = this;
}

NotificationManager::~NotificationManager()
{
}

//// Get
NotificationManager* NotificationManager::Get()
{
  if (!Details::NotificationManager_Instance__)
  {
    Details::NotificationManager_Instance__ = new NotificationManager(QApplication::instance());
  }
  return Details::NotificationManager_Instance__;
}

Notification* NotificationManager::GetNotification(const QString& path)
{
  return Root.GetChild(path);
}

void NotificationManager::AddNotification(const QString& id, Notification::Status status, const QString& text, const QString& header, QNetworkReply::NetworkError error)
{
  auto n = Root.CreateChild(id);
  if(!n)
    return;
  n->SetData(status, text, header, error);
  bool isSubNotification = false;
  auto parent = n->GetParent();
  while(parent)
  {
    if(parent->GetStatus() == Notification::StatusViewChildren)
    {
      isSubNotification = true;
      break;
    }
    parent = parent->GetParent();
  }
  if(!isSubNotification)
    emit notificationAdded(n);
}

Notification* NotificationManager::GetRoot()
{
  return &Root;
}
