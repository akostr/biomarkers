#pragma once

#include <QObject>
#include <QVariant>
#include <QPointer>
#include <QTimer>
#include <QNetworkReply>

///////////////////////////////////////////////////////////////////
//// Notification data
class Notification : public QObject
{
  Q_OBJECT

public:  
  //// Status
  enum Status
  {
    StatusNone,             //// Status not set

    StatusDelete,           //// Shall delete notification
    StatusClear,            //// Shall clear notification status, text & children
    StatusClearSuccess,     //// Shall clear notifications only with status "success", text & children
    StatusViewChildren,     //// Single notification view shall be displayed for this notification with its children status listed within
    StatusProgress,         //// When notification in progress
    
    StatusSuccess,          //// Simple status / success
    StatusInformation,      //// Simple status / information
    StatusHeaderlessInformation,//// Simple status / information
    StatusWarning,          //// Simple status / warning
    StatusError,            //// Simple status / error

    StatusLast
  };

public:
  Notification(Notification* parent);
  ~Notification();

  ///////////////////////////////////////////////////////////////////
  //// Static apis
  static void NotifyHeaderless(const QString& text);
  static void NotifyInfo(const QString& text, const QString& header = QString());
  static void NotifyError(const QString& text, const QString& header = QString(), QNetworkReply::NetworkError error = QNetworkReply::NetworkError::NoError);
  static void NotifyError(const QString& text, QNetworkReply::NetworkError error);
  static void NotifyWarning(const QString& text, const QString& header = QString());
  static void NotifySuccess(const QString& text, const QString& header = QString());
  static void NotifyMulti(const QString& id, const QString& text);
  static void NotifySubMultiProgress(const QString& id, const QString& text);
  static void NotifySubMultiSuccess(const QString& id, const QString& text);
  static void NotifySubMultiError(const QString& id, const QString& text, QNetworkReply::NetworkError error = QNetworkReply::NetworkError::NoError);
  static void NotifyMultiClearSuccess(const QString& id);
  static void NotifyMultiClear(const QString& id);
  //// static void Notify(const QString& id, Status status, const QString& text, const QSqlError& error);

  //// Unique property name
  const char* GetUniquePropertyName() const;

  //// Parent access
  Notification*         GetParent() const;

  //// Chhildren access
  QList<Notification*>  GetChildren() const;
  QList<Notification*>  GetChildrenRecursive() const;
  QList<Notification*>  GetChildrenRecursiveOrdered() const;
  QList<Notification*>  GetChildrenRecursiveOrderedViewable() const;
  Notification*         GetChild(const QString& id);
  Notification*         CreateChild(const QString& id);

  //// Clear
  void Clear();
  void ClearSuccessOnly();

  //// Id
  QString   GetNodeId() const;
  bool      IsRoot() const;
  bool      IsEmpty() const;
  bool      IsViewable() const;

  //// Get slash separated path
  QString   GetPath() const;

  //// Chhildren access by paths
  Notification* FindChild(const QString& path) const;
  Notification* FindChildAbs(const QString& path) const;

  //// Data
  QVariantMap GetData() const { return Data; }
  QVariant  GetData(const QString& key) const;
  void      SetData(const QString& key, const QVariant& data);

  Status    GetStatus() const;
  void      SetStatus(Status status);

  QString   GetText() const;
  void      SetText(const QString& text);

  QString   GetHeader() const;
  void      SetHeader(const QString& header);

  void      SetNotification(Status status, const QString& text, const QString &header, QNetworkReply::NetworkError error);
  void      SetError(QNetworkReply::NetworkError networkError);

  void      StartDestroyTimer();
  void      StopDestroyTimer();
  int       DestroyDelay();

  void      SetData(Status status, const QString& text, const QString &header, QNetworkReply::NetworkError error);
  int       GetDecoyDelay();

signals:
  void Changed();
  void Close();

private:
  void NotifyChanged();
  int GetDestroyDelayMs(Status status);
  static void Notify(const QString& id, Status status, const QString& text, const QString& header, QNetworkReply::NetworkError error);

private:
  QPointer<QTimer> destroyTimer;

protected:
  QVariantMap         Data;
  unsigned long long  Order;
};

///////////////////////////////////////////////////////////////////
//// Notification Manager
class NotificationManager : public QObject
{
  Q_OBJECT

public:
  NotificationManager(QObject* parent = nullptr);
  ~NotificationManager();
  
  //// Get
  static NotificationManager* Get();

  //// Get notification
  Notification* GetNotification(const QString& path);
  void AddNotification(const QString& id, Notification::Status status, const QString& text, const QString& header, QNetworkReply::NetworkError error);

  //// Get root
  Notification* GetRoot();

signals:
  void notificationAdded(Notification* notification);

private:
  //// Notifications tree
  Notification Root;
};
