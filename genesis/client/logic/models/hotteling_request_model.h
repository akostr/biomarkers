#pragma once
#ifndef HOTTELING_REQUEST_MODEL_H
#define HOTTELING_REQUEST_MODEL_H

#include "logic/models/identity_model.h"

namespace Model
{
  class HottelingRequestModel : public IdentityModel
  {
  public:
    HottelingRequestModel();
    ~HottelingRequestModel() override = default;

    static inline std::string T = "T";
    static inline std::string Count = "Count";
  };
}
#endif