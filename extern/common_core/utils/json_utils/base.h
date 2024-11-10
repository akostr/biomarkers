#pragma once

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>

#include <rapidjson/include/rapidjson/document.h>

#ifndef DISABLE_UBJSON_FORMAT
#include <ubjsoncpp/include/value.hpp>
#endif

namespace JsonUtils
{
  namespace Convert
  {
    ////////////////////////////////////////////////
    //// Conversion
#ifndef DISABLE_UBJSON_FORMAT
    void QJsonToUbjson(const QJsonArray& qtJsonArray, ubjson::Value& out);
    void QJsonToUbjson(const QJsonValue& qtJsonValue, ubjson::Value& out);
    void QJsonToUbjson(const QJsonObject& qtJsonObject, ubjson::Value& out);
    void UbjsonToQJson(const ubjson::Value& ubjIn, QJsonValue& out);
    QJsonDocument UbjsonToQJsonDocument(const ubjson::Value& ubj);
#endif

    rapidjson::Value QJsonToRapidJson(const QJsonObject& qtJsonObject, rapidjson::Document::AllocatorType& allocator);
    rapidjson::Value QJsonToRapidJson(const QJsonArray& qtJsonArray, rapidjson::Document::AllocatorType& allocator);

    QJsonValue     RapidJsonToQJson(const rapidjson::Value& value);
    QVariant       RapidJsonToQVariant(const rapidjson::Value& value);
  }
}
