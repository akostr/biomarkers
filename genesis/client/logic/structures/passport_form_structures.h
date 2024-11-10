#ifndef PASSPORT_FORM_STRUCTURES_H
#define PASSPORT_FORM_STRUCTURES_H
#include <QString>
#include <QDate>
#include <QJsonObject>
#include <QJsonArray>

namespace Passport
{
enum DepthType
{
  MD,
  TVD
};
enum FluidType
{
  InvalidFluid = 0,
  FWater = 1,
  FOil = 2,
  FGas = 3,
  FCondensate = 4,
  FMixed = 5,
  FBitumoid = 6,
  FEmpty = 7
};
enum SampleType
{
  InvalidSampleType = 0,
  SPure = 1,
  SMixed = 2,
  SJoint = 3,
  SEmpty = 4
};
enum DetectorType
{
  InvalidDetectorType = 0,
  DFID = 1,
  DFPD = 2,
  DTCD = 3,
  DMD = 4
};
enum CarrierGasType
{
  InvalidCarrierGasType = 0,
  Helium = 1,
  Nitrogen = 2,
  Hydrogen = 3,
  Argon = 4
};
enum SamplePreparationType
{
  InvalidSamplePreparationType = 0,
  DryFluid = 1,
  ExtractedFluid = 2,
  NoSamplePreparation = 3,
  SaturatedFraction = 4,
  AromaticFraction = 5,
  Maltene = 6
};

struct SampleField
{
  QString field;
  int fieldId = -1;
  virtual ~SampleField(){};
  virtual int fieldsCount() const {return 2;};
  //2
};

struct SampleGeoMainCommon
{
  QString wellCluster;
  int wellClusterId = -1;
  QString well;
  int wellId = -1;
  double depth = 0;
  double depth_tvd = 0;
//  DepthType depthType = DepthType::MD;
  QDateTime sampleDate = QDateTime::currentDateTime();
  int passportId = 0;
  //7
};
struct SampleGeoMainNonMixed : public SampleGeoMainCommon, public SampleField
{
  QString layer;
  int layerId = -1;
  int fieldsCount() const override {return 11;};
  //7 + 2 + 2 = 11
};
struct SampleGeoMainJoint : public SampleGeoMainCommon, public SampleField
{
  int fieldsCount() const override {return 9;};
  //7 + 2 = 9
};

struct SampleGeoExtended
{
  QString source;
  QString altitude;
  double perforationInterval = 0;
  QString stratum;
  QString fluidType;
  QString clusterPumpingStation;
  QString boosterPumpingStation;
  QString oilGasProductionStation;
  //8
};

struct PureSampleGeo : public SampleGeoMainNonMixed, public SampleGeoExtended
{
  QJsonObject toJson(FluidType ftype)
  {
    QJsonObject root;
    root["passport_id"] = passportId;
    root["field_id"] = fieldId;
    root["field_title"] = field;
    root["well_cluster_id"] = wellClusterId;
    root["well_cluster_title"] = wellCluster;
    root["well_id"] = wellId;
    root["well_title"] = well;
    root["layer_id"] = layerId;
    root["layer_title"] = layer;
    root["depth"] = depth;
    root["depth_tvd"] = depth_tvd;
    root["depth_type"] = "MD";
    root["file_datetime"] = sampleDate.toString();
    root["data_source"] = source;
    root["altitude"] = altitude;
    root["perforation_interval"] = perforationInterval;
    root["stratum_suite"] = stratum;
    root["fluid_type"] = fluidType;
    if(ftype == FWater)
    {
      root["booster_pumping_station_id"] = boosterPumpingStation;
      root["cluster_pumping_station_id"] = clusterPumpingStation;
    }
    else
      root["production_department"] = oilGasProductionStation;
    return root;
  }
  int fieldsCount() const override {return 19;};
  //11 + 8 = 19
};

struct MixedSampleGeoComponent : public SampleGeoMainCommon, public SampleGeoExtended
{
  QString layer;
  int layerId = -1;
  double percentSubstance = 0;
  //7 + 8 + 3 = 18
};

struct MixedSampleGeo : public SampleField
{
  QList<MixedSampleGeoComponent> components{MixedSampleGeoComponent(), MixedSampleGeoComponent()};
  QJsonObject toJson(FluidType ftype)
  {
    QJsonObject root;
    root["field_id"] = fieldId;
    root["field_title"] = field;
    QJsonArray jcomponents;
    for(auto& c : components)
    {
      QJsonObject jcomp;
      jcomp["passport_id"] = c.passportId;
      jcomp["layer_id"] = c.layerId;
      jcomp["layer_title"] = c.layer;
      jcomp["substance"] = c.percentSubstance;
      jcomp["well_cluster_id"] = c.wellClusterId;
      jcomp["well_cluster_title"] = c.wellCluster;
      jcomp["well_id"] = c.wellId;
      jcomp["well_title"] = c.well;
      jcomp["depth"] = c.depth;
      jcomp["depth_tvd"] = c.depth_tvd;
      jcomp["depth_type"] = "MD";
      jcomp["file_datetime"] = c.sampleDate.toString();
      jcomp["data_source"] = c.source;
      jcomp["altitude"] = c.altitude;
      jcomp["perforation_interval"] = c.perforationInterval;
      jcomp["stratum_suite"] = c.stratum;
      jcomp["fluid_type"] = c.fluidType;
      if(ftype == FWater)
      {
        jcomp["booster_pumping_station_id"] = c.boosterPumpingStation;
        jcomp["cluster_pumping_station_id"] = c.clusterPumpingStation;
      }
      else
        jcomp["production_department"] = c.oilGasProductionStation;
      jcomponents << jcomp;
    }
    root["components"] = jcomponents;
    return root;
  }
  bool isValid()
  {
    for(auto& c : components)
    {
      if(c.layerId == -1)
        return false;
      if(c.percentSubstance < 0 || c.percentSubstance > 100)
        return false;
    }
    return true;
  }
  int fieldsCount() const override {return 2 + 18 * components.count();};

  //2 + 18 * componentsCount
};

struct JointSampleGeo : public SampleGeoMainJoint, public SampleGeoExtended
{
  QList<QPair<int, QString>> layers {{-1, ""}, {-1, ""}};

  QJsonObject toJson(FluidType ftype)
  {
    QJsonObject root;
    QJsonArray jlayers;
    for(auto& layer : layers)
    {
      QJsonObject jlayer;
      jlayer["layer_id"] = layer.first;
      jlayer["layer_title"] = layer.second;
      jlayers << jlayer;
    };
    root["passport_id"] = passportId;
    root["layers"] = jlayers;
    root["field_id"] = fieldId;
    root["field_title"] = field;
    root["well_cluster_id"] = wellClusterId;
    root["well_cluster_title"] = wellCluster;
    root["well_id"] = wellId;
    root["well_title"] = well;
    root["file_datetime"] = sampleDate.toString();
    root["depth"] = depth;
    root["depth_tvd"] = depth_tvd;
    root["depth_type"] = "MD";
    root["data_source"] = source;
    root["altitude"] = altitude;
    root["perforation_interval"] = perforationInterval;
    root["stratum_suite"] = stratum;
    root["fluid_type"] = fluidType;
    if(ftype == FWater)
    {
      root["booster_pumping_station_id"] = boosterPumpingStation;
      root["cluster_pumping_station_id"] = clusterPumpingStation;
    }
    else
      root["production_department"] = oilGasProductionStation;
    return root;
  }
  bool isValid()
  {
    int validCount = 0;
    for(auto& l : layers)
    {
      if(l.first != -1)
        validCount++;
    }
    if(validCount < 2)
      return false;
    return true;
  };
  int fieldsCount() const override {return 17 + layers.count() * 2;};
};

struct SampleChemical
{
  int detectorType = DFID;
  double sampleVolume = 0;
  double columnLength = 0;
  double columnInnerDiameter = 0;
  double columnStaticPhaseThickness = 0;
  QString columnStaticPhaseComposition;
  QString columnMark;
  double evaporatorTemperature;
  double detectorTemperature = 0;
  double initialColumnTemperature = 0;
  double columnTemperatureRiseRate = 0;
  double сolumnHoldingTimeAtMaximumTemperature = 0;
  double carrierGasFlow = 0;
  QString flowSeparation;
  QString innerStandart;
  int carrierGasType = Helium;
  int samplePreparationType = NoSamplePreparation;
  QDateTime date = QDateTime::currentDateTime();
  QString additionalInfo;

  QJsonObject toJson(FluidType ftype)
  {
    QJsonObject root;
    root["detector_type_id"] = detectorType;
    root["sample_injection_volume"] = sampleVolume;
    root["column_length"] = columnLength;
    root["inner_diameter"] = columnInnerDiameter;
    root["static_phase_thickness"] = columnStaticPhaseThickness;
    root["static_phase_composition"] = columnStaticPhaseComposition;
    root["column_mark"] = columnMark;
    root["t_evaporator"] = evaporatorTemperature;
    root["t_detector"] = detectorTemperature;
    root["initial_column_t"] = initialColumnTemperature;
    root["column_t_rise_rate"] = columnTemperatureRiseRate;
    root["column_holding_time"] = сolumnHoldingTimeAtMaximumTemperature;
    root["gas_carrier_flow"] = carrierGasFlow;
    root["flow_separation"] = flowSeparation;
    root["inner_standard"] = innerStandart;
    root["gas_carrier_type_id"] = carrierGasType;
    if(ftype != FGas)
      root["sample_preparation_type_id"] = samplePreparationType;
    root["additional_information"] = additionalInfo;
    root["analysis_date"] = date.toString();
    return root;
  }
};

struct SampleHeaderData
{
  int fluidType = FOil;
  int sampleType = SPure;
  QString comment;
};

struct SampleData : public SampleHeaderData
{
  PureSampleGeo pureGeo;
  MixedSampleGeo mixedGeo;
  JointSampleGeo jointGeo;
  SampleChemical sampleChemical;
  bool isValid()
  {
    switch((SampleType)sampleType)
    {
    case SMixed:
      return mixedGeo.isValid();
    case SJoint:
      return jointGeo.isValid();
    default:
      return true;
    }
  }
  int completionPrecentage() const
  {
    double chemCount = 0;
    double cc = 0;
    if(fluidType != FWater)
    {
      chemCount = 19;
      cc = chemCount;
      if(sampleChemical.detectorType == 0)
        cc--;
      if(sampleChemical.columnStaticPhaseComposition.isEmpty())
        cc--;
      if(sampleChemical.columnMark.isEmpty())
        cc--;
      if(sampleChemical.flowSeparation.isEmpty())
        cc--;
      if(sampleChemical.innerStandart.isEmpty())
        cc--;
      if(sampleChemical.carrierGasType == 0)
        cc--;
      if(sampleChemical.samplePreparationType == 0)
        cc--;
      if(sampleChemical.additionalInfo.isEmpty())
        cc--;
    }
    switch((SampleType)sampleType)
    {
    case InvalidSampleType:
      return -1;
    case SPure:
    case SEmpty:
    {
      double fc = pureGeo.fieldsCount();
      double count = fc + chemCount;
      if(pureGeo.fieldId == 0)
        fc -= 2;
      if(pureGeo.wellClusterId == 0)
        fc -= 2;
      if(pureGeo.wellId == 0)
        fc -= 2;
      if(pureGeo.layerId == 0)
        fc -= 2;
      if(pureGeo.source.isEmpty())
        fc--;
      if(pureGeo.altitude.isEmpty())
        fc--;
      if(pureGeo.stratum.isEmpty())
        fc--;
      if(pureGeo.fluidType.isEmpty())
        fc--;
      if(fluidType == FWater)
      {
        if(pureGeo.clusterPumpingStation.isEmpty())
          fc--;
        if(pureGeo.boosterPumpingStation.isEmpty())
          fc--;
      }
      else
      {
        if(pureGeo.oilGasProductionStation.isEmpty())
          fc--;
      }
      int precentage = (fc + cc) / count * 100.0 + 0.5;
      return precentage;
    }
    case SMixed:
    {
      double fc = mixedGeo.fieldsCount();
      double count = fc + chemCount;
      if(mixedGeo.fieldId == 0)
        fc -= 2;
      for(int i = 0; i < mixedGeo.components.size(); i++)
      {
        const auto& c = mixedGeo.components[i];
        if(c.wellClusterId == 0)
          fc -= 2;
        if(c.wellId == 0)
          fc -= 2;
        if(c.layerId == 0)
          fc -= 2;
        if(c.source.isEmpty())
          fc--;
        if(c.altitude.isEmpty())
          fc--;
        if(c.stratum.isEmpty())
          fc--;
        if(c.fluidType.isEmpty())
          fc--;
        if(fluidType == FWater)
        {
          if(c.clusterPumpingStation.isEmpty())
            fc--;
          if(c.boosterPumpingStation.isEmpty())
            fc--;
        }
        else
        {
          if(c.oilGasProductionStation.isEmpty())
            fc--;
        }
      }
      int precentage = (fc + cc) / count * 100.0 + 0.5;
      return precentage;
    }
    case SJoint:
    {
      double fc = jointGeo.fieldsCount();
      double count = fc + chemCount;
      if(jointGeo.fieldId == 0)
        fc -= 2;
      if(jointGeo.wellClusterId == 0)
        fc -= 2;
      if(jointGeo.wellId == 0)
        fc -= 2;
      if(jointGeo.source.isEmpty())
        fc--;
      if(jointGeo.altitude.isEmpty())
        fc--;
      if(jointGeo.stratum.isEmpty())
        fc--;
      if(jointGeo.fluidType.isEmpty())
        fc--;
      if(fluidType == FWater)
      {
        if(jointGeo.clusterPumpingStation.isEmpty())
          fc--;
        if(jointGeo.boosterPumpingStation.isEmpty())
          fc--;
      }
      else
      {
        if(jointGeo.oilGasProductionStation.isEmpty())
          fc--;
      }
      for(int i = 0; i < jointGeo.layers.size(); i++)
      {
        const auto& l = jointGeo.layers[i];
        if(l.first == 0)
          fc -= 2;
      }
      int precentage = (fc + cc) / count * 100.0 + 0.5;
      return precentage;
    }
    }
    return -1;
  }

};
}//namespace Passport
#endif // PASSPORT_FORM_TRUCTURES_H
