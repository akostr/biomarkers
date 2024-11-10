#include "rapid.h"

#include <algorithm>
#include <cmath>
#include <rapidjson/include/rapidjson/stringbuffer.h>
#include <rapidjson/include/rapidjson/writer.h>
#include <QDebug>

namespace JsonUtils
{
  namespace Convert
  {
    // Remove nans, infs
    void RemoveNansInfs(rapidjson::Value& value)
    {
      if (value.IsArray())
      {
        std::for_each(value.Begin(), value.End(), [](rapidjson::Value& v)
          {
            RemoveNansInfs(v);
          });
      }
      else if (value.IsObject())
      {
        for (auto v = value.MemberBegin(); v != value.MemberEnd(); v++)
        {
          RemoveNansInfs(v->value);
        }
      }
      else if (value.IsDouble())
      {
        if (std::isnan(value.GetDouble())
         || std::isinf(value.GetDouble()))
        {
          value = rapidjson::Value();
        }
      }
    }

    // Vector of doubles
    rapidjson::Value VectorToValue(const std::vector<double>& vector, rapidjson::Document::AllocatorType& allocator)
    {
      rapidjson::Value result;
      result.SetArray();
      result.Reserve(vector.size(), allocator);
      std::for_each(vector.begin(), vector.end(), [&result, &allocator](double v) { result.PushBack(rapidjson::Value(v), allocator); });
      return std::move(result);
    }

    void VectorToValueChild(const std::vector<double>& vector, rapidjson::Value& data, rapidjson::Document::AllocatorType& allocator, const char* child)
    {
      if (data.IsObject())
      {
        rapidjson::Value v = VectorToValue(vector, allocator);

        if (data.HasMember(child))
          data.RemoveMember(child);

        data.AddMember(rapidjson::Value(child, allocator), v, allocator);
      }
    }

    std::vector<double> ValueToVector(const rapidjson::Value& value)
    {
      std::vector<double> result;
      if (value.IsArray())
      {
        result.reserve(value.Size());
        std::for_each(value.Begin(), value.End(), [&result](const rapidjson::Value& v) {
          if (v.IsNumber())
          {
            result.push_back(v.GetDouble());
          }
        });
      }
      return std::move(result);
    }

    std::vector<double> ValueChildToVector(const rapidjson::Value& value, const char* child)
    {
      if (!value.HasMember(child))
        return std::vector<double>();

      return std::move(ValueToVector(value[child]));
    }

    // Vector of intgers
    rapidjson::Value VectorToValue(const std::vector<int>& vector, rapidjson::Document::AllocatorType& allocator)
    {
      rapidjson::Value result;
      result.SetArray();
      result.Reserve(vector.size(), allocator);
      std::for_each(vector.begin(), vector.end(), [&result, &allocator](int v) { result.PushBack(rapidjson::Value(v), allocator); });
      return std::move(result);
    }

    std::vector<int> ValueToVectorOfIntegers(const rapidjson::Value& value)
    {
      std::vector<int> result;
      if (value.IsArray())
      {
        result.reserve(value.Size());
        std::for_each(value.Begin(), value.End(), [&result](const rapidjson::Value& v) {
          if (v.IsInt())
          {
            result.push_back(v.GetInt());
          }
        });
      }
      return std::move(result);
    }

    std::vector<int> ValueChildToVectorOfIntegers(const rapidjson::Value& value, const char* child)
    {
      if (!value.HasMember(child))
        return std::vector<int>();

      return std::move(ValueToVectorOfIntegers(value[child]));
    }

    void VectorOfIntegersToValueChild(const std::vector<int>& vector, rapidjson::Value& data, rapidjson::Document::AllocatorType& allocator, const char* child)
    {
      if (data.IsObject())
      {
        rapidjson::Value v = VectorToValue(vector, allocator);

        if (data.HasMember(child))
          data.RemoveMember(child);

        data.AddMember(rapidjson::Value(child, allocator), v, allocator);
      }
    }

    // Vector of strings
    rapidjson::Value VectorToValue(const std::vector<std::string>& vector, rapidjson::Document::AllocatorType& allocator)
    {
      rapidjson::Value result;
      result.SetArray();
      result.Reserve(vector.size(), allocator);
      std::for_each(vector.begin(), vector.end(), [&result, &allocator](const std::string& v) { result.PushBack(rapidjson::Value(v, allocator), allocator); });
      return std::move(result);
    }

    std::vector<std::string> ValueToVectorOfStrings(const rapidjson::Value& value)
    {
      std::vector<std::string> result;
      if (value.IsArray())
      {
        result.reserve(value.Size());
        std::for_each(value.Begin(), value.End(), [&result](const rapidjson::Value& v) {
          if (v.IsString())
          {
            result.push_back(v.GetString());
          }
        });
      }
      return std::move(result);
    }

    std::vector<std::string> ValueChildToVectorOfStrings(const rapidjson::Value& value, const char* child)
    {
      if (!value.HasMember(child))
        return std::vector<std::string>();

      return std::move(ValueToVectorOfStrings(value[child]));
    }

    void VectorOfStringsToValueChild(const std::vector<std::string>& vector, rapidjson::Value& data, rapidjson::Document::AllocatorType& allocator, const char* child)
    {
      if (data.IsObject())
      {
        rapidjson::Value v = VectorToValue(vector, allocator);

        if (data.HasMember(child))
          data.RemoveMember(child);

        data.AddMember(rapidjson::Value(child, allocator), v, allocator);
      }
    }

    // Matrix
    rapidjson::Value MatrixToValue(const std::vector<std::vector<double>>& matrix, rapidjson::Document::AllocatorType& allocator)
    {
      rapidjson::Value result;
      result.SetArray();
      result.Reserve(matrix.size(), allocator);
      std::for_each(matrix.begin(), matrix.end(), [&result, &allocator](const std::vector<double>& row)
      {
        rapidjson::Value r;
        r.SetArray();
        r.Reserve(row.size(), allocator);
        std::for_each(row.begin(), row.end(), [&r, &allocator](double v) { r.PushBack(rapidjson::Value(v), allocator); });
        result.PushBack(rapidjson::Value(r, allocator), allocator);
      });
      return std::move(result);
    }

    std::vector<std::vector<double>> ValueToMatrix(const rapidjson::Value& value)
    {
      std::vector<std::vector<double>> result;
      if (value.IsArray())
      {
        result.reserve(value.Size());
        for (auto r = value.Begin(); r != value.End(); ++r)
        {
          std::vector<double> row;
          if (r->IsArray())
          {
            row.reserve(r->Size());
            for (auto v = r->Begin(); v != r->End(); ++v)
            {
              if (v->IsDouble())
              {
                row.push_back(v->GetDouble());
              }
            }
          }
          result.push_back(row);
        }
      }
      return std::move(result);
    }

    std::vector<std::vector<double>> ValueChildToMatrix(const rapidjson::Value& value, const char* child)
    {
      if (!value.HasMember(child))
        return std::vector<std::vector<double>>();

      return std::move(ValueToMatrix(value[child]));
    }

    // Doubles
    double ValueToDouble(const rapidjson::Value& value)
    {
      if (value.IsDouble())
        return value.GetDouble();
      return std::numeric_limits<double>::quiet_NaN();
    }

    double ValueChildToDouble(const rapidjson::Value& value, const char* child)
    {
      if (!value.HasMember(child))
        return std::numeric_limits<double>::quiet_NaN();

      return ValueToDouble(value[child]);
    }

    // Bools
    bool ValueToBool(const rapidjson::Value& value)
    {
      if (value.IsBool())
        return value.GetBool();
      return false;
    }

    bool ValueChildToBool(const rapidjson::Value& value, const char* child)
    {
      if (!value.HasMember(child))
        return false;

      return ValueToBool(value[child]);
    }

    // Integers
    int ValueToInteger(const rapidjson::Value& value)
    {
      if (value.IsInt())
        return value.GetInt();
      return 0;
    }

    int ValueChildToInteger(const rapidjson::Value& value, const char* child)
    {
      if (!value.HasMember(child))
        return 0;

      return ValueToInteger(value[child]);
    }

    // Strings
    std::string ValueToString(const rapidjson::Value& value)
    {
      if (value.IsString())
        return std::move(value.GetString());
      return std::move(std::string());
    }

    std::string ValueChildToString(const rapidjson::Value& value, const char* child)
    {
      if (!value.HasMember(child))
        return std::string();

      return std::move(ValueToString(value[child]));
    }
  }
}

QDebug operator<<(QDebug out, const rapidjson::Value& d)
{
	rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  d.Accept(writer);

  return out << buffer.GetString();
}
