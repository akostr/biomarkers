#pragma once
#ifndef MARKUP_REFERENCE_MODEL_H
#define MARKUP_REFERENCE_MODEL_H

#include "logic/models/identity_model.h"

namespace Model
{
  class MarkupReferenceModel : public IdentityModel
  {
  public:
    MarkupReferenceModel();
    //common_structures.h :: ReferenceList
    static inline std::string ReferenceList = "Reference list";
  };
}
#endif
