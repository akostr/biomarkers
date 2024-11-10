#pragma once

#include <rapidjson/include/rapidjson/document.h>
#include <rapidjson/include/rapidjson/rapidjson.h>
#include <vector>
#include <map>

namespace JsonUtils
{
  namespace Convert
  {
    // Remove nans, infs
    void RemoveNansInfs(rapidjson::Value& value);

    // Vector of doubles
    rapidjson::Value VectorToValue(const std::vector<double>& vector, rapidjson::Document::AllocatorType& allocator);
    std::vector<double> ValueToVector(const rapidjson::Value& value);
    std::vector<double> ValueChildToVector(const rapidjson::Value& value, const char* child);

    void VectorToValueChild(const std::vector<double>& vector, rapidjson::Value& data, rapidjson::Document::AllocatorType& allocator, const char* child);

    // Vector of integers
    rapidjson::Value VectorToValue(const std::vector<int>& vector, rapidjson::Document::AllocatorType& allocator);
    std::vector<int> ValueToVectorOfIntegers(const rapidjson::Value& value);
    std::vector<int> ValueChildToVectorOfIntegers(const rapidjson::Value& value, const char* child);

    void VectorOfIntegersToValueChild(const std::vector<int>& vector, rapidjson::Value& data, rapidjson::Document::AllocatorType& allocator, const char* child);

    // Vector of strings
    rapidjson::Value VectorToValue(const std::vector<std::string>& vector, rapidjson::Document::AllocatorType& allocator);
    std::vector<std::string> ValueToVectorOfStrings(const rapidjson::Value& value);
    std::vector<std::string> ValueChildToVectorOfStrings(const rapidjson::Value& value, const char* child);

    void VectorOfStringsToValueChild(const std::vector<std::string>& vector, rapidjson::Value& data, rapidjson::Document::AllocatorType& allocator, const char* child);

    // Matrix
    rapidjson::Value MatrixToValue(const std::vector<std::vector<double>>& matrix, rapidjson::Document::AllocatorType& allocator);
    std::vector<std::vector<double>> ValueToMatrix(const rapidjson::Value& value);
    std::vector<std::vector<double>> ValueChildToMatrix(const rapidjson::Value& value, const char* child);

    // Double
    double ValueToDouble(const rapidjson::Value& value);
    double ValueChildToDouble(const rapidjson::Value& value, const char* child);

    // Bool
    bool ValueToBool(const rapidjson::Value& value);
    bool ValueChildToBool(const rapidjson::Value& value, const char* child);

    // Integer
    int ValueToInteger(const rapidjson::Value& value);
    int ValueChildToInteger(const rapidjson::Value& value, const char* child);

    // String
    std::string ValueToString(const rapidjson::Value& value);
    std::string ValueChildToString(const rapidjson::Value& value, const char* child);

    // Objects
    template <class T>
    T ValueToObject(const rapidjson::Value& value)
    {
      T object;
      object.Load(value);
      return std::move(object);
    }

    template <class T>
    T ValueChildToObject(const rapidjson::Value& value, const char* child)
    {
      T object;
      if (value.HasMember(child))
      {
        const rapidjson::Value& v = value[child];
        object.Load(v);
      }
      return std::move(object);
    }

    template <class T>
    rapidjson::Value ObjectToValue(const T& object, rapidjson::Document::AllocatorType& allocator)
    {
      rapidjson::Value result;
      object.Save(result, allocator);
      return std::move(result);
    }

    template <class T>
    void ObjectToValueChild(const T& object, rapidjson::Value& data, rapidjson::Document::AllocatorType& allocator, const char* child)
    {
      if (data.IsObject())
      {
        rapidjson::Value cv = ObjectToValue<T>(object, allocator);

        if (data.HasMember(child))
          data.RemoveMember(child);

        data.AddMember(rapidjson::Value(child, allocator), cv, allocator);
      }
    }

    template <class T>
    void ToValueChild(rapidjson::Value& data, rapidjson::Document::AllocatorType& allocator, const char* child, T t)
    {
      if (data.IsObject())
      {
        if (data.HasMember(child))
          data.RemoveMember(child);

        data.AddMember(rapidjson::Value(child, allocator), rapidjson::Value(t), allocator);
      }
    }

    template <class T>
    void ToValueChildAllocated(rapidjson::Value& data, rapidjson::Document::AllocatorType& allocator, const char* child, T t)
    {
      if (data.IsObject())
      {
        if (data.HasMember(child))
          data.RemoveMember(child);

        data.AddMember(rapidjson::Value(child, allocator), rapidjson::Value(t, allocator), allocator);
      }
    }

    // Object vectors
    template <class T>
    std::vector<T> ValueToVectorOfObjects(const rapidjson::Value& value)
    {
      std::vector<T> result;
      if (value.IsArray())
      {
        result.reserve(value.Size());
        for (auto objValue = value.Begin(); objValue != value.End(); ++objValue)
        {
          T object;
          object.Load(*objValue);
          result.push_back(std::move(object));
        }
      }
      return std::move(result);
    }

    template <class T>
    std::vector<T> ValueChildToVectorOfObjects(const rapidjson::Value& value, const char* child)
    {
      if (!value.HasMember(child))
        return std::move(std::vector<T>());

      const rapidjson::Value& childValue = value[child];
      return std::move(ValueToVectorOfObjects<T>(childValue));
    }

    template <class T>
    void VectorOfObjectsToValue(const std::vector<T>& objects, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)
    {
      value.SetArray();
      value.Reserve(objects.size(), allocator);
      for (typename std::vector<T>::const_iterator object = objects.begin(); object != objects.end(); ++object)
      {
        rapidjson::Value objectData;
        (*object).Save(objectData, allocator);
        value.PushBack(objectData, allocator);
      }
    }

    template <class T>
    rapidjson::Value VectorOfObjectsToValue(const std::vector<T>& objects, rapidjson::Document::AllocatorType& allocator)
    {
      rapidjson::Value result;
      VectorOfObjectsToValue<T>(objects, result, allocator);
      return std::move(result);
    }

    template <class T>
    void VectorOfObjectsToValueChild(const std::vector<T>& objects, rapidjson::Value& data, const char* child, rapidjson::Document::AllocatorType& allocator)
    {
      if (data.IsObject())
      {
        rapidjson::Value v = VectorOfObjectsToValue(objects, allocator);
        if (data.HasMember(child))
          data.RemoveMember(child);
        data.AddMember(rapidjson::Value(child, allocator), v, allocator);
      }
    }

    // Object maps
    /*
    template <class T>
    std::map<std::string, T> ValueToMapOfObjects(const rapidjson::Value& value)
    {
      std::map<std::string, T> result;
      if (value.IsObject())
      {
        rapidjson::Value::Members members = value.getMemberNames();
        for (rapidjson::Value::Members::const_iterator m = members.begin(); m != members.end(); ++m)
        {
          if (value.isMember(*m))
          {
            T object;
            rapidjson::Value objectValue = value[*m];
            object.Load(objectValue);
            result[*m] = std::move(object);
          }
        }
      }
      return std::move(result);
    }

    template <class T>
    std::map<std::string, T> ValueChildToMapOfObjects(const rapidjson::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return std::move(std::map<std::string, T>());

      rapidjson::Value childValue = value[child];
      return std::move(ValueToMapOfObjects<T>(childValue));
    }

    template <class T>
    rapidjson::Value MapOfObjectsToValue(const std::map<std::string, T>& objects, rapidjson::Document::AllocatorType& allocator)
    {
      rapidjson::Value result;
      for (std::map<std::string, T>::const_iterator object = objects.begin(); object != objects.end(); ++object)
      {
        rapidjson::Value objectData;
        (*object).second.Save(objectData, allocator);
        result[(*object).first] = objectData;
      }
      return std::move(result);
    }
    */
  }
}

class QDebug;

QDebug operator<<(QDebug out, const rapidjson::Value& d);
