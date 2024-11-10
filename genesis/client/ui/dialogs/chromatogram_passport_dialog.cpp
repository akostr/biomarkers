#include "chromatogram_passport_dialog.h"
#include "ui_chromatogram_passport_dialog.h"
#include <genesis_style/style.h>
#include <logic/notification.h>
#include <QLayout>

namespace Dialogs
{

ChromatogramPassportDialog::ChromatogramPassportDialog(bool isFragment, const QJsonObject &json, QWidget* parent)
  : Templates::Info(parent, Btns::Ok | Btns::Cancel)
  , ui(new Ui::ChromatogramPassportDialogBody)
{
  Size = QSizeF(0.8, 0.8);
  UpdateGeometry();
  Settings s;
  s.buttonsNames[Btns::Ok] = tr("Apply");
  // parseJson return proper dialog header depends from chromas count
  s.dialogHeader = tr("Passport data");
  s.contentHeader = "";
  applySettings(s);
  parseJson(json);
  setupUi();
  connect(ui->passportForm, &PassportMainForm::modelsLoaded, this, [this, json]()
  {
    ui->passportForm->setChromatogramms(mChromasData);
  });
  ui->passportForm->loadComboModels(isFragment);
}

ChromatogramPassportDialog::~ChromatogramPassportDialog()
{
  delete ui;
}

QJsonArray ChromatogramPassportDialog::toJson()
{
  return ui->passportForm->toJson();
}

void ChromatogramPassportDialog::Accept()
{
  if(ui->passportForm->CheckValidity())
    Templates::Info::Accept();
  else
    Notification::NotifyError(tr("Invalid passport data"), tr("Passport data error"));
}

void ChromatogramPassportDialog::setupUi()
{
  mBody = new QWidget(this);
  mBody->setStyleSheet(Style::Genesis::GetUiStyle());
  ui->setupUi(mBody);
  getContent()->layout()->addWidget(mBody);
}

QString ChromatogramPassportDialog::parseJson(const QJsonObject &json)
{
  QJsonArray jpassports = json["chrom_passport"].toArray();
  if (jpassports.isEmpty())
    return tr("no chromatogramms");
  QString name;
  if(jpassports.size() == 1)
    name = jpassports.first().toObject()["sample_title"].toString();
  else
    name = tr("Chromatogramms passport edit");
  for(int i = 0; i < jpassports.size(); i++)
  {
    mChromasData.append(PassportMainForm::ChromaData());
    auto& current = mChromasData.last();
    auto& data = current.data;
    auto jpass = jpassports[i].toObject();
    current.id = jpass["file_id"].toInt();
    current.name = jpass["sample_title"].toString();
    if(jpass.contains("no_passport") && jpass["no_passport"].toBool())
      continue;

    data.sampleType = jpass["sample_type"].toInt();
    data.fluidType = jpass["fluid_type"].toInt();
    if(data.sampleType == Passport::SEmpty)
    {
      data.fluidType = Passport::FEmpty;
    }
    data.comment = jpass["comment"].toString();
    if(jpass.contains("chemical_passport"))
    {
      auto jchem = jpass["chemical_passport"].toObject();
      auto& chem = data.sampleChemical;
      chem.columnLength = jchem["column_length"].toDouble();
      chem.columnInnerDiameter = jchem["inner_diameter"].toDouble();
      chem.columnStaticPhaseThickness = jchem["static_phase_thickness"].toDouble();
      chem.columnStaticPhaseComposition = jchem["static_phase_composition"].toString();
      chem.columnMark = jchem["column_mark"].toString();
      chem.evaporatorTemperature = jchem["t_evaporator"].toDouble();
      chem.detectorTemperature = jchem["t_detector"].toDouble();
      chem.initialColumnTemperature = jchem["initial_column_t"].toDouble();
      chem.columnTemperatureRiseRate = jchem["column_t_rise_rate"].toDouble();
      chem.сolumnHoldingTimeAtMaximumTemperature = jchem["column_holding_time"].toDouble();
      chem.carrierGasFlow = jchem["gas_carrier_flow"].toDouble();
      chem.flowSeparation = jchem["flow_separation"].toString();
      chem.innerStandart = jchem["inner_standard"].toString();
      chem.sampleVolume = jchem["sample_injection_volume"].toDouble();
      chem.date = QDateTime::fromString(jchem["analysis_date"].toString(), Qt::ISODate).toLocalTime();
      chem.additionalInfo = jchem["additional_information"].toString();
      chem.detectorType = jchem["detector_type_id"].toInt();
      chem.carrierGasType = jchem["gas_carrier_type_id"].toInt();
      if(jchem.contains("sample_preparation_type_id"))
        chem.samplePreparationType = jchem["sample_preparation_type_id"].toInt();
    }
    auto jgeo = jpass["geological_passport"].toObject();
    switch((Passport::SampleType)data.sampleType)
    {
    case Passport::SPure:
    case Passport::SEmpty:
    {
      auto& geo = data.pureGeo;
      geo.passportId = jgeo["passport_id"].toInt();
      geo.fieldId = jgeo["field_id"].toInt();
      geo.field = jgeo["field_title"].toString();
      geo.source = jgeo["data_source"].toString();
      geo.altitude = jgeo["altitude"].toString();
      geo.perforationInterval = jgeo["perforation_interval"].toDouble();
      geo.stratum = jgeo["stratum_suite"].toString();
      geo.fluidType = jgeo["fluid_type"].toString();
      if(jgeo.contains("booster_pumping_station_id"))
        geo.boosterPumpingStation = jgeo["booster_pumping_station_id"].toString();
      if(jgeo.contains("cluster_pumping_station_id"))
        geo.clusterPumpingStation = jgeo["cluster_pumping_station_id"].toString();
      if(jgeo.contains("production_department"))
        geo.oilGasProductionStation = jgeo["production_department"].toString();
      geo.sampleDate = QDateTime::fromString(jgeo["file_datetime"].toString(), Qt::DateFormat::ISODate).toLocalTime();
      geo.depth = jgeo["depth"].toDouble();
      geo.depth_tvd = jgeo["depth_tvd"].toDouble();
//      geo.depthType = jgeo["depth_type"].toString() == "MD" ? Passport::DepthType::MD : Passport::DepthType::TVD;
      geo.wellId = jgeo["well_id"].toInt();
      geo.well = jgeo["well_title"].toString();
      geo.wellClusterId = jgeo["well_cluster_id"].toInt();
      geo.wellCluster = jgeo["well_cluster_title"].toString();
      geo.layerId = jgeo["layer_id"].toInt();
      geo.layer = jgeo["layer_title"].toString();
      break;
    }
    case Passport::SMixed:
    {
      auto& geo = data.mixedGeo;
      geo.field = jgeo["field_title"].toString();
      geo.fieldId = jgeo["field_id"].toInt();
      auto& components = geo.components;
      components.clear();
      auto jcomponents = jgeo["components"].toArray();
      for(int j = 0; j < jcomponents.size(); j++)
      {
        auto jcomp = jcomponents[j].toObject();
        auto comp = Passport::MixedSampleGeoComponent();
        comp.passportId = jcomp["passport_id"].toInt();
        comp.source = jcomp["data_source"].toString();
        comp.altitude = jcomp["altitude"].toString();
        comp.perforationInterval = jcomp["perforation_interval"].toInt();
        comp.stratum = jcomp["stratum_suite"].toString();
        comp.fluidType = jcomp["fluid_type"].toString();
        if(jcomp.contains("booster_pumping_station_id"))
          comp.boosterPumpingStation = jcomp["booster_pumping_station_id"].toString();
        if(jcomp.contains("cluster_pumping_station_id"))
          comp.clusterPumpingStation = jcomp["cluster_pumping_station_id"].toString();
        if(jcomp.contains("production_department"))
          comp.oilGasProductionStation = jcomp["production_department"].toString();
        comp.sampleDate = QDateTime::fromString(jcomp["file_datetime"].toString(), Qt::DateFormat::ISODate).toLocalTime();
        comp.depth = jcomp["depth"].toDouble();
        comp.depth_tvd = jcomp["depth_tvd"].toDouble();
//        comp.depthType = jcomp["depth_type"].toString() == "MD" ? Passport::DepthType::MD : Passport::DepthType::TVD;
        comp.percentSubstance = jcomp["substance"].toDouble();
        comp.wellId = jcomp["well_id"].toInt();
        comp.well = jcomp["well_title"].toString();
        comp.wellClusterId = jcomp["well_cluster_id"].toInt();
        comp.wellCluster = jcomp["well_cluster_title"].toString();
        comp.layerId = jcomp["layer_id"].toInt();
        comp.layer = jcomp["layer_title"].toString();
        components << comp;
      }
      break;
    }
    case Passport::SJoint:
    {
      auto& geo = data.jointGeo;
      geo.passportId = jgeo["passport_id"].toInt();
      geo.fieldId = jgeo["field_id"].toInt();
      geo.field = jgeo["field_title"].toString();
      geo.source = jgeo["data_source"].toString();
      geo.altitude = jgeo["altitude"].toString();
      geo.perforationInterval = jgeo["perforation_interval"].toDouble();
      geo.stratum = jgeo["stratum_suite"].toString();
      geo.fluidType = jgeo["fluid_type"].toString();
      if(jgeo.contains("booster_pumping_station_id"))
        geo.boosterPumpingStation = jgeo["booster_pumping_station_id"].toString();
      if(jgeo.contains("cluster_pumping_station_id"))
        geo.clusterPumpingStation = jgeo["cluster_pumping_station_id"].toString();
      if(jgeo.contains("production_department"))
        geo.oilGasProductionStation = jgeo["production_department"].toString();
      geo.sampleDate = QDateTime::fromString(jgeo["file_datetime"].toString(), Qt::DateFormat::ISODate).toLocalTime();
      geo.depth = jgeo["depth"].toDouble();
      geo.depth_tvd = jgeo["depth_tvd"].toDouble();
//      geo.depthType = jgeo["depth_type"].toString() == "MD" ? Passport::DepthType::MD : Passport::DepthType::TVD;
      geo.wellId = jgeo["well_id"].toInt();
      geo.well = jgeo["well_title"].toString();
      geo.wellClusterId = jgeo["well_cluster_id"].toInt();
      geo.wellCluster = jgeo["well_cluster_title"].toString();
      auto jlayers = jgeo["layers"].toArray();
      auto& layers = geo.layers;
      layers.clear();
      for(int j = 0; j < jlayers.size(); j++)
      {
        auto jlayer = jlayers[j].toObject();
        layers.append({jlayer["layer_id"].toInt(), jlayer["layer_title"].toString()});
      }
      break;
    }
    case Passport::InvalidSampleType:
      break;
    }
  }
  return name;
}
}//namespace Dialogs
