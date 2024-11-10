#pragma once
#ifndef PLS_MODEL_STRUCTURES_H
#define PLS_MODEL_STRUCTURES_H

#include <string>
#include <vector>

#include "logic/enums.h"

namespace Structures
{
  struct PlsModel
  {
    size_t Id; // _analysis_id
    size_t TableId;
    size_t PCCount;
    size_t PeaksCount; // _peaks_count
    size_t SamplesCount; // _samples_count
    size_t ParentId; // _parent_id
    size_t VersionId; // _version_id
    Constants::AnalysisType Type; // _analysis_type_id
    std::string TypeName; // _analysis_type
    std::string Name; // _analysis_title
    std::string TableTitle;
    std::string Comments; // _analysis_comment
    std::string Author; // _user
    std::string CreateDate; // _created
    std::string VersionTitle; // _version_title
    std::string ParentTitle; // _parent_title
  };
  using PlsModels = std::vector<PlsModel>;
}
#endif