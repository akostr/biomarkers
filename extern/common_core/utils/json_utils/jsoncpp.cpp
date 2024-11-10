#include "jsoncpp.h"

#include <algorithm>

namespace JsonUtils
{
  namespace Convert
  {
    // Vector of doubles
    Json::Value VectorToValue(const std::vector<double>& vector)
    {
      Json::Value result;
      std::for_each(vector.begin(), vector.end(), [&result](double v) { result.append(v); });
      return std::move(result);
    }

    std::vector<double> ValueToVector(const Json::Value& value)
    {
      std::vector<double> result;
      if (value.isArray())
      {
        result.reserve(value.size());
        std::for_each(value.begin(), value.end(), [&result](const Json::Value& v) {
          if (v.isDouble())
          {
            result.push_back(v.asDouble());
          }
        });
      }
      return std::move(result);
    }

    std::vector<double> ValueChildToVector(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return std::vector<double>();

      return std::move(ValueToVector(value[child]));
    }

    // Vector of intgers
    Json::Value VectorToValue(const std::vector<int>& vector)
    {
      Json::Value result;
      std::for_each(vector.begin(), vector.end(), [&result](int v) { result.append(v); });
      return std::move(result);
    }

    std::vector<int> ValueToVectorOfIntegers(const Json::Value& value)
    {
      std::vector<int> result;
      if (value.isArray())
      {
        result.reserve(value.size());
        std::for_each(value.begin(), value.end(), [&result](const Json::Value& v) {
          if (v.isInt())
          {
            result.push_back(v.asInt());
          }
        });
      }
      return std::move(result);
    }

    std::vector<int> ValueChildToVectorOfIntegers(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return std::vector<int>();

      return std::move(ValueToVectorOfIntegers(value[child]));
    }

    // Vector of strings
    Json::Value VectorToValue(const std::vector<std::string>& vector)
    {
      Json::Value result;
      std::for_each(vector.begin(), vector.end(), [&result](const std::string& v) { result.append(v); });
      return std::move(result);
    }

    std::vector<std::string> ValueToVectorOfStrings(const Json::Value& value)
    {
      std::vector<std::string> result;
      if (value.isArray())
      {
        result.reserve(value.size());
        std::for_each(value.begin(), value.end(), [&result](const Json::Value& v) {
          if (v.isString())
          {
            result.push_back(v.asString());
          }
        });
      }
      return std::move(result);
    }

    std::vector<std::string> ValueChildToVectorOfStrings(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return std::vector<std::string>();

      return std::move(ValueToVectorOfStrings(value[child]));
    }

    // Matrix
    Json::Value MatrixToValue(const std::vector<std::vector<double>>& matrix)
    {
      Json::Value result;
      std::for_each(matrix.begin(), matrix.end(), [&result](const std::vector<double>& row)
      {
        Json::Value r;
        std::for_each(row.begin(), row.end(), [&r](double v) { r.append(v); });
        result.append(r);
      });
      return std::move(result);
    }

    std::vector<std::vector<double>> ValueToMatrix(const Json::Value& value)
    {
      std::vector<std::vector<double>> result;
      if (value.isArray())
      {
        result.reserve(value.size());
        for (Json::Value::const_iterator r = value.begin(); r != value.end(); ++r)
        {
          std::vector<double> row;
          if (r->isArray())
          {
            row.reserve(r->size());
            for (Json::Value::const_iterator v = r->begin(); v != r->end(); ++v)
            {
              if (v->isDouble())
              {
                row.push_back(v->asDouble());
              }
            }
          }
          result.push_back(row);
        }
      }
      return std::move(result);
    }

    std::vector<std::vector<double>> ValueChildToMatrix(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return std::vector<std::vector<double>>();

      return std::move(ValueToMatrix(value[child]));
    }

    // Doubles
    double ValueToDouble(const Json::Value& value)
    {
      if (value.isDouble())
        return value.asDouble();
      return std::numeric_limits<double>::quiet_NaN();
    }

    double ValueChildToDouble(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return std::numeric_limits<double>::quiet_NaN();

      return std::move(ValueToDouble(value[child]));
    }

    // Bools
    bool ValueToBool(const Json::Value& value)
    {
      if (value.isBool())
        return value.asBool();
      return false;
    }

    bool ValueChildToBool(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return false;

      return ValueToBool(value[child]);
    }

    // Integers
    int ValueToInteger(const Json::Value& value)
    {
      if (value.isInt())
        return value.isInt();
      return 0;
    }

    int ValueChildToInteger(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return 0;

      return ValueToInteger(value[child]);
    }

    // Strings
    std::string ValueToString(const Json::Value& value)
    {
      if (value.isString())
        return std::move(value.asString());
      return std::move(std::string());
    }

    std::string ValueChildToString(const Json::Value& value, const char* child)
    {
      if (!value.isMember(child))
        return std::string();

      return std::move(ValueToString(value[child]));
    }
  }
}
