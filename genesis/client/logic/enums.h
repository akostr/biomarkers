#ifndef ENUMS_H
#define ENUMS_H

#include <QString>

namespace Import
{
  enum ImportedFilesType
  {
    Chromatogramms,
    Fragments,

    FilesTypeLast
  };

  enum class XlsFileImportedType
  {
    Simple,
    Custom,
  };
}

namespace ProjectFiles
{
  enum FileType
  {
    invalid = 0,
    tic = 1,//chromatogramm, from wich the fragment extracted
    sim = 2,//fragment
    GC_MS = 3,//chromatogramm with fragments
    GC = 4 //chromatogramm without fragments
  };
}

namespace Constants
{
  enum class AnalysisType
  {
    NONE = 0,
    PCA, // МГК
    MCR, // МРК
    PLS, // PLC or PLS??
    PLSPredict
  };
}

namespace LibraryConnection
{
  enum class ElementType
  {
    COMPOUND = 1,
    COEFFICIENT,
    PLOT_TEMPLATE
  };
}

namespace LibraryFilter
{
  enum class FileType
  {
    All,
    System,
    User,
    Favorite
  };

//GET /api/v1/library/classifier
//GET /api/v1/library/classifier_compound_class
//GET /api/v1/library/specifics
  enum class Classifiers
  {
    All,
    GC_PID,
    GC_MS,
    GasComposition,
    Pyrolysis,
    Isotopy,
    WaterComposition
  };

  enum class CompoundClasses
  {
    Saturated,
    Aromatic
  };

  enum class Specifics
  {
    All,
    Genesis,
    Maturity,
    Biodegradation,
    SedimentationConditions,
    NotSpecified
  };
}

namespace TemplatePlot
{
  enum class PlotType
  {
    Orthogonal = 1,
    Triplot
  };
}
#endif // ENUMS_H
