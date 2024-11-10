#include "identity_model.h"

namespace Model
{
  IdentityModel::IdentityModel(const std::string& uid)
    : uid_(uid)
  {
  }

  IdentityModel::IdentityModel(std::string&& uid)
    : uid_(std::forward<std::string>(uid))
  {
  }

  std::string IdentityModel::Uid() const
  {
    return uid_;
  }

  void IdentityModel::SetUid(const std::string& uid)
  {
    uid_ = uid;
  }

  void IdentityModel::SetUid(std::string&& uid)
  {
    uid_ = std::forward<std::string>(uid);
  }
}
