#include "uom.h"

#include <QObject>

#define _USE_MATH_DEFINES
#include <math.h>

#pragma warning(1 : 4061)
#pragma warning(1 : 4062)

namespace UOM
{
  ///////////////////////////////////////////////////////////
  //// Semantic keys
  const char* KeySemanticsNone                        = "SemanticsNone";

  //// Lengths
  const char* KeySemanticsLength                      = "SemanticsLength";                        // Generic
  const char* KeySemanticsDepth                       = "SemanticsDepth";
  const char* KeySemanticsInclinometryMeasure         = KeySemanticsDepth;                        // == KeySemanticsDepth, alias
  const char* KeySemanticsPipeDiameter                = "SemanticsPipeDiameter";
  const char* KeySemanticsPerforationDiameter         = "SemanticsPerforationDiameter";
  const char* KeySemanticsProppantDiameter            = KeySemanticsPerforationDiameter;          // == KeySemanticsPerforationDiameter, alias
  const char* KeySemanticsFractureWidth               = "SemanticsFractureWidth";
  const char* KeySemanticsFractureLength              = "SemanticsFractureLength";

  //// Pressures
  const char* KeySemanticsPressure                    = "SemanticsPressure";                      // Generic
  const char* KeySemanticsYoungModulus                = "SemanticsYoungModulus";
  const char* KeySemanticsStressShiftModulus          = KeySemanticsYoungModulus;                 // == KeySemanticsYoungModulus, alias
  const char* KeySemanticsStress                      = "SemanticsStress";
  const char* KeySemanticsPorePressure                = "SemanticsPorePressure";
  const char* KeySemanticsPipePressure                = "SemanticsPipePressure";

  //// Times
  const char* KeySemanticsTime                        = "SemanticsTime";                          // Generic
  const char* KeySemanticsPumpingTime                 = KeySemanticsTime;                         // == KeySemanticsTime, alias
  const char* KeySemanticsRheologicalTime             = KeySemanticsTime;                         // == KeySemanticsTime, alias

  //// Period
  const char* KeySemanticsPeriod                      = "SemanticsPeriod";                        // Generic
  const char* KeySemanticsPeriodNPVProduction         = "SemanticsPeriodNPVProduction";
  const char* KeySemanticsPeriodNPVPrices             = "SemanticsPeriodNPVPrices";
  const char* KeySemanticsPeriodNPVUtilization        = "SemanticsPeriodNPVUtilization";

  //// Production Over Period
  const char* KeySemanticsProductionOverPeriod        = "SemanticsProductionOverPeriod";          // Generic

  //// Pressures per lengths
  const char* KeySemanticsPressureOverLength          = "SemanticsPressureOverLength";            // Generic
  const char* KeySemanticsStressGradient              = "SemanticsStressGradient";
  const char* KeySemanticsPorePressureGradient        = "SemanticsPorePressureGradient";
  const char* KeySemanticsFluidLinearPressure         = "SemanticsFluidLinearPressure";

  //// Masses over volume
  const char* KeySemanticsSpecificGravity             = "SemanticsSpecificGravity";               // Generic
  
  //// Concentrations
  const char* KeySemanticsConcentrationMass           = "SemanticsConcentrationMass";
  const char* KeySemanticsConcentrationVolume         = "SemanticsConcentrationVolume";

  //// Measureless
  const char* KeySemanticsMeasureless                 = "SemanticsMeasureless";                   // Generic
  const char* KeySemanticsMeasurelessSilent           = "SemanticsMeasurelessSilent";             // Generic limited only to MeasurelessGeneric, e.i. w/ null suffix
  const char* KeySemanticsMeasurelessUnitFraction     = "SemanticsMeasurelessUnitFraction";
  const char* KeySemanticsMeasurelessEfficiency       = "SemanticsMeasurelessEfficiency";
  const char* KeySemanticsMeasurelessPorosity         = "SemanticsMeasurelessPorosity";

  //// Energy over temperature
  const char* KeySemanticsEnergyOverTemperature       = "SemanticsEnergyOverTemperature";         // Generic
  const char* KeySemanticsHeatCapacity                = "SemanticsHeatCapacity";

  //// Energy over mass temperature
  const char* KeySemanticsEnergyOverMassTemperature   = "SemanticsEnergyOverMassTemperature";     // Generic
  const char* KeySemanticsMassSpecificHeatCapacity    = "SemanticsMassSpecificHeatCapacity";

  const char* KeySemanticsReciprocalLength            = "SemanticsyReciprocalLength";             // Generic
  const char* KeySemanticsForhheimerCoefficient       = "SemanticsForhheimerCoefficient";

  //// Plain ratios 
  const char* KeySemanticsPlainRatio                  = "SemanticsPlainRatio";                    // Generic
  const char* KeySemanticsPlainRatioVolume            = "SemanticsPlainRatioVolume";
  const char* KeySemanticsPlainRatioStandardVolume    = "SemanticsPlainRatioStandardVolume";
  const char* KeySemanticsPlainRatioReservoirVolume   = "SemanticsPlainRatioReservoirVolume";

  //// One-to-one semantics to quantity mapping (generics)
  const char* KeySemanticsToughness                       = "SemanticsToughness";
  const char* KeySemanticsLeakoffCoefficient              = "SemanticsLeakoffCoefficient";
  const char* KeySemanticsSpurtLossCoefficient            = "SemanticsSpurtLossCoefficient";
  const char* KeySemanticsTemperature                     = "SemanticsTemperature";
  const char* KeySemanticsThermalConductivity             = "SemanticsThermalConductivity";
  const char* KeySemanticsPermeability                    = "SemanticsPermeability";

  const char* KeySemanticsMass                            = "SemanticsMass";
  const char* KeySemanticsVolume                          = "SemanticsVolume";
  const char* KeySemanticsSpecificGravityOverLength       = "SemanticsSpecificGravityOverLength";
  const char* KeySemanticsSpecificGravityOverArea         = "SemanticsSpecificGravityOverArea";
  const char* KeySemanticsCompressibility                 = "SemanticsCompressibility";
  const char* KeySemanticsMassRate                        = "SemanticsMassRate";
  const char* KeySemanticsVolumeRate                      = "SemanticsVolumeRate";
  const char* KeySemanticsConsistency                     = "SemanticsConsistency";
  const char* KeySemanticsViscosity                       = "SemanticsViscosity";
  const char* KeySemanticsAngle                           = "SemanticsAngle";
  const char* KeySemanticsConductivity                    = "SemanticsConductivity";
  const char* KeySemanticsWaveTravelTime                  = "SemanticsWaveTravelTime";
  const char* KeySemanticsGammaRadiation                  = "SemanticsGammaRadiation";
  const char* KeySemanticsSignalIntensity                 = "SemanticsSignalIntensity";
  const char* KeySemanticsCarterLeaks                     = "SemanticsCarterLeaks";
  const char* KeySemanticsPotentialDifference             = "SemanticsPotentialDifference";
  const char* KeySemanticsSpecificElectricalConductivity  = "SemanticsSpecificElectricalConductivity";
  const char* KeySemanticsResistivity                     = "SemanticsResistivity";
  const char* KeySemanticsFrequency                       = "SemanticsFrequency";
  const char* KeySemanticsPieces                          = "SemanticsPieces";

  ///////////////////////////////////////////////////////////
  //// Named keys
  const char* KeyUOM          = "UOM";
  const char* KeyQuantities   = "Quantities";
  const char* KeyUnitsUI      = "UnitsUI";
  const char* KeyUnitsStored  = "UnitsStored";
  const char* KeySemantics    = "Semantics";

  QMap<PhysicalQuantity, QString> KeysQuantities;
  QMap<QString, PhysicalQuantity> KeysQuantitiesBackwards;
  QMap<QPair<int, int>, QString>  KeysUnits;
  QMap<QString, int>              KeysUnitsBackwards;

  QMap<QString, QString>          SemanticsDisplayNames;
  QMap<QString, QSet<int>>        SemanticsAcceptableUiUnits;

//  QMap<Descriptor/*derived uom*/, std::pair<Descriptor/*numerator*/, Descriptor/*denominator*/>> DerivedUOMS =
//  {
//    //// Specific Gravity
//    {
//      Descriptor({ UOM::PhysicalQuantitySpecificGravity,
//                   UOM::SpecificGravityKilogramOverMeterCubed }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassKilogram }),
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeMeterCubed })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantitySpecificGravity,
//                   UOM::SpecificGravityPoundMassOverFootCubed }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassPoundMass }),
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeFootCubed })
//      }
//    },
//    /* @todo add gramm,
//       @todo add cm3
//    {
//      Descriptor({ UOM::PhysicalQuantitySpecificGravity,
//                   UOM::SpecificGravityGrammOverCentimeterCubed }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM:: }),
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM:: })
//      }
//    },
//    */
//    {
//      Descriptor({ UOM::PhysicalQuantitySpecificGravity,
//                   UOM::SpecificGravityTonneOverMeterCubed }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassTonne }),
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeMeterCubed })
//      }
//    },

//    //// Specific Gravity Over Length
//    {
//      Descriptor({ UOM::PhysicalQuantitySpecificGravityOverLength,
//                   UOM::SpecificGravityOverLengthKilogramOverMeter }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassKilogram }),
//        Descriptor({ UOM::PhysicalQuantityLength, UOM::LengthMeter })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantitySpecificGravityOverLength,
//                   UOM::SpecificGravityOverLengthPoundOverFoot}),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassPoundMass }),
//        Descriptor({ UOM::PhysicalQuantityLength, UOM::LengthFoot })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantitySpecificGravityOverLength,
//                   UOM::SpecificGravityOverLengthTonneOverMeter}),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassTonne }),
//        Descriptor({ UOM::PhysicalQuantityLength, UOM::LengthMeter })
//      }
//    },

//    //// Mass Rate
//    {
//      Descriptor({ UOM::PhysicalQuantityMassRate,
//                   UOM::MassRateKilogramOverSecond }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassKilogram }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeSeconds })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityMassRate,
//                   UOM::MassRateKilogramOverMinute }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassKilogram }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeMinutes })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityMassRate,
//                   UOM::MassRateTonneOverMinute }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassTonne }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeMinutes })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityMassRate,
//                   UOM::MassRatePoundMassOverSecond }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassPoundMass }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeSeconds })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityMassRate,
//                   UOM::MassRatePoundMassOverMinute }),
//      {
//        Descriptor({ UOM::PhysicalQuantityMass,   UOM::MassPoundMass }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeMinutes })
//      }
//    },

//    //// Volume Rate
//    {
//      Descriptor({ UOM::PhysicalQuantityVolumeRate,
//                   UOM::VolumeRateMeterCubedOverSecond }),
//      {
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeMeterCubed }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeSeconds })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityVolumeRate,
//                   UOM::VolumeRateMeterCubedOverMinute }),
//      {
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeMeterCubed }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeMinutes })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityVolumeRate,
//                   UOM::VolumeRateMeterCubedOverDay }),
//      {
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeMeterCubed }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeDays })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityVolumeRate,
//                   UOM::VolumeRateLitreOverSecond }),
//      {
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeLitre }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeSeconds })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityVolumeRate,
//                   UOM::VolumeRateLitreOverMinute }),
//      {
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeLitre }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeMinutes })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityVolumeRate,
//                   UOM::VolumeRateBarrelOverMinute }),
//      {
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeBarrel }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeMinutes })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityVolumeRate,
//                   UOM::VolumeRateFootCubedOverSecond }),
//      {
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeFootCubed }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeSeconds })
//      }
//    },
//    {
//      Descriptor({ UOM::PhysicalQuantityVolumeRate,
//                   UOM::VolumeRateFootCubedOverMinute }),
//      {
//        Descriptor({ UOM::PhysicalQuantityVolume, UOM::VolumeFootCubed }),
//        Descriptor({ UOM::PhysicalQuantityTime,   UOM::TimeMinutes })
//      }
//    },
      
//  };

  QMap<QPair<Descriptor/*numerator*/, Descriptor/*denominator*/>, Descriptor/*derived uom*/> DerivedUOMSBackwards;

  QMap<PhysicalQuantity/*derived*/, QPair<PhysicalQuantity/*numerator*/, PhysicalQuantity/*denominator*/>> DerivedUOMPhysicalQuantities;

  //// Quantities / Units helpers
  QString GetPhysicalQuantityDisplayName(PhysicalQuantity quantity)
  {
    switch (quantity)
    {
    case UOM::PhysicalQuantityNone:
      break;
    case UOM::PhysicalQuantityLength:
      return QObject::tr("Length");
    case UOM::PhysicalQuantityPressure:
      return QObject::tr("Pressure");
    case UOM::PhysicalQuantityPressureOverLength:
      return QObject::tr("Pressure over length");
    case UOM::PhysicalQuantityToughness:
      return QObject::tr("Toughness");
    case UOM::PhysicalQuantityLeakoffCoefficient:
      return QObject::tr("Leakoff coefficient");
    case UOM::PhysicalQuantitySpurtLossCoefficient:
      return QObject::tr("Spurt loss coefficient");
    case UOM::PhysicalQuantityTemperature:
      return QObject::tr("Temperature");
    case UOM::PhysicalQuantityEnergyOverTemperature:
      return QObject::tr("Energy over temperature");
    case UOM::PhysicalQuantityEnergyOverMassTemperature:
      return QObject::tr("Enegry over mass temperature");
    case UOM::PhysicalQuantityThermalConductivity:
      return QObject::tr("Thermal conductivity");
    case UOM::PhysicalQuantityPermeability:
      return QObject::tr("Permeability");
    case UOM::PhysicalQuantityTime:
      return QObject::tr("Time");
    case UOM::PhysicalQuantityPeriod:
      return QObject::tr("Period");
    case UOM::PhysicalQuantityMass:
      return QObject::tr("Mass");
    case UOM::PhysicalQuantityVolume:
      return QObject::tr("Volume");
    case UOM::PhysicalQuantitySpecificGravity:
      return QObject::tr("Specific gravity");
    case UOM::PhysicalQuantitySpecificGravityOverLength:
      return QObject::tr("Specific gravity over length");
    case UOM::PhysicalQuantityMassRate:
      return QObject::tr("Mass rate");
    case UOM::PhysicalQuantityVolumeRate:
      return QObject::tr("Volume rate");
    case UOM::PhysicalQuantityConsistency:
      return QObject::tr("Consistemcy");
    case UOM::PhysicalQuantityViscosity:
      return QObject::tr("Viscosity");
    case UOM::PhysicalQuantityAngle:
      return QObject::tr("Angle");
    case UOM::PhysicalQuantityConductivity:
      return QObject::tr("Conductivity");
    case UOM::PhysicalQuantityMeasureless:
      return QObject::tr("Measureless");
    case UOM::PhysicalQuantitySpecificGravityOverArea:
      return QObject::tr("Specific gravity over area");
    case UOM::PhysicalQuantityCompressibility:
      return QObject::tr("Compressibility");
    case UOM::PhysicalQuantityReciprocalLength:
      return QObject::tr("Reciprocal length");
    case UOM::PhysicalQuantityPlainRatio:
      return QObject::tr("Ratio");
    case UOM::PhysicalQuantityProductionOverPeriod:
      return QObject::tr("Production over period");
    case UOM::PhysicalQuantityConcentration:
      return QObject::tr("Concentration");
    case UOM::PhysicalQuantityWaveTravelTime:
      return QObject::tr("Wave travel time");
    case UOM::PhysicalQuantityGammaRadiation:
      return QObject::tr("Gamma radiation");
    case UOM::PhysicalQuantitySignalIntensity:
      return QObject::tr("Signal intensity");
    case UOM::PhysicalQuantityCarterLeaks:
      return QObject::tr("Carter Leaks");
    case UOM::PhysicalQuantityPotentialDifference:
      return QObject::tr("Potential difference");
    case UOM::PhysicalQuantitySpecificElectricalConductivity:
      return QObject::tr("Specific electrical conductivity");
    case UOM::PhysicalQuantityResistivity:
      return QObject::tr("Resistivity");
    case UOM::PhysicalQuantityFrequency:
      return QObject::tr("Frequency");
    case UOM::PhysicalQuantityPieces:
      return QObject::tr("Pieces");
    case UOM::PhysicalQuantityLast:
      break;
    }
    return QString();
  }

  QString GetUnitsDisplayName(PhysicalQuantity quantity, int units)
  {
    switch (quantity)
    {
    case UOM::PhysicalQuantityNone:
      return QString();

    case UOM::PhysicalQuantityLength:
      switch ((Length)units)
      {
      case LengthNone:
        break;
      case LengthMeter:
        return QObject::tr("m");
      case LengthMillimiter:
        return QObject::tr("mm");
      case LengthFoot:
        return QObject::tr("ft");
      case LengthInch:
        return QObject::tr("in");
      case LengthLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityPressure:
      switch ((Pressure)units)
      { 
      case PressureNone:
        break;
      case PressurePascal:
        return QObject::tr("Pa");
      case PressureKiloPascal:
        return QObject::tr("kPa");
      case PressureMegaPascal:
        return QObject::tr("MPa"); 
      case PressureGigaPascal:
        return QObject::tr("GPa");
      case PressurePSI:
        return QObject::tr("psi");
      case PressureAtmosphere:
        return QObject::tr("atm");
      case PressureBar:
        return QObject::tr("bar");
      case PressureLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityPressureOverLength:
      switch ((PressureOverLength)units)
      {
      case PressureOverLengthNone:
        break;
      case PressureOverLengthPascalOverMeter:
        return QObject::tr("Pa/m");
      case PressureOverLengthKiloPascalOverMeter:
        return QObject::tr("kPa/m");
      case PressureOverLengthMegaPascalOverMeter:
        return QObject::tr("MPa/m");
      case PressureOverLengthGigaPascalOverMeter:
        return QObject::tr("GPa/m");
      case PressureOverLengthAtmosphereOverMeter:
        return QObject::tr("atm/m");
      case PressureOverLengthBarOverMeter:
        return QObject::tr("bar/m");
      case PressureOverLengthPSIOverMeter:
        return QObject::tr("psi/m");
      case PressureOverLengthPSIOverFoot:
        return QObject::tr("psi/ft");
      case PressureOverLengthBarOverFoot:
        return QObject::tr("bar/ft");
      case PressureOverLengthPascalOverFoot:
        return QObject::tr("Pa/ft");
      case PressureOverLengthLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityToughness:
      switch ((Toughness)units)
      {
      case ToughnessNone:
        break;
      case ToughnessAtmosphereMeterToTheOneHalf:
        return QObject::tr("atm*m<sup>0.5</sup>");
      case ToughnessPascalMeterToTheOneHalf:
        return QObject::tr("Pa*m<sup>0.5</sup>");
      case ToughnessKiloPascalMeterToTheOneHalf:
        return QObject::tr("kPa*m<sup>0.5</sup>");
      case ToughnessMegaPascalMeterToTheOneHalf:
        return QObject::tr("MPa*m<sup>0.5</sup>");
      case ToughnessLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityLeakoffCoefficient:
      switch ((LeakoffCoefficient)units)
      {
      case LeakoffCoefficientNone:
        break;
      case LeakoffCoefficientMeterOverSecondToTheOneHalf:
        return QObject::tr("m/sec<sup>0.5</sup>");
      case LeakoffCoefficientMeterOverMinuteToTheOneHalf:
        return QObject::tr("m/min<sup>0.5</sup>");
      case LeakoffCoefficientCentiMeterOverSecondToTheOneHalf:
        return QObject::tr("cm/sec<sup>0.5</sup>");
      case LeakoffCoefficientCentiMeterOverMinuteToTheOneHalf:
        return QObject::tr("cm/min<sup>0.5</sup>");
      case LeakoffCoefficientFootOverSecondToTheOneHalf:
        return QObject::tr("ft/min<sup>0.5</sup>");
      case LeakoffCoefficientFootOverMinuteToTheOneHalf:
        return QObject::tr("ft/sec<sup>0.5</sup>");
      case LeakoffCoefficientLast:
        break;
      }
      break;
    case UOM::PhysicalQuantitySpurtLossCoefficient:
      switch ((SpurtLossCoefficient)units)
      {
      case SpurtLossCoefficientNone:
        break;
      case SpurtLossCoefficientMeterCubedOverMeterSquared:
        return QObject::tr("m<sup>3</sup>/m<sup>2</sup>");
      case SpurtLossCoefficientGallonOverFootSquared:
        return QObject::tr("gal/ft<sup>2</sup>");
      case SpurtLossCoefficientLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityTemperature:
      switch ((Temperature)units)
      {
      case TemperatureNone:
        break;
      case TemperatureCelsius:
        return QObject::tr("°C");
      case TemperatureKelvin:
        return QObject::tr("K");
      case TemperatureLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityEnergyOverTemperature:
      switch ((EnergyOverTemperature)units)
      {
      case EnergyOverTemperatureNone:
        break;
      case EnergyOverTemperatureJouleOverCelsius:
        return QObject::tr("J/°C");
      case EnergyOverTemperatureLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityEnergyOverMassTemperature:
      switch ((EnergyOverMassTemperature)units)
      {
      case EnergyOverMassTemperatureNone:
        break;
      case EnergyOverMassTemperatureJouleOverKilogramCelsius:
        return QObject::tr("J/(kg*°C)");
      case EnergyOverMassTemperatureLast:
        break;
      }
      break;    
    case UOM::PhysicalQuantityThermalConductivity:
      switch ((ThermalConductivity)units)
      {
      case ThermalConductivityNone:
        break;
      case ThermalConductivityWattOverMeterCelsius:
        return QObject::tr("W/(m*°C)");
      case ThermalConductivityLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityPermeability:
      switch ((Permeability)units)
      {
      case PermeabilityNone:
        break;
      case PermeabilityMilliDarcy:
        return QObject::tr("md");
      case PermeabilityLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityTime:
      switch ((Time)units)
      {
      case UOM::TimeNone:
        break;
      case UOM::TimeMilliSeconds:
        return QObject::tr("msec");
      case UOM::TimeSeconds:
        return QObject::tr("sec");
      case UOM::TimeMinutes:
        return QObject::tr("min");
      case UOM::TimeHours:
        return QObject::tr("h");
      case UOM::TimeDays:
        return QObject::tr("d");
      case UOM::TimeLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityPeriod:
      switch ((Period)units)
      {
      case UOM::PeriodNone:
        break;
      case UOM::PeriodDay:
        return QObject::tr("day");
      case UOM::PeriodMonth:
        return QObject::tr("month");
      case UOM::PeriodYear:
        return QObject::tr("year");
      case UOM::PeriodLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityMass:
      switch ((Mass)units)
      {
      case UOM::MassNone:
        break;
      case UOM::MassKilogram:
        return QObject::tr("kg");
      case UOM::MassTonne:
        return QObject::tr("t");
      case UOM::MassPoundMass:
        return QObject::tr("lb");
      case UOM::MassLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityVolume:
      switch ((Volume)units)
      {
      case UOM::VolumeNone:
        break;
      case UOM::VolumeMeterCubed:
        return QObject::tr("m<sup>3</sup>");
      case UOM::VolumeLitre:
        return QObject::tr("l");
      case UOM::VolumeFootCubed:
        return QObject::tr("ft<sup>3</sup>");
      case UOM::VolumeBarrel:
        return QObject::tr("bbls");
      case UOM::VolumeLast:
        break;
      }
      break;
    case UOM::PhysicalQuantitySpecificGravity:
      switch ((SpecificGravity)units)
      {
      case UOM::SpecificGravityNone:
        break;
      case UOM::SpecificGravityKilogramOverMeterCubed:
        return QObject::tr("kg/m<sup>3</sup>");
      case UOM::SpecificGravityPoundMassOverFootCubed:
        return QObject::tr("lb/ft<sup>3</sup>");
      case UOM::SpecificGravityGrammOverCentimeterCubed:
        return QObject::tr("g/cm<sup>3</sup>");
      case UOM::SpecificGravityTonneOverMeterCubed:
        return QObject::tr("t/m<sup>3</sup>");
      case UOM::SpecificGravityLast:
        break;
      }
      break;
    case UOM::PhysicalQuantitySpecificGravityOverLength:
      switch ((SpecificGravityOverLength)units)
      {
      case UOM::SpecificGravityOverLengthNone:
        break;
      case UOM::SpecificGravityOverLengthKilogramOverMeter:
        return QObject::tr("kg/m");
      case UOM::SpecificGravityOverLengthPoundOverFoot:
        return QObject::tr("lb/ft");
      case UOM::SpecificGravityOverLengthTonneOverMeter:
        return QObject::tr("t/m");
      case UOM::SpecificGravityOverLengthLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityMassRate:
      switch ((MassRate)units)
      {
      case UOM::MassRateNone:
        break;
      case UOM::MassRateKilogramOverSecond:
        return QObject::tr("kg/s");
      case UOM::MassRateKilogramOverMinute:
        return QObject::tr("kg/min");
      case UOM::MassRateTonneOverMinute:
        return QObject::tr("t/min");
      case UOM::MassRatePoundMassOverSecond:
        return QObject::tr("lb/s");
      case UOM::MassRatePoundMassOverMinute:
        return QObject::tr("lb/min");
      case UOM::MassRateLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityVolumeRate:
      switch ((VolumeRate)units)
      {
      case UOM::VolumeRateNone:
        break;
      case UOM::VolumeRateMeterCubedOverSecond:
        return QObject::tr("m<sup>3</sup>/s");
      case UOM::VolumeRateMeterCubedOverMinute:
        return QObject::tr("m<sup>3</sup>/min");
      case UOM::VolumeRateMeterCubedOverDay:
        return QObject::tr("m<sup>3</sup>/day");
      case UOM::VolumeRateLitreOverSecond:
        return QObject::tr("L/s");
      case UOM::VolumeRateLitreOverMinute:
        return QObject::tr("L/min");
      case UOM::VolumeRateBarrelOverMinute:
        return QObject::tr("bpm");
      case UOM::VolumeRateFootCubedOverSecond:
        return QObject::tr("ft<sup>3</sup>/s");
      case UOM::VolumeRateFootCubedOverMinute:
        return QObject::tr("ft<sup>3</sup>/min");
      case UOM::VolumeRateLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityConsistency:
      switch ((Consistency)units)
      {
      case UOM::ConsistencyNone:
        break;
      case UOM::ConsistencyPascalSecondToTheN:
        return QObject::tr("Pa*sec<sup>n</sup>");
      case UOM::ConsistencyPoundForceSecondToTheNOverFootSquared:
        return QObject::tr("lbf*sec<sup>n</sup>/ft<sup>2</sup>");
      case UOM::ConsistencyLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityViscosity:
      switch ((Viscosity)units)
      {
      case UOM::ViscosityNone:
        break;
      case UOM::ViscosityPascalSecond:
        return QObject::tr("Pa*sec");
      case UOM::ViscosityPoise:
        return QObject::tr("P");
      case UOM::ViscosityCentiPoise:
        return QObject::tr("cP");
      case UOM::ViscosityLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityAngle:
      switch ((Angle)units)
      {
      case UOM::AngleNone:
        break;
      case UOM::AngleRadians:
        return QObject::tr("rad");
      case UOM::AngleDegrees:
        return QObject::tr("deg");
      case UOM::AngleLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityConductivity:
      switch ((Conductivity)units)
      {
      case UOM::ConductivityNone:
        break;
      case UOM::ConductivityMilliDarcyMeter:
        return QObject::tr("mD*m");
      case UOM::ConductivityLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityMeasureless:
      switch ((Measureless)units)
      {
      case UOM::MeasurelessNone:
        break;

      case UOM::MeasurelessGeneric:
        return "";

      case UOM::MeasurelessUnitFraction:
        return QObject::tr("u.f.");

      case UOM::MeasurelessPercent:
        return QObject::tr("%");

      case UOM::MeasurelessLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityConcentration:
      switch ((Concentration)units)
      {
      case ConcentrationNone:
        break;
      case UOM::ConcentrationLitreOverMeterCubed:
          return QObject::tr("L/m<sup>3</sup>");
      case UOM::ConcentrationCoefficient:
          return QString("");

      case UOM::ConcentrationLast:
        break;
      }

    case UOM::PhysicalQuantitySpecificGravityOverArea:
      switch ((SpecificGravityOverArea)units)
      {
      case SpecificGravityOverAreaNone:
        break;

      case SpecificGravityOverAreaKilogramOverMeterSquared:
        return QObject::tr("kg/m<sup>2</sup>");

      case SpecificGravityOverAreaLast:
        break;

      default:
        break;
      }
      break;
    case UOM::PhysicalQuantityCompressibility:
      switch ((Compressibility)units)
      {
      case CompressibilityNone:
          break;

      case CompressibilityPascalToTheMinusOne:
        return QObject::tr("Pa<sup>-1</sup>");

      case CompressibilityBarToTheMinusOne:
        return QObject::tr("bar<sup>-1</sup>");

      case CompressibilityLast:
        break;

      default:
        break;
      }
      break;
    case UOM::PhysicalQuantityReciprocalLength:
      {
        switch ((ReciprocalLength)units)
        {
        case ReciprocalLengthNone:
          break;

        case ReciprocalLengthOverMeter:
          return QObject::tr("m<sup>-1</sup>");
        case ReciprocalLengthOverFoot:
          return QObject::tr("ft<sup>-1</sup>");

        case ReciprocalLengthLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantityPlainRatio:
      {
        switch ((UOM::PlainRatio)units)
        {
        case PlainRatioNone:
          break;

        case PlainRatioGeneric:
          return "";

        case PlainRatioVolumeMeterCubed:
          return QObject::tr("m<sup>3</sup>/m<sup>3</sup>");
        case PlainRatioVolumeFootCubed:
          return QObject::tr("ft<sup>3</sup>/ft<sup>3</sup>");

        case PlainRatioVolumeStandardMeterCubed:
          return QObject::tr("Sm<sup>3</sup>/Sm<sup>3</sup>");
        case PlainRatioVolumeStandardFootCubed:
          return QObject::tr("Sft<sup>3</sup>/Sft<sup>3</sup>");

        case PlainRatioRservoirVolumeReservoirMeterCubedOverStandardMeterCubed:
          return QObject::tr("Rm<sup>3</sup>/Sm<sup>3</sup>");
        case PlainRatioRservoirVolumeReservoirFootCubedOverStandardFootCubed:
          return QObject::tr("Rft<sup>3</sup>/Sft<sup>3</sup>");

        case PlainRatioLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantityProductionOverPeriod:
      {
        switch ((UOM::ProductionOverPeriod)units)
        {
        case ProductionOverPeriodNone:
          break;

        case ProductionOverPeriodStandardMeterCubedOverDay:
          return QObject::tr("Sm<sup>3</sup>/day");

        case ProductionOverPeriodLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantityWaveTravelTime:
      {
        switch ((UOM::WaveTravelTime)units)
        {
        case WaveTravelTime::WaveTravelTimeNone:
          break;

        case WaveTravelTime::WaveTravelTimeSecondsPerMeter:
          return QObject::tr("s/m");

        case WaveTravelTime::WaveTravelTimeMicroSecondsPerMeter:
          return QObject::tr("ms/m");

        case WaveTravelTime::WaveTravelTimeMicroSecondsPerFoot:
          return QObject::tr("ms/ft");

        case WaveTravelTime::WaveTravelTimeLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantityGammaRadiation:
      {
        switch ((UOM::GammaRadiation)units)
        {
        case GammaRadiation::GammaRadiationNone:
          break;

        case GammaRadiation::GammaRadiationRoentgenPerHour:
          return QObject::tr("UR/H");

        case GammaRadiation::GammaRadiationMicroroentgenPerHour:
          return QObject::tr("mcR/H");

        case GammaRadiation::GammaRadiationAPI:
          return QObject::tr("API");

        case GammaRadiation::GammaRadiationLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantitySignalIntensity:
      {
        switch ((UOM::SignalIntensity)units)
        {
        case SignalIntensity::SignalIntensityNone:
          break;

        case SignalIntensity::SignalIntensityDecibel:
          return QObject::tr("db");

        case SignalIntensity::SignalIntensityLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantityCarterLeaks:
      {
        switch ((UOM::CarterLeaks)units)
        {
        case CarterLeaks::CarterLeaksNone:
          break;

        case CarterLeaks::CarterLeaksMetersPerSecondToTheOneHalf:
          return QObject::tr("m/sec<sup>0.5</sup>");

        case CarterLeaks::CarterLeaksMetersPerMinuteToTheOneHalf:
          return QObject::tr("m/min<sup>0.5</sup>");

        case CarterLeaks::CarterLeaksCentimetersPerSecondToTheOneHalf:
          return QObject::tr("cm/sec<sup>0.5</sup>");

        case CarterLeaks::CarterLeaksCentimetersPerMinuteToTheOneHalf:
          return QObject::tr("cm/min<sup>0.5</sup>");

        case CarterLeaks::CarterLeaksLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantityPotentialDifference:
      {
        switch ((UOM::PotentialDifference)units)
        {
        case PotentialDifference::PotentialDifferenceNone:
          break;

        case PotentialDifference::PotentialDifferenceVolt:
          return QObject::tr("V");

        case PotentialDifference::PotentialDifferenceMillivolt:
          return QObject::tr("mV");

        case PotentialDifference::PotentialDifferenceLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantitySpecificElectricalConductivity:
      {
        switch ((UOM::SpecificElectricalConductivity)units)
        {
        case SpecificElectricalConductivity::SpecificElectricalConductivityNone:
          break;

        case SpecificElectricalConductivity::SpecificElectricalConductivitySiemensPerMeter:
          return QObject::tr("S/m");

        case SpecificElectricalConductivity::SpecificElectricalConductivityMillisiemensPerMeter:
          return QObject::tr("mS/m");

        case SpecificElectricalConductivity::SpecificElectricalConductivityLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantityResistivity:
      {
        switch ((UOM::Resistivity)units)
        {
        case Resistivity::ResistivityNone:
          break;

        case Resistivity::ResistivityOhmMeter:
          return QObject::tr("ohmm");

        case Resistivity::ResistivityLast:
          break;
        }
      }
      break;
    case UOM::PhysicalQuantityFrequency:
      {
        switch ((UOM::Frequency)units)
        {
        case Frequency::FrequencyNone:
          break;

        case Frequency::FrequencyHertz:
          return QObject::tr("Hz");

        case Frequency::FrequencyKiloHertz:
          return QObject::tr("Kgc");

        case Frequency::FrequencyPulsesPerMinute:
          return QObject::tr("imp/min");

        case Frequency::FrequencyLast:
          break;
        }
      }
    case UOM::PhysicalQuantityPieces:
      switch ((UOM::Pieces)units) 
      {
      case Pieces::PiecesNone:
        break;
      case Pieces::PiecesPiece:
        return QObject::tr("pcs");
      case Pieces::PiecesLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityLast:
      return QString();
    }
    return QString();
  }

  QString GetUnitsDisplayNameSimplified(PhysicalQuantity quantity, int units)
  {
    QString unitsDisplayName = GetUnitsDisplayName(quantity, units);
    unitsDisplayName.replace("<sup>", "^");
    unitsDisplayName.remove("</sup>");
    return unitsDisplayName;
  }

  QString GetUnitsDisplayNameUnicoded(PhysicalQuantity quantity, int units)
  {
    QString unitsDisplayName = GetUnitsDisplayNameSimplified(quantity, units);
    unitsDisplayName.replace("^3", "³");
    unitsDisplayName.replace("^2", "²");
    // ^0.5 stay not resolved
    return unitsDisplayName;
  }

  QString GetUnitsDisplayNameSimplifiedAlternative(PhysicalQuantity quantity, int units)
  {
    switch (quantity)
    {
    case UOM::PhysicalQuantityNone:
      return QString();

    case UOM::PhysicalQuantityLength:
      break;
    case UOM::PhysicalQuantityPressure:
      break;
    case UOM::PhysicalQuantityPressureOverLength:
      break;
    case UOM::PhysicalQuantityToughness:
      break;
    case UOM::PhysicalQuantityLeakoffCoefficient:
      break;
    case UOM::PhysicalQuantitySpurtLossCoefficient:
      switch (units)
      {
      case SpurtLossCoefficientMeterCubedOverMeterSquared:
        return QObject::tr("cub m/sq m");
      case SpurtLossCoefficientGallonOverFootSquared:
        return QObject::tr("gal/sq ft");
      default:
        break;
      }
      break;
    case UOM::PhysicalQuantityTemperature:
      break;
    case UOM::PhysicalQuantityEnergyOverTemperature:
      break;
    case UOM::PhysicalQuantityEnergyOverMassTemperature:
      break;
    case UOM::PhysicalQuantityThermalConductivity:
      break;
    case UOM::PhysicalQuantityPermeability:
      break;
    case UOM::PhysicalQuantityTime:
      break;
    case UOM::PhysicalQuantityPeriod:
      break;
    case UOM::PhysicalQuantityMass:
      break;
    case UOM::PhysicalQuantityVolume:
      switch ((Volume)units)
      {
      case UOM::VolumeNone:
        break;
      case UOM::VolumeMeterCubed:
        return QObject::tr("cub m");
      case UOM::VolumeLitre:
        break;
      case UOM::VolumeFootCubed:
        return QObject::tr("cub ft");
      case UOM::VolumeLast:
        break;
      default:
        break;
      }
      break;
    case UOM::PhysicalQuantitySpecificGravity:
      switch ((SpecificGravity)units)
      {
      case SpecificGravityNone:
        break;
      case UOM::SpecificGravityKilogramOverMeterCubed:
        return QObject::tr("kg/cub m");
      case UOM::SpecificGravityPoundMassOverFootCubed:
        return QObject::tr("lb/cub ft");
      case UOM::SpecificGravityGrammOverCentimeterCubed:
        return QObject::tr("g/cub cm");
      case UOM::SpecificGravityTonneOverMeterCubed:
        return QObject::tr("t/cub m");
      case SpecificGravityLast:
        break;
      default:
        break;
      }
      break;
    case UOM::PhysicalQuantitySpecificGravityOverLength:
      break;
    case UOM::PhysicalQuantityMassRate:
      break;
    case UOM::PhysicalQuantityVolumeRate:
      switch ((VolumeRate)units)
      {
      case UOM::VolumeRateNone:
        break;
      case UOM::VolumeRateMeterCubedOverSecond:
        return QObject::tr("cub m/s");
      case UOM::VolumeRateMeterCubedOverMinute:
        return QObject::tr("cub m/min");
      case UOM::VolumeRateMeterCubedOverDay:
        return QObject::tr("cub m/day");
      case UOM::VolumeRateLitreOverSecond:
      case UOM::VolumeRateLitreOverMinute:
      case UOM::VolumeRateBarrelOverMinute:
        break;
      case UOM::VolumeRateFootCubedOverSecond:
        return QObject::tr("cub ft/s");
      case UOM::VolumeRateFootCubedOverMinute:
        return QObject::tr("cub ft/min");
      case UOM::VolumeRateLast:
        break;
      }
      break;
    case UOM::PhysicalQuantityConsistency:
      break;
    case UOM::PhysicalQuantityViscosity:
      break;
    case UOM::PhysicalQuantityAngle:
      break;
    case UOM::PhysicalQuantityConductivity:
      break;
    case UOM::PhysicalQuantityMeasureless:
      break;
    case UOM::PhysicalQuantitySpecificGravityOverArea:
      break;
    case UOM::PhysicalQuantityCompressibility:
      break;
    case UOM::PhysicalQuantityReciprocalLength:
      break;
    case UOM::PhysicalQuantityPlainRatio:
      break;
    case UOM::PhysicalQuantityProductionOverPeriod:
      switch ((ProductionOverPeriod)units)
      {
      case ProductionOverPeriodNone:
        break;

      case ProductionOverPeriodStandardMeterCubedOverDay:
        return QObject::tr("S cub m/day");
      
      case ProductionOverPeriodLast:
        break;
      default:
        break;
      }
      break;
    case UOM::PhysicalQuantityConcentration:
      break;
    case UOM::PhysicalQuantityWaveTravelTime:
      break;
    case UOM::PhysicalQuantityGammaRadiation:
      break;
    case UOM::PhysicalQuantitySignalIntensity:
      break;
    case UOM::PhysicalQuantityCarterLeaks:
      break;
    case UOM::PhysicalQuantityPotentialDifference:
      break;
    case UOM::PhysicalQuantitySpecificElectricalConductivity:
      break;
    case UOM::PhysicalQuantityResistivity:
      break;
    case UOM::PhysicalQuantityFrequency:
      break;
    case UOM::PhysicalQuantityPieces:
      break;
    case UOM::PhysicalQuantityLast:
      return QString();
    }
    return GetUnitsDisplayNameSimplified(quantity, units);
  }

  int GetUnitsCount(PhysicalQuantity quantity)
  {
    switch (quantity)
    {
    case UOM::PhysicalQuantityNone:
      break;

    case UOM::PhysicalQuantityLength:
      return LengthLast;
    case UOM::PhysicalQuantityPressure:
      return PressureLast;
    case UOM::PhysicalQuantityPressureOverLength:
      return PressureOverLengthLast;
    case UOM::PhysicalQuantityToughness:
      return ToughnessLast;
    case UOM::PhysicalQuantityLeakoffCoefficient:
      return LeakoffCoefficientLast;
    case UOM::PhysicalQuantitySpurtLossCoefficient:
      return SpurtLossCoefficientLast;
    case UOM::PhysicalQuantityTemperature:
      return TemperatureLast;
    case UOM::PhysicalQuantityEnergyOverTemperature:
      return EnergyOverTemperatureLast;
    case UOM::PhysicalQuantityEnergyOverMassTemperature:
      return EnergyOverMassTemperatureLast;
    case UOM::PhysicalQuantityThermalConductivity:
      return ThermalConductivityLast;
    case UOM::PhysicalQuantityPermeability:
      return PermeabilityLast;
    case UOM::PhysicalQuantityTime:
      return TimeLast;
    case UOM::PhysicalQuantityPeriod:
      return PeriodLast;
    case UOM::PhysicalQuantityMass:
      return MassLast;
    case UOM::PhysicalQuantityVolume:
      return VolumeLast;
    case UOM::PhysicalQuantitySpecificGravity:
      return SpecificGravityLast;
    case UOM::PhysicalQuantitySpecificGravityOverLength:
      return SpecificGravityOverLengthLast;
    case UOM::PhysicalQuantityMassRate:
      return MassRateLast;
    case UOM::PhysicalQuantityVolumeRate:
      return VolumeRateLast;
    case UOM::PhysicalQuantityConsistency:
      return ConsistencyLast;
    case UOM::PhysicalQuantityViscosity:
      return ViscosityLast;
    case UOM::PhysicalQuantityAngle:
      return AngleLast;
    case UOM::PhysicalQuantityConductivity:
      return ConductivityLast;
    case UOM::PhysicalQuantityMeasureless:
      return MeasurelessLast;
    case UOM::PhysicalQuantitySpecificGravityOverArea:
      return SpecificGravityOverAreaLast;
    case UOM::PhysicalQuantityCompressibility:
      return CompressibilityLast;
    case UOM::PhysicalQuantityReciprocalLength:
      return ReciprocalLengthLast;
    case UOM::PhysicalQuantityPlainRatio:
      return PlainRatioLast;
    case UOM::PhysicalQuantityProductionOverPeriod:
      return ProductionOverPeriodLast;
    case UOM::PhysicalQuantityConcentration:
      return ConcentrationLast;
    case UOM::PhysicalQuantityWaveTravelTime:
      return static_cast<int>(WaveTravelTime::WaveTravelTimeLast);
    case UOM::PhysicalQuantityGammaRadiation:
      return static_cast<int>(GammaRadiation::GammaRadiationLast);
    case UOM::PhysicalQuantitySignalIntensity:
      return static_cast<int>(SignalIntensity::SignalIntensityLast);
    case UOM::PhysicalQuantityCarterLeaks:
      return static_cast<int>(CarterLeaks::CarterLeaksLast);
    case UOM::PhysicalQuantityPotentialDifference:
      return static_cast<int>(PotentialDifference::PotentialDifferenceLast);
    case UOM::PhysicalQuantitySpecificElectricalConductivity:
      return static_cast<int>(SpecificElectricalConductivity::SpecificElectricalConductivityLast);
    case UOM::PhysicalQuantityResistivity:
      return static_cast<int>(Resistivity::ResistivityLast);
    case UOM::PhysicalQuantityFrequency:
      return static_cast<int>(Frequency::FrequencyLast);
    case UOM::PhysicalQuantityPieces:
      return PiecesLast;

    case UOM::PhysicalQuantityLast:
      break;
    }
    return 0;
  }

  QMap<int, QString> GetUnitsWithNamesSimplified(PhysicalQuantity quantity)
  {
    QMap<int, QString> result;
    int units = GetUnitsCount(quantity);
    for (int u = AnyFirst; u < units; u++)
    {
      result[u] = GetUnitsDisplayNameSimplified(quantity, u);
    }
    return std::move(result);
  }

  QMap<int, QString> GetUnitsWithNamesSimplifiedAlternative(PhysicalQuantity quantity)
  {
    QMap<int, QString> result;
    int units = GetUnitsCount(quantity);
    for (int u = AnyFirst; u < units; u++)
    {
      result[u] = GetUnitsDisplayNameSimplifiedAlternative(quantity, u);
    }
    return std::move(result);
  }

  QMap<QString, int> GetUnitsWithNamesSimplifiedBackwards(PhysicalQuantity quantity)
  {
    QMap<QString, int> result;
    int units = GetUnitsCount(quantity);
    for (int u = AnyFirst; u < units; u++)
    {
      result[GetUnitsDisplayNameSimplified(quantity, u)] = u;
    }
    return std::move(result);
  }

  QMap<QString, int> GetUnitsWithNamesSimplifiedAlternativeBackwards(PhysicalQuantity quantity)
  {
    QMap<QString, int> result;
    int units = GetUnitsCount(quantity);
    for (int u = AnyFirst; u < units; u++)
    {
      result[GetUnitsDisplayNameSimplifiedAlternative(quantity, u)] = u;
    }
    return std::move(result);
  }

  QMap<QString, int> GetUnitsWithNamesSimplifiedAndSimplifiedAlternativeBackwards(PhysicalQuantity quantity)
  {
    QMap<QString, int> result;
    int units = GetUnitsCount(quantity);
    for (int u = AnyFirst; u < units; u++)
    {
      result[GetUnitsDisplayNameSimplified(quantity, u)] = u;
      result[GetUnitsDisplayNameSimplifiedAlternative(quantity, u)] = u;
    }
    return std::move(result);
  }

  //// Semantics display names
  QString GetSemanticsDisplayName(const QString& semantics)
  {
    return SemanticsDisplayNames.value(semantics);
  }

  ///// Generalized conversion
  double Convert(double value, UOM::PhysicalQuantity quantity, unsigned sourceUOM, unsigned targetUOM) 
  {
    double siValue = ConvertAnyToSI(value, quantity, sourceUOM);
    return ConvertSIToAny(siValue, quantity, targetUOM);
  }

  double ConvertAnyToSI(double value, UOM::PhysicalQuantity quantity, unsigned sourceUOM) 
  {
    switch (quantity)
    {
    case UOM::PhysicalQuantityNone:
      break;

    case UOM::PhysicalQuantityLength:
      return ConvertLengthAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityPressure:
      return ConvertPressureAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityPressureOverLength:
      return ConvertPressureOverLengthAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityToughness:
      return ConvertToughnessAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityLeakoffCoefficient:
      return ConvertLeakoffCoefficientAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantitySpurtLossCoefficient:
      return ConvertSpurtLossCoefficientAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityTemperature:
      return ConvertTemperatureAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityEnergyOverTemperature:
      return ConvertEnergyOverTemperatureAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityEnergyOverMassTemperature:
      return ConvertEnergyOverMassTemperatureAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityThermalConductivity:
      return ConvertThermalConductivityAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityPermeability:
      return ConvertPermeabilityAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityTime:
      return ConvertTimeAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityPeriod:
      return ConvertPeriodAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityMass:
      return ConvertMassAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityVolume:
      return ConvertVolumeAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantitySpecificGravity:
      return ConvertSpecificGravityAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantitySpecificGravityOverLength:
      return ConvertSpecificGravityOverLengthAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityMassRate:
      return ConvertMassRateAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityVolumeRate:
      return ConvertVolumeRateAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityConsistency:
      return ConvertConsistencyAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityViscosity:
      return ConvertViscosityAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityAngle:
      return ConvertAngleAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityConductivity:
      return ConvertConductivityAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityMeasureless:
      return ConvertMeasurelessAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantitySpecificGravityOverArea:
      return ConvertSpecificGravityOverAreaAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityCompressibility:
      return ConvertCompressibilityAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityReciprocalLength:
      return ConvertReciprocalLengthAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityPlainRatio:
      return ConvertPlainRatioAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityProductionOverPeriod:
      return ConvertProductionOverPeriodAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityConcentration:
      return ConvertConcentrationAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityWaveTravelTime:
      return ConvertWaveTravelTimeAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityGammaRadiation:
      return ConvertGammaRadiationAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantitySignalIntensity:
      return ConvertSignalIntensityAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityCarterLeaks:
      return ConvertCarterLeaksAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityPotentialDifference:
      return ConvertPotentialDifferenceAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantitySpecificElectricalConductivity:
      return ConvertSpecificElectricalConductivityAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityResistivity:
      return ConvertResistivityAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityFrequency:
      return ConvertFrequencyAnyToSI(value, sourceUOM);
    case UOM::PhysicalQuantityPieces:
      return ConvertPiecesAnyToSI(value, sourceUOM);

    case UOM::PhysicalQuantityLast:
      break;
    }
    return 0;
  }

  double ConvertSIToAny(double value, UOM::PhysicalQuantity quantity, unsigned targetUOM) 
  {
    switch (quantity)
    {
    case UOM::PhysicalQuantityNone:
      break;

    case UOM::PhysicalQuantityLength:
      return ConvertLengthSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityPressure:
      return ConvertPressureSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityPressureOverLength:
      return ConvertPressureOverLengthSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityToughness:
      return ConvertToughnessSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityLeakoffCoefficient:
      return ConvertLeakoffCoefficientSIToAny(value, targetUOM);
    case UOM::PhysicalQuantitySpurtLossCoefficient:
      return ConvertSpurtLossCoefficientSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityTemperature:
      return ConvertTemperatureSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityEnergyOverTemperature:
      return ConvertEnergyOverTemperatureSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityEnergyOverMassTemperature:
      return ConvertEnergyOverMassTemperatureSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityThermalConductivity:
      return ConvertThermalConductivitySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityPermeability:
      return ConvertPermeabilitySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityTime:
      return ConvertTimeSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityPeriod:
      return ConvertPeriodSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityMass:
      return ConvertMassSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityVolume:
      return ConvertVolumeSIToAny(value, targetUOM);
    case UOM::PhysicalQuantitySpecificGravity:
      return ConvertSpecificGravitySIToAny(value, targetUOM);
    case UOM::PhysicalQuantitySpecificGravityOverLength:
      return ConvertSpecificGravityOverLengthSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityMassRate:
      return ConvertMassRateSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityVolumeRate:
      return ConvertVolumeRateSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityConsistency:
      return ConvertConsistencySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityViscosity:
      return ConvertViscositySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityAngle:
      return ConvertAngleSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityConductivity:
      return ConvertConductivitySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityMeasureless:
      return ConvertMeasurelessSIToAny(value, targetUOM);
    case UOM::PhysicalQuantitySpecificGravityOverArea:
      return ConvertSpecificGravityOverAreaSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityCompressibility:
      return ConvertCompressibilitySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityReciprocalLength:
      return ConvertReciprocalLengthSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityPlainRatio:
      return ConvertPlainRatioSiToAny(value, targetUOM);
    case UOM::PhysicalQuantityProductionOverPeriod:
      return ConvertProductionOverPeriodSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityConcentration:
      return ConvertConcentrationSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityWaveTravelTime:
      return ConvertWaveTravelTimeSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityGammaRadiation:
      return ConvertGammaRadiationSIToAny(value, targetUOM);
    case UOM::PhysicalQuantitySignalIntensity:
      return ConvertSignalIntensitySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityCarterLeaks:
      return ConvertCarterLeaksSIToAny(value, targetUOM);
    case UOM::PhysicalQuantityPotentialDifference:
      return ConvertPotentialDifferenceSIToAny(value, targetUOM);
    case UOM::PhysicalQuantitySpecificElectricalConductivity:
      return ConvertSpecificElectricalConductivitySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityResistivity:
      return ConvertResistivitySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityFrequency:
      return ConvertFrequencySIToAny(value, targetUOM);
    case UOM::PhysicalQuantityPieces:
      return ConvertPiecesSIToAny(value, targetUOM);

    case UOM::PhysicalQuantityLast:
      break;
    }
    return 0;
  }

  //////////////////////////////////////////////////////////////////////
  ///// Specific quantities conversions
  double ConvertLengthAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::Length)sourceUOM)
    {
    case UOM::LengthNone:
      break;

    case UOM::LengthMeter:
      return value;
    case UOM::LengthMillimiter:
      return value / 1000L;
    case UOM::LengthFoot:
      return value / 3.280839895L;
    case UOM::LengthInch:
      return value / 39.370079L;

    case UOM::LengthLast:
      break;
    }
    return 0;
  }

  double ConvertLengthSIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::Length)targetUOM)
    {
    case UOM::LengthNone:
      break;

    case UOM::LengthMeter:
      return value;
    case UOM::LengthMillimiter:
      return value * 1000L;
    case UOM::LengthFoot:
      return value * 3.280839895L;
    case UOM::LengthInch:
      return value * 39.370079L;

    case UOM::LengthLast:
      break;
    }
    return 0;
  }

  double ConvertPressureAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::Pressure)sourceUOM)
    {
    case UOM::PressureNone:
      break;

    case UOM::PressurePascal:
      return value;
    case UOM::PressureKiloPascal:
      return value * 1000L;
    case UOM::PressureMegaPascal:
      return value * 1000000L;
    case UOM::PressureGigaPascal:
      return value * 1000000000L;
    case UOM::PressurePSI:
      return value * 6894.75L;
    case UOM::PressureAtmosphere:
      return value * 101325L;
    case UOM::PressureBar:
      return value * 100000L;

    case UOM::PressureLast:
      break;
    }
    return 0;
  }

  double ConvertPressureSIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::Pressure)targetUOM)
    {
    case UOM::PressureNone:
      break;

    case UOM::PressurePascal:
      return value;
    case UOM::PressureKiloPascal:
      return value / 1000L;
    case UOM::PressureMegaPascal:
      return value / 1000000L;
    case UOM::PressureGigaPascal:
      return value / 1000000000L;
    case UOM::PressurePSI:
      return value / 6894.75L;
    case UOM::PressureAtmosphere:
      return value / 101325L;
    case UOM::PressureBar:
      return value / 100000L;

    case UOM::PressureLast:
      break;
    }
    return 0;
  }

  double ConvertPressureOverLengthAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::PressureOverLength)sourceUOM)
    {
    case UOM::PressureOverLengthNone:
      break;

    case UOM::PressureOverLengthPascalOverMeter:
      return value;
    case UOM::PressureOverLengthKiloPascalOverMeter:
      return ConvertPressureAnyToSI(value, UOM::PressureKiloPascal);
    case UOM::PressureOverLengthMegaPascalOverMeter:
      return ConvertPressureAnyToSI(value, UOM::PressureMegaPascal);
    case UOM::PressureOverLengthGigaPascalOverMeter:
      return ConvertPressureAnyToSI(value, UOM::PressureGigaPascal);
    case UOM::PressureOverLengthAtmosphereOverMeter:
      return ConvertPressureAnyToSI(value, UOM::PressureAtmosphere);
    case UOM::PressureOverLengthBarOverMeter:
      return ConvertPressureAnyToSI(value, UOM::PressureBar);

    case UOM::PressureOverLengthPSIOverMeter:
      return ConvertPressureAnyToSI(value, UOM::PressurePSI);
    case UOM::PressureOverLengthPSIOverFoot:
      return ConvertPressureAnyToSI(value, UOM::PressurePSI)
           / ConvertLengthAnyToSI(1.0, UOM::LengthFoot);
    case UOM::PressureOverLengthBarOverFoot:
      return ConvertPressureAnyToSI(value, UOM::PressureBar)
           / ConvertLengthAnyToSI(1.0, UOM::LengthFoot);
    case UOM::PressureOverLengthPascalOverFoot:
      return value
           / ConvertLengthAnyToSI(1.0, UOM::LengthFoot);

    case UOM::PressureOverLengthLast:
      break;
    }
    return 0;
  }

  double ConvertPressureOverLengthSIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::PressureOverLength)targetUOM)
    {
    case UOM::PressureOverLengthNone:
      break;

    case UOM::PressureOverLengthPascalOverMeter:
      return value;
    case UOM::PressureOverLengthKiloPascalOverMeter:
      return ConvertPressureSIToAny(value, UOM::PressureKiloPascal);
    case UOM::PressureOverLengthMegaPascalOverMeter:
      return ConvertPressureSIToAny(value, UOM::PressureMegaPascal);
    case UOM::PressureOverLengthGigaPascalOverMeter:
      return ConvertPressureSIToAny(value, UOM::PressureGigaPascal);
    case UOM::PressureOverLengthAtmosphereOverMeter:
      return ConvertPressureSIToAny(value, UOM::PressureAtmosphere);
    case UOM::PressureOverLengthBarOverMeter:
      return ConvertPressureSIToAny(value, UOM::PressureBar);

    case UOM::PressureOverLengthPSIOverMeter:
      return ConvertPressureSIToAny(value, UOM::PressurePSI);
    case UOM::PressureOverLengthPSIOverFoot:
      return ConvertPressureSIToAny(value, UOM::PressurePSI)
           / ConvertLengthSIToAny(1.0, UOM::LengthFoot);
    case UOM::PressureOverLengthBarOverFoot:
      return ConvertPressureSIToAny(value, UOM::PressureBar)
           / ConvertLengthSIToAny(1.0, UOM::LengthFoot);
    case UOM::PressureOverLengthPascalOverFoot:
      return value
           / ConvertLengthSIToAny(1.0, UOM::LengthFoot);

    case UOM::PressureOverLengthLast:
      break;
    }
    return 0;
  }

  double ConvertToughnessAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::Toughness)(sourceUOM))
    {
    case UOM::ToughnessNone:
      break;

    case UOM::ToughnessAtmosphereMeterToTheOneHalf:
      return value * 101325L;

    case UOM::ToughnessPascalMeterToTheOneHalf:
      return value;

    case UOM::ToughnessKiloPascalMeterToTheOneHalf:
      return value / 1000;

    case UOM::ToughnessMegaPascalMeterToTheOneHalf:
      return value / 1000000;

    case UOM::ToughnessLast:
      break;
    }
    return 0;
  }

  double ConvertToughnessSIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::Toughness)(targetUOM))
    {
    case UOM::ToughnessNone:
      break;

    case UOM::ToughnessAtmosphereMeterToTheOneHalf:
      return value / 101325L;

    case UOM::ToughnessPascalMeterToTheOneHalf:
      return value;

    case UOM::ToughnessKiloPascalMeterToTheOneHalf:
      return value * 1000;

    case UOM::ToughnessMegaPascalMeterToTheOneHalf:
      return value * 1000000;

    case UOM::ToughnessLast:
      break;
    }
    return 0;
  }

  double ConvertLeakoffCoefficientAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::LeakoffCoefficient)(sourceUOM))
    {
    case UOM::LeakoffCoefficientNone:
      break;

    case UOM::LeakoffCoefficientMeterOverSecondToTheOneHalf:
      return value;

    case UOM::LeakoffCoefficientMeterOverMinuteToTheOneHalf:
      return value * 0.12909944487L;

    case UOM::LeakoffCoefficientCentiMeterOverSecondToTheOneHalf:
      return value / 100.0L;

    case UOM::LeakoffCoefficientCentiMeterOverMinuteToTheOneHalf:
      return value * 0.0012909944487L;

    case UOM::LeakoffCoefficientFootOverSecondToTheOneHalf:
      return value / 3.280839895;

    case UOM::LeakoffCoefficientFootOverMinuteToTheOneHalf:
      return value / 3.280839895 * 0.12909944487L;

    case UOM::LeakoffCoefficientLast:
      break;
    }
    return 0;
  }

  double ConvertLeakoffCoefficientSIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::LeakoffCoefficient)(targetUOM))
    {
    case UOM::LeakoffCoefficientNone:
      break;

    case UOM::LeakoffCoefficientMeterOverSecondToTheOneHalf:
      return value;

    case UOM::LeakoffCoefficientMeterOverMinuteToTheOneHalf:
      return value / 0.12909944487L;

    case UOM::LeakoffCoefficientCentiMeterOverSecondToTheOneHalf:
      return value * 100.0L;

    case UOM::LeakoffCoefficientCentiMeterOverMinuteToTheOneHalf:
      return value / 0.0012909944487L;

    case UOM::LeakoffCoefficientFootOverSecondToTheOneHalf:
      return value * 3.280839895;

    case UOM::LeakoffCoefficientFootOverMinuteToTheOneHalf:
      return value * 3.280839895 / 0.12909944487L;

    case UOM::LeakoffCoefficientLast:
      break;
    }
    return 0;
  }

  double ConvertSpurtLossCoefficientAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::SpurtLossCoefficient)sourceUOM)
    {
    case UOM::SpurtLossCoefficientNone:
      break;

    case UOM::SpurtLossCoefficientMeterCubedOverMeterSquared:
      return value;

    case UOM::SpurtLossCoefficientGallonOverFootSquared:
      return value * 0.003785411784 / 0.09290304;

    case UOM::SpurtLossCoefficientLast:
      break;
    }
    return 0;
  }

  double ConvertSpurtLossCoefficientSIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::SpurtLossCoefficient)targetUOM)
    {
    case UOM::SpurtLossCoefficientNone:
      break;

    case UOM::SpurtLossCoefficientMeterCubedOverMeterSquared:
      return value;

    case UOM::SpurtLossCoefficientGallonOverFootSquared:
      return value / 0.003785411784 * 0.09290304;

    case UOM::SpurtLossCoefficientLast:
      break;
    }
    return 0;
  }

  double ConvertTemperatureAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::Temperature)sourceUOM)
    {
    case UOM::TemperatureNone:
      break;

    case UOM::TemperatureCelsius:
      return value;

    case UOM::TemperatureKelvin:
      return value - 273.15;

    case UOM::TemperatureLast:
      break;
    }
    return 0;
  }

  double ConvertTemperatureSIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::Temperature)targetUOM)
    {
    case UOM::TemperatureNone:
      break;

    case UOM::TemperatureCelsius:
      return value;

    case UOM::TemperatureKelvin:
      return value + 273.15;

    case UOM::TemperatureLast:
      break;
    }
    return 0;
  }

  double ConvertEnergyOverTemperatureAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::EnergyOverTemperature)sourceUOM)
    {
    case UOM::EnergyOverTemperatureNone:
      break;

    case UOM::EnergyOverTemperatureJouleOverCelsius:
      return value;

    case UOM::EnergyOverTemperatureLast:
      break;
    }
    return 0;
  }

  double ConvertEnergyOverTemperatureSIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::EnergyOverTemperature)targetUOM)
    {
    case UOM::EnergyOverTemperatureNone:
      break;

    case UOM::EnergyOverTemperatureJouleOverCelsius:
      return value;

    case UOM::EnergyOverTemperatureLast:
      break;
    }
    return 0;
  }

  double ConvertEnergyOverMassTemperatureAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::EnergyOverMassTemperature)sourceUOM)
    {
    case UOM::EnergyOverMassTemperatureNone:
      break;

    case UOM::EnergyOverMassTemperatureJouleOverKilogramCelsius:
      return value;

    case UOM::EnergyOverMassTemperatureLast:
      break;
    }
    return 0;
  }

  double ConvertEnergyOverMassTemperatureSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::EnergyOverMassTemperature)targetUOM)
    {
    case UOM::EnergyOverMassTemperatureNone:
      break;

    case UOM::EnergyOverMassTemperatureJouleOverKilogramCelsius:
      return value;

    case UOM::EnergyOverMassTemperatureLast:
      break;
    }
    return 0;
  }

  double ConvertThermalConductivityAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::ThermalConductivity)sourceUOM)
    {
    case UOM::ThermalConductivityNone:
      break;

    case UOM::ThermalConductivityWattOverMeterCelsius:
      return value;

    case UOM::ThermalConductivityLast:
      break;
    }
    return 0;
  }

  double ConvertThermalConductivitySIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::ThermalConductivity)targetUOM)
    {
    case UOM::ThermalConductivityNone:
      break;

    case UOM::ThermalConductivityWattOverMeterCelsius:
      return value;

    case UOM::ThermalConductivityLast:
      break;
    }
    return 0;
  }

  double ConvertPermeabilityAnyToSI(double value, unsigned sourceUOM) 
  {
    switch ((UOM::Permeability)sourceUOM)
    {
    case UOM::PermeabilityNone:
      break;

    case UOM::PermeabilityMilliDarcy:
      return value;

    case UOM::PermeabilityLast:
      break;
    }
    return 0;
  }

  double ConvertPermeabilitySIToAny(double value, unsigned targetUOM) 
  {
    switch ((UOM::Permeability)targetUOM)
    {
    case UOM::PermeabilityNone:
      break;

    case UOM::PermeabilityMilliDarcy:
      return value;

    case UOM::PermeabilityLast:
      break;
    }
    return 0;
  }

  double ConvertTimeAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Time)sourceUOM)
    {
    case UOM::TimeNone:
      break;
    case UOM::TimeMilliSeconds:
      return value / 1000.0;
    case UOM::TimeSeconds:
      return value;
    case UOM::TimeMinutes:
      return value * 60.0;
    case UOM::TimeHours:
      return value * 3600.0;
    case UOM::TimeDays:
      return value * 3600.0 * 24.0;

    case UOM::TimeLast:
      break;
    }
    return 0;
  }

  double ConvertTimeSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Time)targetUOM)
    {
    case UOM::TimeNone:
      break;
    case UOM::TimeMilliSeconds:
      return value * 1000.0;
    case UOM::TimeSeconds:
      return value;
    case UOM::TimeMinutes:
      return value / 60.0;
    case UOM::TimeHours:
      return value / 3600.0;
    case UOM::TimeDays:
      return value / (3600.0 * 24.0);

    case UOM::TimeLast:
      break;
    }
    return 0;
  }

  double ConvertPeriodAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Period)sourceUOM)
    {
    case UOM::PeriodNone:
      break;
    case UOM::PeriodDay:
      return value;
    case UOM::PeriodMonth:
      return value * 30.4;
    case UOM::PeriodYear:
      return value * 365.0;

    case UOM::PeriodLast:
      break;
    }
    return 0;
  }

  double ConvertPeriodSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Period)targetUOM)
    {
    case UOM::PeriodNone:
      break;
    case UOM::PeriodDay:
      return value;
    case UOM::PeriodMonth:
      return value / 30.4;
    case UOM::PeriodYear:
      return value / 365;

    case UOM::PeriodLast:
      break;
    }
    return 0;
  }

  double ConvertProductionOverPeriodAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::ProductionOverPeriod)sourceUOM)
    {
    case ProductionOverPeriodNone:
      break;

    case ProductionOverPeriodStandardMeterCubedOverDay:
      return value;

    case ProductionOverPeriodLast:
      break;

    default:
      break;
    }
    return 0;
  }

  double ConvertProductionOverPeriodSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::ProductionOverPeriod)targetUOM)
    {
    case ProductionOverPeriodNone:
      break;

    case ProductionOverPeriodStandardMeterCubedOverDay:
      return value;

    case ProductionOverPeriodLast:
      break;

    default:
      break;
    }
    return 0;
  }

  double ConvertMassAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Mass)sourceUOM)
    {
    case UOM::MassNone:
      break;

    case UOM::MassKilogram:
      return value;
    case UOM::MassTonne:
      return value * 1000L;
    case UOM::MassPoundMass:
      return value * 0.453592L;
    case UOM::MassLast:
      break;
    }
    return 0;
  }

  double ConvertMassSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Mass)targetUOM)
    {
    case UOM::MassNone:
      break;

    case UOM::MassKilogram:
      return value;
    case UOM::MassTonne:
      return value / 1000L;
    case UOM::MassPoundMass:
      return value / 0.453592L;

    case UOM::MassLast:
      break;
    }
    return 0;
  }

  double ConvertVolumeAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Volume)sourceUOM)
    {
    case UOM::VolumeNone:
      break;

    case UOM::VolumeMeterCubed:
      return value;
    case UOM::VolumeLitre:
      return value * 0.001L;
    case UOM::VolumeFootCubed:
      return value * 0.02831685L;
    case UOM::VolumeBarrel:
      return value * 0.117348L;

    case UOM::VolumeLast:
      break;
    }
    return 0;
  }

  double ConvertVolumeSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Volume)targetUOM)
    {
    case UOM::VolumeNone:
      break;

    case UOM::VolumeMeterCubed:
      return value;
    case UOM::VolumeLitre:
      return value / 0.001L;
    case UOM::VolumeFootCubed:
      return value / 0.02831685L;
    case UOM::VolumeBarrel:
      return value / 0.117348L;

    case UOM::VolumeLast:
      break;
    }
    return 0;
  }

  double ConvertSpecificGravityAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::SpecificGravity)sourceUOM)
    {
    case UOM::SpecificGravityNone:
      break;

    case UOM::SpecificGravityKilogramOverMeterCubed:
      return value;
    case UOM::SpecificGravityPoundMassOverFootCubed:
      return value * 16.01948L;
    case UOM::SpecificGravityGrammOverCentimeterCubed:
      return value * 1'000L;
    case UOM::SpecificGravityTonneOverMeterCubed:
      return value * 1'000L;

    case UOM::SpecificGravityLast:
      break;
    }
    return 0;
  }

  double ConvertSpecificGravitySIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::SpecificGravity)targetUOM)
    {
    case UOM::SpecificGravityNone:
      break;

    case UOM::SpecificGravityKilogramOverMeterCubed:
      return value;
    case UOM::SpecificGravityPoundMassOverFootCubed:
      return value / 16.01948L;
    case UOM::SpecificGravityGrammOverCentimeterCubed:
      return value / 1'000L;
    case UOM::SpecificGravityTonneOverMeterCubed:
      return value / 1'000L;

    case UOM::SpecificGravityLast:
      break;
    }
    return 0;
  }

  double ConvertSpecificGravityOverLengthAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::SpecificGravityOverLength)sourceUOM)
    {
    case SpecificGravityOverLengthNone:
      break;

    case SpecificGravityOverLengthKilogramOverMeter:
      return value;
    case SpecificGravityOverLengthPoundOverFoot:
      return value / 0.671969L;
    case SpecificGravityOverLengthTonneOverMeter:
      return value * 1'000L;

    case SpecificGravityOverLengthLast:
      break;
    }
    return 0;
  }

  double ConvertSpecificGravityOverLengthSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::SpecificGravityOverLength)targetUOM)
    {
    case SpecificGravityOverLengthNone:
      break;

    case SpecificGravityOverLengthKilogramOverMeter:
      return value;
    case SpecificGravityOverLengthPoundOverFoot:
      return value * 0.671969;
    case SpecificGravityOverLengthTonneOverMeter:
      return value / 1'000L;

    case SpecificGravityOverLengthLast:
      break;
    }
    return 0;
  }

  double ConvertSpecificGravityOverAreaAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::SpecificGravityOverArea)sourceUOM)
    {
    case SpecificGravityOverAreaNone:
      break;

    case SpecificGravityOverAreaKilogramOverMeterSquared:
      return value;

    case SpecificGravityOverAreaLast:
      break;
    }
    return 0;
  }

  double ConvertSpecificGravityOverAreaSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::SpecificGravityOverArea)targetUOM)
    {
    case SpecificGravityOverAreaNone:
      break;

    case SpecificGravityOverAreaKilogramOverMeterSquared:
      return value;

    case SpecificGravityOverAreaLast:
      break;
    }
    return 0;
  }

  double ConvertCompressibilityAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Compressibility)sourceUOM)
    {
    case CompressibilityNone:
      break;

    case CompressibilityPascalToTheMinusOne:
      return value;

    case CompressibilityBarToTheMinusOne:
      return value / 100000L;

    case CompressibilityLast:
      break;
    }
    return 0;
  }

  double ConvertCompressibilitySIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Compressibility)targetUOM)
    {
    case CompressibilityNone:
      break;

    case CompressibilityPascalToTheMinusOne:
      return value;
    case CompressibilityBarToTheMinusOne:
      return value * 100000L;

    case CompressibilityLast:
      break;
    }
    return 0;
  }

  double ConvertReciprocalLengthAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::ReciprocalLength)sourceUOM)
    {
    case UOM::ReciprocalLengthNone:
      break;

    case UOM::ReciprocalLengthOverMeter:
      return value;
    case UOM::ReciprocalLengthOverFoot:
      return value * 3.280839895L;

    case UOM::ReciprocalLengthLast:
      break;
    }
    return 0;
  }

  double ConvertReciprocalLengthSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::ReciprocalLength)targetUOM)
    {
    case UOM::ReciprocalLengthNone:
      break;

    case UOM::ReciprocalLengthOverMeter:
      return value;
    case UOM::ReciprocalLengthOverFoot:
      return value / 3.280839895L;

    case UOM::ReciprocalLengthLast:
      break;
    }
    return 0;
  }

  double ConvertPlainRatioAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::PlainRatio)sourceUOM)
    {
    case PlainRatioNone:
      break;

    case PlainRatioGeneric:
    case PlainRatioVolumeMeterCubed:
    case PlainRatioVolumeFootCubed:
    case PlainRatioVolumeStandardMeterCubed:
    case PlainRatioVolumeStandardFootCubed:
    case PlainRatioRservoirVolumeReservoirMeterCubedOverStandardMeterCubed:
    case PlainRatioRservoirVolumeReservoirFootCubedOverStandardFootCubed:
      return value;

    case PlainRatioLast:
      break;
    }
    return 0;
  }

  double ConvertPlainRatioSiToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::PlainRatio)targetUOM)
    {
    case PlainRatioNone:
      break;

    case PlainRatioGeneric:
    case PlainRatioVolumeMeterCubed:
    case PlainRatioVolumeFootCubed:
    case PlainRatioVolumeStandardMeterCubed:
    case PlainRatioVolumeStandardFootCubed:
    case PlainRatioRservoirVolumeReservoirMeterCubedOverStandardMeterCubed:
    case PlainRatioRservoirVolumeReservoirFootCubedOverStandardFootCubed:
      return value;

    case PlainRatioLast:
      break;
    }
    return 0;
  }

  double ConvertMassRateAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::MassRate)sourceUOM)
    {
    case UOM::MassRateNone:
      break;

    case UOM::MassRateKilogramOverSecond:
      return value;
    case UOM::MassRateKilogramOverMinute:
      return value / 60L;
    case UOM::MassRateTonneOverMinute:
      return value / 0.06L;
    case UOM::MassRatePoundMassOverSecond:
      return value / 2.204623L;
    case UOM::MassRatePoundMassOverMinute:
      return value / 132.27738L;

    case UOM::MassRateLast:
      break;
    }
    return 0;
  }

  double ConvertMassRateSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::MassRate)targetUOM)
    {
    case UOM::MassRateNone:
      break;

    case UOM::MassRateKilogramOverSecond:
      return value;
    case UOM::MassRateKilogramOverMinute:
      return value * 60L;
    case UOM::MassRateTonneOverMinute:
      return value * 0.06L;
    case UOM::MassRatePoundMassOverSecond:
      return value * 2.204623L;
    case UOM::MassRatePoundMassOverMinute:
      return value * 132.27738L;

    case UOM::MassRateLast:
      break;
    }
    return 0;
  }

  double ConvertVolumeRateAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::VolumeRate)sourceUOM)
    {
    case UOM::VolumeRateNone:
      break;

    case UOM::VolumeRateMeterCubedOverSecond:
      return value;

    case UOM::VolumeRateMeterCubedOverMinute:
      return value / 60L;
    case UOM::VolumeRateMeterCubedOverDay:
      return value / 60L / 60L / 24L;
    case UOM::VolumeRateLitreOverSecond:
      return value / 1000L;
    case UOM::VolumeRateLitreOverMinute:
      return value / 60000L;
    case UOM::VolumeRateBarrelOverMinute:
      return value / 377.3886L;
    case UOM::VolumeRateFootCubedOverSecond:
      return value / 35.314667L;
    case UOM::VolumeRateFootCubedOverMinute:
      return value / 2118.88002L;

    case UOM::VolumeRateLast:
      break;
    }
    return 0;
  }

  double ConvertVolumeRateSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::VolumeRate)targetUOM)
    {
    case UOM::VolumeRateNone:
      break;

    case UOM::VolumeRateMeterCubedOverSecond:
      return value;

    case UOM::VolumeRateMeterCubedOverMinute:
      return value * 60L;
    case UOM::VolumeRateMeterCubedOverDay:
      return value * 60L * 60L * 24L;
    case UOM::VolumeRateLitreOverSecond:
      return value * 1000L;
    case UOM::VolumeRateLitreOverMinute:
      return value * 60000L;
    case UOM::VolumeRateBarrelOverMinute:
      return value * 377.3886L;
    case UOM::VolumeRateFootCubedOverSecond:
      return value * 35.314667L;
    case UOM::VolumeRateFootCubedOverMinute:
      return value * 2118.88002L;

    case UOM::VolumeRateLast:
      break;
    }
    return 0;
  }

  double ConvertConsistencyAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Consistency)sourceUOM)
    {
    case UOM::ConsistencyNone:
      break;

    case UOM::ConsistencyPascalSecondToTheN:
      return value;

    case UOM::ConsistencyPoundForceSecondToTheNOverFootSquared:
      return value * 47.880259;

    case UOM::ConsistencyLast:
      break;
    }
    return 0;
  }

  double ConvertConsistencySIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Consistency)targetUOM)
    {
    case UOM::ConsistencyNone:
      break;

    case UOM::ConsistencyPascalSecondToTheN:
      return value;
    
    case UOM::ConsistencyPoundForceSecondToTheNOverFootSquared:
      return value / 47.880259;

    case UOM::ConsistencyLast:
      break;
    }
    return 0;
  }

  double ConvertViscosityAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Viscosity)sourceUOM)
    {
    case ViscosityNone:
      break;

    case UOM::ViscosityPascalSecond:
      return value;
    case UOM::ViscosityPoise:
      return value / 10.0L;
    case UOM::ViscosityCentiPoise:
      return value / 1000.0L;
    
    case UOM::ViscosityLast:
      break;
    }
    return 0;
  }

  double ConvertViscositySIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Viscosity)targetUOM)
    {
    case ViscosityNone:
      break;

    case UOM::ViscosityPascalSecond:
      return value;
    case UOM::ViscosityPoise:
      return value * 10.0L;
    case UOM::ViscosityCentiPoise:
      return value * 1000.0L;

    case UOM::ViscosityLast:
      break;
    }
    return 0;
  }

  double ConvertAngleAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Angle)sourceUOM)
    {
    case AngleNone:
      break;

    case AngleRadians:
      return value;

    case AngleDegrees:
      return value / 180.0 * M_PI;

    case AngleLast:
      break;
    }
    return 0;
  }

  double ConvertAngleSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Angle)targetUOM)
    {
    case AngleNone:
      break;

    case AngleRadians:
      return value;

    case AngleDegrees:
      return value * 180.0 / M_PI;

    case AngleLast:
      break;
    }
    return 0;
  }
  
  double ConvertConductivityAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Conductivity)sourceUOM)
    {
    case ConductivityNone:
      break;

    case ConductivityMilliDarcyMeter:
      return value;

    case ConductivityLast:
      break;
    }
    return 0;
  }

  double ConvertConductivitySIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Conductivity)targetUOM)
    {
    case ConductivityNone:
      break;

    case ConductivityMilliDarcyMeter:
      return value;

    case ConductivityLast:
      break;
    }
    return 0;
  }

  double ConvertMeasurelessAnyToSI(double value, unsigned targetUOM)
  {
    switch ((UOM::Measureless)targetUOM)
    {
    case MeasurelessNone:
      break;

    case MeasurelessGeneric:
    case MeasurelessUnitFraction:
      return value;

    case MeasurelessPercent:
      return value / 100.0;

    case MeasurelessLast:
      break;
    }
    return 0;
  }

  double ConvertMeasurelessSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Measureless)targetUOM)
    {
    case MeasurelessNone:
      break;

    case MeasurelessGeneric:
    case MeasurelessUnitFraction:
      return value;

    case MeasurelessPercent:
      return value * 100.0;


    case MeasurelessLast:
      break;
    }
    return 0;
  }


  double ConvertConcentrationAnyToSI(double value, unsigned targetUOM)
  {
    switch ((UOM::Concentration)targetUOM)
    {
    case ConcentrationNone:
      break;
    case ConcentrationLitreOverMeterCubed:
      return value / 1000.0;

    case UOM::ConcentrationCoefficient:
      return value;

    case ConcentrationLast:
      break;
    }
    return 0;
  }

  double ConvertConcentrationSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Concentration)targetUOM)
    {
    case ConcentrationNone:
      break;
    case ConcentrationLitreOverMeterCubed:
      return value * 1000.0;

    case UOM::ConcentrationCoefficient:
      return value;

    case ConcentrationLast:
      break;
    }
    return 0;
  }

  double ConvertWaveTravelTimeAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::WaveTravelTime)sourceUOM)
    {
    case WaveTravelTime::WaveTravelTimeNone:
      break;

    case WaveTravelTime::WaveTravelTimeSecondsPerMeter:
      return value;
    case WaveTravelTime::WaveTravelTimeMicroSecondsPerMeter:
      return value / 1'000'000L;
    case WaveTravelTime::WaveTravelTimeMicroSecondsPerFoot:
      return value * 3.280'839'895L / 1'000'000L;

    case WaveTravelTime::WaveTravelTimeLast:
      break;
    }
    return 0;
  }

  double ConvertWaveTravelTimeSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::WaveTravelTime)targetUOM)
    {
    case WaveTravelTime::WaveTravelTimeNone:
      break;

    case WaveTravelTime::WaveTravelTimeSecondsPerMeter:
      return value;
    case WaveTravelTime::WaveTravelTimeMicroSecondsPerMeter:
      return value * 1'000'000L;
    case WaveTravelTime::WaveTravelTimeMicroSecondsPerFoot:
      return value * 1'000'000L / 3.280'839'895L;

    case WaveTravelTime::WaveTravelTimeLast:
      break;
    }
    return 0;
  }

  double ConvertGammaRadiationAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::GammaRadiation)sourceUOM)
    {
    case GammaRadiation::GammaRadiationNone:
      break;

    case GammaRadiation::GammaRadiationRoentgenPerHour:
      return value;
    case GammaRadiation::GammaRadiationMicroroentgenPerHour:
      return value / 1'000'000L;
    case GammaRadiation::GammaRadiationAPI:
      return value * 10'000'000L;

    case GammaRadiation::GammaRadiationLast:
      break;
    }
    return 0;
  }

  double ConvertGammaRadiationSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::GammaRadiation)targetUOM)
    {
    case GammaRadiation::GammaRadiationNone:
      break;

    case GammaRadiation::GammaRadiationRoentgenPerHour:
      return value;
    case GammaRadiation::GammaRadiationMicroroentgenPerHour:
      return value * 1'000'000L;
    case GammaRadiation::GammaRadiationAPI:
      return value / 10'000'000L;

    case GammaRadiation::GammaRadiationLast:
      break;
    }
    return 0;
  }

  double ConvertSignalIntensityAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::SignalIntensity)sourceUOM)
    {
    case SignalIntensity::SignalIntensityNone:
      break;

    case SignalIntensity::SignalIntensityDecibel:
      return value;

    case SignalIntensity::SignalIntensityLast:
      break;
    }
    return 0;
  }

  double ConvertSignalIntensitySIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::SignalIntensity)targetUOM)
    {
    case SignalIntensity::SignalIntensityNone:
      break;

    case SignalIntensity::SignalIntensityDecibel:
      return value;

    case SignalIntensity::SignalIntensityLast:
      break;
    }
    return 0;
  }

  double ConvertCarterLeaksAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::CarterLeaks)(sourceUOM))
    {
    case CarterLeaks::CarterLeaksNone:
      break;

    case CarterLeaks::CarterLeaksMetersPerSecondToTheOneHalf:
      return value;

    case CarterLeaks::CarterLeaksMetersPerMinuteToTheOneHalf:
      return value * 0.129'099'444'87L;

    case CarterLeaks::CarterLeaksCentimetersPerSecondToTheOneHalf:
      return value / 100.0L;

    case CarterLeaks::CarterLeaksCentimetersPerMinuteToTheOneHalf:
      return value * 0.001'290'994'448'7L;

    case CarterLeaks::CarterLeaksLast:
      break;
    }
    return 0;
  }

  double ConvertCarterLeaksSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::CarterLeaks)(targetUOM))
    {
    case CarterLeaks::CarterLeaksNone:
      break;

    case CarterLeaks::CarterLeaksMetersPerSecondToTheOneHalf:
      return value;

    case CarterLeaks::CarterLeaksMetersPerMinuteToTheOneHalf:
      return value / 0.129'099'444'87L;

    case CarterLeaks::CarterLeaksCentimetersPerSecondToTheOneHalf:
      return value * 100.0L;

    case CarterLeaks::CarterLeaksCentimetersPerMinuteToTheOneHalf:
      return value / 0.001'290'994'448'7L;

    case CarterLeaks::CarterLeaksLast:
      break;
    }
    return 0;
  }

  double ConvertPotentialDifferenceAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::PotentialDifference)sourceUOM)
    {
    case PotentialDifference::PotentialDifferenceNone:
      break;

    case PotentialDifference::PotentialDifferenceVolt:
      return value;
    case PotentialDifference::PotentialDifferenceMillivolt:
      return value / 1'000L;

    case PotentialDifference::PotentialDifferenceLast:
      break;
    }
    return 0;
  }

  double ConvertPotentialDifferenceSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::PotentialDifference)targetUOM)
    {
    case PotentialDifference::PotentialDifferenceNone:
      break;

    case PotentialDifference::PotentialDifferenceVolt:
      return value;
    case PotentialDifference::PotentialDifferenceMillivolt:
      return value * 1'000L;

    case PotentialDifference::PotentialDifferenceLast:
      break;
    }
    return 0;
  }

  double ConvertSpecificElectricalConductivityAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::SpecificElectricalConductivity)sourceUOM)
    {
    case SpecificElectricalConductivity::SpecificElectricalConductivityNone:
      break;

    case SpecificElectricalConductivity::SpecificElectricalConductivitySiemensPerMeter:
      return value;
    case SpecificElectricalConductivity::SpecificElectricalConductivityMillisiemensPerMeter:
      return value / 1'000L;

    case SpecificElectricalConductivity::SpecificElectricalConductivityLast:
      break;
    }
    return 0;
  }

  double ConvertSpecificElectricalConductivitySIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::SpecificElectricalConductivity)targetUOM)
    {
    case SpecificElectricalConductivity::SpecificElectricalConductivityNone:
      break;

    case SpecificElectricalConductivity::SpecificElectricalConductivitySiemensPerMeter:
      return value;
    case SpecificElectricalConductivity::SpecificElectricalConductivityMillisiemensPerMeter:
      return value * 1'000L;

    case SpecificElectricalConductivity::SpecificElectricalConductivityLast:
      break;
    }
    return 0;
  }

  double ConvertResistivityAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Resistivity)sourceUOM)
    {
    case Resistivity::ResistivityNone:
      break;

    case Resistivity::ResistivityOhmMeter:
      return value;

    case Resistivity::ResistivityLast:
      break;
    }
    return 0;
  }

  double ConvertResistivitySIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Resistivity)targetUOM)
    {
    case Resistivity::ResistivityNone:
      break;

    case Resistivity::ResistivityOhmMeter:
      return value;

    case Resistivity::ResistivityLast:
      break;
    }
    return 0;
  }

  double ConvertFrequencyAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Frequency)sourceUOM)
    {
    case Frequency::FrequencyNone:
      break;

    case Frequency::FrequencyHertz:
      return value;
    case Frequency::FrequencyKiloHertz:
      return value * 1'000L;
    case Frequency::FrequencyPulsesPerMinute:
      return value / 60L;

    case Frequency::FrequencyLast:
      break;
    }
    return 0;
  }

  double ConvertFrequencySIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Frequency)targetUOM)
    {
    case Frequency::FrequencyNone:
      break;

    case Frequency::FrequencyHertz:
      return value;
    case Frequency::FrequencyKiloHertz:
      return value / 1'000L;
    case Frequency::FrequencyPulsesPerMinute:
      return value * 60L;

    case Frequency::FrequencyLast:
      break;
    }
    return 0;
  }

  double ConvertPiecesAnyToSI(double value, unsigned sourceUOM)
  {
    switch ((UOM::Pieces)sourceUOM)
    {
    case Pieces::PiecesNone:
      break;

    case Pieces::PiecesPiece:
      return value;

    case Pieces::PiecesLast:
      break;
    }
    return 0;
  }

  double ConvertPiecesSIToAny(double value, unsigned targetUOM)
  {
    switch ((UOM::Pieces)targetUOM)
    {
    case Pieces::PiecesNone:
      break;

    case Pieces::PiecesPiece:
      return value;

    case Pieces::PiecesLast:
      break;
    }
    return 0;
  }

  ///////////////////////////////////////////////////////
  //// Units from quantity
  int GetSIUnitsForQuantity(UOM::PhysicalQuantity quantity)
  {
    switch (quantity)
    {
    case UOM::PhysicalQuantityNone:
      break;

    case UOM::PhysicalQuantityLength:
      return UOM::LengthSI;
    case UOM::PhysicalQuantityPressure:
      return UOM::PressureSI;
    case UOM::PhysicalQuantityPressureOverLength:
      return UOM::PressureOverLengthSI;
    case UOM::PhysicalQuantityToughness:
      return UOM::ToughnessSI;
    case UOM::PhysicalQuantityLeakoffCoefficient:
      return UOM::LeakoffCoefficientSI;
    case UOM::PhysicalQuantitySpurtLossCoefficient:
      return UOM::SpurtLossCoefficientSI;
    case UOM::PhysicalQuantityTemperature:
      return UOM::TemperatureSI;
    case UOM::PhysicalQuantityEnergyOverTemperature:
      return UOM::EnergyOverTemperatureSI;
    case UOM::PhysicalQuantityEnergyOverMassTemperature:
      return UOM::EnergyOverMassTemperatureSI;
    case UOM::PhysicalQuantityThermalConductivity:
      return UOM::ThermalConductivitySI;
    case UOM::PhysicalQuantityPermeability:
      return UOM::PermeabilitySI;
    case UOM::PhysicalQuantityTime:
      return UOM::TimeSI;
    case UOM::PhysicalQuantityPeriod:
      return UOM::PeriodSI;
    case UOM::PhysicalQuantityMass:
      return UOM::MassSI;
    case UOM::PhysicalQuantityVolume:
      return UOM::VolumeSI;
    case UOM::PhysicalQuantitySpecificGravity:
      return UOM::SpecificGravitySI;
    case UOM::PhysicalQuantitySpecificGravityOverLength:
      return UOM::SpecificGravityOverLengthSI;
    case UOM::PhysicalQuantityMassRate:
      return UOM::MassRateSI;
    case UOM::PhysicalQuantityVolumeRate:
      return UOM::VolumeRateSI;
    case UOM::PhysicalQuantityConsistency:
      return UOM::ConsistencySI;
    case UOM::PhysicalQuantityViscosity:
      return UOM::ViscositySI;
    case UOM::PhysicalQuantityAngle:
      return UOM::AngleSI;
    case UOM::PhysicalQuantityConductivity:
      return UOM::ConductivitySI;
    case UOM::PhysicalQuantityMeasureless:
      return UOM::MeasurelessSI;
    case UOM::PhysicalQuantitySpecificGravityOverArea:
      return UOM::SpecificGravityOverAreaSI;
    case UOM::PhysicalQuantityCompressibility:
      return UOM::CompressibilitySI;
    case UOM::PhysicalQuantityReciprocalLength:
      return UOM::ReciprocalLengthSI;
    case UOM::PhysicalQuantityPlainRatio:
      return UOM::PlainRatioSI;
    case UOM::PhysicalQuantityProductionOverPeriod:
      return UOM::ProductionOverPeriodSI;
    case UOM::PhysicalQuantityConcentration:
      return UOM::ConcentrationSI;
    case UOM::PhysicalQuantityWaveTravelTime:
      return static_cast<int>(WaveTravelTime::WaveTravelTimeSI);
    case UOM::PhysicalQuantityGammaRadiation:
      return static_cast<int>(GammaRadiation::GammaRadiationSI);
    case UOM::PhysicalQuantitySignalIntensity:
      return static_cast<int>(SignalIntensity::SignalIntensitySI);
    case UOM::PhysicalQuantityCarterLeaks:
      return static_cast<int>(CarterLeaks::CarterLeaksSI);
    case UOM::PhysicalQuantityPotentialDifference:
      return static_cast<int>(PotentialDifference::PotentialDifferenceSI);
    case UOM::PhysicalQuantitySpecificElectricalConductivity:
      return static_cast<int>(SpecificElectricalConductivity::SpecificElectricalConductivitySI);
    case UOM::PhysicalQuantityResistivity:
      return static_cast<int>(Resistivity::ResistivitySI);
    case UOM::PhysicalQuantityFrequency:
      return static_cast<int>(Frequency::FrequencySI);
    case UOM::PhysicalQuantityPieces:
      return static_cast<int>(Pieces::PiecesSI);

    case UOM::PhysicalQuantityLast:
      break;
    }
    return UOM::AnyNone;
  }

  Descriptor GetNumerator(const Descriptor& descriptor)
  {
//    auto d = DerivedUOMS.find(descriptor);
//    if (d != DerivedUOMS.end())
//    {
//      return d->first;
//    }

    return Descriptor({ PhysicalQuantityNone, AnyNone });
  }

  Descriptor GetDenominator(const Descriptor& descriptor)
  {
//    auto d = DerivedUOMS.find(descriptor);
//    if (d != DerivedUOMS.end())
//    {
//      return d->second;
//    }

    return Descriptor({ PhysicalQuantityNone, AnyNone });
  }

  Descriptor GetDerived(const Descriptor& numerator, const Descriptor& denominator)
  {
    QPair<Descriptor, Descriptor> key = { numerator, denominator };
//    if (DerivedUOMSBackwards.empty())
//    {
//      for (auto d = DerivedUOMS.begin(); d != DerivedUOMS.end(); d++)
//      {
//        DerivedUOMSBackwards[d.value()] = d.key();
//      }
//    }
    return DerivedUOMSBackwards.value({ numerator, denominator });
  }

  namespace
  {
    void FillDerivedUOMPhysicalQuantities()
    {
//      if (DerivedUOMPhysicalQuantities.empty())
//      {
//        for (auto d = DerivedUOMS.begin(); d != DerivedUOMS.end(); d++)
//        {
//          DerivedUOMPhysicalQuantities[d.key().Quantity] = { d.value().first.Quantity,
//                                                             d.value().second.Quantity };
//        }
//      }
    }
  }

  PhysicalQuantity GetNumerator(PhysicalQuantity q)
  {
    FillDerivedUOMPhysicalQuantities();
    
    auto d = DerivedUOMPhysicalQuantities.find(q);
    if (d != DerivedUOMPhysicalQuantities.end())
    {
      return d->first;
    }

    return PhysicalQuantityNone;
  }

  PhysicalQuantity GetDenominator(PhysicalQuantity q)
  {
    FillDerivedUOMPhysicalQuantities();

    auto d = DerivedUOMPhysicalQuantities.find(q);
    if (d != DerivedUOMPhysicalQuantities.end())
    {
      return d->second;
    }

    return PhysicalQuantityNone;
  }

  PhysicalQuantity GetDerived(PhysicalQuantity numerator, PhysicalQuantity denominator)
  {
    FillDerivedUOMPhysicalQuantities();

    return DerivedUOMPhysicalQuantities.key({ numerator, denominator }, PhysicalQuantityNone);
  }
};
