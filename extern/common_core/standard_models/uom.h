#pragma once

#include <QString>
#include <QMap>
#include <QSet>

namespace UOM
{
  //// Нумерация перечислений используется для хранения данных.
  //// Новым значениям нужно давать значения так, чтобы *Last
  //// имел значение, равное числу вариантов в перечислении и
  //// не менял нумерацию уже существующих единиц измерения
  
  ///////////////////////////////////////////////////////
  //// Physical Quantities
  enum PhysicalQuantity
  {
    PhysicalQuantityNone = 0,

    PhysicalQuantityLength = 1,
    PhysicalQuantityPressure = 2,
    PhysicalQuantityPressureOverLength = 3,
    PhysicalQuantityToughness = 4,
    PhysicalQuantityLeakoffCoefficient = 5,
    PhysicalQuantitySpurtLossCoefficient = 6,
    PhysicalQuantityTemperature = 7,
    PhysicalQuantityEnergyOverTemperature = 8,        // J/K
    PhysicalQuantityEnergyOverMassTemperature = 9,    // J/(kg*K)
    PhysicalQuantityThermalConductivity = 10,
    PhysicalQuantityPermeability = 11,
    PhysicalQuantityTime = 12,
    PhysicalQuantityPeriod = 13,
    PhysicalQuantityMass = 14,
    PhysicalQuantityVolume = 15,
    PhysicalQuantitySpecificGravity = 16,
    PhysicalQuantitySpecificGravityOverLength = 17,
    PhysicalQuantityMassRate = 18,
    PhysicalQuantityVolumeRate = 19,
    PhysicalQuantityConsistency = 20,
    PhysicalQuantityViscosity = 21,
    PhysicalQuantityAngle = 22,
    PhysicalQuantityConductivity = 23,
    PhysicalQuantityMeasureless = 24,                ///// e.g. 1.20 u.f. 120%
    PhysicalQuantitySpecificGravityOverArea = 25,
    PhysicalQuantityCompressibility = 26,
    PhysicalQuantityReciprocalLength = 27,
    PhysicalQuantityPlainRatio = 28,                 //// e.g. m^3/m^3 ft^3/ft^3
    PhysicalQuantityProductionOverPeriod = 29,
    PhysicalQuantityConcentration = 30,              //// e.g. l/m^3
    PhysicalQuantityWaveTravelTime = 31,
    PhysicalQuantityGammaRadiation = 32,
    PhysicalQuantitySignalIntensity = 33,
    PhysicalQuantityCarterLeaks = 34,
    PhysicalQuantityPotentialDifference = 35,
    PhysicalQuantitySpecificElectricalConductivity = 36,
    PhysicalQuantityResistivity = 37,
    PhysicalQuantityFrequency = 38,
    PhysicalQuantityPieces = 39,

    PhysicalQuantityLast
  };

  ///////////////////////////////////////////////////////
  //// Any quantity / no UOM
  enum Any
  {
    AnyNone,
    AnyFirst,

    AnyDimensionless = AnyNone,
  };

  ///////////////////////////////////////////////////////
  //// Length
  enum Length
  {
    LengthNone = 0,

    LengthMeter = 1,
    LengthMillimiter = 2,
    LengthFoot = 3,
    LengthInch = 4,

    LengthLast,

    LengthSI            = LengthMeter, // SI alias
    LengthUIDefault     = LengthSI,    // UI default
    LengthStored        = LengthSI,    // Stored in SI
  };

  ///////////////////////////////////////////////////////
  //// Pressure
  enum Pressure
  {
    PressureNone = 0,

    PressurePascal = 1,
    PressureKiloPascal = 2,
    PressureMegaPascal = 3,
    PressureGigaPascal = 4,
    PressurePSI = 5,
    PressureAtmosphere = 6,
    PressureBar = 7,

    PressureLast,

    PressureSI                     = PressurePascal,         // SI alias
    PressureUIDefault              = PressureAtmosphere,     // Default UI units
    PressureStored                 = PressureSI,             // Store in SI unit
  };

  ///////////////////////////////////////////////////////
  //// Pressure over Length
  enum PressureOverLength
  {
    PressureOverLengthNone = 0,

    PressureOverLengthPascalOverMeter = 1,
    PressureOverLengthKiloPascalOverMeter = 2,
    PressureOverLengthMegaPascalOverMeter = 3,
    PressureOverLengthGigaPascalOverMeter = 4,
    PressureOverLengthAtmosphereOverMeter = 5,
    PressureOverLengthBarOverMeter = 6,

    PressureOverLengthPSIOverMeter = 7,
    PressureOverLengthPSIOverFoot = 8,
    PressureOverLengthBarOverFoot = 9,
    PressureOverLengthPascalOverFoot = 10,

    PressureOverLengthLast,

    PressureOverLengthSI                    = PressureOverLengthPascalOverMeter,      // SI alias
    PressureOverLengthUIDefault             = PressureOverLengthAtmosphereOverMeter,  // Default UI
    PressureOverLengthStored                = PressureOverLengthSI,                   // Store in SI
  };

  ///////////////////////////////////////////////////////
  //// Toughness
  enum Toughness
  {
    ToughnessNone = 0,

    ToughnessAtmosphereMeterToTheOneHalf = 1,
    ToughnessPascalMeterToTheOneHalf = 2,
    ToughnessKiloPascalMeterToTheOneHalf = 3,
    ToughnessMegaPascalMeterToTheOneHalf = 4,

    ToughnessLast,

    ToughnessSI         = ToughnessPascalMeterToTheOneHalf,     // SI
    ToughnessUIDefault  = ToughnessAtmosphereMeterToTheOneHalf, // Conventional atmospheres
    ToughnessStored     = ToughnessSI,                          // SI
  };

  ///////////////////////////////////////////////////////
  //// Leakoff Coefficient
  enum LeakoffCoefficient
  {
    LeakoffCoefficientNone = 0,

    LeakoffCoefficientMeterOverSecondToTheOneHalf = 1,
    LeakoffCoefficientMeterOverMinuteToTheOneHalf = 2,
    LeakoffCoefficientCentiMeterOverSecondToTheOneHalf = 3,
    LeakoffCoefficientCentiMeterOverMinuteToTheOneHalf = 4,
    LeakoffCoefficientFootOverSecondToTheOneHalf = 5,
    LeakoffCoefficientFootOverMinuteToTheOneHalf = 6,

    LeakoffCoefficientLast,

    LeakoffCoefficientSI        = LeakoffCoefficientMeterOverSecondToTheOneHalf, // SI
    LeakoffCoefficientUIDefault = LeakoffCoefficientMeterOverSecondToTheOneHalf, // SI
    LeakoffCoefficientStored    = LeakoffCoefficientMeterOverSecondToTheOneHalf, // SI
  };

  ///////////////////////////////////////////////////////
  //// Spurt Loss Coefficient
  enum SpurtLossCoefficient
  {
    SpurtLossCoefficientNone = 0,

    SpurtLossCoefficientMeterCubedOverMeterSquared = 1,
    SpurtLossCoefficientGallonOverFootSquared = 2,

    SpurtLossCoefficientLast,

    SpurtLossCoefficientSI        = SpurtLossCoefficientMeterCubedOverMeterSquared, // SI
    SpurtLossCoefficientUIDefault = SpurtLossCoefficientMeterCubedOverMeterSquared, // SI
    SpurtLossCoefficientStored    = SpurtLossCoefficientMeterCubedOverMeterSquared, // SI
  };

  ///////////////////////////////////////////////////////
  //// Temperature
  enum Temperature
  {
    TemperatureNone = 0,

    TemperatureCelsius = 1,
    TemperatureKelvin = 2,

    TemperatureLast,

    TemperatureSI         = TemperatureCelsius, // SI
    TemperatureUIDefault  = TemperatureCelsius, // SI
    TemperatureStored     = TemperatureCelsius, // SI
  };

  ///////////////////////////////////////////////////////
  //// Energy Over Temperature
  enum EnergyOverTemperature
  {
    EnergyOverTemperatureNone = 0,

    EnergyOverTemperatureJouleOverCelsius = 1,

    EnergyOverTemperatureLast,

    EnergyOverTemperatureSI         = EnergyOverTemperatureJouleOverCelsius, // SI
    EnergyOverTemperatureUIDefault  = EnergyOverTemperatureJouleOverCelsius, // SI
    EnergyOverTemperatureStored     = EnergyOverTemperatureJouleOverCelsius, // SI
  };

  ///////////////////////////////////////////////////////
  //// Energy Over Mass Temperature
  enum EnergyOverMassTemperature
  {
    EnergyOverMassTemperatureNone = 0,

    EnergyOverMassTemperatureJouleOverKilogramCelsius = 1,

    EnergyOverMassTemperatureLast,

    EnergyOverMassTemperatureSI         = EnergyOverMassTemperatureJouleOverKilogramCelsius, // SI
    EnergyOverMassTemperatureUIDefault  = EnergyOverMassTemperatureJouleOverKilogramCelsius, // SI
    EnergyOverMassTemperatureStored     = EnergyOverMassTemperatureJouleOverKilogramCelsius, // SI
  };

  ///////////////////////////////////////////////////////
  //// Thermal Conductivity
  enum ThermalConductivity
  {
    ThermalConductivityNone = 0,

    ThermalConductivityWattOverMeterCelsius = 1,

    ThermalConductivityLast,

    ThermalConductivitySI         = ThermalConductivityWattOverMeterCelsius, // SI
    ThermalConductivityUIDefault  = ThermalConductivityWattOverMeterCelsius, // SI
    ThermalConductivityStored     = ThermalConductivityWattOverMeterCelsius, // SI
  };

  ///////////////////////////////////////////////////////
  //// Permeability
  enum Permeability
  {
    PermeabilityNone = 0,

    PermeabilityMilliDarcy = 1,

    PermeabilityLast,

    PermeabilitySI        = PermeabilityMilliDarcy, // SI
    PermeabilityUIDefault = PermeabilityMilliDarcy, // SI
    PermeabilityStored    = PermeabilityMilliDarcy, // SI
  };

  //// Time
  enum Time
  {
    TimeNone = 0,

    TimeMilliSeconds = 1,
    TimeSeconds      = 2,
    TimeMinutes      = 3,
    TimeHours        = 4,
    TimeDays         = 5,

    TimeLast,

    TimeSI                    = TimeSeconds, // SI
    TimeUIDefault             = TimeMinutes, // Minutes
    TimeStored                = TimeSeconds, // SI
  };

  //// Period
  enum Period
  {
    PeriodNone = 0,

    PeriodDay = 1,
    PeriodMonth = 2,
    PeriodYear = 3,

    PeriodLast,

    PeriodSI        = PeriodDay,
    PeriodUIDefault = PeriodDay,
    PeriodStored    = PeriodDay
  };

  //// Production Over Period
  enum ProductionOverPeriod
  {
    ProductionOverPeriodNone = 0,

    ProductionOverPeriodStandardMeterCubedOverDay = 1,

    ProductionOverPeriodLast,

    ProductionOverPeriodSI        = ProductionOverPeriodStandardMeterCubedOverDay,
    ProductionOverPeriodUIDefault = ProductionOverPeriodStandardMeterCubedOverDay,
    ProductionOverPeriodStored    = ProductionOverPeriodStandardMeterCubedOverDay
  };

  //// Mass
  enum Mass
  {
    MassNone = 0,

    MassKilogram  = 1,
    MassTonne     = 2,
    MassPoundMass = 3,

    MassLast,

    MassSI                    = MassKilogram, // SI
    MassUIDefault             = MassKilogram, // SI
    MassStored                = MassKilogram, // SI
  };

  //// Volume
  enum Volume
  {
    VolumeNone = 0,

    VolumeMeterCubed = 1,
    VolumeLitre = 2,
    VolumeFootCubed = 3,
    VolumeBarrel = 4,

    VolumeLast,

    VolumeSI                  = VolumeMeterCubed, // SI
    VolumeUIDefault           = VolumeMeterCubed, // SI
    VolumeStored              = VolumeMeterCubed, // SI
  };

  //// Specific Gravity
  enum SpecificGravity
  {
    SpecificGravityNone = 0,

    SpecificGravityKilogramOverMeterCubed = 1,
    SpecificGravityPoundMassOverFootCubed = 2,
    SpecificGravityGrammOverCentimeterCubed = 3,
    SpecificGravityTonneOverMeterCubed = 4,

    SpecificGravityLast,

    SpecificGravitySI         = SpecificGravityKilogramOverMeterCubed, // SI
    SpecificGravityUIDefault  = SpecificGravityKilogramOverMeterCubed, // SI
    SpecificGravityStrored    = SpecificGravityKilogramOverMeterCubed, // SI
  };

  //// Specific Gravity Over Length
  enum SpecificGravityOverLength
  {
    SpecificGravityOverLengthNone = 0,

    SpecificGravityOverLengthKilogramOverMeter = 1,
    SpecificGravityOverLengthPoundOverFoot = 2,
    SpecificGravityOverLengthTonneOverMeter = 3,

    SpecificGravityOverLengthLast,

    SpecificGravityOverLengthSI         = SpecificGravityOverLengthKilogramOverMeter, // SI
    SpecificGravityOverLengthUIDefault  = SpecificGravityOverLengthKilogramOverMeter, // SI
    SpecificGravityOverLengthStored     = SpecificGravityOverLengthKilogramOverMeter, // SI
  };

  //// Specific Gravity Over Area
  enum SpecificGravityOverArea
  {
    SpecificGravityOverAreaNone = 0,

    SpecificGravityOverAreaKilogramOverMeterSquared = 1,

    SpecificGravityOverAreaLast,

    SpecificGravityOverAreaSI           = SpecificGravityOverAreaKilogramOverMeterSquared, // SI
    SpecificGravityOverAreaUIDefault    = SpecificGravityOverAreaKilogramOverMeterSquared, // SI
    SpecificGravityOverAreaStored       = SpecificGravityOverAreaKilogramOverMeterSquared, // SI
  };

  //// Compressibility
  enum Compressibility
  {
    CompressibilityNone = 0,

    CompressibilityPascalToTheMinusOne = 1,
    CompressibilityBarToTheMinusOne = 2,

    CompressibilityLast,

    CompressibilitySI           = CompressibilityPascalToTheMinusOne,   //// SI
    CompressibilityUIDefault    = CompressibilityBarToTheMinusOne,      //// Bar
    CompressibilityStored       = CompressibilitySI,                    //// SI
  };

  //// ReciprocalLength
  enum ReciprocalLength
  {
    ReciprocalLengthNone = 0,

    ReciprocalLengthOverMeter = 1,
    ReciprocalLengthOverFoot = 2,

    ReciprocalLengthLast,

    ReciprocalLengthSI         = ReciprocalLengthOverMeter, //// SI
    ReciprocalLengthUIDefault  = ReciprocalLengthOverMeter, //// SI
    ReciprocalLengthStored     = ReciprocalLengthSI         //// SI
  };

  //// Plain ratio - assumed to be used w/ specific semantics, see UOM::SemanticsAcceptableUiUnits[]
  enum PlainRatio
  {
    PlainRatioNone = 0,

    //// Generic
    PlainRatioGeneric = 1,            //// u.f.

    //// Specific / Volume ratios
    PlainRatioVolumeMeterCubed = 2,   //// m3 / m3
    PlainRatioVolumeFootCubed = 3,    //// ft3 / ft3

    //// Specific / Standard volume
    PlainRatioVolumeStandardMeterCubed = 4,   //// Sm3 / Sm3
    PlainRatioVolumeStandardFootCubed = 5,    //// Sft3 / Sft3

    //// Specific / Reservoir volume ratios
    PlainRatioRservoirVolumeReservoirMeterCubedOverStandardMeterCubed = 6,    //// Rm3 / Sm3
    PlainRatioRservoirVolumeReservoirFootCubedOverStandardFootCubed = 7,      //// Rft3 / Sft3

    PlainRatioLast,

    //// Generic defaults
    PlainRatioSI                = PlainRatioGeneric,
    PlainRatioUiDefault         = PlainRatioSI,
    PlainRatioStored            = PlainRatioSI,

    //// Specific / Volume defaults
    PlainRatioVolumeSI          = PlainRatioVolumeMeterCubed,
    PlainRatioVolumeUIDefault   = PlainRatioVolumeMeterCubed,
    PlainRatioVolumeStored      = PlainRatioVolumeSI,

    //// Specific / Rservoir volume defaults
    PlainRatioStandardVolumeSI           = PlainRatioVolumeStandardMeterCubed,
    PlainRatioStandardVolumeUIDefault    = PlainRatioVolumeStandardMeterCubed,
    PlainRatioStandardVolumeStored       = PlainRatioStandardVolumeSI,

    PlainRatioReservoirVolumeSI          = PlainRatioRservoirVolumeReservoirMeterCubedOverStandardMeterCubed,
    PlainRatioReservoirVolumeUIDefault   = PlainRatioRservoirVolumeReservoirMeterCubedOverStandardMeterCubed,
    PlainRatioReservoirVolumeStored      = PlainRatioReservoirVolumeSI,
  };

  //// Mass Rate
  enum MassRate
  {
    MassRateNone = 0,

    MassRateKilogramOverSecond = 1,
    MassRateKilogramOverMinute = 2,
    MassRateTonneOverMinute = 3,
    MassRatePoundMassOverSecond = 4,
    MassRatePoundMassOverMinute = 5,

    MassRateLast,

    MassRateSI                    = MassRateKilogramOverSecond, // SI
    MassRateUIDefault             = MassRateKilogramOverMinute, // Conventional over minute
    MassRateStored                = MassRateKilogramOverSecond, // SI
  };

  //// Volume Rate
  enum VolumeRate
  {
    VolumeRateNone = 0,

    VolumeRateMeterCubedOverSecond = 1,
    VolumeRateMeterCubedOverMinute = 2,
    VolumeRateMeterCubedOverDay = 3,
    VolumeRateLitreOverSecond = 4,
    VolumeRateLitreOverMinute = 5,
    VolumeRateBarrelOverMinute = 6,
    VolumeRateFootCubedOverSecond = 7,
    VolumeRateFootCubedOverMinute = 8,

    VolumeRateLast,

    VolumeRateSI                    = VolumeRateMeterCubedOverSecond, // SI
    VolumeRateUIDefault             = VolumeRateMeterCubedOverMinute, // Conventional minutes
    VolumeRateRateStored            = VolumeRateMeterCubedOverSecond, // SI
  };

  //// Consistency
  enum Consistency
  {
    ConsistencyNone = 0,

    ConsistencyPascalSecondToTheN = 1,
    ConsistencyPoundForceSecondToTheNOverFootSquared = 2,

    ConsistencyLast,

    ConsistencySI                   = ConsistencyPascalSecondToTheN, // SI
    ConsistencyUIDefault            = ConsistencyPascalSecondToTheN, // SI
    ConsistencyStored               = ConsistencySI
  };

  //// Viscosity
  enum Viscosity
  {
    ViscosityNone = 0,

    ViscosityPascalSecond = 1,
    ViscosityPoise = 2,
    ViscosityCentiPoise = 3,

    ViscosityLast,

    ViscositySI                     = ViscosityPascalSecond, // SI
    ViscosityUIDefault              = ViscosityCentiPoise,   // Convetional centi poise
    ViscosityStored                 = ViscositySI,
  };

  //// Angle
  enum Angle
  {
    AngleNone = 0,

    AngleRadians = 1,
    AngleDegrees = 2,

    AngleLast,

    AngleSI                         = AngleRadians, // SI
    AngleUIDefault                  = AngleDegrees, // Convetional
    AngleStored                     = AngleSI,      // Convetional
  };

  //// Conductivity
  enum Conductivity
  {
    ConductivityNone = 0,

    ConductivityMilliDarcyMeter = 1,

    ConductivityLast,

    ConductivitySI                  = ConductivityMilliDarcyMeter, // Conventional
    ConductivityUIDefault           = ConductivityMilliDarcyMeter, // Conventional
    ConductivityStored              = ConductivitySI,              // Conventional
  };

  //// Measuless
  enum Measureless
  {
    MeasurelessNone = 0,

    MeasurelessGeneric = 1,
    MeasurelessUnitFraction = 2,  //// == MeasurelessGeneric but has "u.f." suffix
    MeasurelessPercent = 3,

    MeasurelessLast,

    MeasurelessSI             = MeasurelessGeneric,
    MeasurelessUIDefault      = MeasurelessGeneric,
    MeasurelessStored         = MeasurelessGeneric,
  };

  enum Concentration 
  {
    ConcentrationNone = 0,

    ConcentrationLitreOverMeterCubed = 1,
    ConcentrationCoefficient = 2,

    ConcentrationLast,

    ConcentrationSI             = ConcentrationCoefficient,
    ConcentrationUIDefault      = ConcentrationCoefficient,
    ConcentrationStored         = ConcentrationCoefficient,
  };

  //// Wave travel time
  enum class WaveTravelTime 
  {
    WaveTravelTimeNone = 0,

    WaveTravelTimeSecondsPerMeter       = 1,
    WaveTravelTimeMicroSecondsPerMeter  = 2,
    WaveTravelTimeMicroSecondsPerFoot   = 3,

    WaveTravelTimeLast,

    WaveTravelTimeSI          = WaveTravelTimeSecondsPerMeter,
    WaveTravelTimeDefault     = WaveTravelTimeSecondsPerMeter,
    WaveTravelTimeStored      = WaveTravelTimeSecondsPerMeter,
  };

  //// Gamma radiation
  enum class GammaRadiation {
    GammaRadiationNone = 0,

    GammaRadiationRoentgenPerHour = 1,
    GammaRadiationMicroroentgenPerHour = 2,
    GammaRadiationAPI = 3,

    GammaRadiationLast,

    GammaRadiationSI          = GammaRadiationRoentgenPerHour,
    GammaRadiationDefault     = GammaRadiationRoentgenPerHour,
    GammaRadiationStored      = GammaRadiationRoentgenPerHour,
  };

  //// Signal intensity
  enum class SignalIntensity {
    SignalIntensityNone = 0,

    SignalIntensityDecibel = 1,

    SignalIntensityLast,

    SignalIntensitySI          = SignalIntensityDecibel,
    SignalIntensityDefault     = SignalIntensityDecibel,
    SignalIntensityStored      = SignalIntensityDecibel,
  };

  //// Carter Leaks
  enum class CarterLeaks 
  {
    CarterLeaksNone = 0,

    CarterLeaksMetersPerSecondToTheOneHalf = 1,
    CarterLeaksMetersPerMinuteToTheOneHalf = 2,
    CarterLeaksCentimetersPerSecondToTheOneHalf = 3,
    CarterLeaksCentimetersPerMinuteToTheOneHalf = 4,

    CarterLeaksLast,

    CarterLeaksSI          = CarterLeaksMetersPerSecondToTheOneHalf,
    CarterLeaksDefault     = CarterLeaksMetersPerSecondToTheOneHalf,
    CarterLeaksStored      = CarterLeaksMetersPerSecondToTheOneHalf,
  };

  //// Potential difference
  enum class PotentialDifference {
    PotentialDifferenceNone = 0,

    PotentialDifferenceVolt = 1,
    PotentialDifferenceMillivolt = 2,

    PotentialDifferenceLast,

    PotentialDifferenceSI          = PotentialDifferenceVolt,
    PotentialDifferenceDefault     = PotentialDifferenceVolt,
    PotentialDifferenceStored      = PotentialDifferenceVolt,
  };

  //// Specific electrical conductivity
  enum class SpecificElectricalConductivity {
    SpecificElectricalConductivityNone = 0,

    SpecificElectricalConductivitySiemensPerMeter = 1,
    SpecificElectricalConductivityMillisiemensPerMeter = 2,

    SpecificElectricalConductivityLast,

    SpecificElectricalConductivitySI = SpecificElectricalConductivitySiemensPerMeter,
    SpecificElectricalConductivityDefault = SpecificElectricalConductivitySiemensPerMeter,
    SpecificElectricalConductivityStored = SpecificElectricalConductivitySiemensPerMeter,
  };

  //// Resistivity
  enum class Resistivity {
    ResistivityNone = 0,

    ResistivityOhmMeter = 1,

    ResistivityLast,

    ResistivitySI         = ResistivityOhmMeter,
    ResistivityDefault    = ResistivityOhmMeter,
    ResistivityStored     = ResistivityOhmMeter,
  };

  //// Frequency
  enum class Frequency {
    FrequencyNone = 0,

    FrequencyHertz = 1,
    FrequencyKiloHertz = 2,
    FrequencyPulsesPerMinute = 3,

    FrequencyLast,

    FrequencySI         = FrequencyHertz,
    FrequencyDefault    = FrequencyHertz,
    FrequencyStored     = FrequencyHertz,
  };

  //// Frequency
  enum Pieces {
    PiecesNone = 0,

    PiecesPiece = 1,

    PiecesLast,

    PiecesSI         = PiecesPiece,
    PiecesDefault    = PiecesPiece,
    PiecesStored     = PiecesPiece,
  };

  //// Descriptor
  struct Descriptor
  {
    bool operator<(const Descriptor& other) const
    {
      if (Quantity < other.Quantity)
        return true;
      return Units < other.Units;
    }

    bool IsNull() const
    {
      return Quantity == PhysicalQuantityNone
          && Units    == AnyNone;
    }

    PhysicalQuantity  Quantity  = PhysicalQuantityNone;
    unsigned          Units     = AnyNone;
  };

  ///////////////////////////////////////////////////////
  //// Semantic keys
  extern const char* KeySemanticsNone;

  //// Lengths
  extern const char* KeySemanticsLength;                      // Generic length
  extern const char* KeySemanticsDepth;
  extern const char* KeySemanticsInclinometryMeasure;         // == KeySemanticsDepth (alias)
  extern const char* KeySemanticsPipeDiameter;
  extern const char* KeySemanticsPerforationDiameter;
  extern const char* KeySemanticsProppantDiameter;            // == KeySemanticsPerforationDiameter (alias)
  extern const char* KeySemanticsFractureWidth;
  extern const char* KeySemanticsFractureLength;

  //// Pressures
  extern const char* KeySemanticsPressure;                    // Generic
  extern const char* KeySemanticsYoungModulus;
  extern const char* KeySemanticsStressShiftModulus;          // == KeySemanticsYoungModulus, alias
  extern const char* KeySemanticsStress;
  extern const char* KeySemanticsPorePressure;
  extern const char* KeySemanticsPipePressure;

  //// Times
  extern const char* KeySemanticsTime;                        // Generic
  extern const char* KeySemanticsPumpingTime;                 // == KeySemanticsTime (alias)
  extern const char* KeySemanticsRheologicalTime;             // == KeySemanticsTime (alias)

  //// Period
  extern const char* KeySemanticsPeriod;                      // Generic
  extern const char* KeySemanticsPeriodNPVProduction;
  extern const char* KeySemanticsPeriodNPVPrices;
  extern const char* KeySemanticsPeriodNPVUtilization;

  //// Production Over Period
  extern const char* KeySemanticsProductionOverPeriod;        // Generic

  //// Pressures over lengths
  extern const char* KeySemanticsPressureOverLength;          // Generic
  extern const char* KeySemanticsStressGradient;
  extern const char* KeySemanticsPorePressureGradient;
  extern const char* KeySemanticsFluidLinearPressure;

  //// Masses over volume
  extern const char* KeySemanticsSpecificGravity;             // Generic

  //// Concentrations
  extern const char* KeySemanticsConcentrationMass;
  extern const char* KeySemanticsConcentrationVolume;

  ///// Measureless
  extern const char* KeySemanticsMeasureless;                 // Generic
  extern const char* KeySemanticsMeasurelessSilent;
  extern const char* KeySemanticsMeasurelessUnitFraction;
  extern const char* KeySemanticsMeasurelessEfficiency;
  extern const char* KeySemanticsMeasurelessPorosity;

  //// Energy over temperature
  extern const char* KeySemanticsEnergyOverTemperature;       // Generic
  extern const char* KeySemanticsHeatCapacity;

  //// Energy over mass temperature
  extern const char* KeySemanticsEnergyOverMassTemperature;   // Generic
  extern const char* KeySemanticsMassSpecificHeatCapacity;

  //// Reciprocal lengths
  extern const char* KeySemanticsReciprocalLength;            // Generic
  extern const char* KeySemanticsForhheimerCoefficient;

  //// Plain ratios
  extern const char* KeySemanticsPlainRatio;                  // Generic
  extern const char* KeySemanticsPlainRatioVolume;
  extern const char* KeySemanticsPlainRatioStandardVolume;
  extern const char* KeySemanticsPlainRatioReservoirVolume;

  //// One-to-one semantics to quantity mapping (generics)
  extern const char* KeySemanticsToughness;
  extern const char* KeySemanticsLeakoffCoefficient;
  extern const char* KeySemanticsSpurtLossCoefficient;
  extern const char* KeySemanticsTemperature;
  extern const char* KeySemanticsThermalConductivity;
  extern const char* KeySemanticsPermeability;

  extern const char* KeySemanticsMass;
  extern const char* KeySemanticsVolume;
  extern const char* KeySemanticsSpecificGravityOverLength;
  extern const char* KeySemanticsSpecificGravityOverArea;
  extern const char* KeySemanticsCompressibility;
  extern const char* KeySemanticsMassRate;
  extern const char* KeySemanticsVolumeRate;
  extern const char* KeySemanticsConsistency;
  extern const char* KeySemanticsViscosity;
  extern const char* KeySemanticsAngle;
  extern const char* KeySemanticsConductivity;
  extern const char* KeySemanticsWaveTravelTime;
  extern const char* KeySemanticsGammaRadiation;
  extern const char* KeySemanticsSignalIntensity;
  extern const char* KeySemanticsCarterLeaks;
  extern const char* KeySemanticsPotentialDifference;
  extern const char* KeySemanticsSpecificElectricalConductivity;
  extern const char* KeySemanticsResistivity;
  extern const char* KeySemanticsFrequency;
  extern const char* KeySemanticsPieces;

  ///////////////////////////////////////////////////////
  //// Named property keys
  extern const char* KeyUOM;
  extern const char* KeyQuantities;
  extern const char* KeyUnitsUI;
  extern const char* KeyUnitsStored;
  extern const char* KeySemantics;

  ///////////////////////////////////////////////////////
  //// Named keys
  extern QMap<PhysicalQuantity, QString>  KeysQuantities;
  extern QMap<QString, PhysicalQuantity>  KeysQuantitiesBackwards;

  extern QMap<QPair<int, int>, QString>   KeysUnits;
  extern QMap<QString, int>               KeysUnitsBackwards;

  extern QMap<QString, QString>           SemanticsDisplayNames;
  extern QMap<QString, QSet<int>>         SemanticsAcceptableUiUnits;

  ///////////////////////////////////////////////////////
  //// Quantities / Units helpers
  QString GetPhysicalQuantityDisplayName(PhysicalQuantity quantity);
  QString GetUnitsDisplayName(PhysicalQuantity quantity, int units);
  QString GetUnitsDisplayNameSimplified(PhysicalQuantity quantity, int units);
  QString GetUnitsDisplayNameSimplifiedAlternative(PhysicalQuantity quantity, int units);
  QString GetUnitsDisplayNameUnicoded(PhysicalQuantity quantity, int units);
  int     GetUnitsCount(PhysicalQuantity quantity);

  QMap<int, QString> GetUnitsWithNamesSimplified(PhysicalQuantity quantity);
  QMap<int, QString> GetUnitsWithNamesSimplifiedAlternative(PhysicalQuantity quantity);
  QMap<QString, int> GetUnitsWithNamesSimplifiedBackwards(PhysicalQuantity quantity);
  QMap<QString, int> GetUnitsWithNamesSimplifiedAlternativeBackwards(PhysicalQuantity quantity);
  QMap<QString, int> GetUnitsWithNamesSimplifiedAndSimplifiedAlternativeBackwards(PhysicalQuantity quantity);

  //// Semantics display names
  QString GetSemanticsDisplayName(const QString& semantics);

  ///////////////////////////////////////////////////////
  ///// Generalized conversion
  double Convert(double value, UOM::PhysicalQuantity quantity, unsigned sourceUOM, unsigned targetUOM);

  double ConvertAnyToSI(double value, UOM::PhysicalQuantity quantity, unsigned sourceUOM);
  double ConvertSIToAny(double value, UOM::PhysicalQuantity quantity, unsigned targetUOM);

  ///////////////////////////////////////////////////////
  ///// Specific quantities conversions
  double ConvertLengthAnyToSI(double value, unsigned sourceUOM);
  double ConvertLengthSIToAny(double value, unsigned targetUOM);

  double ConvertPressureAnyToSI(double value, unsigned sourceUOM);
  double ConvertPressureSIToAny(double value, unsigned targetUOM);

  double ConvertPressureOverLengthAnyToSI(double value, unsigned sourceUOM);
  double ConvertPressureOverLengthSIToAny(double value, unsigned targetUOM);

  double ConvertToughnessAnyToSI(double value, unsigned sourceUOM);
  double ConvertToughnessSIToAny(double value, unsigned targetUOM);

  double ConvertLeakoffCoefficientAnyToSI(double value, unsigned sourceUOM);
  double ConvertLeakoffCoefficientSIToAny(double value, unsigned targetUOM);

  double ConvertSpurtLossCoefficientAnyToSI(double value, unsigned sourceUOM);
  double ConvertSpurtLossCoefficientSIToAny(double value, unsigned targetUOM);

  double ConvertTemperatureAnyToSI(double value, unsigned sourceUOM);
  double ConvertTemperatureSIToAny(double value, unsigned targetUOM);

  double ConvertEnergyOverTemperatureAnyToSI(double value, unsigned sourceUOM);
  double ConvertEnergyOverTemperatureSIToAny(double value, unsigned targetUOM);

  double ConvertEnergyOverMassTemperatureAnyToSI(double value, unsigned sourceUOM);
  double ConvertEnergyOverMassTemperatureSIToAny(double value, unsigned targetUOM);

  double ConvertThermalConductivityAnyToSI(double value, unsigned sourceUOM);
  double ConvertThermalConductivitySIToAny(double value, unsigned targetUOM);

  double ConvertPermeabilityAnyToSI(double value, unsigned sourceUOM);
  double ConvertPermeabilitySIToAny(double value, unsigned targetUOM);

  double ConvertTimeAnyToSI(double value, unsigned sourceUOM);
  double ConvertTimeSIToAny(double value, unsigned targetUOM);

  double ConvertPeriodAnyToSI(double value, unsigned sourceUOM);
  double ConvertPeriodSIToAny(double value, unsigned targetUOM);

  double ConvertProductionOverPeriodAnyToSI(double value, unsigned sourceUOM);
  double ConvertProductionOverPeriodSIToAny(double value, unsigned targetUOM);

  double ConvertMassAnyToSI(double value, unsigned sourceUOM);
  double ConvertMassSIToAny(double value, unsigned targetUOM);

  double ConvertVolumeAnyToSI(double value, unsigned sourceUOM);
  double ConvertVolumeSIToAny(double value, unsigned targetUOM);

  double ConvertSpecificGravityAnyToSI(double value, unsigned sourceUOM);
  double ConvertSpecificGravitySIToAny(double value, unsigned targetUOM);

  double ConvertSpecificGravityOverLengthAnyToSI(double value, unsigned sourceUOM);
  double ConvertSpecificGravityOverLengthSIToAny(double value, unsigned targetUOM);

  double ConvertSpecificGravityOverAreaAnyToSI(double value, unsigned sourceUOM);
  double ConvertSpecificGravityOverAreaSIToAny(double value, unsigned targetUOM);

  double ConvertCompressibilityAnyToSI(double value, unsigned sourceUOM);
  double ConvertCompressibilitySIToAny(double value, unsigned targetUOM);

  double ConvertReciprocalLengthAnyToSI(double value, unsigned sourceUOM);
  double ConvertReciprocalLengthSIToAny(double value, unsigned targetUOM);

  double ConvertPlainRatioAnyToSI(double value, unsigned sourceUOM);
  double ConvertPlainRatioSiToAny(double value, unsigned targetUOM);

  double ConvertMassRateAnyToSI(double value, unsigned sourceUOM);
  double ConvertMassRateSIToAny(double value, unsigned targetUOM);

  double ConvertVolumeRateAnyToSI(double value, unsigned sourceUOM);
  double ConvertVolumeRateSIToAny(double value, unsigned targetUOM);

  double ConvertConsistencyAnyToSI(double value, unsigned sourceUOM);
  double ConvertConsistencySIToAny(double value, unsigned targetUOM);

  double ConvertViscosityAnyToSI(double value, unsigned sourceUOM);
  double ConvertViscositySIToAny(double value, unsigned targetUOM);

  double ConvertAngleAnyToSI(double value, unsigned sourceUOM);
  double ConvertAngleSIToAny(double value, unsigned targetUOM);

  double ConvertConductivityAnyToSI(double value, unsigned sourceUOM);
  double ConvertConductivitySIToAny(double value, unsigned targetUOM);

  double ConvertMeasurelessAnyToSI(double value, unsigned targetUOM);
  double ConvertMeasurelessSIToAny(double value, unsigned targetUOM);

  double ConvertConcentrationAnyToSI(double value, unsigned targetUOM);
  double ConvertConcentrationSIToAny(double value, unsigned targetUOM);

  double ConvertWaveTravelTimeAnyToSI(double value, unsigned sourceUOM);
  double ConvertWaveTravelTimeSIToAny(double value, unsigned targetUOM);

  double ConvertGammaRadiationAnyToSI(double value, unsigned sourceUOM);
  double ConvertGammaRadiationSIToAny(double value, unsigned targetUOM);

  double ConvertSignalIntensityAnyToSI(double value, unsigned sourceUOM);
  double ConvertSignalIntensitySIToAny(double value, unsigned targetUOM);

  double ConvertCarterLeaksAnyToSI(double value, unsigned sourceUOM);
  double ConvertCarterLeaksSIToAny(double value, unsigned targetUOM);

  double ConvertPotentialDifferenceAnyToSI(double value, unsigned sourceUOM);
  double ConvertPotentialDifferenceSIToAny(double value, unsigned targetUOM);

  double ConvertSpecificElectricalConductivityAnyToSI(double value, unsigned sourceUOM);
  double ConvertSpecificElectricalConductivitySIToAny(double value, unsigned targetUOM);

  double ConvertResistivityAnyToSI(double value, unsigned sourceUOM);
  double ConvertResistivitySIToAny(double value, unsigned targetUOM);

  double ConvertFrequencyAnyToSI(double value, unsigned sourceUOM);
  double ConvertFrequencySIToAny(double value, unsigned targetUOM);

  double ConvertPiecesAnyToSI(double value, unsigned sourceUOM);
  double ConvertPiecesSIToAny(double value, unsigned targetUOM);

  //// Units from quantity
  int    GetSIUnitsForQuantity(UOM::PhysicalQuantity quantity);

  //// Split ratio units (e.g. kg/m3) to numerator and denominator
  Descriptor GetNumerator(const Descriptor& descriptor);
  Descriptor GetDenominator(const Descriptor& descriptor);
  Descriptor GetDerived(const Descriptor& numerator, const Descriptor& denominator);

  PhysicalQuantity GetNumerator(PhysicalQuantity q);
  PhysicalQuantity GetDenominator(PhysicalQuantity q);
  PhysicalQuantity GetDerived(PhysicalQuantity numerator, PhysicalQuantity denominator);
};
