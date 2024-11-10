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
#include <jsoncpp/include/json/json.h>


namespace JsonUtils
{
  namespace Files
  {
    enum FormatFlags
    {
      FormatCompressed,
      FormatBinary,
      FormatPretty,
      FormatAuto
    };

    FormatFlags ResolveFormat(const QString& fileName);

    void WriteJsonFile(QByteArray& data, const QJsonValue& value, FormatFlags f);

    void WriteJsonFile(const QString& fileName, const QJsonDocument& value, FormatFlags f = FormatAuto);
    void WriteJsonFile(const QString& fileName, const QJsonValue& value, FormatFlags f = FormatAuto);
#ifndef DISABLE_UBJSON_FORMAT
    void WriteJsonFile(const QString& fileName, const ubjson::Value& value, FormatFlags f = FormatAuto);
#endif
    void WriteJsonFile(const QString& fileName, const rapidjson::Value& value, FormatFlags f = FormatAuto);

    void ReadJsonFile(const QByteArray& data, QJsonValue& value, FormatFlags f);

    void ReadJsonFile(const QString& fileName, QJsonDocument& document, FormatFlags f = FormatAuto);
    void ReadJsonFile(const QString& fileName, QJsonValue& value, FormatFlags f = FormatAuto);
#ifndef DISABLE_UBJSON_FORMAT
    void ReadJsonFile(const QString& fileName, ubjson::Value& value, FormatFlags f = FormatAuto);
#endif
    void ReadJsonFile(const QString& fileName, rapidjson::Document& document, FormatFlags f = FormatAuto);
  }
}
