#pragma once
#ifndef MCR_ANALYIS_IDENTITY_MODEL_H
#define MCR_ANALYIS_IDENTITY_MODEL_H

#include "analysis_identity_model.h"

namespace Model
{
  class McrAnalysisIdentityModel : public AnalysisIdentityModel
  {
    Q_OBJECT
  public:
    McrAnalysisIdentityModel();
    McrAnalysisIdentityModel(std::string&& name);

    static inline std::string CalculatedParameters = "Calculated parameters";
    static inline std::string InputParameters = "Input parameters";
    static inline std::string ConcentrationsTable = "Concentrations table";
    static inline std::string Concentrations = "Concentrations";
    static inline std::string Spectra = "Spectra";
    static inline std::string Difference = "Difference";
    static inline std::string Reconstructions = "Reconstructions";
    static inline std::string Originals = "Originals";
    static inline std::string Pids = "P_id";
  };
}
#endif
