#pragma once
#ifndef KNOWN_VIEW_NAMES_H
#define KNOWN_VIEW_NAMES_H

#include <QString>
#include <QMap>

namespace ViewPageNames
{
  Q_NAMESPACE
  const QString ViewPageProjectPCAGraphicsPlotPageName = "view_page_project_pca_graphics_plot_page";
  const QString ViewPageProjectPLSGraphicsPlotPageName = "view_page_project_pls_graphics_plot_page";
  const QString ViewPageProjectPlsPredictName = "view_page_project_pls_predict";
  const QString ViewPagePlsModelPageName = "view_page_pls_model_page";
  const QString ViewPageProjectMcrGraphicPlotPageName = "view_page_mcr_analysis_model_page";
  const QString ViewPageProjectMarkupVersionPageName = "view_page_project_markup_version_page";
  const QString ViewPageMcrModelPageName = "view_page_mcr_model_page";
  const QString ViewPageHeightRatioMatrixPageName = "view_page_height_ratio_matrix_page";
  const QString ViewLoginPageName = "view_login";
  const QString ViewLoginCreatePageName = "view_login_create";
  const QString ViewRootPageName = "view_root";
  const QString ViewRootLayoutPagesName = "view_root_layout_pages";
  const QString ViewRootLayoutPagesMenuPageName = "view_root_layout_pages_menu";
  const QString ViewPageModuleSelection = "view_page_module_selection";
  const QString ViewRootLayoutWelcomePageName = "view_root_layout_welcome";
  const QString ViewRootLayoutWelcomeBannersPageName = "view_root_layout_welcome_banners";
  const QString ViewRootToolbarPageName = "view_root_toolbar";
  const QString ViewRootToolbarAccountMenuPageName = "view_root_toolbar_account_menu";
  const QString ViewProjectChildrenPageName = "view_page_project_children";
  const QString ViewProjectDataPageName = "view_page_project_data";
  const QString ViewProjectImportedFilesPageName = "view_page_project_imported_files";
  const QString ViewChromatogrammsFragmentsPageName = "view_page_chromatogramms_fragments";
  const QString ViewProjectMarkupPageName = "view_page_project_markup";
  const QString ViewLibrary = "view_page_library";
  const QString ViewProjectMarkupsListPageName = "view_page_project_markups_list";
  const QString ViewProjectModelsListPageName = "view_page_project_models_list";
  const QString ViewProjectInfoPageName = "view_page_project_info";
  const QString ViewProjectLogPageName = "view_page_project_log";
  const QString ViewProjectsPageName = "view_page_projects";
  const QString ViewIdentificationPageName = "view_page_identification";
  const QString ViewInterpretationPageName = "view_page_interpretation";
  const QString ViewLoginEditPageName = "view_page_login_edit";
  const QString ViewPageAnalysisDataTablePageName = "view_page_analysis_data_table_page";
  const QString ViewPageProjectMarkupName = "view_page_project_markup";
  const QString ViewNumericData = "view_numeric_data";
  const QString ViewPageProfileName = "view_page_profile";
  const QString ViewPageStatisticName = "view_page_statistic";
  const QString ViewPagePcaModelName = "view_page_pca_model";
  const QString ViewPageCreateIdentificationPlotName = "view_page_create_identification_plot";
  const QString ViewPageIdentificationPlotListName = "view_page_identification_plot_list";

  const QMap<QString, const char *> ViewPageTranslatedName =
    {
      {ViewPageProjectPCAGraphicsPlotPageName, QT_TRANSLATE_NOOP("View page names", "PCA plot page")},
      {ViewPagePcaModelName, QT_TRANSLATE_NOOP("View page pca model", "PCA model page")},
      {ViewPageProjectPLSGraphicsPlotPageName, QT_TRANSLATE_NOOP("View page names", "PLS plot page")},
      {ViewPageProjectPlsPredictName, QT_TRANSLATE_NOOP("View page names", "PLS predict page")},
      {ViewPagePlsModelPageName, QT_TRANSLATE_NOOP("View page names", "PLS model page")},
      {ViewPageProjectMcrGraphicPlotPageName, QT_TRANSLATE_NOOP("View page names", "MCR plot page")},
      {ViewPageProjectMarkupVersionPageName, QT_TRANSLATE_NOOP("View page names", "Markup version page")},
      {ViewPageMcrModelPageName, QT_TRANSLATE_NOOP("View page names", "MCR model page")},
      {ViewPageHeightRatioMatrixPageName, QT_TRANSLATE_NOOP("View page names", "Height-ration matrix page")},
      {ViewLoginPageName, QT_TRANSLATE_NOOP("View page names", "Login page")},
      {ViewLoginCreatePageName, QT_TRANSLATE_NOOP("View page names", "User creation page")},
      {ViewRootPageName, QT_TRANSLATE_NOOP("View page names", "Root page")},
      {ViewRootLayoutPagesName, QT_TRANSLATE_NOOP("View page names", "Pages layout")},
      {ViewRootLayoutPagesMenuPageName, QT_TRANSLATE_NOOP("View page names", "Pages menu")},
      {ViewRootLayoutWelcomePageName, QT_TRANSLATE_NOOP("View page names", "Welcome page")},
      {ViewRootLayoutWelcomeBannersPageName, QT_TRANSLATE_NOOP("View page names", "Welcome banners")},
      {ViewRootToolbarPageName, QT_TRANSLATE_NOOP("View page names", "Toolbar")},
      {ViewRootToolbarAccountMenuPageName, QT_TRANSLATE_NOOP("View page names", "Toolbar account menu")},
      {ViewProjectChildrenPageName, QT_TRANSLATE_NOOP("View pagde names", "Children project page")},
      {ViewProjectDataPageName, QT_TRANSLATE_NOOP("View page names", "Project data page")},
      {ViewProjectImportedFilesPageName, QT_TRANSLATE_NOOP("View page names", "Imported files page")},
      {ViewChromatogrammsFragmentsPageName, QT_TRANSLATE_NOOP("View page names", "Chromatogramms and fragments page")},
      {ViewProjectMarkupPageName, QT_TRANSLATE_NOOP("View page names", "Markup page")},
      {ViewLibrary, QT_TRANSLATE_NOOP("View page names", "Library page")},
      {ViewProjectMarkupsListPageName, QT_TRANSLATE_NOOP("View page names", "Markups list page")},
      {ViewProjectModelsListPageName, QT_TRANSLATE_NOOP("View page names", "Models list page")},
      {ViewProjectInfoPageName, QT_TRANSLATE_NOOP("View page names", "Project info page")},
      {ViewProjectLogPageName, QT_TRANSLATE_NOOP("View page names", "Log page")},
      {ViewProjectsPageName, QT_TRANSLATE_NOOP("View page names", "Projects list page")},
      {ViewPageProfileName, QT_TRANSLATE_NOOP("View page names", "Profile page")},
      {ViewPageStatisticName, QT_TRANSLATE_NOOP("View page names", "Statistic page")},
      {ViewIdentificationPageName, QT_TRANSLATE_NOOP("View page names", "Identification page")},
      {ViewInterpretationPageName, QT_TRANSLATE_NOOP("View page names", "Interpretation page")},
      {ViewLoginEditPageName, QT_TRANSLATE_NOOP("View page names", "User edit page")},
      {ViewPageAnalysisDataTablePageName, QT_TRANSLATE_NOOP("View page names", "Analysis data table page")},
      {ViewPageProjectMarkupName, QT_TRANSLATE_NOOP("View page names", "Markup name page")},
      {ViewNumericData, QT_TRANSLATE_NOOP("View page names", "Numeric data page")},
    {ViewPageCreateIdentificationPlotName, QT_TRANSLATE_NOOP("View page names", "Create plot page")},
    {ViewPageIdentificationPlotListName, QT_TRANSLATE_NOOP("View page names", "Plots page")}
  };

}

#endif
