#pragma once
#ifndef IDENTITY_MODEL_H
#define IDENTITY_MODEL_H

#include <string>
#include <memory>
#include <qobject>

#include "abstract_model.h"

namespace Model
{
  class IdentityModel : public QObject, public AbstractModel
  {
    Q_OBJECT
  public:
    explicit IdentityModel(const std::string& uid = "");
    IdentityModel(std::string&& uid);

    virtual ~IdentityModel() = default;

    std::string Uid() const;
    void SetUid(const std::string& uid);
    void SetUid(std::string&& uid);

  private:
    std::string uid_;
  };
  using IdentityModelPtr = std::shared_ptr<IdentityModel>;
}

#endif