#ifndef FWD_H
#define FWD_H
#include "logic/markup/genesis_markup_enums.h"
#include <QSharedPointer>
namespace GenesisMarkup{

  //classes
  class MarkupDataModel;
  class ChromatogramDataModel;

  //using
  using MarkupModelPtr = QSharedPointer<MarkupDataModel>;
  using ChromatogrammModelPtr = QSharedPointer<ChromatogramDataModel>;

  using ChromaId = int;

  struct IntermarkerInterval
  {
      double window;
      double start;
      double end;
  };

  namespace Defaults
  {//defaults:
    const int    kChromatogrammSettingsBaseLineSmoothLvlDefault      = 2;
    const double kChromatogrammSettingsMinimalPeakHeightDefault      = 10;
    const bool   kChromatogrammSettingsNoisyDefault                  = true;
    const int    kChromatogrammSettingsMedianFilterWindowSizeDefault = 11;
    const double kChromatogrammSettingsSignalToMedianRatioDefault    = 1;
    const int    kChromatogrammSettingsDegreeOfApproximationDefault  = 10;
    const bool   kChromatogrammSettingsBoundsByMedianDefault         = false; // not used anymore in ui
    const int    kChromatogrammSettingsCoelutionDefault              = 99;
    const bool   kChromatogrammSettingsViewSmoothDefault             = false; //median filter for noisy chromatogramm
    const int    kChromatogrammModeDefault                           = CMMarkup;
  }

  struct ChromaSettings
  {
    double Min_h           = Defaults::kChromatogrammSettingsMinimalPeakHeightDefault;
    bool Noisy             = Defaults::kChromatogrammSettingsNoisyDefault;
    bool View_smooth       = Defaults::kChromatogrammSettingsViewSmoothDefault;
    int Window_size        = Defaults::kChromatogrammSettingsMedianFilterWindowSizeDefault;
    double Sign_to_med     = Defaults::kChromatogrammSettingsSignalToMedianRatioDefault;
    int Doug_peuck         = Defaults::kChromatogrammSettingsDegreeOfApproximationDefault;
    bool Med_bounds        = Defaults::kChromatogrammSettingsBoundsByMedianDefault;
    int Coel               = Defaults::kChromatogrammSettingsCoelutionDefault;
    int BLineSmoothLvl = Defaults::kChromatogrammSettingsBaseLineSmoothLvlDefault;
    bool operator==(const ChromaSettings& other) const
    {
      return Min_h == other.Min_h &&
             Noisy == other.Noisy &&
             View_smooth == other.View_smooth &&
             Window_size == other.Window_size &&
             Sign_to_med == other.Sign_to_med &&
             Doug_peuck == other.Doug_peuck &&
             Med_bounds == other.Med_bounds &&
             Coel == other.Coel &&
             BLineSmoothLvl == other.BLineSmoothLvl;
    }
    bool operator!=(const ChromaSettings& other) const {return !((*this)==other);};
  };
  using IntermakrerTransferDialogDataPair = QPair<int, QVector<IntermarkerInterval>>;
}

#endif // FWD_H
