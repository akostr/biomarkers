#pragma once
#ifndef KNOWN_CONTEXT_TAG_NAMES_H
#define KNOWN_CONTEXT_TAG_NAMES_H
#include <QString>
#include <QMap>

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Names
{
  namespace ContextTagNames
  {
    const QString Page = "page";
    const QString PlsJsonResponse = "PlsJsonResponse";
    const QString PlsAnalysisModelPtr= "Pls analysis model ptr";
    const QString ProjectId = "id";
    const QString ProjectStatusIcon = "Project_status_icon";
    const QString ComponentCount = "Component count";
    const QString ToleranceErrorChange = "Tolerance error change";
    const QString Method = "Method";
    const QString MaximumIterations = "Maximum iterations";
    const QString ReferenceSettings = "reference settings";
    const QString ReferenceId = "reference id";
    const QString StepBasedMarkupId = "markup id";
    const QString MasterPlotId = "Master plot id";
    const QString IsClosed = "Is closed system";
    const QString IsNomdata = "Is normadata";
    const QString AnalysisId = "Analysis id";
    const QString MCRData = "MCR data";
    const QString McrResponseJson = "mcr response json";
    const QString PCAData = "PCA data";
    const QString analysis_builded_model = "analysis_builded_model";
    const QString checked_peaks = "checked peaks";
    const QString checked_samples = "checked samples";
    const QString yConcentrations = "Y Concentrations";
    const QString ExcludedPeaks = "Excluded peaks";
    const QString ExcludedSamples = "Excluded samples";
  }

  namespace ModulesContextTags
  {
    enum Module
    {
      MNoModule,
      MReservoir,
      MBiomarkers,
      MPlots,

      MLast
    };

    const QString kModule = "module";
    const QMap<QString, int> kLicenseToModule = {{"licence-reservoir", ModulesContextTags::Module::MReservoir},
                                                 {"licence-biomarkers", ModulesContextTags::Module::MBiomarkers}};
  }

  namespace UserContextTags
  {
    enum UserState
    {
      notAuthorized,
      authorized
    };

    const QString kInitialPageId = "initial_page_id";
    const QString kUserId = "id";
    const QString kUserState = "state";
    const QString kEmail = "email";
    const QString kPassword = "password";
    const QString kLogin = "login";
    const QString kJSonArrGroups = "groups";
    const QString kGroups = "groups_int_arr";
    const QString kCurrentGroup = "current_group";
    const QString kJSonArrLicenses = "licence";
    const QString kAvailableModules = "licenses_int_arr";

    const QString kUsername = "username";
    const QString kSuperuser = "superuser";
  }
  namespace LibraryContextTags
  {
    enum LibraryTab
    {
      kCompoundsTab = 0,
      kCoefficientsTab = 1,
      kTemplatesTab = 2
    };
    const QString kCurrentTabInd = "current_tab_ind";
  }
  namespace MarkupContextTags
  {
    const QString kMarkupId = "id";
    const QString kMarkupTitle = "title";
    const QString kPeakTitleDisplayMode = "peak_title_display_mode";
  }
  namespace MarkupVersionContextTags
  {
    const QString kMarkupVersionId = "id";
  }

  namespace Group
  {
      const QString ImportedTable = "ImportedTable";
      const QString MergedTable = "MergedTable";
      const QString HeigthRatioMatrix = "HeightRatioMatrix";
      const QString DataTable = "DataTable";

      const QString CoefficientTableIdentified = "CoefficientTableIdentified";
      const QString ImportedCoefficientTableIdentified = "ImportedCoefficientTableIdentified";
      const QString DataTableIdentified = "DataTableIdentified";
      const QString ImportedTableIdentified = "ImportedTableIdentified";
      const QString MergedTableIdentified = "MergedTableIdentified";
  }

  namespace CreationPlot
  {
    const QString TableId = "table_id";
    const QString ParameterId = "parameter_id";
    const QString ModelJson = "model_json";
  }
}
#endif //KNOWN_CONTEXT_TAG_NAMES_H
