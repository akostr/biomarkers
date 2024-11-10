#pragma once
#ifndef HOTTELING_RESPONSE_MODEL_H
#define HOTTELING_RESPONSE_MODEL_H

#include "logic/models/identity_model.h"

namespace Model
{
  class HottelingResponseModel : public IdentityModel
  {
  public:
    HottelingResponseModel();
    ~HottelingResponseModel() override = default;

    static inline std::string HottelingTable = "Hotteling table";
    static inline std::string Headers = "Headers";
    static inline std::string Values = "Values";
    static inline std::string Colors = "Colors";
  };
}

#endif