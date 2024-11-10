#include "network_manager.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QFile>
#include <QTimer>
#include <QSslConfiguration>

NetworkManager* NetworkManager::_Instance = nullptr;

namespace NetworkManagerDetails
{
  static QMap<int, QString> PSQLTypes =
  {
    { 16,  "bool" },
    { 17,  "bytea" },
    { 18,  "char" },
    { 19,  "name" },
    { 20,  "int8" },
    { 21,  "int2" },
    { 22,  "int2vector" },
    { 23,  "int4" },
    { 24,  "regproc" },
    { 25,  "text" },
    { 26,  "oid" },
    { 27,  "tid" },
    { 28,  "xid" },
    { 29,  "cid" },
    { 30,  "oidvector" },
    { 32,  "pg_ddl_command" },
    { 71,  "pg_type" },
    { 75,  "pg_attribute" },
    { 81,  "pg_proc" },
    { 83,  "pg_class" },
    { 114,  "json" },
    { 142,  "xml" },
    { 143,  "_xml" },
    { 194,  "pg_node_tree" },
    { 199,  "_json" },
    { 269,  "table_am_handler" },
    { 325,  "index_am_handler" },
    { 600,  "point" },
    { 601,  "lseg" },
    { 602,  "path" },
    { 603,  "box" },
    { 604,  "polygon" },
    { 628,  "line" },
    { 629,  "_line" },
    { 650,  "cidr" },
    { 651,  "_cidr" },
    { 700,  "float4" },
    { 701,  "float8" },
    { 705,  "unknown" },
    { 718,  "circle" },
    { 719,  "_circle" },
    { 774,  "macaddr8" },
    { 775,  "_macaddr8" },
    { 790,  "money" },
    { 791,  "_money" },
    { 829,  "macaddr" },
    { 869,  "inet" },
    { 1000,  "_bool" },
    { 1001,  "_bytea" },
    { 1002,  "_char" },
    { 1003,  "_name" },
    { 1005,  "_int2" },
    { 1006,  "_int2vector" },
    { 1007,  "_int4" },
    { 1008,  "_regproc" },
    { 1009,  "_text" },
    { 1010,  "_tid" },
    { 1011,  "_xid" },
    { 1012,  "_cid" },
    { 1013,  "_oidvector" },
    { 1014,  "_bpchar" },
    { 1015,  "_varchar" },
    { 1016,  "_int8" },
    { 1017,  "_point" },
    { 1018,  "_lseg" },
    { 1019,  "_path" },
    { 1020,  "_box" },
    { 1021,  "_float4" },
    { 1022,  "_float8" },
    { 1027,  "_polygon" },
    { 1028,  "_oid" },
    { 1033,  "aclitem" },
    { 1034,  "_aclitem" },
    { 1040,  "_macaddr" },
    { 1041,  "_inet" },
    { 1042,  "bpchar" },
    { 1043,  "varchar" },
    { 1082,  "date" },
    { 1083,  "time" },
    { 1114,  "timestamp" },
    { 1115,  "_timestamp" },
    { 1182,  "_date" },
    { 1183,  "_time" },
    { 1184,  "timestamptz" },
    { 1185,  "_timestamptz" },
    { 1186,  "interval" },
    { 1187,  "_interval" },
    { 1231,  "_numeric" },
    { 1248,  "pg_database" },
    { 1263,  "_cstring" },
    { 1266,  "timetz" },
    { 1270,  "_timetz" },
    { 1560,  "bit" },
    { 1561,  "_bit" },
    { 1562,  "varbit" },
    { 1563,  "_varbit" },
    { 1700,  "numeric" },
    { 1790,  "refcursor" },
    { 2201,  "_refcursor" },
    { 2202,  "regprocedure" },
    { 2203,  "regoper" },
    { 2204,  "regoperator" },
    { 2205,  "regclass" },
    { 2206,  "regtype" },
    { 2207,  "_regprocedure" },
    { 2208,  "_regoper" },
    { 2209,  "_regoperator" },
    { 2210,  "_regclass" },
    { 2211,  "_regtype" },
    { 2249,  "record" },
    { 2275,  "cstring" },
    { 2276,  "any" },
    { 2277,  "anyarray" },
    { 2278,  "void" },
    { 2279,  "trigger" },
    { 2280,  "language_handler" },
    { 2281,  "internal" },
    { 2282,  "opaque" },
    { 2283,  "anyelement" },
    { 2287,  "_record" },
    { 2776,  "anynonarray" },
    { 2842,  "pg_authid" },
    { 2843,  "pg_auth_members" },
    { 2949,  "_txid_snapshot" },
    { 2950,  "uuid" },
    { 2951,  "_uuid" },
    { 2970,  "txid_snapshot" },
    { 3115,  "fdw_handler" },
    { 3220,  "pg_lsn" },
    { 3221,  "_pg_lsn" },
    { 3310,  "tsm_handler" },
    { 3361,  "pg_ndistinct" },
    { 3402,  "pg_dependencies" },
    { 3500,  "anyenum" },
    { 3614,  "tsvector" },
    { 3615,  "tsquery" },
    { 3642,  "gtsvector" },
    { 3643,  "_tsvector" },
    { 3644,  "_gtsvector" },
    { 3645,  "_tsquery" },
    { 3734,  "regconfig" },
    { 3735,  "_regconfig" },
    { 3769,  "regdictionary" },
    { 3770,  "_regdictionary" },
    { 3802,  "jsonb" },
    { 3807,  "_jsonb" },
    { 3831,  "anyrange" },
    { 3838,  "event_trigger" },
    { 3904,  "int4range" },
    { 3905,  "_int4range" },
    { 3906,  "numrange" },
    { 3907,  "_numrange" },
    { 3908,  "tsrange" },
    { 3909,  "_tsrange" },
    { 3910,  "tstzrange" },
    { 3911,  "_tstzrange" },
    { 3912,  "daterange" },
    { 3913,  "_daterange" },
    { 3926,  "int8range" },
    { 3927,  "_int8range" },
    { 4066,  "pg_shseclabel" },
    { 4072,  "jsonpath" },
    { 4073,  "_jsonpath" },
    { 4089,  "regnamespace" },
    { 4090,  "_regnamespace" },
    { 4096,  "regrole" },
    { 4097,  "_regrole" },
    { 5017,  "pg_mcv_list" },
    { 6101,  "pg_subscription" },
    { 12000,  "pg_attrdef" },
    { 12001,  "pg_constraint" },
    { 12002,  "pg_inherits" },
    { 12003,  "pg_index" },
    { 12004,  "pg_operator" },
    { 12005,  "pg_opfamily" },
    { 12006,  "pg_opclass" },
    { 12007,  "pg_am" },
    { 12008,  "pg_amop" },
    { 12009,  "pg_amproc" },
    { 12010,  "pg_language" },
    { 12011,  "pg_largeobject_metadata" },
    { 12012,  "pg_largeobject" },
    { 12013,  "pg_aggregate" },
    { 12014,  "pg_statistic_ext" },
    { 12015,  "pg_statistic_ext_data" },
    { 12016,  "pg_statistic" },
    { 12017,  "pg_rewrite" },
    { 12018,  "pg_trigger" },
    { 12019,  "pg_event_trigger" },
    { 12020,  "pg_description" },
    { 12021,  "pg_cast" },
    { 12022,  "pg_enum" },
    { 12023,  "pg_namespace" },
    { 12024,  "pg_conversion" },
    { 12025,  "pg_depend" },
    { 12026,  "pg_db_role_setting" },
    { 12027,  "pg_tablespace" },
    { 12028,  "pg_pltemplate" },
    { 12029,  "pg_shdepend" },
    { 12030,  "pg_shdescription" },
  };

  enum PSQLTypesSupportedOIDs
  {
    QBOOLOID = 16,
    QINT8OID = 20,
    QINT2OID = 21,
    QINT4OID = 23,
    QNUMERICOID = 1700,
    QFLOAT4OID = 700,
    QFLOAT8OID = 701,
    QABSTIMEOID = 702,
    QRELTIMEOID = 703,
    QDATEOID = 1082,
    QTIMEOID = 1083,
    QTIMETZOID = 1266,
    QTIMESTAMPOID = 1114,
    QTIMESTAMPTZOID = 1184,
    QOIDOID = 2278,
    QBYTEAOID = 17,
    QREGPROCOID = 24,
    QXIDOID = 28,
    QCIDOID = 29,
    QBITOID = 1560,
    QVARBITOID = 1562,
  };

  QMetaType::Type TransformTableVariantType(const QString& typeName)
  {
    //// Map
    static QMap<QString, int> PSQLTypeOids;
    if (PSQLTypeOids.empty())
    {
      for (auto t = PSQLTypes.begin(); t != PSQLTypes.end(); t++)
      {
        PSQLTypeOids[t.value()] = t.key();
      }
    }

    //// Resolve
    QMetaType::Type type = QMetaType::UnknownType;
    switch (PSQLTypeOids.value(typeName, 0))
    {
    case QBOOLOID:
      type = QMetaType::Bool;
      break;
    case QINT8OID:
      type = QMetaType::LongLong;
      break;
    case QINT2OID:
    case QINT4OID:
    case QOIDOID:
    case QREGPROCOID:
    case QXIDOID:
    case QCIDOID:
      type = QMetaType::Int;
      break;
    case QNUMERICOID:
    case QFLOAT4OID:
    case QFLOAT8OID:
      type = QMetaType::Double;
      break;
    case QABSTIMEOID:
    case QRELTIMEOID:
    case QDATEOID:
      type = QMetaType::QDate;
      break;
    case QTIMEOID:
    case QTIMETZOID:
      type = QMetaType::QTime;
      break;
    case QTIMESTAMPOID:
    case QTIMESTAMPTZOID:
      type = QMetaType::QDateTime;
      break;
    case QBYTEAOID:
      type = QMetaType::QByteArray;
      break;
    case 0:
      type = QMetaType::UnknownType;
      break;
    default:
      type = QMetaType::QString;
      break;
    }
    return type;
  }

  QVariant TransformTableVariant(const QString& typeName, const QVariant& raw)
  {
    QMetaType::Type type = TransformTableVariantType(typeName);
    switch (type)
    {
    case QMetaType::QDate:
      return QVariant(QDate::fromString(raw.toString(), Qt::ISODate));
    case QMetaType::QTime:
      return QVariant(QTime::fromString(raw.toString(), Qt::ISODate));
    case QMetaType::QDateTime:
      return QVariant(QDateTime::fromString(raw.toString(),
        Qt::ISODate).toLocalTime());
    case QMetaType::QByteArray:
      return QByteArray::fromStdString(raw.toString().toStdString());
    case QMetaType::QJsonArray:
        return raw;
    default:
      break;
    }
    return raw;
  }

  QVariantMap TransformTable(const QVariantMap& raw)
  {
    QVariantMap result;
    if (raw.contains("columns"))
    {
      //// Raw columns
      QVariantList columns = raw["columns"].toList();
      if (raw.contains("data"))
      {
        //// Raw data
        QVariantList data = raw["data"].toList();

        QVariantList children;
        for (auto d = data.begin(); d != data.end(); d++)
        {
          QVariantMap child;
          QVariantList row = d->toList();
          for (auto r = row.begin(); r != row.end(); r++)
          {
            int index = std::distance(row.begin(), r);
            if (index < columns.size())
            {
              QVariantMap mp = columns[index].toMap();
              QString name = mp["name"].toString();
              QString type;
              if (mp.contains("type"))
                type = mp["type"].toString();
              else if (mp.contains("typename"))
                type = mp["typename"].toString();
              else if (mp.contains("typname"))
                type = mp["typname"].toString();

              QVariant value = TransformTableVariant(type, *r);
              child[name] = value;
            }
          }
          children.push_back(child);
        }
        result["children"] = children;
      }
    }
    return result;
  }
}

NetworkManager::NetworkManager()
//  : Manager(new QNetworkAccessManager()) //normal
  :
#ifdef KEY_CLOACK
    KCAutoRefreshTimerPtr(nullptr),
    KCAccessTokenTimerPtr(nullptr),
#endif
    Manager(new DebugNetworkAccessManager())//debug
  , ConnectionType(Qt::DirectConnection)
{

#ifndef PLATFORM_WASM

    auto ssl_conf = QSslConfiguration::defaultConfiguration();
    ssl_conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(ssl_conf);
    qDebug() <<"OpenSSL status:" << QSslSocket::supportsSsl() << QSslSocket::sslLibraryVersionString();
#endif
#ifdef DEBUG
  connect(Manager.get(), &DebugNetworkAccessManager::dataGot, this, &NetworkManager::CatchOutgoingData);
#endif
}

NetworkManager::~NetworkManager()
{
}

void NetworkManager::ConnectReply(QNetworkReply* reply
  , API::REST::ReplyHandleFunc handle
  , API::REST::ReplyErrorFunc error)
{
  ConnectReply(reply, [handle](QNetworkReply* reply, QByteArray data)
    {
      if (handle)
      {
        handle(reply, QJsonDocument::fromJson(data));
      }
    }, error);
}

void NetworkManager::ConnectReply(QNetworkReply* reply
  , API::REST::ReplyHandleFuncTable handle
  , API::REST::ReplyErrorFunc error)
{
  ConnectReply(reply, [handle](QNetworkReply* reply, QByteArray data)
    {
      if (handle)
      {
        const auto vm = QJsonDocument::fromJson(data).object().toVariantMap();
        handle(reply, NetworkManagerDetails::TransformTable(vm));
      }
    }, error);
}

void NetworkManager::ConnectReply(QNetworkReply* reply, API::REST::ReplyHandleByteArray handle, API::REST::ReplyErrorFunc error, bool noRefreshIfTokenExpired)
{
  auto onFinished = [this, reply, handle, error, noRefreshIfTokenExpired]()
  {
    if (reply->error() == QNetworkReply::NoError)
    {
      if (handle)
      {
        handle(reply, reply->readAll());
      }
    }
    else
    {
#ifdef KEY_CLOACK
      //if we catch the session expired error, we call refresh, and recall request after success.
      //if error occured while refresh, this error will be send as error instead "session expired" error.
      if(!noRefreshIfTokenExpired && (reply->error() == QNetworkReply::ServiceUnavailableError
                                      || reply->error() == QNetworkReply::AuthenticationRequiredError
                                      || reply->error() == QNetworkReply::InternalServerError))
      {
        KCRefresh([this, reply, handle, error](bool refreshError)
        {
          if(refreshError)
          {
            if (error)
            {
              if(KCAutoRefreshTimerPtr)
                KCAutoRefreshTimerPtr->stop();
              if(KCAccessTokenTimerPtr)
                KCAccessTokenTimerPtr->stop();
              AccessTokenValid = false;

              emit reloginRequired();
              error(reply, reply->error());
            }
          }
          else
          {
            if(reply)
            {
              qDebug()<<"Network error: "<<(QNetworkReply::NetworkError)reply->error();
              bool repeat = false;
              QNetworkReply* newReply;
              QNetworkRequest newRequest;

              switch ((QNetworkAccessManager::Operation)reply->operation())
              {
                case QNetworkAccessManager::GetOperation:
                  newRequest = MakeRequest(reply->url().url(), reply->url().query());
                  newRequest.setHeader(QNetworkRequest::ContentTypeHeader, reply->header(QNetworkRequest::ContentTypeHeader));
                  newReply = Manager->get(newRequest);
                  repeat = true;
                  break;

                case QNetworkAccessManager::DeleteOperation:
                  newRequest = MakeRequest(reply->url().url(), reply->url().query());
                  newRequest.setHeader(QNetworkRequest::ContentTypeHeader, reply->header(QNetworkRequest::ContentTypeHeader));
                  newReply = Manager->deleteResource(newRequest);
                  repeat = true;
                  break;

                case QNetworkAccessManager::PutOperation:
                case QNetworkAccessManager::PostOperation:
                case QNetworkAccessManager::HeadOperation:
                case QNetworkAccessManager::CustomOperation:
                case QNetworkAccessManager::UnknownOperation:
                  error(reply, reply->error());
                  break;
              }
              if(repeat)
              {
                ConnectReply(newReply, handle, error, true);
              }
            }
          }

        });
      }
      else
#endif
      {

        qDebug() << "Network error:" << reply->errorString();
        if (error)
          error(reply, reply->error());
      }
    }
//    reply->deleteLater();

    //// Remove from queue
    QList<QSharedPointer<QueuedRequest>>::iterator recieved = std::find_if(
      Queue.begin(),
      Queue.end(),
      [reply](QSharedPointer<QueuedRequest> request) -> bool { return std::get<RequestFieldReply>(*request) == reply; });

    //// Was queued?
    if (recieved != Queue.end())
    {
      //// Done
      Queue.removeOne(*recieved);

      //// Proceed
      ProcessQueued();
    }
  };
  QObject::connect(reply, &QNetworkReply::finished, onFinished);
}

void NetworkManager::ProcessQueued()
{
  if(QueueBlocked)
       return;
  if (!Queue.empty())
  {
    QList<QSharedPointer<QueuedRequest>>::iterator sent = std::find_if(
      Queue.begin(),
      Queue.end(),
      [](QSharedPointer<QueuedRequest> request) -> bool { return std::get<RequestFieldReply>(*request); });

    QList<QSharedPointer<QueuedRequest>>::iterator unsent = std::find_if(
      Queue.begin(),
      Queue.end(),
      [](QSharedPointer<QueuedRequest> request) -> bool { return !std::get<RequestFieldReply>(*request); });

    if (sent == Queue.end()
      && unsent != Queue.end())
    {
      //// Send
      QNetworkReply* reply = nullptr;
      switch (std::get<RequestFieldType>(**unsent))
      {
      case RequestTypeGet:
        reply = GetRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldQuery>(**unsent),
          std::get<RequestFieldHandle>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      case RequestTypeGetTable:
        reply = GetRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldQuery>(**unsent),
          std::get<RequestFieldHandleTable>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      case RequestTypePost:
        reply = PostRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldQuery>(**unsent),
          std::get<RequestFieldHandle>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      case RequestTypePostData:
        reply = PostRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldData>(**unsent),
          std::get<RequestFieldHandle>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      case RequestTypePostDataArray:
        reply = PostRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldDataArray>(**unsent),
          std::get<RequestFieldHandle>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      case RequestTypePostDataTable:
        reply = PostRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldData>(**unsent),
          std::get<RequestFieldHandleTable>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      case RequestTypePostDataAndFile:
        reply = PostRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldVariantMap>(**unsent),
          std::get<RequestFieldBlob>(**unsent),
          std::get<RequestFieldHandle>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection,
          std::get<RequestFieldProgressHandle>(**unsent));
        break;
      case RequestTypePut:
        reply = PutRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldQuery>(**unsent),
          std::get<RequestFieldHandle>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      case RequestTypePutData:
        reply = PutRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldData>(**unsent),
          std::get<RequestFieldHandle>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      case RequestTypePutDataTable:
        reply = PutRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldData>(**unsent),
          std::get<RequestFieldHandleTable>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      case RequestTypeDelete:
        reply = DeleteRequest(
          std::get<RequestFieldUrl>(**unsent),
          std::get<RequestFieldQuery>(**unsent),
          std::get<RequestFieldHandle>(**unsent),
          std::get<RequestFieldError>(**unsent),
          std::get<RequestFieldTag>(**unsent),
          Qt::DirectConnection);
        break;
      default:
        break;
      }

      //// Mark
      std::get<RequestFieldReply>(**unsent) = reply;
    }
  }
}

void NetworkManager::ForceClearQueue()
{
  while (!Queue.empty())
  {
    auto request = Queue.takeFirst();
    FailRequest(std::get<RequestFieldError>(*request));
  }
}


void NetworkManager::SetHost(const QString &url)
{
  App_Url = url;
}


void NetworkManager::SetKeyCloak(const QString &url, const QString &client)
{
  KeycloakUrl = url;
  client_id = client;
}

void NetworkManager::Authorize(const QString& username, const QString& password
  , API::REST::ReplyHandleFunc handle
  , API::REST::ReplyErrorFunc error)
{
  ////UNUSED NOW
  /*
  Manager->connectToHost(Host, Port.toUInt());

  QString concatenated = username + ":" + password;
  QByteArray data = concatenated.toLocal8Bit().toBase64();
  QString headerData = "Basic " + data;
  QNetworkRequest request = QNetworkRequest(QUrl(QString("http://%1:%2").arg(Host, Port)));
  request.setRawHeader("Authorization", headerData.toLocal8Bit());

  ConnectReply(Manager->get(request), handle, error);
  */
}

void NetworkManager::Authorize(QString url, const QString& username, const QString& password,
  API::REST::ReplyHandleFunc handle, API::REST::ReplyErrorFunc error)
{
#ifdef KEY_CLOACK
  KCAuthorize(username, password, [username, password, handle, error](bool authError, QNetworkReply* reply)
  {
    if(authError)
    {
      //TODO: handle error
      qDebug() << "Key Cloack auth error";
      error(reply, reply->error());
    }
    else
    {
      //ok;
      handle(nullptr, QJsonDocument());
    }
  });
#else
  QJsonObject obj;
  obj["username"] = username;
  obj["password"] = password;
  QNetworkRequest request = MakeRequest("api/v1/login");
  QNetworkReply* reply = Manager->post(request, QJsonDocument(obj).toJson(QJsonDocument::Compact));


  auto onFinished = [this, reply, handle, error]()
  {
    if (reply->error() == QNetworkReply::NoError)
    {
#ifdef JWT_ENABLED
      if (reply->hasRawHeader("Authorization"))
        JWtoken = reply->rawHeader("Authorization");
      if (reply->hasRawHeader("Refresh"))
        JwtRefreshToken = reply->rawHeader("Refresh");
#endif

      if (handle)
      {
        QJsonDocument doc;
        QJsonObject obj;
#ifdef JWT_ENABLED
        obj["Authorization"] = JWtoken;
        obj["Refresh"] = JwtRefreshToken;
#endif
        doc.setObject(obj);

        handle(reply, doc);
      }
    }
    else
    {
      qDebug() << "Network error:" << reply->errorString();
      if (error)
        error(reply, reply->error());
    }
    reply->deleteLater();
  };
  QObject::connect(reply, &QNetworkReply::finished, onFinished);
#endif
}

NetworkManager* NetworkManager::Get()
{
  if (!_Instance)
    _Instance = new NetworkManager;
  return _Instance;
}

void NetworkManager::FailRequest(API::REST::ReplyErrorFunc error)
{
  error(nullptr, QNetworkReply::RemoteHostClosedError);
}

//// Get
QNetworkReply *NetworkManager::GetRequest(const QString &url,
  const QUrlQuery &params,
  API::REST::ReplyHandleFunc handle,
  API::REST::ReplyErrorFunc error,
  QVariant tag,
  Qt::ConnectionType connectionType,
  API::REST::ReplyUploadProgressFunc progressHandle)
{
  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {
    QNetworkRequest request = MakeRequest(url, params);

    qDebug() << "GET" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->get(request);

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error](QNetworkReply* r, QNetworkReply::NetworkError e)
    {
       if (e == JWT_REFRESH_CODE)
         SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
           [this, request, handle, error](QNetworkReply* r, QJsonDocument)
           {
             QNetworkReply* reply = Manager->get(request);
             ConnectReply(reply, handle, error);
           },
           error);
       else
         error(r, e);
     });
#else
    ConnectReply(reply, handle, error);
#endif
    if(progressHandle)
    {
      connect(reply, &QNetworkReply::downloadProgress, this, [progressHandle](qint64 bytesSent, qint64 bytesTotal)
              {
                progressHandle(bytesSent, bytesTotal);
              });
    }
    reply->setProperty("tag", tag);
    return reply;
  }
  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypeGet;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldQuery>(*request) = params;
    std::get<RequestFieldHandle>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
}

//// Get
QNetworkReply* NetworkManager::GetRequest(const QString& url
  , const QUrlQuery& params
  , API::REST::ReplyHandleFuncTable handle
  , API::REST::ReplyErrorFunc error
  , QVariant tag
  , Qt::ConnectionType connectionType
  , API::REST::ReplyUploadProgressFunc progressHandle)
{
  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {
    QNetworkRequest request = MakeRequest(url, params);

    qDebug() << "GET" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->get(request);

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error](QNetworkReply* r, QNetworkReply::NetworkError e)
      {
        if (e == JWT_REFRESH_CODE)
          SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
            [this, request, handle, error](QNetworkReply* r, QJsonDocument)
            {
              QNetworkReply* reply = Manager->get(request);
              ConnectReply(reply, handle, error);
            },
            error);
        else
          error(r, e);
      });
#else
    ConnectReply(reply, handle, error);
#endif
    if(progressHandle)
    {
      connect(reply, &QNetworkReply::downloadProgress, this, [progressHandle](qint64 bytesSent, qint64 bytesTotal)
              {
                progressHandle(bytesSent, bytesTotal);
              });
    }
    reply->setProperty("tag", tag);
    return reply;
    }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypeGetTable;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldQuery>(*request) = params;
    std::get<RequestFieldHandleTable>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
}

QNetworkReply *NetworkManager::GetRequest(const QString &url,
                                          const QUrlQuery &params,
                                          API::REST::ReplyHandleByteArray handle,
                                          API::REST::ReplyErrorFunc error,
                                          QVariant tag,
                                          Qt::ConnectionType connectionType,
                                          API::REST::ReplyUploadProgressFunc progressHandle)
{
  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {
    QNetworkRequest request = MakeRequest(url, params);

    qDebug() << "GET" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->get(request);

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error](QNetworkReply* r, QNetworkReply::NetworkError e)
                 {
                   if (e == JWT_REFRESH_CODE)
                     SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
                       [this, request, handle, error](QNetworkReply* r, QJsonDocument)
                       {
                         QNetworkReply* reply = Manager->get(request);
                         ConnectReply(reply, handle, error);
                       },
                       error);
                   else
                     error(r, e);
                 });
#else
    ConnectReply(reply, handle, error);
#endif
    if(progressHandle)
    {
      connect(reply, &QNetworkReply::downloadProgress, this, [progressHandle](qint64 bytesSent, qint64 bytesTotal)
              {
                progressHandle(bytesSent, bytesTotal);
              });
    }
    reply->setProperty("tag", tag);
    return reply;
  }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypeGetTable;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldQuery>(*request) = params;
    std::get<RequestFieldHandleByteArray>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
}

//// Post / url
QNetworkReply* NetworkManager::PostRequest(const QString& url
  , const QUrlQuery& params
  , API::REST::ReplyHandleFunc handle
  , API::REST::ReplyErrorFunc error
  , QVariant tag
  , Qt::ConnectionType connectionType
  , API::REST::ReplyUploadProgressFunc progressHandle)
{

  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {
    QNetworkRequest request = MakeRequest(url, params);

    qDebug() << "POST" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->post(request, params.toString(QUrl().FullyEncoded).toUtf8());

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error, params](QNetworkReply* r, QNetworkReply::NetworkError e)
      {
        if (e == JWT_REFRESH_CODE)
          SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
            [this, request, handle, error, params](QNetworkReply* /*r*/, QJsonDocument)
            {
              QNetworkReply* reply = Manager->post(request, params.toString(QUrl().FullyEncoded).toUtf8());
              ConnectReply(reply, handle, error);
            },
            error);
        else
          error(r, e);
      });
#else
    ConnectReply(reply, handle, error);
#endif
    if(progressHandle)
    {
      connect(reply, &QNetworkReply::uploadProgress, this, [progressHandle](qint64 bytesSent, qint64 bytesTotal)
              {
                progressHandle(bytesSent, bytesTotal);
              });
    }
    reply->setProperty("tag", tag);
    return reply;
    }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypePost;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldQuery>(*request) = params;
    std::get<RequestFieldHandle>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
  }

//// Post / data
QNetworkReply* NetworkManager::PostRequest(const QString& url
  , const QJsonObject &data
  , API::REST::ReplyHandleFunc handle
  , API::REST::ReplyErrorFunc error
  , QVariant tag
  , Qt::ConnectionType connectionType
  , API::REST::ReplyUploadProgressFunc progressHandle)
{

  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {

    QNetworkRequest request = MakeRequest(url);

    qDebug() << "POST" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->post(request, QJsonDocument(data).toJson(QJsonDocument::Compact));

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error, data](QNetworkReply* r, QNetworkReply::NetworkError e)
      {
        if (e == JWT_REFRESH_CODE)
          SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
            [this, request, handle, error, data](QNetworkReply*, QJsonDocument)
            {
              QNetworkReply* reply = Manager->post(request, QJsonDocument(data).toJson(QJsonDocument::Compact));
              ConnectReply(reply, handle, error);
            },
            error);
        else
          error(r, e);
      });
#else
    ConnectReply(reply, handle, error);
#endif
    if(progressHandle)
    {
      connect(reply, &QNetworkReply::uploadProgress, this, [progressHandle](qint64 bytesSent, qint64 bytesTotal)
              {
                progressHandle(bytesSent, bytesTotal);
              });
    }
    reply->setProperty("tag", tag);
    return reply;
    }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypePostData;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldData>(*request) = data;
    std::get<RequestFieldHandle>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
}

QNetworkReply *NetworkManager::PostRequest(const QString &url, const QJsonArray &data, API::REST::ReplyHandleFunc handle, API::REST::ReplyErrorFunc error, QVariant tag, Qt::ConnectionType connectionType)
{

  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {

    QNetworkRequest request = MakeRequest(url);

    qDebug() << "POST" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->post(request, QJsonDocument(data).toJson(QJsonDocument::Compact));

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error, data](QNetworkReply* r, QNetworkReply::NetworkError e)
      {
        if (e == JWT_REFRESH_CODE)
          SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
            [this, request, handle, error, data](QNetworkReply*, QJsonDocument)
            {
              QNetworkReply* reply = Manager->post(request, QJsonDocument(data).toJson(QJsonDocument::Compact));
              ConnectReply(reply, handle, error);
            },
            error);
        else
          error(r, e);
      });
#else
    ConnectReply(reply, handle, error);
#endif
    reply->setProperty("tag", tag);
    return reply;
    }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypePostDataArray;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldDataArray>(*request) = data;
    std::get<RequestFieldHandle>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
}

//// Post / data / table result
QNetworkReply* NetworkManager::PostRequest(const QString& url
  , const QJsonObject& data
  , API::REST::ReplyHandleFuncTable handle
  , API::REST::ReplyErrorFunc error
  , QVariant tag
  , Qt::ConnectionType connectionType)
{

  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {

    QNetworkRequest request = MakeRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    qDebug() << "POST" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->post(request, QJsonDocument(data).toJson(QJsonDocument::Compact));
    ConnectReply(reply, handle, error);
    reply->setProperty("tag", tag);
    return reply;
  }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypePostDataTable;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldData>(*request) = data;
    std::get<RequestFieldHandleTable>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
}

QNetworkReply* NetworkManager::PostRequest(const QString& url,
  const QJsonObject& data,
  API::REST::ReplyHandleByteArray handle,
  API::REST::ReplyErrorFunc error,
  QVariant tag,
  Qt::ConnectionType connectionType)
{

  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {

    QNetworkRequest request = MakeRequest(url);

    qDebug() << "POST" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->post(request, QJsonDocument(data).toJson());

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error, data](QNetworkReply* r, QNetworkReply::NetworkError e)
      {
        if (e == JWT_REFRESH_CODE)
          SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
            [this, request, handle, error, data](QNetworkReply*, QJsonDocument)
            {
              QNetworkReply* reply = Manager->post(request, QJsonDocument(data).toJson() );
              ConnectReply(reply, handle, error);
            },
            error);
        else
          error(r, e);
      });
#else
    ConnectReply(reply, handle, error);
#endif
    reply->setProperty("tag", tag);
    return reply;
  }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypePostData;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldData>(*request) = data;
    std::get<RequestFieldHandleByteArray>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
}

//// Post / data & raw file data
QNetworkReply* NetworkManager::PostRequest(const QString& url
  , const QVariantMap& requestParts
  , const QByteArray& file
  , API::REST::ReplyHandleFunc handle
  , API::REST::ReplyErrorFunc error
  , QVariant tag
  , Qt::ConnectionType connectionType
  , API::REST::ReplyUploadProgressFunc progressHandle)
{

  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {
    //// Request
    QNetworkRequest request = MakeRequest(url);

    //// As multipart
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    //// Json parts
    {
      for (auto iter = requestParts.begin(); iter != requestParts.end(); iter++)
      {
        QHttpPart jsonPart;
        jsonPart.setHeader(QNetworkRequest::ContentDispositionHeader,
          QVariant(QString("form-data; name=\"%1\"").arg(iter.key())));
        jsonPart.setHeader(QNetworkRequest::ContentTypeHeader,
          QVariant("text/plain"));

        // auto value = j.value().toVariant();
        // QString stringValue = value.toString();
        // QByteArray rawData = QByteArray::fromStdString(stringValue.toStdString());
        QByteArray rawData = QByteArray::fromStdString(iter.value().toString().toStdString());
        // auto value = QByteArray::fromStdString(j.value().toString().toStdString());
        jsonPart.setBody(rawData);
        multiPart->append(jsonPart);
      }
    }

    //// File part
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
      QVariant(QString("form-data; name=\"file\"; filename=\"") + requestParts["file_name"].toString() + "\""));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader,
      QVariant("application/octet-stream"));
    filePart.setBody(file);
    multiPart->append(filePart);

    //// Send
    qDebug() << "POST" << request.url().toDisplayString(QUrl().FullyEncoded);

    Manager->startCatching();

    QNetworkReply* reply = Manager->post(request, multiPart);
    multiPart->setParent(reply);//needs pointer handling if "session expired" error
#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error, multiPart](QNetworkReply* r, QNetworkReply::NetworkError e)
      {
        if (e == JWT_REFRESH_CODE)
        {
          multiPart->setParent(nullptr);//pointer handling if "session expired" error
          SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
            [this, request, handle, error, multiPart](QNetworkReply*, QJsonDocument)
            {
              QNetworkReply* reply = Manager->post(request, multiPart);
              multiPart->setParent(reply);//pointer handling if "session expired" error
              ConnectReply(reply, handle, error);
            },
            [multiPart, error](QNetworkReply* r, QNetworkReply::NetworkError e)
            {
              multiPart->setParent(r);//pointer handling if "session expired" error
              error(r, e);
            });
        }
        else
          error(r, e);
      });
#else
    ConnectReply(reply, handle, error);
#endif
    if(progressHandle)
    {
      connect(reply, &QNetworkReply::uploadProgress, this, [progressHandle](qint64 bytesSent, qint64 bytesTotal)
      {
        progressHandle(bytesSent, bytesTotal);
      });
    }
    reply->setProperty("tag", tag);
    Manager->stopCatching();
    return reply;
    }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypePostDataAndFile;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldVariantMap>(*request) = requestParts;
    std::get<RequestFieldBlob>(*request) = file;
    std::get<RequestFieldHandle>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldProgressHandle>(*request) = progressHandle;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
  }

//// Put / url
QNetworkReply* NetworkManager::PutRequest(const QString& url
  , const QUrlQuery& params
  , API::REST::ReplyHandleFunc handle
  , API::REST::ReplyErrorFunc error
  , QVariant tag
  , Qt::ConnectionType connectionType)
{

  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {
    QNetworkRequest request = MakeRequest(url, params);

    qDebug() << "PUT" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->put(request, params.toString(QUrl().FullyEncoded).toUtf8());

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error, params](QNetworkReply* r, QNetworkReply::NetworkError e)
      {
        if (e == JWT_REFRESH_CODE)
          SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
            [this, request, handle, error, params](QNetworkReply*, QJsonDocument)
            {
              QNetworkReply* reply = Manager->put(request, params.toString(QUrl().FullyEncoded).toUtf8());
              ConnectReply(reply, handle, error);
            },
            error);
        else
          error(r, e);
      });
#else
    ConnectReply(reply, handle, error);
#endif
    reply->setProperty("tag", tag);
    return reply;
    }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypePut;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldQuery>(*request) = params;
    std::get<RequestFieldHandle>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
  }

//// Put / data
QNetworkReply* NetworkManager::PutRequest(const QString& url
  , const QJsonObject& data
  , API::REST::ReplyHandleFunc handle
  , API::REST::ReplyErrorFunc error
  , QVariant tag
  , Qt::ConnectionType connectionType)
{

  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {
    QNetworkRequest request = MakeRequest(url);

    qDebug() << "PUT" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->put(request, QJsonDocument(data).toJson(QJsonDocument::Compact));

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error, data](QNetworkReply* r, QNetworkReply::NetworkError e)
      {
        if (e == JWT_REFRESH_CODE)
          SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
            [this, request, handle, error, data](QNetworkReply*, QJsonDocument)
            {
              QNetworkReply* reply = Manager->put(request, QJsonDocument(data).toJson(QJsonDocument::Compact));
              ConnectReply(reply, handle, error);
            },
            error);
        else
          error(r, e);
      });
#else
    ConnectReply(reply, handle, error);
#endif
    reply->setProperty("tag", tag);
    return reply;
    }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypePutData;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldData>(*request) = data;
    std::get<RequestFieldHandle>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
  }

//// Put / data / table reult
QNetworkReply* NetworkManager::PutRequest(const QString& url
  , const QJsonObject& data
  , API::REST::ReplyHandleFuncTable handle
  , API::REST::ReplyErrorFunc error
  , QVariant tag
  , Qt::ConnectionType connectionType)
{

  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {
    QNetworkRequest request = MakeRequest(url);

    qDebug() << "PUT" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->put(request, QJsonDocument(data).toJson(QJsonDocument::Compact));

#ifdef JWT_ENABLED
    //if we catch the session expired error, we call refresh, and recall request after success.
    //if error occured while refresh, this error will be send as error instead "session expired" error.
    ConnectReply(reply, handle, [this, request, handle, error, data](QNetworkReply* r, QNetworkReply::NetworkError e)
      {
        if (e == JWT_REFRESH_CODE)
          SendJwtRefreshRequest(JWT_REFRESH_ENDPOINT,
            [this, request, handle, error, data](QNetworkReply*, QJsonDocument)
            {
              QNetworkReply* reply = Manager->put(request, QJsonDocument(data).toJson(QJsonDocument::Compact));
              ConnectReply(reply, handle, error);
            },
            error);
        else
          error(r, e);
      });
#else
    ConnectReply(reply, handle, error);
#endif
    reply->setProperty("tag", tag);
    return reply;
    }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypePutDataTable;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldData>(*request) = data;
    std::get<RequestFieldHandleTable>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
  }

//// Delete / url
QNetworkReply* NetworkManager::DeleteRequest(const QString& url
  , const QUrlQuery& params
  , API::REST::ReplyHandleFunc handle
  , API::REST::ReplyErrorFunc error
  , QVariant tag
  , Qt::ConnectionType connectionType)
{
  if(!AccessTokenValid)
  {
    BlockQueue();
    KCRefresh();
    connectionType = Qt::QueuedConnection;
  }

  //// Resolve auto
  if (connectionType == Qt::AutoConnection)
  {
    connectionType = ConnectionType;
  }

  //// Immediate
  if (connectionType == Qt::DirectConnection)
  {
    QNetworkRequest request = MakeRequest(url);
    qDebug() << "DELETE" << request.url().toDisplayString(QUrl().FullyEncoded);

    QNetworkReply* reply = Manager->deleteResource(request);
    ConnectReply(reply, handle, error);
    reply->setProperty("tag", tag);
    return reply;
  }

  //// Queued
  if (connectionType == Qt::QueuedConnection)
  {
    QSharedPointer<QueuedRequest> request(new QueuedRequest);
    std::get<RequestFieldType>(*request) = RequestTypeDelete;
    std::get<RequestFieldUrl>(*request) = url;
    std::get<RequestFieldQuery>(*request) = params;
    std::get<RequestFieldHandle>(*request) = handle;
    std::get<RequestFieldError>(*request) = error;
    std::get<RequestFieldReply>(*request) = nullptr;
    std::get<RequestFieldTag>(*request) = tag;

    Queue.push_back(request);
    ProcessQueued();
  }
  return nullptr;
}

void NetworkManager::StopTimers()
{
  auto refreshTimer = getAutoRefreshTimer();
  auto accessTimer = getAccessTokenTimer();
  if(refreshTimer->isActive())
  {
    refreshTimer->stop();
  }

  if(accessTimer->isActive())
  {
    accessTimer->stop();
  }
  AccessTokenValid = false;
  KCToken = QString();
  KCRefreshToken = QString();
}

QNetworkRequest NetworkManager::MakeRequest(QString url)
{
  QNetworkRequest r(App_Url + "/" + url);
//#ifdef HTTPS_ENABLED
//  QNetworkRequest r(QString("https://%1/%2").arg(Host, url));
////  QNetworkRequest r(QString("https://%1:%2/%3").arg(Host, Port, url));
//#else
//  QNetworkRequest r(QString("http://%1:%2/%3").arg(Host, Port, url));
//#endif

#ifdef JWT_ENABLED
  r.setRawHeader("Authorization", JWtoken.toUtf8());
#endif
#ifdef KEY_CLOACK
  r.setRawHeader("Authorization", (QString("Bearer ") + KCToken).toUtf8());
#endif

  return r;
}

QNetworkRequest NetworkManager::MakeRequest(QString url, const QUrlQuery& params)
{

  QUrl u(App_Url + "/" + url);

  u.setQuery(params);
  QNetworkRequest r(u);
#ifdef JWT_ENABLED
  r.setRawHeader("Authorization", JWtoken.toUtf8());
#endif
#ifdef KEY_CLOACK
  r.setRawHeader("Authorization", (QString("Bearer ") + KCToken).toUtf8());
#endif
  return r;
}

QNetworkRequest NetworkManager::MakeRequest(QString url, const QString params)
{
  QUrl u(url);

  u.setQuery(params);
  QNetworkRequest r(u);
#ifdef JWT_ENABLED
  r.setRawHeader("Authorization", JWtoken.toUtf8());
#endif
#ifdef KEY_CLOACK
  r.setRawHeader("Authorization", (QString("Bearer ") + KCToken).toUtf8());
#endif
  return r;
}

void NetworkManager::CatchOutgoingData(QSharedPointer<QByteArray> data)
{
  QFile f("catched_output.txt");
  f.open(QIODevice::WriteOnly);
  f.write(*data);
  f.close();
  //  qDebug() << "Catched output: " << *data;
}

void NetworkManager::KCRefresh()
{
  KCRefresh([this](bool refreshError)
  {
    if(refreshError)
    {
      qCritical() << "Error in refresh token, queue cleared.";
      ForceClearQueue();
      emit reloginRequired();
    }
    UnblockQueue();
  });
}

#ifdef KEY_CLOACK
void NetworkManager::KCRefresh(std::function<void (bool)> refreshCallback)
{
//  qDebug() << "start testing invalid refresh operation";
  auto refreshTimer = getAutoRefreshTimer();
  auto accessTimer = getAccessTokenTimer();
  if(refreshTimer->isActive())
  {
    refreshTimer->stop();
  }

  if(accessTimer->isActive())
  {
    accessTimer->stop();
    AccessTokenValid = false;
  }

  QUrl u(KeycloakUrl);

  QUrlQuery params;
  params.addQueryItem("client_id", client_id);
  params.addQueryItem("grant_type", "refresh_token");
  params.addQueryItem("refresh_token", KCRefreshToken);
  //REMOVE_FOR_PROPER:
//  params.addQueryItem("refresh_token", "A specially crafted invalid debug token");

  QNetworkRequest request(u);

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  QNetworkReply* reply = Manager->post(request, params.toString(QUrl().FullyEncoded).toUtf8());

  auto onRefreshFinished =
      [this, reply, refreshCallback]()
  {
    if (reply->error() == QNetworkReply::NoError)
    {
      QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
      auto obj = doc.object();
      KCToken = obj["access_token"].toString();
      KCRefreshToken = obj["refresh_token"].toString();

      auto accessIntervalMsec = (obj["expires_in"].toInt()) * 1000;
      auto refreshIntervalMsec = (obj["refresh_expires_in"].toInt()) * 1000;
//      auto refreshIntervalMsec = 60 * 1000;
#ifdef DEBUG
      qDebug()<<QString("New Access token expires in %1").arg(QTime::currentTime().addMSecs(accessIntervalMsec).toString());
      qDebug()<<QString("New Refresh token expires in %1").arg(QTime::currentTime().addMSecs(refreshIntervalMsec).toString());
#endif
      if(refreshIntervalMsec > 0)
      {
        auto timer = getAutoRefreshTimer();
        timer->setInterval(refreshIntervalMsec);
        timer->setSingleShot(true);
        timer->start();
      }

      if(accessIntervalMsec > 0)
      {
        auto timer = getAccessTokenTimer();
        timer->setInterval(accessIntervalMsec);
        timer->setSingleShot(true);
        timer->start();
        AccessTokenValid = true;
      }

      refreshCallback(false);
    }
    else
    {
      qDebug() << "Network error:" << reply->errorString();
      refreshCallback(true);
    }
  };
  QObject::connect(reply, &QNetworkReply::finished, onRefreshFinished);
}

void NetworkManager::KCAuthorize(QString username, QString password, std::function<void (bool, QNetworkReply *)> authCallback)
{
  QUrl u(KeycloakUrl);

  QUrlQuery params;
  params.addQueryItem("client_id", client_id);
  params.addQueryItem("username", username);
  params.addQueryItem("password", password);
  params.addQueryItem("grant_type", "password");
  QNetworkRequest request(u);

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  QNetworkReply* reply = Manager->post(request, params.toString(QUrl().FullyEncoded).toUtf8());
  auto onAuthFinished = [this, reply, authCallback]()
  {
    if (reply->error() == QNetworkReply::NoError)
    {
      qInfo() <<"Auth success";
      auto data = reply->readAll();

      QJsonDocument doc = QJsonDocument::fromJson(data);
#ifdef DEBUG
      qDebug().noquote() << doc;
#endif
      auto obj = doc.object();
      KCToken = obj["access_token"].toString();
      KCRefreshToken = obj["refresh_token"].toString();
      auto refreshTimer = getAutoRefreshTimer();
      auto accessTimer = getAccessTokenTimer();
      if(refreshTimer->isActive())
      {
        refreshTimer->stop();
      }

      if(accessTimer->isActive())
      {
        accessTimer->stop();
      }

      auto accessIntervalMsec = (obj["expires_in"].toInt()) * 1000;
      auto refreshIntervalMsec = (obj["refresh_expires_in"].toInt()) * 1000;
//      auto refreshIntervalMsec = 60 * 1000;
#ifdef DEBUG
      qDebug()<<QString("Access token expires in %1").arg(QTime::currentTime().addMSecs(accessIntervalMsec).toString());
      qDebug()<<QString("Refresh token expires in %1").arg(QTime::currentTime().addMSecs(refreshIntervalMsec).toString());
#endif
//      auto intervalMsec = 10000; //DEBUG, for testing refresh by timer
      if(refreshIntervalMsec > 0)
      {
        auto timer = getAutoRefreshTimer();
        timer->setInterval(refreshIntervalMsec);
        timer->setSingleShot(true);
        timer->start();
      }

      if(accessIntervalMsec > 0)
      {
        auto timer = getAccessTokenTimer();
        timer->setInterval(accessIntervalMsec);
        timer->setSingleShot(true);
        timer->start();
        AccessTokenValid = true;
      }

      authCallback(false, reply);
    }
    else
    {
      qDebug() << "Network error:" << reply->errorString();
      authCallback(true, reply);
    }
  };

  QObject::connect(reply, &QNetworkReply::finished, onAuthFinished);
}

QTimer *NetworkManager::getAutoRefreshTimer()
{
  if(!KCAutoRefreshTimerPtr)
  {
    KCAutoRefreshTimerPtr = new QTimer(this);
    connect(KCAutoRefreshTimerPtr, &QTimer::timeout, this, [this]
    {
#ifdef DEBUG
      qDebug()<<"Refresh token expired, need relogin";
#endif
      KCAutoRefreshTimerPtr->stop();
      emit reloginRequired();
      if(KCAccessTokenTimerPtr)
        KCAccessTokenTimerPtr->stop();
      AccessTokenValid = false;
      /*KCRefresh([this](bool error)
      {
        if(error)
        {
          KCAutoRefreshTimerPtr->stop();
          delete KCAutoRefreshTimerPtr;
          KCAutoRefreshTimerPtr=nullptr;
          emit reloginRequired();
          //TODO: user have to enter login/password again
        }
        else
        {
          //ok
          qDebug()<<"Refresh success";
        }
      });*/
    });
  }
  return KCAutoRefreshTimerPtr;
}

QTimer *NetworkManager::getAccessTokenTimer()
{
  if(!KCAccessTokenTimerPtr)
  {
    KCAccessTokenTimerPtr = new QTimer(this);
    connect(KCAccessTokenTimerPtr, &QTimer::timeout, this, [this]
    {
#ifdef DEBUG
      qDebug()<<"Access token expired";
#endif
      KCAccessTokenTimerPtr->stop();
      AccessTokenValid = false;
    });
  }
  return KCAccessTokenTimerPtr;
}
#endif

#ifdef JWT_ENABLED
void NetworkManager::SendJwtRefreshRequest(QString refreshEndpointUrl
  , API::REST::ReplyHandleFunc handle
  , API::REST::ReplyErrorFunc error)
{
  auto refreshRequest = MakeRequest(refreshEndpointUrl);
  refreshRequest.setRawHeader("Refresh", JwtRefreshToken.toUtf8());
  refreshRequest.setRawHeader("Authorization", JWtoken.toUtf8());
  QNetworkReply* reply = Manager->get(refreshRequest);
  ConnectReply(reply, handle, error);
}
#endif
