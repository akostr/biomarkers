#pragma once
#ifndef IJSON_SERIALIZER_H
#define IJSON_SERIALIZER_H

#include <qjsonobject>

#include "iservice.h"
#include "logic/models/identity_model.h"

namespace Service
{
  using namespace Model;

  class IJsonSerializer : public IService
  {
  public:
    virtual QJsonObject ToJson(const IdentityModelPtr model) const = 0;
    virtual IdentityModelPtr ToModel(const QJsonObject& json) const = 0;
  };
}

#endif