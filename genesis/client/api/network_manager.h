#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QTimer>

#include "api_rest.h"

//use this define to enable https
//#define HTTPS_ENABLED

#define JWT_AUTH_CODE 401
#define JWT_REFRESH_CODE 403
#define JWT_REFRESH_ENDPOINT "/refresh"

////////////////////////////////////////////////////
//// Debug NetworkAccessManager implimentation for
//// catching output requests payload

namespace NetworkManagerDetails
{
  QVariantMap TransformTable(const QVariantMap& raw);
}

class DebugNetworkAccessManager : public QNetworkAccessManager
{
  Q_OBJECT
public:
  virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest& request, QIODevice* outgoingData = 0)
  {
    if (isCatching)
    {
      QSharedPointer<QByteArray> baTest(new QByteArray("NoData"));
      if (outgoingData != 0)
      {
        *baTest = outgoingData->peek(outgoingData->bytesAvailable());
      }
      emit dataGot(baTest);
    }

    return QNetworkAccessManager::createRequest(op, request, outgoingData);;
  }

  void startCatching()
  {
    isCatching = true;
  }
  void stopCatching()
  {
    isCatching = false;
  }
signals:
  void dataGot(QSharedPointer<QByteArray> data);

private:
  bool isCatching = false;
};

////////////////////////////////////////////////////
//// Network Manager
class NetworkManager : public QObject
{
  Q_OBJECT
protected:
  static NetworkManager* _Instance;
  NetworkManager();
  ~NetworkManager();

  void ConnectReply(QNetworkReply* reply
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error);
  void ConnectReply(QNetworkReply* reply
    , API::REST::ReplyHandleFuncTable handle
    , API::REST::ReplyErrorFunc error);

  void ConnectReply(QNetworkReply* reply
    , API::REST::ReplyHandleByteArray handle
    , API::REST::ReplyErrorFunc error, bool noRefreshIfTokenExpired = false);

  void ProcessQueued();
  void BlockQueue(){QueueBlocked = true;};
  void ForceClearQueue();
  void UnblockQueue(){QueueBlocked = false; ProcessQueued();};

public:
  //// Server address
  void SetHost(const QString& url);
  void SetKeyCloak(const QString& url, const QString& client);

  //// Authorize
  void Authorize(const QString& username, const QString& password
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error);

  void Authorize(QString url, const QString& username, const QString& password
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error);

  //// Get instance
  static NetworkManager* Get();

  void FailRequest(API::REST::ReplyErrorFunc error);

  QNetworkReply* GetRequest(const QString& url
    , const QUrlQuery& params
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection
    , API::REST::ReplyUploadProgressFunc progressHandle = nullptr);

  //// Get / table result
  QNetworkReply* GetRequest(const QString& url
    , const QUrlQuery& params
    , API::REST::ReplyHandleFuncTable handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection
    , API::REST::ReplyUploadProgressFunc progressHandle = nullptr);

  //// Get / file result
  QNetworkReply* GetRequest(const QString& url
    , const QUrlQuery& params
    , API::REST::ReplyHandleByteArray handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection
    , API::REST::ReplyUploadProgressFunc progressHandle = nullptr);

  //// Post / url
  QNetworkReply* PostRequest(const QString& url
    , const QUrlQuery& params
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection
    , API::REST::ReplyUploadProgressFunc progressHandle = nullptr);

  //// Post / data
  QNetworkReply* PostRequest(const QString& url
    , const QJsonObject& data
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection
    , API::REST::ReplyUploadProgressFunc progressHandle = nullptr);
  //// Post / data
  QNetworkReply* PostRequest(const QString& url
    , const QJsonArray& data
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection);

  //// Post / data / table result
  QNetworkReply* PostRequest(const QString& url
    , const QJsonObject& data
    , API::REST::ReplyHandleFuncTable handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection);

  //// Post / data / qbyte array
  QNetworkReply* PostRequest(const QString& url
    , const QJsonObject& data
    , API::REST::ReplyHandleByteArray handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection);

  //// Post / requestParts & raw file requestParts
  QNetworkReply* PostRequest(const QString& url
                             , const QVariantMap &requestParts
                             , const QByteArray& file
                             , API::REST::ReplyHandleFunc handle
                             , API::REST::ReplyErrorFunc error
                             , QVariant tag = QVariant()
                             , Qt::ConnectionType connectionType = Qt::AutoConnection
                             , API::REST::ReplyUploadProgressFunc progressHandle = nullptr);

  //// Put / url
  QNetworkReply* PutRequest(const QString& url
    , const QUrlQuery& params
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection);

  //// Put / data
  QNetworkReply* PutRequest(const QString& url
    , const QJsonObject& data
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection);

  //// Put / data / table result
  QNetworkReply* PutRequest(const QString& url
    , const QJsonObject& data
    , API::REST::ReplyHandleFuncTable handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection);

  //// Delete / url
  QNetworkReply* DeleteRequest(const QString& url
    , const QUrlQuery& params
    , API::REST::ReplyHandleFunc handle
    , API::REST::ReplyErrorFunc error
    , QVariant tag = QVariant()
    , Qt::ConnectionType connectionType = Qt::AutoConnection);

public slots:
  void StopTimers();

signals:
  void reloginRequired();

private:
  QNetworkRequest MakeRequest(QString url);
  QNetworkRequest MakeRequest(QString url, const QUrlQuery& params);
  QNetworkRequest MakeRequest(QString url, const QString params);
  void CatchOutgoingData(QSharedPointer<QByteArray> data);

#ifdef JWT_ENABLED
  void SendJwtRefreshRequest(QString refreshEndpointUrl, API::REST::ReplyHandleFunc handle, API::REST::ReplyErrorFunc error);
#endif
#ifdef KEY_CLOACK
  void KCRefresh();
  void KCRefresh(std::function<void(bool error)> refreshCallback);
  void KCAuthorize(QString username, QString password, std::function<void (bool, QNetworkReply *)> authCallback);
#endif


protected:

#ifdef JWT_ENABLED
  //// JWT
  QString JWtoken;
  QString JwtRefreshToken;
#endif
#ifdef KEY_CLOACK
  QString KCToken;
  QString KCRefreshToken;
  QTimer* getAutoRefreshTimer();
  QTimer* getAccessTokenTimer();

private:
  QTimer* KCAutoRefreshTimerPtr;
  QTimer* KCAccessTokenTimerPtr;
  bool AccessTokenValid;
#endif

  bool QueueBlocked = false;

  //// Server address
  QString App_Url;

  QString KeycloakUrl;
  QString client_id;

  //// Qt connectivity
//  QScopedPointer<QNetworkAccessManager> Manager; //normal access manager
  QScopedPointer<DebugNetworkAccessManager> Manager; //access manager with outgoing payload catching

  //// Default strategy
  Qt::ConnectionType ConnectionType;

  //// Queue
  enum RequestFields
  {
    RequestFieldType,
    RequestFieldUrl,
    RequestFieldQuery,
    RequestFieldData,
    RequestFieldVariantMap,
    RequestFieldDataArray,
    RequestFieldBlob,
    RequestFieldHandle,
    RequestFieldHandleTable,
    RequestFieldHandleByteArray,
    RequestFieldError,
    RequestFieldProgressHandle,
    RequestFieldReply,
    RequestFieldTag
  };

  //// Request type
  enum RequestType
  {
    RequestTypeGet,
    RequestTypeGetTable,
    RequestTypePost,
    RequestTypePostData,
    RequestTypePostDataArray,
    RequestTypePostDataTable,
    RequestTypePostDataAndFile,
    RequestTypePut,
    RequestTypePutData,
    RequestTypePutDataTable,
    RequestTypeDelete
  };

  //// Quued request
  typedef std::tuple<RequestType,
    QString,
    QUrlQuery,
    QJsonObject,
    QVariantMap,
    QJsonArray,
    QByteArray,
    API::REST::ReplyHandleFunc,
    API::REST::ReplyHandleFuncTable,
    API::REST::ReplyHandleByteArray,
    API::REST::ReplyErrorFunc,
    API::REST::ReplyUploadProgressFunc,
    QNetworkReply*,
    QVariant> QueuedRequest;

  //// Queue of requests
  QList<QSharedPointer<QueuedRequest>> Queue;
};
