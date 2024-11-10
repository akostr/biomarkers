#include "base.h"

#include <QJsonObject>
#include <QVariant>

#ifdef GetObject
#undef GetObject
#endif 

namespace JsonUtils
{
  namespace Convert
  {

#ifndef DISABLE_UBJSON_FORMAT
    void QJsonToUbjson(const QJsonDocument& qtJsonDocument, ubjson::Value& out)
    {
      if (qtJsonDocument.isArray())
        return QJsonToUbjson(qtJsonDocument.array(), out);
      if (qtJsonDocument.isObject())
        return QJsonToUbjson(qtJsonDocument.object(), out);
      out = ubjson::Value();
    }

    void QJsonToUbjson(const QJsonArray& qtJsonArray, ubjson::Value& out)
    {
      out = ubjson::Value();
      QJsonArray::const_iterator arrayChild = qtJsonArray.constBegin();
      while (arrayChild != qtJsonArray.end())
      {
        QJsonValue qtJsonValue = *arrayChild;

        out.push_back(ubjson::Value());
        ubjson::Value& ubjChild = out[out.size() - 1];
        QJsonToUbjson(qtJsonValue, ubjChild);

        ++arrayChild;
      }
    }

    void QJsonToUbjson(const QJsonValue& qtJsonValue, ubjson::Value& out)
    {
      if (qtJsonValue.isNull())
      {
        out = ubjson::Value();
        return;
      }
      if (qtJsonValue.isArray())
        return QJsonToUbjson(qtJsonValue.toArray(), out);
      if (qtJsonValue.isObject())
        return QJsonToUbjson(qtJsonValue.toObject(), out);
      if (qtJsonValue.isBool())
      {
        out = ubjson::Value(qtJsonValue.toBool());
        return;
      }
      if (qtJsonValue.isDouble())
      {
        out = ubjson::Value(qtJsonValue.toDouble());
        return;
      }
      if (qtJsonValue.isString())
      {
        out = ubjson::Value(qtJsonValue.toString().toStdString());
        return;
      }
      
      out = ubjson::Value();
    }

    void QJsonToUbjson(const QJsonObject& qtJson, ubjson::Value& out)
    {
      if (qtJson.isEmpty())
      {
        out = ubjson::Value();
        return;
      }

      QJsonObject::const_iterator objectChild = qtJson.constBegin();
      while (objectChild != qtJson.constEnd())
      {
        std::string key  = objectChild.key().toStdString();
        QJsonValue value = objectChild.value();

        ubjson::Value& ubjChild = out[key.c_str()];
        QJsonToUbjson(value, ubjChild);

        ++objectChild;
      }
    }

    void UbjsonToQJson(const ubjson::Value& ubjValue, QJsonValue& out)
    {
      if (ubjValue.isNull())
      {
        out = QJsonValue();
        return;
      }
      if (ubjValue.isArray())
      {
        QJsonArray qtJsonArray;
        ubjson::Value::const_iterator ubjValueChild = ubjValue.cbegin();
        while (ubjValueChild != ubjValue.cend())
        {
          QJsonValue value;
          UbjsonToQJson(*ubjValueChild, value);
          qtJsonArray.append(value);
          ++ubjValueChild;
        }
        out = qtJsonArray;
        return;
      }
      if (ubjValue.isMap())
      {
        QJsonObject qtJsonMap;
        ubjson::Value::Keys keys = ubjValue.keys();
        for (auto key = keys.cbegin(); key != keys.cend(); ++key)
        {
          const ubjson::Value& ubjValueChild = ubjValue[*key];
          QJsonValue value;
          UbjsonToQJson(ubjValueChild, value);
          qtJsonMap.insert(QString::fromStdString(*key), value);
        }
        out = qtJsonMap;
        return;
      }
      if (ubjValue.isBool())
      {
        out = QJsonValue(ubjValue.asBool());
        return;
      }
      if (ubjValue.isFloat())
      {
        out = QJsonValue(ubjValue.asFloat());
        return;
      }
      if (ubjValue.isInteger())
      {
        out = QJsonValue(ubjValue.asInt());
        return;
      }
      if (ubjValue.isString())
      {
        out = QJsonValue(QString(ubjValue.asString().c_str()));
        return;
      }       

      out = QJsonValue();
    }

    QJsonDocument UbjsonToQJsonDocument(const ubjson::Value& ubj)
    {
      QJsonValue value;
      UbjsonToQJson(ubj, value);
      QJsonDocument document;
      document.setObject(value.toObject());
      return document;
    }
#endif

    rapidjson::Value QJsonToRapidJson(const QJsonObject& qtJsonObject, rapidjson::Document::AllocatorType& allocator)
    {
      rapidjson::Value result;
      result.SetObject();
      for (QJsonObject::const_iterator i = qtJsonObject.begin(); i != qtJsonObject.end(); ++i)
      {
        rapidjson::Value k(i.key().toStdString().c_str(), allocator);
        if (i->isObject())
        {
          rapidjson::Value child = QJsonToRapidJson(i->toObject(), allocator);
          result.AddMember(k, child, allocator);

        }
        else if (i->isArray())
        {
          rapidjson::Value child = QJsonToRapidJson(i->toArray(), allocator);
          result.AddMember(k, child, allocator);
        }
        else if (i->isBool())
        {
          result.AddMember(k, i->toBool(), allocator);
        }
        else if (i->isDouble())
        {
          result.AddMember(k, i->toDouble(), allocator);
        }
        else if (i->isString())
        {
          std::string utf8 = i->toString().toUtf8().toStdString();
          rapidjson::Value child(utf8, allocator);
          result.AddMember(k, child, allocator);
        }
        else if (i->isNull())
        {
          result.AddMember(k, rapidjson::Value(), allocator);
        }
      }
      return result;
    }

    rapidjson::Value QJsonToRapidJson(const QJsonArray& qtJsonArray, rapidjson::Document::AllocatorType& allocator)
    {
      rapidjson::Value result;
      result.SetArray();
      result.Reserve(qtJsonArray.size(), allocator);
      for (QJsonArray::const_iterator i = qtJsonArray.begin(); i != qtJsonArray.end(); ++i)
      {
        if (i->isObject())
        {
          rapidjson::Value child = QJsonToRapidJson(i->toObject(), allocator);
          result.PushBack(child, allocator);
        }
        else if (i->isArray())
        {
          rapidjson::Value child = QJsonToRapidJson(i->toArray(), allocator);
          result.PushBack(child, allocator);
        }
        else if (i->isBool())
        {
          result.PushBack(i->toBool(), allocator);
        }
        else if (i->isDouble())
        {
          result.PushBack(i->toDouble(), allocator);
        }
        else if (i->isString())
        {
          std::string utf8 = i->toString().toUtf8().toStdString();
          rapidjson::Value child(utf8, allocator);
          result.PushBack(child, allocator);
        }
        else if (i->isNull())
        {
          result.PushBack(rapidjson::Value(), allocator);
        }
      }
      return result;
    }

    QJsonValue RapidJsonToQJson(const rapidjson::Value& value)
    {
      if (value.IsObject())
      {
        auto vobj = value.GetObject();

        QJsonObject object;
        for (rapidjson::Value::ConstMemberIterator i = vobj.MemberBegin(); i != vobj.MemberEnd(); ++i)
        {
          std::string k(i->name.GetString());
          object[QString::fromStdString(k)] = RapidJsonToQJson(i->value);
        }
        return object;
      }
      if (value.IsArray())
      {
        QJsonArray array;
        for (int i = 0; i < value.Size(); ++i)
        {
          array.append(RapidJsonToQJson(value[i]));
        }
        return array;
      }
      if (value.IsBool())
      {
        return QJsonValue(value.GetBool());
      }
      if (value.IsInt() || value.IsUint())
      {
        return QJsonValue(value.GetInt());
      }
      if (value.IsInt64() || value.IsUint64())
      {
        return QJsonValue(value.GetInt64());
      }
      if (value.IsDouble())
      {
        return QJsonValue(value.GetDouble());
      }
      if (value.IsString())
      {
        return QJsonValue(QString::fromUtf8(value.GetString()));
      }
      return QJsonValue();
    }

    QVariant RapidJsonToQVariant(const rapidjson::Value& value)
    {
      if (value.IsNull())
        return QVariant();
      if (value.IsDouble())
        return value.GetDouble();
      if (value.IsString())
        return value.GetString();
      if (value.IsInt64())
        return value.GetInt64();
      if (value.IsBool())
        return value.GetBool();
      if (value.IsObject())
      {
        QVariantMap qvm;
        for (rapidjson::Value::ConstMemberIterator m = value.MemberBegin(); m != value.MemberEnd(); ++m)
        {
          qvm[QString(m->name.GetString())] = RapidJsonToQVariant(m->value);
        }
        return qvm;
      }
      if (value.IsArray())
      {
        QVariantList qvl;
        for (int i = 0; i < value.Size(); ++i)
        {
          qvl.append(RapidJsonToQVariant(value[i]));
        }
        return qvl;
      }
      return QVariant();
    }
  } // ~namespace Convert
} // ~namespace Utils
