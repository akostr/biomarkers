#pragma once

#include <jsoncpp/include/json/json.h>

namespace JsonUtils
{
  namespace Convert
  {
    // Vector of doubles
    Json::Value VectorToValue(const std::vector<double>& vector);
    std::vector<double> ValueToVector(const Json::Value& value);
    std::vector<double> ValueChildToVector(const Json::Value& value, const char* child);

    // Vector of integers
    Json::Value VectorToValue(const std::vector<int>& vector);
    std::vector<int> ValueToVectorOfIntegers(const Json::Value& value);
    std::vector<int> ValueChildToVectorOfIntegers(const Json::Value& value, const char* child);

    // Vector of strings
    Json::Value VectorToValue(const std::vector<std::string>& vector);
    std::vector<std::string> ValueToVectorOfStrings(const Json::Value& value);
    std::vector<std::string> ValueChildToVectorOfStrings(const Json::Value& value, const char* child);

    // Matrix
    Json::Value MatrixToValue(const std::vector<std::vector<double>>& matrix);
    std::vector<std::vector<double>> ValueToMatrix(const Json::Value& value);
    std::vector<std::vector<double>> ValueChildToMatrix(const Json::Value& value, const char* child);

    // Double
    double ValueToDouble(const Json::Value& value);
    double ValueChildToDouble(const Json::Value& value, const char* child);

    // Bool
    bool ValueToBool(const Json::Value& value);
    bool ValueChildToBool(const Json::Value& value, const char* child);

    // Integer
    int ValueToInteger(const Json::Value& value);
    int ValueChildToInteger(const Json::Value& value, const char* child);

    // String
    std::string ValueToString(const Json::Value& value);
    std::string ValueChildToString(const Json::Value& value, const char* child);

    // Objects
    template <class T>
    T ValueToObject(const Json::Value& value)
    {
      T object;
      object.Load(value);
      return std::move(object);
    }

    template <class T>
    T ValueChildToObject(const Json::Value& value, const char* child)
    {
      T object;
      if (value.isMember(child))
      {
        Json::Value v = value[child];
        object.Load(v);
      }
      return std::move(object);
    }

    template <class T>
    Json::Value ObjectToValue(const T& object)
    {
      Json::Value result;
      object.Save(result);
      return std::move(result);
    }

    // Object vectors
    template <class T>
    std::vector<T> ValueToVectorOfObjects(const Json::Value& value)
    {
      std::vector<T> result;
      if (value.isArray())
      {
        result.reserve(value.size());
        for (Json::Value::const_iterator objValue = value.begin(); objValue != value.end(); ++objValue)
        {
          T object;
          object.Load(*objValue);
          result.push_back(object);
        }
      }
      return std::move(result);
    }

    template <class T>
    std::vector<T> ValueChildToVectorOfObjects(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return std::move(std::vector<T>());

      Json::Value childValue = value[child];
      return std::move(ValueToVectorOfObjects<T>(childValue));
    }

//// #ifndef __clang__
    template <class T>
    Json::Value VectorOfObjectsToValue(const std::vector<T>& objects)
    {
      Json::Value result;
      for (typename std::vector<T>::const_iterator object = objects.begin(); object != objects.end(); ++object)
      {
        Json::Value objectData;
        (*object).Save(objectData);
        result.append(objectData);
      }
      return std::move(result);
    }
//// #endif

    // Object maps
    template <class T>
    std::map<std::string, T> ValueToMapOfObjects(const Json::Value& value)
    {
      std::map<std::string, T> result;
      if (value.isObject())
      {
        Json::Value::Members members = value.getMemberNames();
        for (Json::Value::Members::const_iterator m = members.begin(); m != members.end(); ++m)
        {
          if (value.isMember(*m))
          {
            T object;
            Json::Value objectValue = value[*m];
            object.Load(objectValue);
            result[*m] = object;
          }
        }
      }
      return std::move(result);
    }

    template <class T>
    std::map<std::string, T> ValueChildToMapOfObjects(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return std::move(std::map<std::string, T>());

      Json::Value childValue = value[child];
      return std::move(ValueToMapOfObjects<T>(childValue));
    }

//// #ifndef __clang__
    template <class T>
    Json::Value MapOfObjectsToValue(const std::map<std::string, T>& objects)
    {
      Json::Value result;
      for (typename std::map<std::string, T>::const_iterator object = objects.begin(); object != objects.end(); ++object)
      {
        Json::Value objectData;
        (*object).second.Save(objectData);
        result[(*object).first] = objectData;
      }
      return std::move(result);
    }
//// #endif
  }
}
