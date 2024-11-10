#pragma once
#ifndef KNOWN_CONTEXT_NAMES_H
#define KNOWN_CONTEXT_NAMES_H

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()
#include <QString>

namespace GenesisContextNames
{
  const QString Ui = "ui";
  const QString Root = "root";
  const QString User = "user";
  const QString Modules = "modules";
  const QString Project = "project";
  const QString FileCDF = "file_cdf";
  const QString Markup = "markup";
  const QString MarkupVersion = "markup_version";
  const QString MarkupVersionAnalysisPCA = "markup_version_analysis_pca";
  const QString MarkupStepBased = "markup_step_based";
  const QString MarkupVersionAnalysisMCR = "markup_version_analysis_mcr";
  const QString MarkupVersionAnalysisPLS = "markup_version_analysis_pls";
  const QString MarkupVersionAnalysisPLSPredict = "markup_version_analysis_pls_redict";
  const QString MarkupVersionAnalysisId = "markup_version_analysis_id";
  const QString PCAPlots = "pca_plots";
  const QString RatioMatrix = "ratio_matrix";
  const QString Library = "library";
  const QString CreationPlot = "creation_plot";
}
#endif
