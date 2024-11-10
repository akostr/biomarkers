#include "uom_settings.h"

#include <QSharedPointer>
#include <QJsonDocument>
#include <QDebug>

#if defined(USE_SETTINGS)
#include <settings/settings.h>
#endif

#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
#include <utils/json_utils/files.h>
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION

#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
#include <licensing/licensing.h>
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API

#pragma warning(1 : 4061)
#pragma warning(1 : 4062)

namespace Details
{
  QString UOMSettingsSavePath = "cyberfrac";
  QSharedPointer<UOMSettings> UOMSetting_Instance__;
  
  const QString KeySemanticsQuantities          = "SemanticsQuantities";
  const QString KeySemanticsUiUnits             = "SemanticsUiUnits";
  const QString KeySemanticsLegacyUntaggedUnits = "SemanticsLegacyUntaggedUnits";

  //// Legacies & typos etc
  QMap<QString, QString> LegacyAliases = {
    { "SemanticsMesurelessEfficiency",              UOM::KeySemanticsMeasurelessEfficiency },
    { "SemanticsConcentration",                     UOM::KeySemanticsConcentrationMass },
    { "KeySemanticsConcentrationMass",              UOM::KeySemanticsConcentrationMass },
    { "KeySemanticsConcentrationVolume",            UOM::KeySemanticsConcentrationVolume },
    { "UOMSpecificGravityGramOverCentimeterCubed",  "UOMSpecificGravityGrammOverCentimeterCubed"},
  };
};

QString UOMSettings::Normalize(const QString& key)
{
  auto alias = Details::LegacyAliases.find(key);
  if (alias != Details::LegacyAliases.end())
  {
    return alias.value();
  }
  return key;
}

///////////////////////////////////////////////////////
//// Centralized UOM settings
UOMSettings::UOMSettings(QObject* parent)
  : QObject(parent)
{
  Setup();
  Load();
  Save();

#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
  CET_LICENSE_CHECK_OR_DELETE(this);
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
}

UOMSettings::~UOMSettings()
{
}

UOMSettings& UOMSettings::Get()
{
  if (!Details::UOMSetting_Instance__.data())
  {
    Details::UOMSetting_Instance__.reset(new UOMSettings);
  }
  return *Details::UOMSetting_Instance__.data();
}

void UOMSettings::SetSavePath(const QString& savePath)
{
  Details::UOMSettingsSavePath = savePath;
}

QString UOMSettings::GetStringKey(UOM::PhysicalQuantity quantity)
{
  return UOM::KeysQuantities.value(quantity, "PhysicalQuantityNone");
}

QString UOMSettings::GetStringKey(UOM::PhysicalQuantity quantity, unsigned units)
{
  return UOM::KeysUnits.value({ quantity, (int)units });
}

UOM::PhysicalQuantity UOMSettings::GetQuantityFromStringKey(const QString& key)
{
  return UOM::KeysQuantitiesBackwards.value(key, UOM::PhysicalQuantityNone);
}

unsigned UOMSettings::GetUnitsFromStringKey(const QString& key)
{
  return UOM::KeysUnitsBackwards.value(key, UOM::AnyNone);
}

UOM::Descriptor UOMSettings::Parse(const QString& uiUnits)
{
#if defined(USE_SETTINGS)
#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
  static QMap<QString, UOM::Descriptor> descriptors;
  if (descriptors.empty())
  {
    QStringList files;
    files << ":/resource/standard_models/standard_models_heuristics_en_EN.json";
    files << ":/resource/standard_models/standard_models_heuristics_ru_RU.json";
    
    for (auto f : files)
    {
      QJsonDocument doc;
      JsonUtils::Files::ReadJsonFile(f, doc);
      if (doc.isObject())
      {
        QJsonObject object = doc.object();
        for (auto qi = object.begin(); qi != object.end(); qi++)
        {
          UOM::PhysicalQuantity q = UOMSettings::GetQuantityFromStringKey(qi.key());
          if (q != UOM::PhysicalQuantityNone)
          {
            if (qi.value().isObject())
            {
              QJsonObject objectUnits = qi.value().toObject();
              for (auto ui = objectUnits.begin(); ui != objectUnits.end(); ui++)
              {
                unsigned u = UOMSettings::GetUnitsFromStringKey(ui.key());
                if (u != UOM::AnyNone)
                {
                  if (ui.value().isArray())
                  {
                    for (auto uis : ui.value().toArray())
                    {
                      QString v = uis.toString();
                      if (!v.isEmpty())
                      {
                        QSet<QString> variations = { v.toLower() };

                        // transformations / spaces variations
                        auto t01 = [](QString& s) { s.replace("/", " / "); };
                        auto t02 = [](QString& s) { s.replace("*", " * "); };
                        auto t03 = [](QString& s) { s.replace("^", " ^ "); };
                        
                        auto t04 = [](QString& s) { s.replace(" / ", "/"); };
                        auto t05 = [](QString& s) { s.replace(" * ", "*"); };
                        auto t06 = [](QString& s) { s.replace(" ^ ", "^"); };

                        // transformations / dots variations
                        auto t07 = [](QString& s) { s.replace(".", ". "); };
                        auto t08 = [](QString& s) { s.replace(". ", "."); };
                        auto t09 = [](QString& s) { s.replace(". ", " "); };

                        // transformations / multiplications / degrees
                        auto t10 = [](QString& s) { s.replace("*", " "); };
                        auto t11 = [](QString& s) { s.replace("*", ""); };
                        auto t12 = [](QString& s) { s.replace("^", ""); };

                        std::vector<std::function<void(QString&)>> transformations = {
                          t01,
                          t02,
                          t03,
                          t04,
                          t05,
                          t06,
                          t07,
                          t08,
                          t09,
                          t10,
                          t11,
                          t12,
                        };

                        for (auto& t : transformations)
                        {
                          QSet<QString> tvs;
                          for (auto& v : variations)
                          {
                            QString tv = v;
                            t(tv);
                            tvs << tv;
                          }
                          for (auto tv : tvs)
                          {
                            variations << tv.simplified();
                          }
                        }

                        // store
                        for (auto v : variations)
                        {
                          UOM::Descriptor d;
                          d.Quantity = q;
                          d.Units = u;
                          descriptors[v] = d;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  auto d = descriptors.find(uiUnits.toLower());
  if (d != descriptors.end())
  {
    return d.value();
  }
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION
#endif
  return UOM::Descriptor();
}

void UOMSettings::Setup()
{
  //// Set up quantities
  if (UOM::KeysQuantities.empty())
  {
    //// Quantities
    UOM::KeysQuantities[UOM::PhysicalQuantityNone]                            = "PhysicalQuantityNone";
    UOM::KeysQuantities[UOM::PhysicalQuantityLength]                          = "PhysicalQuantityLength";
    UOM::KeysQuantities[UOM::PhysicalQuantityPressure]                        = "PhysicalQuantityPressure";
    UOM::KeysQuantities[UOM::PhysicalQuantityPressureOverLength]              = "PhysicalQuantityPressureOverLength";
    UOM::KeysQuantities[UOM::PhysicalQuantityToughness]                       = "PhysicalQuantityToughness";
    UOM::KeysQuantities[UOM::PhysicalQuantityLeakoffCoefficient]              = "PhysicalQuantityLeakoffCoefficient";
    UOM::KeysQuantities[UOM::PhysicalQuantitySpurtLossCoefficient]            = "PhysicalQuantitySpurtLossCoefficient";
    UOM::KeysQuantities[UOM::PhysicalQuantityTemperature]                     = "PhysicalQuantityTemperature";
    UOM::KeysQuantities[UOM::PhysicalQuantityEnergyOverTemperature]           = "PhysicalQuantityEnergyOverTemperature";
    UOM::KeysQuantities[UOM::PhysicalQuantityEnergyOverMassTemperature]       = "PhysicalQuantityEnergyOverMassTemperature";
    UOM::KeysQuantities[UOM::PhysicalQuantityThermalConductivity]             = "PhysicalQuantityThermalConductivity";
    UOM::KeysQuantities[UOM::PhysicalQuantityPermeability]                    = "PhysicalQuantityPermeability";
    UOM::KeysQuantities[UOM::PhysicalQuantityTime]                            = "PhysicalQuantityTime";
    UOM::KeysQuantities[UOM::PhysicalQuantityMass]                            = "PhysicalQuantityMass";
    UOM::KeysQuantities[UOM::PhysicalQuantityVolume]                          = "PhysicalQuantityVolume";
    UOM::KeysQuantities[UOM::PhysicalQuantitySpecificGravity]                 = "PhysicalQuantitySpecificGravity";
    UOM::KeysQuantities[UOM::PhysicalQuantitySpecificGravityOverLength]       = "PhysicalQuantitySpecificGravityOverLength";
    UOM::KeysQuantities[UOM::PhysicalQuantityMassRate]                        = "PhysicalQuantityMassRate";
    UOM::KeysQuantities[UOM::PhysicalQuantityVolumeRate]                      = "PhysicalQuantityVolumeRate";
    UOM::KeysQuantities[UOM::PhysicalQuantityConsistency]                     = "PhysicalQuantityConsistency";
    UOM::KeysQuantities[UOM::PhysicalQuantityViscosity]                       = "PhysicalQuantityViscosity";
    UOM::KeysQuantities[UOM::PhysicalQuantityAngle]                           = "PhysicalQuantityAngle";
    UOM::KeysQuantities[UOM::PhysicalQuantityConductivity]                    = "PhysicalQuantityConductivity";
    UOM::KeysQuantities[UOM::PhysicalQuantityMeasureless]                     = "PhysicalQuantityMeasureless";
    UOM::KeysQuantities[UOM::PhysicalQuantitySpecificGravityOverArea]         = "PhysicalQuantitySpecificGravityOverArea";
    UOM::KeysQuantities[UOM::PhysicalQuantityCompressibility]                 = "PhysicalQuantityCompressibility";
    UOM::KeysQuantities[UOM::PhysicalQuantityReciprocalLength]                = "PhysicalQuantityReciprocalLength";
    UOM::KeysQuantities[UOM::PhysicalQuantityPlainRatio]                      = "PhysicalQuantityPlainRatio";
    UOM::KeysQuantities[UOM::PhysicalQuantityProductionOverPeriod]            = "PhysicalQuantityProductionOverPeriod";
    UOM::KeysQuantities[UOM::PhysicalQuantityConcentration]                   = "PhysicalQuantityConcentration";
    UOM::KeysQuantities[UOM::PhysicalQuantityWaveTravelTime]                  = "PhysicalQuantityWaveTravelTime";
    UOM::KeysQuantities[UOM::PhysicalQuantityGammaRadiation]                  = "PhysicalQuantityGammaRadiation";
    UOM::KeysQuantities[UOM::PhysicalQuantitySignalIntensity]                 = "PhysicalQuantitySignalIntensity";
    UOM::KeysQuantities[UOM::PhysicalQuantityCarterLeaks]                     = "PhysicalQuantityCarterLeaks";
    UOM::KeysQuantities[UOM::PhysicalQuantityPotentialDifference]             = "PhysicalQuantityPotentialDifference";
    UOM::KeysQuantities[UOM::PhysicalQuantitySpecificElectricalConductivity]  = "PhysicalQuantitySpecificElectricalConductivity";
    UOM::KeysQuantities[UOM::PhysicalQuantityResistivity]                     = "PhysicalQuantityResistivity";
    UOM::KeysQuantities[UOM::PhysicalQuantityFrequency]                       = "PhysicalQuantityFrequency";
    UOM::KeysQuantities[UOM::PhysicalQuantityPieces]                          = "PhysicalQuantityPieces";

    //// Backward keys
    for (auto q = UOM::KeysQuantities.begin(); q != UOM::KeysQuantities.end(); ++q)
    {
      UOM::KeysQuantitiesBackwards[q.value()] = (UOM::PhysicalQuantity)q.key();
    }
  }

  //// Set up units
  if (UOM::KeysUnits.empty())
  {
    //// Any / none
    UOM::KeysUnits[{UOM::PhysicalQuantityNone, UOM::AnyNone}] = "UOMAnyNone";

    //// Length
    UOM::KeysUnits[{UOM::PhysicalQuantityLength, UOM::LengthNone}]        = "UOMLengthNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityLength, UOM::LengthMeter}]       = "UOMLengthMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityLength, UOM::LengthMillimiter}]  = "UOMLengthMillimiter";
    UOM::KeysUnits[{UOM::PhysicalQuantityLength, UOM::LengthFoot}]        = "UOMLengthFoot";
    UOM::KeysUnits[{UOM::PhysicalQuantityLength, UOM::LengthInch}]        = "UOMLengthInch";

    //// Pressure
    UOM::KeysUnits[{UOM::PhysicalQuantityPressure, UOM::PressureNone}]        = "UOMPressureNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressure, UOM::PressurePascal}]      = "UOMPressurePascal";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressure, UOM::PressureKiloPascal}]  = "UOMPressureKiloPascal";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressure, UOM::PressureMegaPascal}]  = "UOMPressureMegaPascal";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressure, UOM::PressureGigaPascal}]  = "UOMPressureGigaPascal";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressure, UOM::PressurePSI}]         = "UOMPressurePSI";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressure, UOM::PressureAtmosphere}]  = "UOMPressureAtmosphere";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressure, UOM::PressureBar}]         = "UOMPressureBar";

    //// Pressure per Length
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthNone}]                = "UOMPressureOverLengthNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthPascalOverMeter}]     = "UOMPressureOverLengthPascalOverMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthKiloPascalOverMeter}] = "UOMPressureOverLengthKiloPascalOverMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthMegaPascalOverMeter}] = "UOMPressureOverLengthMegaPascalOverMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthGigaPascalOverMeter}] = "UOMPressureOverLengthGigaPascalOverMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthAtmosphereOverMeter}] = "UOMPressureOverLengthAtmosphereOverMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthBarOverMeter}]        = "UOMPressureOverLengthBarOverMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthPSIOverMeter}]        = "UOMPressureOverLengthPSIOverMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthPSIOverFoot}]         = "UOMPressureOverLengthPSIOverFoot";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthBarOverFoot}]         = "UOMPressureOverLengthBarOverFoot";
    UOM::KeysUnits[{UOM::PhysicalQuantityPressureOverLength, UOM::PressureOverLengthPascalOverFoot}]      = "UOMPressureOverLengthPascalOverFoot";

    //// Toughness
    UOM::KeysUnits[{UOM::PhysicalQuantityToughness, UOM::ToughnessNone}]                        = "UOMToughnessNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityToughness, UOM::ToughnessAtmosphereMeterToTheOneHalf}] = "UOMToughnessAtmosphereMeterToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityToughness, UOM::ToughnessPascalMeterToTheOneHalf}]     = "UOMToughnessPascalMeterToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityToughness, UOM::ToughnessKiloPascalMeterToTheOneHalf}] = "UOMToughnessKiloPascalMeterToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityToughness, UOM::ToughnessMegaPascalMeterToTheOneHalf}] = "UOMToughnessMegaPascalMeterToTheOneHalf";

    //// Leakoff Coefficient
    UOM::KeysUnits[{UOM::PhysicalQuantityLeakoffCoefficient, UOM::LeakoffCoefficientNone}]                              = "UOMLeakoffCoefficientNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityLeakoffCoefficient, UOM::LeakoffCoefficientMeterOverSecondToTheOneHalf}]       = "UOMLeakoffCoefficientMeterOverSecondToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityLeakoffCoefficient, UOM::LeakoffCoefficientMeterOverMinuteToTheOneHalf}]       = "UOMLeakoffCoefficientMeterOverMinuteToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityLeakoffCoefficient, UOM::LeakoffCoefficientCentiMeterOverSecondToTheOneHalf}]  = "UOMLeakoffCoefficientCentiMeterOverSecondToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityLeakoffCoefficient, UOM::LeakoffCoefficientCentiMeterOverMinuteToTheOneHalf}]  = "UOMLeakoffCoefficientCentiMeterOverMinuteToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityLeakoffCoefficient, UOM::LeakoffCoefficientFootOverSecondToTheOneHalf}]        = "UOMLeakoffCoefficientFootOverSecondToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityLeakoffCoefficient, UOM::LeakoffCoefficientFootOverMinuteToTheOneHalf}]        = "UOMLeakoffCoefficientFootOverMinuteToTheOneHalf";

    //// Spurt Loss Coefficient
    UOM::KeysUnits[{UOM::PhysicalQuantitySpurtLossCoefficient, UOM::SpurtLossCoefficientNone}]                       = "UOMSpurtLossCoefficientNone";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpurtLossCoefficient, UOM::SpurtLossCoefficientMeterCubedOverMeterSquared}] = "UOMSpurtLossCoefficientMeterCubedOverMeterSquared";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpurtLossCoefficient, UOM::SpurtLossCoefficientGallonOverFootSquared}]      = "UOMSpurtLossCoefficientGallonOverFootSquared";

    //// Temperature
    UOM::KeysUnits[{UOM::PhysicalQuantityTemperature, UOM::TemperatureNone}]      = "UOMTemperatureNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityTemperature, UOM::TemperatureCelsius}]   = "UOMTemperatureCelsius";
    UOM::KeysUnits[{UOM::PhysicalQuantityTemperature, UOM::TemperatureKelvin}]    = "UOMTemperatureKelvin";

    //// Energy Over Temperature
    UOM::KeysUnits[{UOM::PhysicalQuantityEnergyOverTemperature, UOM::EnergyOverTemperatureNone}]              = "UOMEnergyOverTemperatureNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityEnergyOverTemperature, UOM::EnergyOverTemperatureJouleOverCelsius}]  = "UOMEnergyOverTemperatureJouleOverCelsius";

    //// Energy Over Temperature
    UOM::KeysUnits[{UOM::PhysicalQuantityEnergyOverMassTemperature, UOM::EnergyOverMassTemperatureNone}]                     = "UOMEnergyOverMassTemperatureNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityEnergyOverMassTemperature, UOM::EnergyOverMassTemperatureJouleOverKilogramCelsius}] = "UOMEnergyOverMassTemperatureJouleOverKilogramCelsius";

    //// Thermal Conductivity
    UOM::KeysUnits[{UOM::PhysicalQuantityThermalConductivity, UOM::ThermalConductivityNone}]                 = "UOMThermalConductivityNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityThermalConductivity, UOM::ThermalConductivityWattOverMeterCelsius}] = "UOMThermalConductivityWattOverMeterCelsius";

    //// Permeability
    UOM::KeysUnits[{UOM::PhysicalQuantityPermeability, UOM::PermeabilityNone}]        = "UOMPermeabilityNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityPermeability, UOM::PermeabilityMilliDarcy}]  = "UOMPermeabilityMilliDarcy";

    //// Time
    UOM::KeysUnits[{UOM::PhysicalQuantityTime, UOM::TimeNone}]            = "UOMTimeNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityTime, UOM::TimeMilliSeconds}]    = "UOMTimeMilliSeconds";
    UOM::KeysUnits[{UOM::PhysicalQuantityTime, UOM::TimeSeconds}]         = "UOMTimeSeconds";
    UOM::KeysUnits[{UOM::PhysicalQuantityTime, UOM::TimeMinutes}]         = "UOMTimeMinutes";
    UOM::KeysUnits[{UOM::PhysicalQuantityTime, UOM::TimeHours}]           = "UOMTimeHours";
    UOM::KeysUnits[{UOM::PhysicalQuantityTime, UOM::TimeDays}]            = "UOMTimeDays";


    //// Period
    UOM::KeysUnits[{UOM::PhysicalQuantityPeriod, UOM::PeriodNone}]        = "UOMPeriodNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityPeriod, UOM::PeriodDay}]         = "UOMPeriodDays";
    UOM::KeysUnits[{UOM::PhysicalQuantityPeriod, UOM::PeriodMonth}]       = "UOMPeriodMonths";
    UOM::KeysUnits[{UOM::PhysicalQuantityPeriod, UOM::PeriodYear}]        = "UOMPeriodYears";

    //// Production over period
    UOM::KeysUnits[{UOM::PhysicalQuantityProductionOverPeriod, UOM::ProductionOverPeriodNone}]                      = "UOMProductionOverPeriodNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityProductionOverPeriod, UOM::ProductionOverPeriodStandardMeterCubedOverDay}] = "UOMProductionOverPeriodStandardMeterCubedOverDay";
    UOM::KeysUnits[{UOM::PhysicalQuantityProductionOverPeriod, UOM::ProductionOverPeriodLast}]                      = "UOMProductionOverPeriodLast";

    //// Mass
    UOM::KeysUnits[{UOM::PhysicalQuantityMass, UOM::MassNone}]      = "UOMMassNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityMass, UOM::MassKilogram}]  = "UOMMassKilogram";
    UOM::KeysUnits[{UOM::PhysicalQuantityMass, UOM::MassTonne}]     = "UOMMassTonne";
    UOM::KeysUnits[{UOM::PhysicalQuantityMass, UOM::MassPoundMass}] = "UOMMassPoundMass";

    //// Volume
    UOM::KeysUnits[{UOM::PhysicalQuantityVolume, UOM::VolumeNone}]        = "UOMVolumeNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolume, UOM::VolumeMeterCubed}]  = "UOMVolumeMeterCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolume, UOM::VolumeLitre}]       = "UOMVolumeLitre";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolume, UOM::VolumeFootCubed}]   = "UOMVolumeFootCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolume, UOM::VolumeBarrel}]      = "UOMVolumeBarrel";

    //// Specific Gravity
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificGravity, UOM::SpecificGravityNone}]                      = "UOMSpecificGravityNone";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificGravity, UOM::SpecificGravityKilogramOverMeterCubed}]    = "UOMSpecificGravityKilogramOverMeterCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificGravity, UOM::SpecificGravityPoundMassOverFootCubed}]    = "UOMSpecificGravityPoundMassOverFootCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificGravity, UOM::SpecificGravityGrammOverCentimeterCubed}]  = "UOMSpecificGravityGrammOverCentimeterCubed";

    //// Specific Gravity Over Length
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificGravityOverLength, UOM::SpecificGravityOverLengthNone}]              = "UOMSpecificGravityOverLengthNone";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificGravityOverLength, UOM::SpecificGravityOverLengthKilogramOverMeter}] = "UOMSpecificGravityOverLengthKilogramOverMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificGravityOverLength, UOM::SpecificGravityOverLengthPoundOverFoot}]     = "UOMSpecificGravityOverLengthPoundOverFoot";

    //// Mass rate
    UOM::KeysUnits[{UOM::PhysicalQuantityMassRate, UOM::MassRateNone}]                = "UOMMassRateNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityMassRate, UOM::MassRateKilogramOverSecond}]  = "UOMMassRateKilogramOverSecond";
    UOM::KeysUnits[{UOM::PhysicalQuantityMassRate, UOM::MassRateKilogramOverMinute}]  = "UOMMassRateKilogramOverMinute";
    UOM::KeysUnits[{UOM::PhysicalQuantityMassRate, UOM::MassRateTonneOverMinute}]     = "UOMMassRateTonneOverMinute";
    UOM::KeysUnits[{UOM::PhysicalQuantityMassRate, UOM::MassRatePoundMassOverSecond}] = "UOMMassRatePoundMassOverSecond";
    UOM::KeysUnits[{UOM::PhysicalQuantityMassRate, UOM::MassRatePoundMassOverMinute}] = "UOMMassRatePoundMassOverMinute";

    //// Volume rate
    UOM::KeysUnits[{UOM::PhysicalQuantityVolumeRate, UOM::VolumeRateNone}]                  = "UOMVolumeRateNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolumeRate, UOM::VolumeRateMeterCubedOverSecond}]  = "UOMVolumeRateMeterCubedOverSecond";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolumeRate, UOM::VolumeRateMeterCubedOverMinute}]  = "UOMVolumeRateMeterCubedOverMinute";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolumeRate, UOM::VolumeRateMeterCubedOverDay}]     = "UOMVolumeRateMeterCubedOverDay";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolumeRate, UOM::VolumeRateLitreOverSecond}]       = "UOMVolumeRateLitreOverSecond";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolumeRate, UOM::VolumeRateLitreOverMinute}]       = "UOMVolumeRateLitreOverMinute";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolumeRate, UOM::VolumeRateBarrelOverMinute}]      = "UOMVolumeRateBarrelOverMinute";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolumeRate, UOM::VolumeRateFootCubedOverSecond}]   = "UOMVolumeRateFootCubedOverSecond";
    UOM::KeysUnits[{UOM::PhysicalQuantityVolumeRate, UOM::VolumeRateFootCubedOverMinute}]   = "UOMVolumeRateFootCubedOverMinute";

    //// Consistency
    UOM::KeysUnits[{UOM::PhysicalQuantityConsistency, UOM::ConsistencyNone}]                                  = "UOMConsistencyNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityConsistency, UOM::ConsistencyPascalSecondToTheN}]                    = "UOMConsistencyPascalSecondToTheN";
    UOM::KeysUnits[{UOM::PhysicalQuantityConsistency, UOM::ConsistencyPoundForceSecondToTheNOverFootSquared}] = "UOMConsistencyPoundForceSecondToTheNOverFootSquared";

    //// Viscosity
    UOM::KeysUnits[{UOM::PhysicalQuantityViscosity, UOM::ViscosityNone}]                    = "UOMViscosityNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityViscosity, UOM::ViscosityPascalSecond}]            = "UOMViscosityPascalSecond";
    UOM::KeysUnits[{UOM::PhysicalQuantityViscosity, UOM::ViscosityPoise}]                   = "UOMViscosityPoise";
    UOM::KeysUnits[{UOM::PhysicalQuantityViscosity, UOM::ViscosityCentiPoise}]              = "UOMViscosityCentiPoise";

    //// Angle
    UOM::KeysUnits[{UOM::PhysicalQuantityAngle, UOM::AngleNone}]                          = "UOMAngleNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityAngle, UOM::AngleRadians}]                       = "UOMAngleRadians";
    UOM::KeysUnits[{UOM::PhysicalQuantityAngle, UOM::AngleDegrees}]                       = "UOMAngleDegrees";

    //// Conductivity
    UOM::KeysUnits[{UOM::PhysicalQuantityConductivity, UOM::ConductivityNone}]            = "UOMConductivityNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityConductivity, UOM::ConductivityMilliDarcyMeter}] = "UOMConductivityMilliDarcyMeter";

    //// Measureless
    UOM::KeysUnits[{UOM::PhysicalQuantityMeasureless, UOM::MeasurelessNone}]              = "UOMMeasurelessNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityMeasureless, UOM::MeasurelessGeneric}]           = "UOMMeasurelessGeneric";
    UOM::KeysUnits[{UOM::PhysicalQuantityMeasureless, UOM::MeasurelessUnitFraction}]      = "UOMMeasurelessUnitFraction";
    UOM::KeysUnits[{UOM::PhysicalQuantityMeasureless, UOM::MeasurelessPercent}]           = "UOMMeasurelessPercent";

    //// Specific Gravity Over Area
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificGravityOverArea, UOM::SpecificGravityOverAreaNone}]                     = "UOMSpecificGravityOverAreaNone";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificGravityOverArea, UOM::SpecificGravityOverAreaKilogramOverMeterSquared}] = "UOMSpecificGravityOverAreaKilogramOverMeterSquared";

    //// Compressibility
    UOM::KeysUnits[{UOM::PhysicalQuantityCompressibility, UOM::CompressibilityNone}]                = "UOMCompressibilityNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityCompressibility, UOM::CompressibilityPascalToTheMinusOne}] = "UOMCompressibilityPascalToTheMinusOne";
    UOM::KeysUnits[{UOM::PhysicalQuantityCompressibility, UOM::CompressibilityBarToTheMinusOne}]    = "UOMCompressibilityBarToTheMinusOne";

    //// Reciprocal Length
    UOM::KeysUnits[{UOM::PhysicalQuantityReciprocalLength, UOM::ReciprocalLengthNone}]              = "UOMReciprocalLengthNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityReciprocalLength, UOM::ReciprocalLengthOverMeter}]         = "UOMReciprocalLengthOverMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityReciprocalLength, UOM::ReciprocalLengthOverFoot}]          = "UOMReciprocalLengthOverFoot";

    //// Plain ratio
    UOM::KeysUnits[{UOM::PhysicalQuantityPlainRatio, UOM::PlainRatioNone}]                                                    = "UOMPlainRatioNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityPlainRatio, UOM::PlainRatioGeneric}]                                                 = "UOMPlainRatioGeneric";
    UOM::KeysUnits[{UOM::PhysicalQuantityPlainRatio, UOM::PlainRatioVolumeMeterCubed}]                                        = "UOMPlainRatioVolumeMeterCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantityPlainRatio, UOM::PlainRatioVolumeFootCubed}]                                         = "UOMPlainRatioVolumeFootCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantityPlainRatio, UOM::PlainRatioVolumeStandardMeterCubed}]                                = "UOMPlainRatioVolumeStandardMeterCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantityPlainRatio, UOM::PlainRatioVolumeStandardFootCubed}]                                 = "UOMPlainRatioVolumeStandardFootCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantityPlainRatio, UOM::PlainRatioRservoirVolumeReservoirMeterCubedOverStandardMeterCubed}] = "UOMPlainRatioRservoirVolumeReservoirMeterCubedOverStandardMeterCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantityPlainRatio, UOM::PlainRatioRservoirVolumeReservoirFootCubedOverStandardFootCubed}]   = "UOMPlainRatioRservoirVolumeReservoirFootCubedOverStandardFootCubed";

    //// Production Over Period
    UOM::KeysUnits[{UOM::PhysicalQuantityProductionOverPeriod, UOM::ProductionOverPeriodNone}]                                = "UOMProductionOverPeriodNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityProductionOverPeriod, UOM::ProductionOverPeriodStandardMeterCubedOverDay}]           = "UOMProductionOverPeriodStandardMeterCubedOverDay";

    //// Concentration
    UOM::KeysUnits[{UOM::PhysicalQuantityConcentration, UOM::ConcentrationNone}]                                              = "UOMConcentrationNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityConcentration, UOM::ConcentrationLitreOverMeterCubed}]                               = "UOMConcentrationLitreOverMeterCubed";
    UOM::KeysUnits[{UOM::PhysicalQuantityConcentration, UOM::ConcentrationCoefficient}]                                       = "UOMConcentrationCoefficient";

    //// Wave travel time
    UOM::KeysUnits[{UOM::PhysicalQuantityWaveTravelTime, static_cast<int>(UOM::WaveTravelTime::WaveTravelTimeNone)}]                  = "UOMWaveTravelTimeNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityWaveTravelTime, static_cast<int>(UOM::WaveTravelTime::WaveTravelTimeSecondsPerMeter)}]       = "UOMWaveTravelTimeSecondsPerMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityWaveTravelTime, static_cast<int>(UOM::WaveTravelTime::WaveTravelTimeMicroSecondsPerMeter)}]  = "UOMWaveTravelTimeMicroSecondsPerMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantityWaveTravelTime, static_cast<int>(UOM::WaveTravelTime::WaveTravelTimeMicroSecondsPerFoot)}]   = "UOMWaveTravelTimeMicroSecondsPerFoot";

    //// Gamma radiation
    UOM::KeysUnits[{UOM::PhysicalQuantityGammaRadiation, static_cast<int>(UOM::GammaRadiation::GammaRadiationNone)}]                  = "UOMGammaRadiationNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityGammaRadiation, static_cast<int>(UOM::GammaRadiation::GammaRadiationRoentgenPerHour)}]       = "UOMGammaRadiationRoentgenPerHour";
    UOM::KeysUnits[{UOM::PhysicalQuantityGammaRadiation, static_cast<int>(UOM::GammaRadiation::GammaRadiationMicroroentgenPerHour)}]  = "UOMGammaRadiationMicroroentgenPerHour";
    UOM::KeysUnits[{UOM::PhysicalQuantityGammaRadiation, static_cast<int>(UOM::GammaRadiation::GammaRadiationAPI)}]                   = "UOMGammaRadiationAPI";

    //// Signal intensity
    UOM::KeysUnits[{UOM::PhysicalQuantitySignalIntensity, static_cast<int>(UOM::SignalIntensity::SignalIntensityNone)}]     = "UOMSignalIntensityNone";
    UOM::KeysUnits[{UOM::PhysicalQuantitySignalIntensity, static_cast<int>(UOM::SignalIntensity::SignalIntensityDecibel)}]  = "UOMSignalIntensityDecibel";

    //// Carter Leaks
    UOM::KeysUnits[{UOM::PhysicalQuantityCarterLeaks, static_cast<int>(UOM::CarterLeaks::CarterLeaksNone)}]                             = "UOMCarterLeaksNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityCarterLeaks, static_cast<int>(UOM::CarterLeaks::CarterLeaksMetersPerSecondToTheOneHalf)}]      = "UOMCarterLeaksMetersPerSecondToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityCarterLeaks, static_cast<int>(UOM::CarterLeaks::CarterLeaksMetersPerMinuteToTheOneHalf)}]      = "UOMCarterLeaksMetersPerMinuteToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityCarterLeaks, static_cast<int>(UOM::CarterLeaks::CarterLeaksCentimetersPerSecondToTheOneHalf)}] = "UOMCarterLeaksCentimetersPerSecondToTheOneHalf";
    UOM::KeysUnits[{UOM::PhysicalQuantityCarterLeaks, static_cast<int>(UOM::CarterLeaks::CarterLeaksCentimetersPerMinuteToTheOneHalf)}] = "UOMCarterLeaksCentimetersPerMinuteToTheOneHalf";

    //// Potential difference
    UOM::KeysUnits[{UOM::PhysicalQuantityPotentialDifference, static_cast<int>(UOM::PotentialDifference::PotentialDifferenceNone)}]       = "UOMPotentialDifferenceNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityPotentialDifference, static_cast<int>(UOM::PotentialDifference::PotentialDifferenceVolt)}]       = "UOMPotentialDifferenceVolt";
    UOM::KeysUnits[{UOM::PhysicalQuantityPotentialDifference, static_cast<int>(UOM::PotentialDifference::PotentialDifferenceMillivolt)}]  = "UOMPotentialDifferenceMillivolt";

    //// Specific electrical conductivity
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificElectricalConductivity, static_cast<int>(UOM::SpecificElectricalConductivity::SpecificElectricalConductivityNone)}]                  = "UOMSpecificElectricalConductivityNone";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificElectricalConductivity, static_cast<int>(UOM::SpecificElectricalConductivity::SpecificElectricalConductivitySiemensPerMeter)}]       = "UOMSpecificElectricalConductivitySiemensPerMeter";
    UOM::KeysUnits[{UOM::PhysicalQuantitySpecificElectricalConductivity, static_cast<int>(UOM::SpecificElectricalConductivity::SpecificElectricalConductivityMillisiemensPerMeter)}]  = "UOMSpecificElectricalConductivityMillisiemensPerMeter";

    //// Resistivity
    UOM::KeysUnits[{UOM::PhysicalQuantityResistivity, static_cast<int>(UOM::Resistivity::ResistivityNone)}]       = "UOMResistivityNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityResistivity, static_cast<int>(UOM::Resistivity::ResistivityOhmMeter)}]   = "UOMResistivityOhmMeter";

    //// Frequency
    UOM::KeysUnits[{UOM::PhysicalQuantityFrequency, static_cast<int>(UOM::Frequency::FrequencyNone)}]             = "UOMFrequencyNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityFrequency, static_cast<int>(UOM::Frequency::FrequencyHertz)}]            = "UOMFrequencyHertz";
    UOM::KeysUnits[{UOM::PhysicalQuantityFrequency, static_cast<int>(UOM::Frequency::FrequencyKiloHertz)}]        = "UOMFrequencyKiloHertz";
    UOM::KeysUnits[{UOM::PhysicalQuantityFrequency, static_cast<int>(UOM::Frequency::FrequencyPulsesPerMinute)}]  = "UOMFrequencyPulsesPerMinute";

    //// Pieces
    UOM::KeysUnits[{UOM::PhysicalQuantityPieces, static_cast<int>(UOM::Pieces::PiecesNone)}]  = "UOMPiecesNone";
    UOM::KeysUnits[{UOM::PhysicalQuantityPieces, static_cast<int>(UOM::Pieces::PiecesPiece)}] = "UOMPiecesPiece";

    //// Backward keys
    for (auto u = UOM::KeysUnits.begin(); u != UOM::KeysUnits.end(); ++u)
    {
      UOM::KeysUnitsBackwards[u.value()] = u.key().second;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  //// Set up semantics defaults
  {
    SetupDefautSettings();
    SetupSemanticsDisplayNames();
    SetupSemanticsAcceptableUiUnits();

    /////////////////////////////////////////////////////////////////////////////////////////
    // Legacy Untagged Units
    QVariantMap legacyUntaggedUnits;

    // Legacy
    legacyUntaggedUnits[UOM::KeySemanticsPipeDiameter]          = UOM::LengthMillimiter;

    legacyUntaggedUnits[UOM::KeySemanticsYoungModulus]          = UOM::PressureAtmosphere;
    legacyUntaggedUnits[UOM::KeySemanticsStress]                = UOM::PressureAtmosphere;
    legacyUntaggedUnits[UOM::KeySemanticsPorePressure]          = UOM::PressureAtmosphere;
    legacyUntaggedUnits[UOM::KeySemanticsPipePressure]          = UOM::PressureAtmosphere;

    legacyUntaggedUnits[UOM::KeySemanticsStressGradient]        = UOM::PressureOverLengthAtmosphereOverMeter;
    legacyUntaggedUnits[UOM::KeySemanticsPorePressureGradient]  = UOM::PressureOverLengthAtmosphereOverMeter;
    legacyUntaggedUnits[UOM::KeySemanticsFluidLinearPressure]   = UOM::PressureOverLengthAtmosphereOverMeter;

    legacyUntaggedUnits[UOM::KeySemanticsTime]                  = UOM::TimeMinutes;

    legacyUntaggedUnits[UOM::KeySemanticsPeriod]                = UOM::PeriodYear;
    legacyUntaggedUnits[UOM::KeySemanticsPeriodNPVProduction]   = UOM::PeriodYear;
    legacyUntaggedUnits[UOM::KeySemanticsPeriodNPVPrices]       = UOM::PeriodYear;
    legacyUntaggedUnits[UOM::KeySemanticsPeriodNPVUtilization]  = UOM::PeriodYear;

    legacyUntaggedUnits[UOM::KeySemanticsMassRate]              = UOM::MassRateKilogramOverMinute;
    
    legacyUntaggedUnits[UOM::KeySemanticsVolumeRate]            = UOM::VolumeRateMeterCubedOverMinute;
    
    legacyUntaggedUnits[UOM::KeySemanticsViscosity]             = UOM::ViscosityCentiPoise;

    Settings[Details::KeySemanticsLegacyUntaggedUnits] = legacyUntaggedUnits;
  }
}

void UOMSettings::SetupDefautSettings()
{
  QVariantMap quantities;
  QVariantMap units;

  //// Lengths
  quantities[UOM::KeySemanticsLength]                         = UOM::PhysicalQuantityLength;
  units[UOM::KeySemanticsLength]                              = UOM::LengthMeter;

  quantities[UOM::KeySemanticsDepth]                          = UOM::PhysicalQuantityLength;
  units[UOM::KeySemanticsDepth]                               = UOM::LengthMeter;

  quantities[UOM::KeySemanticsPipeDiameter]                   = UOM::PhysicalQuantityLength;
  units[UOM::KeySemanticsPipeDiameter]                        = UOM::LengthMillimiter;

  quantities[UOM::KeySemanticsPerforationDiameter]            = UOM::PhysicalQuantityLength;
  units[UOM::KeySemanticsPerforationDiameter]                 = UOM::LengthMillimiter;

  //// Longths / fracture dimensions
  quantities[UOM::KeySemanticsFractureWidth]                  = UOM::PhysicalQuantityLength;
  units[UOM::KeySemanticsFractureWidth]                       = UOM::LengthMillimiter;

  quantities[UOM::KeySemanticsFractureLength]                 = UOM::PhysicalQuantityLength;
  units[UOM::KeySemanticsFractureLength]                      = UOM::LengthMeter;

  //// Pressures
  quantities[UOM::KeySemanticsPressure]                       = UOM::PhysicalQuantityPressure;
  units[UOM::KeySemanticsPressure]                            = UOM::PressureAtmosphere;

  quantities[UOM::KeySemanticsPressureOverLength]             = UOM::PhysicalQuantityPressureOverLength;
  units[UOM::KeySemanticsPressureOverLength]                  = UOM::PressureOverLengthAtmosphereOverMeter;

  quantities[UOM::KeySemanticsYoungModulus]                   = UOM::PhysicalQuantityPressure;
  units[UOM::KeySemanticsYoungModulus]                        = UOM::PressureGigaPascal;

  quantities[UOM::KeySemanticsStress]                         = UOM::PhysicalQuantityPressure;
  units[UOM::KeySemanticsStress]                              = UOM::PressureAtmosphere;

  quantities[UOM::KeySemanticsPorePressure]                   = UOM::PhysicalQuantityPressure;
  units[UOM::KeySemanticsPorePressure]                        = UOM::PressureAtmosphere;

  quantities[UOM::KeySemanticsPipePressure]                   = UOM::PhysicalQuantityPressure;
  units[UOM::KeySemanticsPipePressure]                        = UOM::PressureAtmosphere;

  //// Pressures over lengths
  quantities[UOM::KeySemanticsStressGradient]                 = UOM::PhysicalQuantityPressureOverLength;
  units[UOM::KeySemanticsStressGradient]                      = UOM::PressureOverLengthAtmosphereOverMeter;

  quantities[UOM::KeySemanticsPorePressureGradient]           = UOM::PhysicalQuantityPressureOverLength;
  units[UOM::KeySemanticsPorePressureGradient]                = UOM::PressureOverLengthAtmosphereOverMeter;

  quantities[UOM::KeySemanticsFluidLinearPressure]            = UOM::PhysicalQuantityPressureOverLength;
  units[UOM::KeySemanticsFluidLinearPressure]                 = UOM::PressureOverLengthAtmosphereOverMeter;

  //// Toughness
  quantities[UOM::KeySemanticsToughness]                      = UOM::PhysicalQuantityToughness;
  units[UOM::KeySemanticsToughness]                           = UOM::ToughnessAtmosphereMeterToTheOneHalf;

  //// Leakoff
  quantities[UOM::KeySemanticsLeakoffCoefficient]             = UOM::PhysicalQuantityLeakoffCoefficient;
  units[UOM::KeySemanticsLeakoffCoefficient]                  = UOM::LeakoffCoefficientMeterOverSecondToTheOneHalf;

  //// Spurt Loss
  quantities[UOM::KeySemanticsSpurtLossCoefficient]           = UOM::PhysicalQuantitySpurtLossCoefficient;
  units[UOM::KeySemanticsSpurtLossCoefficient]                = UOM::SpurtLossCoefficientMeterCubedOverMeterSquared;

  //// Temperature
  quantities[UOM::KeySemanticsTemperature]                    = UOM::PhysicalQuantityTemperature;
  units[UOM::KeySemanticsTemperature]                         = UOM::TemperatureCelsius;

  //// Enegry over temperature
  quantities[UOM::KeySemanticsEnergyOverTemperature]          = UOM::PhysicalQuantityEnergyOverTemperature;
  units[UOM::KeySemanticsEnergyOverTemperature]               = UOM::EnergyOverTemperatureJouleOverCelsius;

  quantities[UOM::KeySemanticsHeatCapacity]                   = UOM::PhysicalQuantityEnergyOverTemperature;
  units[UOM::KeySemanticsHeatCapacity]                        = UOM::EnergyOverTemperatureJouleOverCelsius;

  //// Enegry over mass temperature
  quantities[UOM::KeySemanticsEnergyOverMassTemperature]      = UOM::PhysicalQuantityEnergyOverMassTemperature;
  units[UOM::KeySemanticsEnergyOverMassTemperature]           = UOM::EnergyOverMassTemperatureJouleOverKilogramCelsius;

  quantities[UOM::KeySemanticsMassSpecificHeatCapacity]       = UOM::PhysicalQuantityEnergyOverMassTemperature;
  units[UOM::KeySemanticsMassSpecificHeatCapacity]            = UOM::EnergyOverMassTemperatureJouleOverKilogramCelsius;

  //// Thermal conductivity
  quantities[UOM::KeySemanticsThermalConductivity]            = UOM::PhysicalQuantityThermalConductivity;
  units[UOM::KeySemanticsThermalConductivity]                 = UOM::ThermalConductivityWattOverMeterCelsius;

  //// Permeability
  quantities[UOM::KeySemanticsPermeability]                   = UOM::PhysicalQuantityPermeability;
  units[UOM::KeySemanticsPermeability]                        = UOM::PermeabilityMilliDarcy;

  //// Time
  quantities[UOM::KeySemanticsTime]                           = UOM::PhysicalQuantityTime;
  units[UOM::KeySemanticsTime]                                = UOM::TimeUIDefault;

  //// Period
  quantities[UOM::KeySemanticsPeriod]                         = UOM::PhysicalQuantityPeriod;
  units[UOM::KeySemanticsPeriod]                              = UOM::PeriodUIDefault;

  quantities[UOM::KeySemanticsPeriodNPVProduction]            = UOM::PhysicalQuantityPeriod;
  units[UOM::KeySemanticsPeriodNPVProduction]                 = UOM::PeriodUIDefault;

  quantities[UOM::KeySemanticsPeriodNPVPrices]                = UOM::PhysicalQuantityPeriod;
  units[UOM::KeySemanticsPeriodNPVPrices]                     = UOM::PeriodUIDefault;

  quantities[UOM::KeySemanticsPeriodNPVUtilization]           = UOM::PhysicalQuantityPeriod;
  units[UOM::KeySemanticsPeriodNPVUtilization]                = UOM::PeriodUIDefault;

  //// Production over paeriod
  quantities[UOM::KeySemanticsProductionOverPeriod]           = UOM::PhysicalQuantityProductionOverPeriod;
  units[UOM::KeySemanticsProductionOverPeriod]                = UOM::ProductionOverPeriodUIDefault;

  //// Mass
  quantities[UOM::KeySemanticsMass]                           = UOM::PhysicalQuantityMass;
  units[UOM::KeySemanticsMass]                                = UOM::MassUIDefault;

  //// Volume
  quantities[UOM::KeySemanticsVolume]                         = UOM::PhysicalQuantityVolume;
  units[UOM::KeySemanticsVolume]                              = UOM::VolumeUIDefault;

  //// Densities
  quantities[UOM::KeySemanticsSpecificGravity]                = UOM::PhysicalQuantitySpecificGravity;
  units[UOM::KeySemanticsSpecificGravity]                     = UOM::SpecificGravityUIDefault;

  //// Concentrations
  quantities[UOM::KeySemanticsConcentrationMass]              = UOM::PhysicalQuantitySpecificGravity;
  units[UOM::KeySemanticsConcentrationMass]                   = UOM::SpecificGravityUIDefault;

  quantities[UOM::KeySemanticsConcentrationVolume]            = UOM::PhysicalQuantityConcentration;
  units[UOM::KeySemanticsConcentrationVolume]                 = UOM::ConcentrationUIDefault;

  //// Liner masses
  quantities[UOM::KeySemanticsSpecificGravityOverLength]      = UOM::PhysicalQuantitySpecificGravityOverLength;
  units[UOM::KeySemanticsSpecificGravityOverLength]           = UOM::SpecificGravityOverLengthUIDefault;

  //// Areal masses
  quantities[UOM::KeySemanticsSpecificGravityOverArea]        = UOM::PhysicalQuantitySpecificGravityOverArea;
  units[UOM::KeySemanticsSpecificGravityOverArea]             = UOM::SpecificGravityOverAreaUIDefault;

  //// Compressibility
  quantities[UOM::KeySemanticsCompressibility]                = UOM::PhysicalQuantityCompressibility;
  units[UOM::KeySemanticsCompressibility]                     = UOM::CompressibilityUIDefault;

  //// Reciprocal Length
  quantities[UOM::KeySemanticsReciprocalLength]               = UOM::PhysicalQuantityReciprocalLength;
  units[UOM::KeySemanticsReciprocalLength]                    = UOM::ReciprocalLengthUIDefault;

  quantities[UOM::KeySemanticsForhheimerCoefficient]          = UOM::PhysicalQuantityReciprocalLength;
  units[UOM::KeySemanticsForhheimerCoefficient]               = UOM::ReciprocalLengthUIDefault;

  //// Ratios
  quantities[UOM::KeySemanticsPlainRatio]                     = UOM::PhysicalQuantityPlainRatio;
  units[UOM::KeySemanticsPlainRatio]                          = UOM::PlainRatioUiDefault;

  quantities[UOM::KeySemanticsPlainRatioVolume]               = UOM::PhysicalQuantityPlainRatio;
  units[UOM::KeySemanticsPlainRatioVolume]                    = UOM::PlainRatioVolumeUIDefault;

  quantities[UOM::KeySemanticsPlainRatioReservoirVolume]      = UOM::PhysicalQuantityPlainRatio;
  units[UOM::KeySemanticsPlainRatioReservoirVolume]           = UOM::PlainRatioReservoirVolumeUIDefault;

  quantities[UOM::KeySemanticsPlainRatioStandardVolume]       = UOM::PhysicalQuantityPlainRatio;
  units[UOM::KeySemanticsPlainRatioStandardVolume]            = UOM::PlainRatioStandardVolumeUIDefault;

  //// Mass rate
  quantities[UOM::KeySemanticsMassRate]                       = UOM::PhysicalQuantityMassRate;
  units[UOM::KeySemanticsMassRate]                            = UOM::MassRateUIDefault;

  //// Volume rate
  quantities[UOM::KeySemanticsVolumeRate]                     = UOM::PhysicalQuantityVolumeRate;
  units[UOM::KeySemanticsVolumeRate]                          = UOM::VolumeRateUIDefault;

  //// Consistency
  quantities[UOM::KeySemanticsConsistency]                    = UOM::PhysicalQuantityConsistency;
  units[UOM::KeySemanticsConsistency]                         = UOM::ConsistencyPascalSecondToTheN;

  //// Viscosity
  quantities[UOM::KeySemanticsViscosity]                      = UOM::PhysicalQuantityViscosity;
  units[UOM::KeySemanticsViscosity]                           = UOM::ViscosityCentiPoise;

  //// Angle
  quantities[UOM::KeySemanticsAngle]                          = UOM::PhysicalQuantityAngle;
  units[UOM::KeySemanticsAngle]                               = UOM::AngleDegrees;

  //// Conductivity
  quantities[UOM::KeySemanticsConductivity]                   = UOM::PhysicalQuantityConductivity;
  units[UOM::KeySemanticsConductivity]                        = UOM::ConductivityMilliDarcyMeter;

  //// Measureless
  quantities[UOM::KeySemanticsMeasureless]                    = UOM::PhysicalQuantityMeasureless;
  units[UOM::KeySemanticsMeasureless]                         = UOM::MeasurelessGeneric;

  quantities[UOM::KeySemanticsMeasurelessSilent]              = UOM::PhysicalQuantityMeasureless;
  units[UOM::KeySemanticsMeasurelessSilent]                   = UOM::MeasurelessGeneric;

  quantities[UOM::KeySemanticsMeasurelessUnitFraction]        = UOM::PhysicalQuantityMeasureless;
  units[UOM::KeySemanticsMeasurelessUnitFraction]             = UOM::MeasurelessUnitFraction;

  quantities[UOM::KeySemanticsMeasurelessEfficiency]          = UOM::PhysicalQuantityMeasureless;
  units[UOM::KeySemanticsMeasurelessEfficiency]               = UOM::MeasurelessPercent;

  quantities[UOM::KeySemanticsMeasurelessPorosity]            = UOM::PhysicalQuantityMeasureless;
  units[UOM::KeySemanticsMeasurelessPorosity]                 = UOM::MeasurelessPercent;

  //// Concentrations / Mass
  quantities[UOM::KeySemanticsConcentrationMass]              = UOM::PhysicalQuantitySpecificGravity;
  units[UOM::KeySemanticsConcentrationMass]                   = UOM::SpecificGravityUIDefault;

  //// Concentrations / Volume
  quantities[UOM::KeySemanticsConcentrationVolume]            = UOM::PhysicalQuantityConcentration;
  units[UOM::KeySemanticsConcentrationVolume]                 = UOM::ConcentrationUIDefault;

  quantities[UOM::KeySemanticsWaveTravelTime]                 = UOM::PhysicalQuantityWaveTravelTime;
  units[UOM::KeySemanticsWaveTravelTime]                      = (int)UOM::WaveTravelTime::WaveTravelTimeDefault;

  quantities[UOM::KeySemanticsGammaRadiation]                 = UOM::PhysicalQuantityGammaRadiation;
  units[UOM::KeySemanticsGammaRadiation]                      = (int)UOM::GammaRadiation::GammaRadiationDefault;

  quantities[UOM::KeySemanticsSignalIntensity]                = UOM::PhysicalQuantitySignalIntensity;
  units[UOM::KeySemanticsSignalIntensity]                     = (int)UOM::SignalIntensity::SignalIntensityDefault;

  quantities[UOM::KeySemanticsCarterLeaks]                    = UOM::PhysicalQuantityCarterLeaks;
  units[UOM::KeySemanticsCarterLeaks]                         = (int)UOM::CarterLeaks::CarterLeaksDefault;

  quantities[UOM::KeySemanticsPotentialDifference]            = UOM::PhysicalQuantityPotentialDifference;
  units[UOM::KeySemanticsPotentialDifference]                 = (int)UOM::PotentialDifference::PotentialDifferenceDefault;

  quantities[UOM::KeySemanticsSpecificElectricalConductivity] = UOM::PhysicalQuantitySpecificElectricalConductivity;
  units[UOM::KeySemanticsSpecificElectricalConductivity]      = (int)UOM::SpecificElectricalConductivity::SpecificElectricalConductivityDefault;

  quantities[UOM::KeySemanticsResistivity]                    = UOM::PhysicalQuantityResistivity;
  units[UOM::KeySemanticsResistivity]                         = (int)UOM::Resistivity::ResistivityDefault;

  quantities[UOM::KeySemanticsFrequency]                      = UOM::PhysicalQuantityFrequency;
  units[UOM::KeySemanticsFrequency]                           = (int)UOM::Frequency::FrequencyDefault;

  quantities[UOM::KeySemanticsPieces]                         = UOM::PhysicalQuantityPieces;
  units[UOM::KeySemanticsPieces]                              = (int)UOM::Pieces::PiecesDefault;

  //// Store
  Settings[Details::KeySemanticsQuantities] = quantities;
  Settings[Details::KeySemanticsUiUnits]    = units;

  for (auto it = units.begin(); it != units.end(); ++it)
    emit UnitsChanged(it.key(), it.value().toInt());
}

void UOMSettings::SetupSemanticsDisplayNames()
{
  /////////////////////////////////////////////////////////////////////////////////////////
  //// Lengths
  UOM::SemanticsDisplayNames[UOM::KeySemanticsLength]                     = tr("Length");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsDepth]                      = tr("Depth");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPipeDiameter]               = tr("Pipe diameter");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPerforationDiameter]        = tr("Perforation diameter");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsFractureWidth]              = tr("Fracture width");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsFractureLength]             = tr("Fracture length");

  //// Pressures
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPressure]                   = tr("Pressure");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsYoungModulus]               = tr("Young's modulus");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsStress]                     = tr("Stress");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPorePressure]               = tr("Pore pressure");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPipePressure]               = tr("Tubing pressure");

  //// Pressures over lengths
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPressureOverLength]         = tr("Linear pressure");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsStressGradient]             = tr("Stress gradient");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPorePressureGradient]       = tr("Pore pressure gradient");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsFluidLinearPressure]        = tr("Fluid pressure loss");

  //// Masses over volume
  UOM::SemanticsDisplayNames[UOM::KeySemanticsSpecificGravity]            = tr("Specific Gravity");
  
  //// Concentrations
  UOM::SemanticsDisplayNames[UOM::KeySemanticsConcentrationMass]          = tr("Concentration");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsConcentrationVolume]        = tr("Volume concentration");

  //// Measureless
  UOM::SemanticsDisplayNames[UOM::KeySemanticsMeasureless]                = tr("Measureless quantity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsMeasurelessSilent]          = tr("Measureless quantity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsMeasurelessUnitFraction]    = tr("Measureless quantity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsMeasurelessEfficiency]      = tr("Efficiency");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsMeasurelessPorosity]        = tr("Porosity");

  //// One-to-one semantics to quantity mapping
  UOM::SemanticsDisplayNames[UOM::KeySemanticsToughness]                  = tr("Toughness");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsLeakoffCoefficient]         = tr("Leakoff Coefficient");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsSpurtLossCoefficient]       = tr("Spurt Loss Coefficient");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsTemperature]                = tr("Temperature");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsEnergyOverTemperature]      = tr("Heat Capacity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsHeatCapacity]               = tr("Heat Capacity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsEnergyOverMassTemperature]  = tr("Specific Heat Capacity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsMassSpecificHeatCapacity]   = tr("Specific Heat Capacity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsThermalConductivity]        = tr("Thermal Conductivity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPermeability]               = tr("Permeability");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsTime]                       = tr("Time");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPeriod]                     = tr("Period");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPeriodNPVProduction]        = tr("Period NPV Production");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPeriodNPVPrices]            = tr("Period NPV Prices");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPeriodNPVUtilization]       = tr("Period NPV Utilization");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsMass]                       = tr("Mass");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsVolume]                     = tr("Volume");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsSpecificGravityOverLength]  = tr("Linear density");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsSpecificGravityOverArea]    = tr("Areal density");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsCompressibility]            = tr("Compressibility");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsReciprocalLength]           = tr("Reciprocal length");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPlainRatio]                 = tr("Ratio");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPlainRatioVolume]           = tr("Volumes ratio");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsPlainRatioReservoirVolume]  = tr("Reservoir volumes ratio");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsForhheimerCoefficient]      = tr("Forhheimer coefficient");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsMassRate]                   = tr("Mass Rate");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsVolumeRate]                 = tr("Volume Rate");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsConcentrationMass]          = tr("Concentration");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsConsistency]                = tr("Consistency");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsViscosity]                  = tr("Viscosity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsAngle]                      = tr("Angle");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsConductivity]               = tr("Conductivity");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsMeasureless]                = tr("Measureless");
  UOM::SemanticsDisplayNames[UOM::KeySemanticsProductionOverPeriod]       = tr("Production over period");
}

void UOMSettings::SetupSemanticsAcceptableUiUnits()
{
  //// Depth
  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsDepth] = 
  {
    UOM::LengthMeter,
    UOM::LengthFoot,
  };

  //// Pipe diameter
  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsPipeDiameter] = 
  {
    UOM::LengthMeter,
    UOM::LengthMillimiter,
    UOM::LengthInch,
  };

  //// Perforation diameter, proppant diameter
  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsPerforationDiameter] = 
  {
    UOM::LengthMillimiter,
    UOM::LengthInch,
  };

  //// Time
  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsTime] = 
  {
    UOM::TimeSeconds,
    UOM::TimeMinutes,
    UOM::TimeHours,
  };

  //// Measureless
  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsMeasurelessSilent] = 
  {
    UOM::MeasurelessGeneric
  };

  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsMeasurelessUnitFraction] =
  {
    UOM::MeasurelessUnitFraction
  };

  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsMeasurelessEfficiency] =
  {
    UOM::MeasurelessPercent
  };

  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsMeasurelessPorosity] =
  {
    UOM::MeasurelessUnitFraction,
    UOM::MeasurelessPercent
  };

  //// Ratios
  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsPlainRatio] =
  {
    UOM::PlainRatioGeneric,
  };
  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsPlainRatioVolume] =
  {
    UOM::PlainRatioVolumeMeterCubed,
    UOM::PlainRatioVolumeFootCubed,
  };
  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsPlainRatioStandardVolume] =
  {
    UOM::PlainRatioVolumeStandardMeterCubed,
    UOM::PlainRatioVolumeStandardFootCubed,
  };
  UOM::SemanticsAcceptableUiUnits[UOM::KeySemanticsPlainRatioReservoirVolume] =
  {
    UOM::PlainRatioRservoirVolumeReservoirMeterCubedOverStandardMeterCubed,
    UOM::PlainRatioRservoirVolumeReservoirFootCubedOverStandardFootCubed,
  };
}

void UOMSettings::RetranslateUi()
{
  SetupSemanticsDisplayNames();
}

//// Semantic properties setters / getters
UOM::PhysicalQuantity UOMSettings::GetQuantityForSemantics(const QString& semantics)
{
  QVariantMap quantities = Settings[Details::KeySemanticsQuantities].toMap();
  return (UOM::PhysicalQuantity)quantities.value(semantics, UOM::PhysicalQuantityNone).toInt();
}

void UOMSettings::SetQuantityForSemantics(const QString& semantics, UOM::PhysicalQuantity quantity)
{
  QVariantMap quantities = Settings[Details::KeySemanticsQuantities].toMap();
  quantities[semantics] = quantity;
  Settings[Details::KeySemanticsQuantities] = quantities;
  Save();
  emit QuantitiesChanged(semantics, quantity);
}

QString UOMSettings::GetDefaultSemanticsForQuantity(UOM::PhysicalQuantity quantity)
{
  switch (quantity)
  {
  case UOM::PhysicalQuantityNone:
    break;
  case UOM::PhysicalQuantityLength:
    return UOM::KeySemanticsLength;
  case UOM::PhysicalQuantityPressure:
    return UOM::KeySemanticsPressure;
  case UOM::PhysicalQuantityPressureOverLength:
    return UOM::KeySemanticsPressureOverLength;
  case UOM::PhysicalQuantityToughness:
    return UOM::KeySemanticsToughness;
  case UOM::PhysicalQuantityLeakoffCoefficient:
    return UOM::KeySemanticsLeakoffCoefficient;
  case UOM::PhysicalQuantitySpurtLossCoefficient:
    return UOM::KeySemanticsSpurtLossCoefficient;
  case UOM::PhysicalQuantityTemperature:
    return UOM::KeySemanticsTemperature;
  case UOM::PhysicalQuantityEnergyOverTemperature:
    return UOM::KeySemanticsEnergyOverTemperature;
  case UOM::PhysicalQuantityEnergyOverMassTemperature:
    return UOM::KeySemanticsEnergyOverMassTemperature;
  case UOM::PhysicalQuantityThermalConductivity:
    return UOM::KeySemanticsThermalConductivity;
  case UOM::PhysicalQuantityPermeability:
    return UOM::KeySemanticsPermeability;
  case UOM::PhysicalQuantityTime:
    return UOM::KeySemanticsTime;
  case UOM::PhysicalQuantityPeriod:
    return UOM::KeySemanticsPeriod;
  case UOM::PhysicalQuantityMass:
    return UOM::KeySemanticsMass;
  case UOM::PhysicalQuantityVolume:
    return UOM::KeySemanticsVolume;
  case UOM::PhysicalQuantitySpecificGravity:
    return UOM::KeySemanticsSpecificGravity;
  case UOM::PhysicalQuantitySpecificGravityOverLength:
    return UOM::KeySemanticsSpecificGravityOverLength;
  case UOM::PhysicalQuantityMassRate:
    return UOM::KeySemanticsMassRate;
  case UOM::PhysicalQuantityVolumeRate:
    return UOM::KeySemanticsVolumeRate;
  case UOM::PhysicalQuantityConsistency:
    return UOM::KeySemanticsConsistency;
  case UOM::PhysicalQuantityViscosity:
    return UOM::KeySemanticsViscosity;
  case UOM::PhysicalQuantityAngle:
    return UOM::KeySemanticsAngle;
  case UOM::PhysicalQuantityConductivity:
    return UOM::KeySemanticsConductivity;
  case UOM::PhysicalQuantityMeasureless:
    return UOM::KeySemanticsMeasureless;
  case UOM::PhysicalQuantitySpecificGravityOverArea:
    return UOM::KeySemanticsSpecificGravityOverArea;
  case UOM::PhysicalQuantityCompressibility:
    return UOM::KeySemanticsCompressibility;
  case UOM::PhysicalQuantityReciprocalLength:
    return UOM::KeySemanticsReciprocalLength;
  case UOM::PhysicalQuantityPlainRatio:
    return UOM::KeySemanticsPlainRatio;
  case UOM::PhysicalQuantityProductionOverPeriod:
    return UOM::KeySemanticsProductionOverPeriod;
  case UOM::PhysicalQuantityConcentration:
    return UOM::KeySemanticsConcentrationVolume;
  case UOM::PhysicalQuantityWaveTravelTime:
    return UOM::KeySemanticsWaveTravelTime;
  case UOM::PhysicalQuantityGammaRadiation:
    return UOM::KeySemanticsGammaRadiation;
  case UOM::PhysicalQuantitySignalIntensity:
    return UOM::KeySemanticsSignalIntensity;
  case UOM::PhysicalQuantityCarterLeaks:
    return UOM::KeySemanticsCarterLeaks;
  case UOM::PhysicalQuantityPotentialDifference:
    return UOM::KeySemanticsPotentialDifference;
  case UOM::PhysicalQuantitySpecificElectricalConductivity:
    return UOM::KeySemanticsSpecificElectricalConductivity;
  case UOM::PhysicalQuantityResistivity:
    return UOM::KeySemanticsResistivity;
  case UOM::PhysicalQuantityFrequency:
    return UOM::KeySemanticsFrequency;
  case UOM::PhysicalQuantityPieces:
    return UOM::KeySemanticsPieces;
  case UOM::PhysicalQuantityLast:
    break;
  default:
    break;
  }
  return QString();
}

int UOMSettings::GetUiUnitsForSemantics(const QString& semantics)
{
  QVariantMap quantities = Settings[Details::KeySemanticsQuantities].toMap();
  UOM::PhysicalQuantity quantity = (UOM::PhysicalQuantity)quantities.value(semantics, UOM::PhysicalQuantityNone).toInt();
  if (quantity != UOM::PhysicalQuantityNone)
  {
    QVariantMap units = Settings[Details::KeySemanticsUiUnits].toMap();
    return units.value(semantics, UOM::AnyNone).toInt();
  }
  return UOM::AnyNone;
}

void UOMSettings::SetUiUnitsForSemantics(const QString& semantics, int units)
{
  QVariantMap quantities = Settings[Details::KeySemanticsUiUnits].toMap();
  if (quantities[semantics] != units)
  { 
    quantities[semantics] = units;
    Settings[Details::KeySemanticsUiUnits] = quantities;
    Save();
    emit UnitsChanged(semantics, units);

    ///////////////////////////////////////////////////////////////////////
    //// Some implicit semantic dependencies
    static QMap<QString/*from*/, QSet<QString>/*to*/> dependencies;
    if (dependencies.empty())
    {
      //// Dependencies map
      dependencies[UOM::KeySemanticsStressGradient] = { UOM::KeySemanticsStress, UOM::KeySemanticsLength };

      //// Fill mutual dependencies
      QMap<QString/*from*/, QSet<QString>/*to*/> dependenciesBackwards;
      for (auto d = dependencies.begin(); d != dependencies.end(); d++)
      {
        for (auto dd = d.value().begin(); dd != d.value().end(); dd++)
        {
          dependenciesBackwards[*dd] << d.key();
        }
      }

      //// Back to map
      for (auto d = dependenciesBackwards.begin(); d != dependenciesBackwards.end(); d++)
      {
        for (auto dd = d.value().begin(); dd != d.value().end(); dd++)
        {
          dependencies[d.key()] << *dd;
        }
      }
    }

    //// Mapping
    typedef QPair<QString/*semantics*/, int/*uom*/> mappingRecord;
    static QMultiMap<mappingRecord, mappingRecord> dependenciesMapping;
    if (dependenciesMapping.empty())
    {
      dependenciesMapping = {
        
        //// StressGradient -> Depth
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthPascalOverMeter },        { UOM::KeySemanticsDepth, UOM::LengthMeter } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthKiloPascalOverMeter },    { UOM::KeySemanticsDepth, UOM::LengthMeter } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthMegaPascalOverMeter },    { UOM::KeySemanticsDepth, UOM::LengthMeter } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthGigaPascalOverMeter },    { UOM::KeySemanticsDepth, UOM::LengthMeter } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthAtmosphereOverMeter },    { UOM::KeySemanticsDepth, UOM::LengthMeter } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthBarOverMeter },           { UOM::KeySemanticsDepth, UOM::LengthMeter } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthPSIOverMeter },           { UOM::KeySemanticsDepth, UOM::LengthMeter } },
        
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthPSIOverFoot },            { UOM::KeySemanticsDepth, UOM::LengthFoot } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthBarOverFoot },            { UOM::KeySemanticsDepth, UOM::LengthFoot } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthPascalOverFoot },         { UOM::KeySemanticsDepth, UOM::LengthFoot } },
        
        //// StressGradient -> Stress
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthPascalOverMeter },        { UOM::KeySemanticsStress, UOM::PressurePascal } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthKiloPascalOverMeter },    { UOM::KeySemanticsStress, UOM::PressureKiloPascal } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthMegaPascalOverMeter },    { UOM::KeySemanticsStress, UOM::PressureMegaPascal } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthGigaPascalOverMeter },    { UOM::KeySemanticsStress, UOM::PressureGigaPascal } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthAtmosphereOverMeter },    { UOM::KeySemanticsStress, UOM::PressureAtmosphere } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthBarOverMeter },           { UOM::KeySemanticsStress, UOM::PressureBar } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthPSIOverMeter },           { UOM::KeySemanticsStress, UOM::PressurePSI } },
        
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthPSIOverFoot },            { UOM::KeySemanticsStress, UOM::PressurePSI } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthBarOverFoot },            { UOM::KeySemanticsStress, UOM::PressureBar } },
        { { UOM::KeySemanticsStressGradient, UOM::PressureOverLengthPascalOverFoot },         { UOM::KeySemanticsStress, UOM::PressurePascal } },
      };
    }

    //// Dual dependencies mapping
    static std::map<QPair<mappingRecord, mappingRecord>, mappingRecord> dualDependenciesMapping;
    if (dualDependenciesMapping.empty())
    {
      //// Dual dependencies mapping for StressGradient
      {
        QMap<mappingRecord, QSet<int>> dependenciesMappingStressGradientNumerator = {
          { { UOM::KeySemanticsStress, UOM::PressurePascal },     { UOM::PressureOverLengthPascalOverMeter,
                                                                    UOM::PressureOverLengthPascalOverFoot
                                                                  } },
          { { UOM::KeySemanticsStress, UOM::PressureKiloPascal }, { UOM::PressureOverLengthKiloPascalOverMeter,
                                                                    UOM::PressureOverLengthPascalOverFoot
                                                                  } },
          { { UOM::KeySemanticsStress, UOM::PressureMegaPascal }, { UOM::PressureOverLengthMegaPascalOverMeter,
                                                                    UOM::PressureOverLengthPascalOverFoot
                                                                  } },
          { { UOM::KeySemanticsStress, UOM::PressureGigaPascal }, { UOM::PressureOverLengthGigaPascalOverMeter,
                                                                    UOM::PressureOverLengthPascalOverFoot
                                                                  } },
          { { UOM::KeySemanticsStress, UOM::PressurePSI},         { UOM::PressureOverLengthPSIOverFoot,
                                                                    UOM::PressureOverLengthPSIOverMeter,
                                                                  } },
          { { UOM::KeySemanticsStress, UOM::PressureAtmosphere }, { UOM::PressureOverLengthAtmosphereOverMeter } },
          { { UOM::KeySemanticsStress, UOM::PressureBar },        { UOM::PressureOverLengthBarOverFoot,
                                                                    UOM::PressureOverLengthBarOverMeter,
                                                                  } },
        };

        QMap<mappingRecord, QSet<int>> dependenciesMappingStressGradientDenominator = {
          { { UOM::KeySemanticsDepth, UOM::LengthMeter }, { 
                                                            UOM::PressureOverLengthPascalOverMeter,
                                                            UOM::PressureOverLengthKiloPascalOverMeter,
                                                            UOM::PressureOverLengthMegaPascalOverMeter,
                                                            UOM::PressureOverLengthGigaPascalOverMeter,
                                                            UOM::PressureOverLengthAtmosphereOverMeter,
                                                            UOM::PressureOverLengthBarOverMeter,
                                                            UOM::PressureOverLengthPSIOverMeter
                                                          } },
          { { UOM::KeySemanticsDepth, UOM::LengthMillimiter }, { 
                                                            UOM::PressureOverLengthPascalOverMeter,
                                                            UOM::PressureOverLengthKiloPascalOverMeter,
                                                            UOM::PressureOverLengthMegaPascalOverMeter,
                                                            UOM::PressureOverLengthGigaPascalOverMeter,
                                                            UOM::PressureOverLengthAtmosphereOverMeter,
                                                            UOM::PressureOverLengthBarOverMeter,
                                                            UOM::PressureOverLengthPSIOverMeter
                                                          } },
          { { UOM::KeySemanticsDepth, UOM::LengthFoot }, {
                                                            UOM::PressureOverLengthPSIOverFoot,
                                                            UOM::PressureOverLengthBarOverFoot,
                                                            UOM::PressureOverLengthPascalOverFoot,
                                                          } },
          { { UOM::KeySemanticsDepth, UOM::LengthInch }, {
                                                            UOM::PressureOverLengthPSIOverFoot,
                                                            UOM::PressureOverLengthBarOverFoot,
                                                            UOM::PressureOverLengthPascalOverFoot,
                                                          } },
        };

        //// Fill 
        for (auto dn = dependenciesMappingStressGradientNumerator.begin(); dn != dependenciesMappingStressGradientNumerator.end(); dn++)
        {
          for (auto dd = dependenciesMappingStressGradientDenominator.begin(); dd != dependenciesMappingStressGradientDenominator.end(); dd++)
          {
            mappingRecord k1 = dn.key();
            mappingRecord k2 = dd.key();

            QSet<int> v1 = dn.value();
            QSet<int> v2 = dd.value();
            QSet<int> vs = v1.intersect(v2);

            if (!vs.empty())
            {
              dualDependenciesMapping[{k1, k2}] = { UOM::KeySemanticsStressGradient, *vs.begin() };
              dualDependenciesMapping[{k2, k1}] = { UOM::KeySemanticsStressGradient, *vs.begin() };
            }
          }
        }
      }
    }

    //// Dispatch to denepdent semantics
    QMap<QString/*semantics*/, int/*uom*/> dependentSemantics;
    mappingRecord appliedKey = { semantics, units };
    
    //// Straight mapping
    {
      auto m = dependenciesMapping.find(appliedKey);
      for (; m != dependenciesMapping.end() && m.key() == appliedKey; m++)
      {
        mappingRecord& d = m.value();
        dependentSemantics[d.first] = d.second;
      }
    }

    //// Dual mapping
    {
      //// Find first mapping
      auto firstMapping = std::find_if(dualDependenciesMapping.begin(), dualDependenciesMapping.end(), [appliedKey](const std::pair<QPair<mappingRecord, mappingRecord>, mappingRecord>& record)
      {
        return record.first.first == appliedKey;
      });

      //// Proceed with sibling semantic dependency
      if (firstMapping != dualDependenciesMapping.end())
      {
        QString siblingDependency = firstMapping->first/*applied, sibling*/.second/*sibling*/.first/*semantics*/;
        int siblingDependencyUoms = quantities[siblingDependency].toInt();

        //// Now, get precise mapping
        mappingRecord appliedSiblingKey = { siblingDependency, siblingDependencyUoms };
        auto m = dualDependenciesMapping.find({ appliedKey, appliedSiblingKey });
        if (m != dualDependenciesMapping.end())
        {
          dependentSemantics[m->second.first] = m->second.second;
        }
      }
    }

    //// Do apply dependents
    if (!dependentSemantics.empty())
    {
      for (auto d = dependentSemantics.begin(); d != dependentSemantics.end(); d++)
      {
        quantities[d.key()] = d.value();
      }

      Settings[Details::KeySemanticsUiUnits] = quantities;
      Save();

      for (auto d = dependentSemantics.begin(); d != dependentSemantics.end(); d++)
      {
        emit UnitsChanged(d.key(), d.value());
      }
    }
  }
}

QSet<int> UOMSettings::GetAcceptableUiUnitsForSemantics(const QString& semantics)
{
  QSet<int>& explicitAcceptableUiUnits = UOM::SemanticsAcceptableUiUnits[semantics];
  if (!explicitAcceptableUiUnits.empty())
  {
    return explicitAcceptableUiUnits;
  }
  else
  {
    UOM::PhysicalQuantity quantity = GetQuantityForSemantics(semantics);
    if (quantity != UOM::PhysicalQuantityNone)
    {
      int unitsCount = UOM::GetUnitsCount(quantity);
      QSet<int> units;
      for (int u = UOM::AnyFirst; u < unitsCount; u++)
      {
        units << u;
      }
      return units;
    }
  }
  return QSet<int>();
}

int UOMSettings::GetLegacyUntaggedUnitsForematics(const QString& semantics)
{
  QVariantMap quantities = Settings[Details::KeySemanticsLegacyUntaggedUnits].toMap();
  UOM::PhysicalQuantity quantity = (UOM::PhysicalQuantity)quantities.value(semantics, UOM::PhysicalQuantityNone).toInt();
  if (quantity != UOM::PhysicalQuantityNone)
  {
    QVariantMap units = Settings[Details::KeySemanticsLegacyUntaggedUnits].toMap();
    return units.value(semantics, UOM::AnyNone).toInt();
  }
  return UOM::AnyNone;
}

void UOMSettings::Load(const QVariantMap& object)
{
  QVariantMap unitsObject = object[Details::KeySemanticsUiUnits].toMap();
  QVariantMap units = Settings[Details::KeySemanticsUiUnits].toMap();
  for (auto u = unitsObject.begin(); u != unitsObject.end(); ++u)
  {
    int unit = UOM::KeysUnitsBackwards.value(u.value().toString());
    if (unit != UOM::AnyNone)
    {
      if(units[u.key()] != unit)
      {
        units[u.key()] = unit;
        emit UnitsChanged(u.key(), unit);
      }
    }
  }

  Settings[Details::KeySemanticsUiUnits] = units;
}

void UOMSettings::Save(QVariantMap& object)
{
  QVariantMap quantitiesSettings = Settings[Details::KeySemanticsQuantities].toMap();
  
  QVariantMap units;
  QVariantMap unitsSettings = Settings[Details::KeySemanticsUiUnits].toMap();
  for (auto u = unitsSettings.begin(); u != unitsSettings.end(); ++u)
  {
    units[u.key()] = UOM::KeysUnits.value({ quantitiesSettings.value(u.key()).toInt(), u.value().toInt() });
  }

  object[Details::KeySemanticsUiUnits] = units;
}

void UOMSettings::Load()
{
#if defined(USE_SETTINGS)
  QVariantMap object = Common::Settings::Get().GetSection(Details::UOMSettingsSavePath + "/UOM");
  Load(object);
#endif
}

void UOMSettings::Save()
{
#if defined(USE_SETTINGS)
  QVariantMap object;
  Save(object);
  Common::Settings::Get().SetSection(Details::UOMSettingsSavePath + "/UOM", object);
#endif
}

void UOMSettings::Export(QString filePath)
{
  Save();
#if defined(USE_SETTINGS)
#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
  Common::Settings::Get().SaveSectionToFile(Details::UOMSettingsSavePath + "/UOM", filePath);
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION
#endif
}

void UOMSettings::Import(QString filePath)
{
#if defined(USE_SETTINGS)
#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
  Common::Settings::Get().LoadSectionFromFile(Details::UOMSettingsSavePath + "/UOM", filePath);
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION
#endif
  Load();
}
