#include "analysis_identity_model_json_serializer.h"

#include "logic/models/analysis_identity_model.h"
#include "logic/models/analysis_entity_model.h"
#include "logic/known_json_tag_names.h"
#include "logic/serializers/json_combiner.h"

#include <string>

#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaEnum>
#include <QColor>
#include <QPointer>

#define _Q(str) QString::fromStdString(str)
#define _S(str) str.toStdString()

namespace Service
{
  QJsonObject AnalysisIdentityModelJsonSerializer::ToJson(const IdentityModelPtr model) const
  {
    auto analysisDataJson = AnalysisDataToJson(model->Get<AnalysisData>(AnalysisIdentityModel::AnalysisData));
    model->Exec<QByteArray>(AnalysisIdentityModel::ExportRequestBody,
      [&](const QByteArray& data) {
        analysisDataJson.insert(JsonTagNames::ExportRequestBody, QJsonDocument::fromJson(data).object());
      });

    model->Exec<QPointer<AnalysisEntityModel>>(AnalysisIdentityModel::EntityModel,
      [&](QPointer<AnalysisEntityModel> model) {
        analysisDataJson.insert(JsonTagNames::EntityModelBody, AnalysisEntityModelToJson(model));
      });

    QJsonObject result =
    {
      { JsonTagNames::TypeId, static_cast<int>(model->Get<size_t>(AnalysisIdentityModel::AnalysisTypeId))},
      { JsonTagNames::TableId, static_cast<int>(model->Get<size_t>(AnalysisIdentityModel::AnalysisTableId))},

      { JsonTagNames::Title, _Q(model->Get<std::string>(AnalysisIdentityModel::AnalysisTitle))},
      { JsonTagNames::Comment, _Q(model->Get<std::string>(AnalysisIdentityModel::AnalysisComment))},
      { JsonTagNames::Parameters,  ParametersToJson(model)},
      { JsonTagNames::AnalysisTable, AnalysisTableToJson(model->Get<AnalysisTable>(AnalysisIdentityModel::AnalysisTable)) },
      { JsonTagNames::AnalysisData, analysisDataJson},
    };
    model->Exec<size_t>(AnalysisIdentityModel::AnalysisId,
      [&](size_t analisysId)
      {
        result.insert(JsonTagNames::analysis_id, static_cast<int>(analisysId));
      });
    return  result;
  }

  IdentityModelPtr AnalysisIdentityModelJsonSerializer::ToModel(const QJsonObject& json) const
  {
    const auto model = std::make_shared<AnalysisIdentityModel>();

    model->Set<size_t>(AnalysisIdentityModel::AnalysisTypeId, ParseUnsignedInt(json, _S(JsonTagNames::TypeId)));
    model->Set<size_t>(AnalysisIdentityModel::AnalysisUserId, ParseUnsignedInt(json, _S(JsonTagNames::UserId)));
    model->Set<size_t>(AnalysisIdentityModel::AnalysisTableId, ParseUnsignedInt(json, _S(JsonTagNames::TableId)));

    model->Set<std::string>(AnalysisIdentityModel::AnalysisTitle, ParseString(json, _S(JsonTagNames::Title)));
    model->Set<std::string>(AnalysisIdentityModel::AnalysisComment, ParseString(json, _S(JsonTagNames::Comment)));
    ParseParameters(ParseSubObject(json, _S(JsonTagNames::Parameters)), model);
    model->Set<AnalysisTable>(AnalysisIdentityModel::AnalysisTable,
      ParseAnalysisTable(ParseSubObject(json, _S(JsonTagNames::AnalysisTable))));
    const auto analysisData = json.value(JsonTagNames::AnalysisData).toObject();
    model->Set<AnalysisData>(AnalysisIdentityModel::AnalysisData, ParseAnalysisData(analysisData));
    model->Set(AnalysisIdentityModel::ExportRequestBody,
      QJsonDocument(analysisData.value(JsonTagNames::ExportRequestBody).toObject()).toJson(QJsonDocument::Compact));
    model->Set(AnalysisIdentityModel::EntityModel, ParseAnalysisEntityModel(analysisData.value(JsonTagNames::EntityModelBody).toObject()));
    model->Set<size_t>(AnalysisIdentityModel::AnalysisId, ParseUnsignedInt(json, _S(JsonTagNames::analysis_id)));
    return model;
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::ParametersToJson(const IdentityModelPtr model) const
  {
    const auto parameters = model->Get<Parameters>(AnalysisIdentityModel::AnalysisParameters);
    return
    {
      { JsonTagNames::ValueType, _Q(parameters.ValueType) }
    };
  }

  void AnalysisIdentityModelJsonSerializer::ParseParameters(const QJsonObject& json, const IdentityModelPtr model) const
  {
    model->Set<Parameters>(AnalysisIdentityModel::AnalysisParameters, { ParseString(json, _S(JsonTagNames::ValueType)) });
  }

  // Parsing Analysis table from qjson
  AnalysisTable AnalysisIdentityModelJsonSerializer::ParseAnalysisTable(const QJsonObject& json) const
  {
    return
    {
      ParseCheckedPeaksId(ParseSubObject(json, JsonTagNames::CheckedValues.toStdString())),
      ParseOptionalDouble2DVector(json, JsonTagNames::Concentrations.toStdString()),
    };
  }

  CheckedPeakIds AnalysisIdentityModelJsonSerializer::ParseCheckedPeaksId(const QJsonObject& json) const
  {
    return
    {
      ParseIntVector(json, _S(JsonTagNames::MarkersIds)),
      ParseIntVector(json, _S(JsonTagNames::SamplesIds)),
    };
  }

  AnalysisMetaInfoListByFileId AnalysisIdentityModelJsonSerializer::ParseChromatogramData(const QJsonObject& json) const
  {
    AnalysisMetaInfoListByFileId list;
    for (const auto& key : json.keys())
    {
      list.insert({ _S(key), ParseMetaInfo(json[key].toObject()) });
    }
    return list;
  }

  AnalysisPeaks AnalysisIdentityModelJsonSerializer::ParseAnalysisPeaks(const QJsonObject& json) const
  {
    AnalysisPeaks peaks;
    const auto peakArray = json.value(JsonTagNames::Peaks).toArray();
    std::transform(peakArray.begin(), peakArray.end(), std::back_inserter(peaks),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParseAnalysisPeak(value.toObject()); });
    return peaks;
  }

  AnalysisPeak AnalysisIdentityModelJsonSerializer::ParseAnalysisPeak(const QJsonObject& json) const
  {
    return
    {
      0,
      0,
      {// GeoParams
      ParseDouble(json, _S(JsonTagNames::RetantionTime)),
      ParseDouble(json, _S(JsonTagNames::Index)),
      ParseDouble(json, _S(JsonTagNames::MarkerWindow)),
      ParseDouble(json, _S(JsonTagNames::AreaUpperCase)),
      ParseDouble(json, _S(JsonTagNames::HeightUpperCase)),
      },
      {// Interval XStart, XEnd
      ParseDouble(json, _S(JsonTagNames::XStart)),
      ParseDouble(json, _S(JsonTagNames::XEnd)),
      },
      {// Interval YStart, YEnd
      ParseDouble(json, _S(JsonTagNames::YStart)),
      ParseDouble(json, _S(JsonTagNames::YEnd)),
      },
      ParseString(json, _S(JsonTagNames::TitleUpperCase)),
      ParseString(json, _S(JsonTagNames::FormFactor)),
      {// Rectangle
      ParseDouble(json, _S(JsonTagNames::XTop)),
      ParseDouble(json, _S(JsonTagNames::XBottom)),
      ParseDouble(json, _S(JsonTagNames::YTop)),
      ParseDouble(json, _S(JsonTagNames::YBottom)),
      },
      static_cast<PeakType>(ParseUnsignedInt(json, _S(JsonTagNames::Type))),
      ParseString(json, _S(JsonTagNames::Color)),
    };
  }

  //Parsing analysis data from qjson

  AnalysisData AnalysisIdentityModelJsonSerializer::ParseAnalysisData(const QJsonObject& json) const
  {
    return
    {
      ParseAnalysisInnerData(json.value(JsonTagNames::Data).toObject()),
      ParseCountPlotSettings(json.value(JsonTagNames::CountPlotSettings).toObject()),
      ParseWidgetSettings(json.value(JsonTagNames::WidgetSettings).toObject())
    };
  }

  AnalysisInnerData AnalysisIdentityModelJsonSerializer::ParseAnalysisInnerData(const QJsonObject& json) const
  {
    return
    {
          ParseAnalysisMetaInfoList(json, _S(JsonTagNames::TTitle)),
      ParseAnalysisMetaInfoList(json, _S(JsonTagNames::TestData)),
          ParseAnalysisMetaInfoMap(json, _S(JsonTagNames::TTitle)),
      ParseAnalysisComponents(json),
      ParseIntVector(json, _S(JsonTagNames::Pids)),
      ParseStringList(json, _S(JsonTagNames::PTitle)),
    };
  }

  CountPlotSettings AnalysisIdentityModelJsonSerializer::ParseCountPlotSettings(const QJsonObject& json) const
  {
    const auto stylesJson = json.value(JsonTagNames::Style).toObject();
    return
    {
      ParseGroupingParams(json.value(JsonTagNames::GroupingParams).toObject()),
      ParseStyles(stylesJson, _S(JsonTagNames::ColorStyle)),
      ParseStyles(stylesJson, _S(JsonTagNames::CustomStyle)),
      ParseStyles(stylesJson, _S(JsonTagNames::ShapeStyle)),
    };
  }

  WidgetSetting AnalysisIdentityModelJsonSerializer::ParseWidgetSettings(const QJsonObject& json) const
  {
    const auto pickedComponents = json.value(JsonTagNames::PickedComponents).toObject();
    return
    {
      ParsePickedComponent(pickedComponents, _S(JsonTagNames::Counts)),
      ParsePickedComponent(pickedComponents, _S(JsonTagNames::Loads)),
    };
  }

  PickedComponent AnalysisIdentityModelJsonSerializer::ParsePickedComponent(const QJsonObject& json, std::string_view tagName) const
  {
    const auto jsonObject = json.value(tagName.data()).toObject();
    return
    {
      ParseDouble(jsonObject , _S(JsonTagNames::XComponentNum)),
      ParseDouble(jsonObject , _S(JsonTagNames::YComponentNum)),
    };
  }

  AnalysisComponents AnalysisIdentityModelJsonSerializer::ParseAnalysisComponents(const QJsonObject& json) const
  {
    AnalysisComponents compList;
    const auto compArray = json.value(JsonTagNames::Components).toArray();
    std::transform(compArray.begin(), compArray.end(), std::back_inserter(compList),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParseAnalysisComponent(value.toObject()); });
    return compList;
  }

  AnalysisComponent AnalysisIdentityModelJsonSerializer::ParseAnalysisComponent(const QJsonObject& json) const
  {
    return
    {
      ParseUnsignedInt(json, _S(JsonTagNames::Number)),
      ParseDouble(json, _S(JsonTagNames::ExplPCsVariance)),
      ParseDoubleVector(json, _S(JsonTagNames::P)),
      ParseDoubleVector(json, _S(JsonTagNames::T)),
    };
  }

  GroupingParams AnalysisIdentityModelJsonSerializer::ParseGroupingParams(const QJsonObject& json) const
  {
    return
    {
      ParseUnsignedInt(json, _S(JsonTagNames::Color)),
      json.value(JsonTagNames::Shape).toInt(),
    };
  }

  ObjectStyles AnalysisIdentityModelJsonSerializer::ParseStyles(const QJsonObject& json, std::string_view tagName) const
  {
    ObjectStyles styles;
    const auto arr = json.value(tagName.data()).toArray();
    std::transform(arr.begin(), arr.end(), std::back_inserter(styles),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParseStyle(value.toObject()); });
    return styles;
  }

  ObjectStyle AnalysisIdentityModelJsonSerializer::ParseStyle(const QJsonObject& json) const
  {
    return
    {
      ParseUnsignedInt(json, _S(JsonTagNames::ParameterId)),
      ParseAnalysisStyles(json, _S(JsonTagNames::Style))
    };
  }

  AnalysisStyles AnalysisIdentityModelJsonSerializer::ParseAnalysisStyles(const QJsonObject& json, std::string_view tagName) const
  {
    AnalysisStyles styles;
    const auto styleArray = json.value(tagName.data()).toArray();
    std::transform(styleArray.begin(), styleArray.end(), std::back_inserter(styles),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    {
      const auto object = value.toObject();
      return AnalysisStyle
      {
        ParseUnsignedInt(object, _S(JsonTagNames::ColorId)),
        ParseString(object, _S(JsonTagNames::ParameterValue))
      };
    });
    return styles;
  }

  QPointer<AnalysisEntityModel> AnalysisIdentityModelJsonSerializer::ParseAnalysisEntityModel(const QJsonObject& json) const
  {
    using namespace AnalysisEntity;
    if (json.isEmpty())
      return nullptr;
    QPointer<AnalysisEntityModel> analysisModel = new AnalysisEntityModel();
    QMetaEnum modelDataRoleEnum = QMetaEnum::fromType<AnalysisModelDataRoles>();
    QMetaEnum entityRootRolesEnum = QMetaEnum::fromType<AbstractEntityDataModel::EntityDataRoles>();
    QMetaEnum entityDataRoleEnum = QMetaEnum::fromType<AnalysisEntity::EntityDataRoles>();
    QMetaEnum entityTypes = QMetaEnum::fromType<EntityTypes>();
    QHash<QString, int> entityDataRolesMap;
    for (int i = 0; i < entityRootRolesEnum.keyCount(); i++)
      entityDataRolesMap[entityRootRolesEnum.key(i)] = entityRootRolesEnum.value(i);
    for (int i = 0; i < entityDataRoleEnum.keyCount(); i++)
      entityDataRolesMap[entityDataRoleEnum.key(i)] = entityDataRoleEnum.value(i);

    QHash<QString, int> entityTypeMap;
    for (int i = 0; i < entityTypes.keyCount(); i++)
      entityTypeMap[entityTypes.key(i)] = entityTypes.value(i);

    QHash<TEntityUid, AbstractEntityDataModel::DataPtr> newData;
    auto chromaData = json[JsonTagNames::ChromatogrammData].toObject();
    auto jchromaEntities = chromaData["entities"].toArray();
    auto jmodelData = json["modelData"].toObject();
    QHash<int, QVariant> modelData;
    for (auto& stringKey : jmodelData.keys())
    {
      int key = modelDataRoleEnum.keyToValue(stringKey.toStdString().c_str());
      if (key == -1)
        continue;
      AnalysisModelDataRoles role = (AnalysisModelDataRoles)key;
      switch (role)
      {
      case ModelRoleIsSpectral:
        modelData[role] = jmodelData[stringKey].toBool();
        break;
      case ModelRoleSpectralRenderType:
        modelData[role] = (RenderType)(jmodelData[stringKey].toInt());
        break;
      case ModelRoleCurrentXComponentNum:
      case ModelRoleCurrentYComponentNum:
      case ModelRoleCurrentSpectralComponentNum:
        modelData[role] = jmodelData[stringKey].toInt();
        break;
      case ModelRoleComponentsExplVarianceMap:
      {
        TComponentsExplVarianceMap map;
        QJsonArray cevArr = jmodelData[stringKey].toArray();
        for (int i = 0; i < cevArr.size(); i++)
        {
          QJsonObject cevElem = cevArr[i].toObject();
          auto key = cevElem[JsonTagNames::Key].toInt();
          auto value = cevElem[JsonTagNames::Value].toDouble();
          map[key] = value;
        }
        modelData[role] = QVariant::fromValue(map);
        break;
      }
      case ModelRoleColorFilter:
      case ModelRoleShapeFilter:
      {
        auto jfilter = jmodelData[stringKey].toObject();
        TPassportFilter filter{ jfilter.toVariantMap() };
        modelData[role] = QVariant::fromValue(filter);
        break;
      }
      case ModelRoleLast:
      default:
        break;
      }
    }

    QString typeKey = entityRootRolesEnum.valueToKey(AbstractEntityDataModel::RoleEntityType);
    QString uidKey = entityRootRolesEnum.valueToKey(AbstractEntityDataModel::RoleEntityUid);
    for (int i = 0; i < jchromaEntities.size(); i++)
    {
      auto jentity = jchromaEntities[i].toObject();
      auto uid = QUuid::fromString(jentity["uid"].toString());

      AbstractEntityDataModel::DataPtr dataPtr = AbstractEntityDataModel::DataPtr::create();
      newData[uid] = dataPtr;
      auto& dataRef = *dataPtr;
      auto entityTypeStr = jentity[typeKey].toString().toStdString();

      if (entityTypeMap.contains(entityTypeStr.c_str()))
        dataRef[AbstractEntityDataModel::RoleEntityType] = entityTypeMap[entityTypeStr.c_str()];
      else
        dataRef[AbstractEntityDataModel::RoleEntityType] = TypeLast;

      dataRef[AbstractEntityDataModel::RoleEntityUid] = uid;
      for (auto& stringDataRole : jentity.keys())
      {
        auto enumKeyStr = stringDataRole.toStdString();
        int dataRole = entityDataRolesMap.value(enumKeyStr.c_str(), -1);
        if (dataRole == -1)
          continue;
        switch (dataRole)
        {
        case RoleShape:
        {
          auto path_bin = QByteArray::fromBase64(jentity[stringDataRole].toString().toLatin1());
          QDataStream stream(path_bin);
          QPainterPath path;
          stream >> path;
          dataRef[dataRole] = QVariant::fromValue(path);
          break;
        }
        case RoleShapePixelSize:
        case RoleIntId:
        case RoleOrderNum:
        {
          dataRef[dataRole] = jentity[stringDataRole].toInt();
          break;
        }
        case RoleForceShowName:
        case RoleForceShowPassport:
        case RoleGroupUserCreated:
        case RoleExcluded:
        {
          dataRef[dataRole] = jentity[stringDataRole].toBool();
          break;
        }
        case RoleTitle:
        {
          dataRef[dataRole] = jentity[stringDataRole].toString();
          break;
        }
        case RoleColor:
        {
          auto jobj = jentity[stringDataRole].toObject();
          auto red = jobj["red"].toInt();
          auto green = jobj["green"].toInt();
          auto blue = jobj["blue"].toInt();
          auto alpha = jobj["alpha"].toInt();
          dataRef[dataRole] = QColor(red, green, blue, alpha);
          break;
        }
        case RoleGroupsUidSet:
        {
          TGroupsUidSet set;
          auto jarr = jentity[stringDataRole].toArray();
          for (int i = 0; i < jarr.size(); i++)
            set << TEntityUid::fromString(jarr[i].toString());
          dataRef[dataRole] = QVariant::fromValue(set);
          break;
        }
        case RolePassport:
        {
          auto jpassport = jentity[stringDataRole].toObject();
          TPassport passport{ jpassport.toVariantMap() };
          dataRef[dataRole] = QVariant::fromValue(passport);
          break;
        }
        case RolePassportFilter:
        {
          auto jfilter = jentity[stringDataRole].toObject();
          TPassportFilter filter{ jfilter.toVariantMap() };
          dataRef[dataRole] = QVariant::fromValue(filter);
          break;
        }
        case RolePeakData:
        {
          auto jpeak = jentity[stringDataRole].toObject();
          TPeakData peakData;
          peakData.title = jpeak[JsonTagNames::Title].toString();
          peakData.orderNum = jpeak["order_num"].toInt();
          dataRef[dataRole] = QVariant::fromValue(peakData);
          break;
        }
        case RoleComponentsData:
        {
          auto jCompData = jentity[stringDataRole].toArray();
          TComponentsData data;
          for (int i = 0; i < jCompData.size(); i++)
          {
            auto jcomp = jCompData[i].toObject();
            auto key = jcomp[JsonTagNames::Key].toInt();
            auto value = jcomp[JsonTagNames::Value].toDouble();
            data[key] = value;
          }
          dataRef[dataRole] = QVariant::fromValue(data);
          break;
        }
        case AbstractEntityDataModel::RoleEntityLast:
        case RoleLast:
        default:
          break;
        }
      }

      //compatibility patch:
      if (dataRef[AbstractEntityDataModel::RoleEntityType] == TypeSample
        && !dataRef.contains(RoleTitle))
      {
        auto passport = dataRef.value(RolePassport).value<TPassport>();
        auto title = AnalysisEntityModel::passportToSampleTitle(passport);
        dataRef[RoleTitle] = title;
      }
      if (dataRef[AbstractEntityDataModel::RoleEntityType] == TypePeak
        && !dataRef.contains(RoleTitle))
      {
        auto peakData = dataRef.value(RolePeakData).value<TPeakData>();
        dataRef[RoleTitle] = peakData.title;
      }
      //end of compat patch
    }
    analysisModel->setData(newData);
    analysisModel->setModelData(modelData);
    return analysisModel;
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::AnalysisTableToJson(const AnalysisTable& analysis_table) const
  {
    return
    {
      { JsonTagNames::CheckedValues, CheckedPeaksIdsToJson(analysis_table.CheckedValues) },
      { JsonTagNames::Concentrations, DoubleOptionalVector2DToJsonArray(analysis_table.Concentrations)},
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::CheckedPeaksIdsToJson(const CheckedPeakIds& checkedPeaks) const
  {
    return
    {
      { JsonTagNames::MarkersIds, IntVectorToJsonArray(checkedPeaks.MarkersIds) },
      { JsonTagNames::SamplesIds, IntVectorToJsonArray(checkedPeaks.SamplesIds) }
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::ChromatogrammDataToJson(const AnalysisMetaInfoListByFileId& chromatogramm_data) const
  {
    QJsonObject obj;
    for (const auto& [id, data] : chromatogramm_data)
    {
      obj.insert(_Q(id), ChromatogrammDataToJson(data));
    }
    return obj;
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::ChromatogrammDataToJson(const MetaInfo& chromatogramm_data) const
  {
    return
    {
      { JsonTagNames::Date, _Q(chromatogramm_data.Date) },
      { JsonTagNames::Field, _Q(chromatogramm_data.Field) },
      { JsonTagNames::FileId, static_cast<int>(chromatogramm_data.FileId) },
      { JsonTagNames::FileName, _Q(chromatogramm_data.FileName) },
      { JsonTagNames::Layer, _Q(chromatogramm_data.Layer) },
      { JsonTagNames::Well, _Q(chromatogramm_data.Well) },
      { JsonTagNames::WellCluster, _Q(chromatogramm_data.WellCluster) },
    };
  }

  QJsonArray AnalysisIdentityModelJsonSerializer::PeaksToJsonArray(const AnalysisPeaks& peaks) const
  {
    QJsonArray peakArray;
    std::transform(peaks.begin(), peaks.end(), std::back_inserter(peakArray),
      [&](const AnalysisPeak& peak) { return PeakToJson(peak); });
    return peakArray;
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::PeakToJson(const AnalysisPeak& peak) const
  {
    return
    {
      { JsonTagNames::FormFactor, _Q(peak.FormFactor) },
      { JsonTagNames::Area, peak.Parameters.Area },
      { JsonTagNames::Index, peak.Parameters.CovatsIndex },
      { JsonTagNames::Height, peak.Parameters.Height },
      { JsonTagNames::MarkerWindow, peak.Parameters.MarkerWindow },
      { JsonTagNames::RetantionTime, peak.Parameters.RetantionTime },
      { JsonTagNames::Color, _Q(peak.PeakColor)},
      { JsonTagNames::XBottom, peak.PeakRectangle.XBottom},
      { JsonTagNames::XTop, peak.PeakRectangle.XTop },
      { JsonTagNames::YBottom, peak.PeakRectangle.YBottom },
      { JsonTagNames::YTop, peak.PeakRectangle.YTop },
      { JsonTagNames::Title, _Q(peak.Title) },
      { JsonTagNames::XEnd, peak.XInterval.End },
      { JsonTagNames::XStart, peak.XInterval.Start },
      { JsonTagNames::YEnd, peak.YInterval.End },
      { JsonTagNames::YStart, peak.YInterval.Start },
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::AnalysisDataToJson(const AnalysisData& analysis_data) const
  {
    return
    {
      { JsonTagNames::CountPlotSettings, CountPlotSettingsToJson(analysis_data.PlotSettings) },
      { JsonTagNames::Data, AnalysisInnerDataToJson(analysis_data.Data) },
      { JsonTagNames::WidgetSettings, WidgetSettginsToJson(analysis_data.WidgetSettings) },
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::CountPlotSettingsToJson(const CountPlotSettings& count_plot_settings) const
  {
    return
    {
      { JsonTagNames::GroupingParams, GroupingParamsToJson(count_plot_settings.Params) },
      // for style json object
      {
      JsonTagNames::Style,
      QJsonObject
      {
        { JsonTagNames::ColorStyle, ObjectStylesToJson(count_plot_settings.ColorStyles) },
        { JsonTagNames::CustomStyle, ObjectStylesToJson(count_plot_settings.CustomStyles) },
        { JsonTagNames::ShapeStyle, ObjectStylesToJson(count_plot_settings.ShapeStyles) },
      }
      }
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::GroupingParamsToJson(const GroupingParams& grouping_params) const
  {
    return
    {
      { JsonTagNames::Color, static_cast<int>(grouping_params.Color) },
      { JsonTagNames::Shape, static_cast<int>(grouping_params.Shape) },
    };
  }

  QJsonArray AnalysisIdentityModelJsonSerializer::ObjectStylesToJson(const ObjectStyles& styles) const
  {
    QJsonArray arr;
    std::transform(styles.begin(), styles.end(), std::back_inserter(arr),
      [&](const ObjectStyle& style)
      {
        return ObjectStyleToJson(style);
      });
    return arr;
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::ObjectStyleToJson(const ObjectStyle& style) const
  {
    return
    {
      { JsonTagNames::ParameterId, static_cast<int>(style.ParameterId) },
      { JsonTagNames::Style, AnalysisStylesToJson(style.Styles) },
    };
  }

  QJsonArray AnalysisIdentityModelJsonSerializer::AnalysisStylesToJson(const AnalysisStyles& styles) const
  {
    QJsonArray analysisStyles;
    std::transform(styles.begin(), styles.end(), std::back_inserter(analysisStyles),
      [&](const AnalysisStyle& style) { return AnalysisStyleToJson(style); });
    return analysisStyles;
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::AnalysisStyleToJson(const AnalysisStyle& style) const
  {
    return
    {
      { JsonTagNames::ColorId, static_cast<int>(style.ColorId) },
      { JsonTagNames::ParameterValue, _Q(style.ParameterValue) },
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::AnalysisInnerDataToJson(const AnalysisInnerData& inner_data) const
  {
    return
    {
      JsonPair{ JsonTagNames::PTitle, StringListToJsonArray(inner_data.P_Title) },
      JsonPair{ JsonTagNames::Pids, IntVectorToJsonArray(inner_data.P_Id)},
      JsonPair{ JsonTagNames::TTitle, AnalysisMetaInfoListToJson(inner_data.T_Title) },
      JsonPair{ JsonTagNames::TestData, AnalysisMetaInfoListToJson(inner_data.TestData) },
      JsonPair{ JsonTagNames::Components, AnalysisComponentsToJson(inner_data.Components) },
    };
  }

  QJsonArray AnalysisIdentityModelJsonSerializer::AnalysisComponentsToJson(const AnalysisComponents& comps) const
  {
    QJsonArray compList;
    std::transform(comps.begin(), comps.end(), std::back_inserter(compList),
      [&](const AnalysisComponent& comp) { return AnalysisComponentToJson(comp); });
    return compList;
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::AnalysisComponentToJson(const AnalysisComponent& comp) const
  {
    return
    {
      { JsonTagNames::ExplPCsVariance, comp.ExplPCsVariance },
      { JsonTagNames::P, DoubleVectorToJsonArray(comp.P) },
      { JsonTagNames::T, DoubleVectorToJsonArray(comp.T) },
      { JsonTagNames::Number, static_cast<int>(comp.Number) },
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::WidgetSettginsToJson(const WidgetSetting& w_setting) const
  {
    return
    {
      {
      JsonTagNames::PickedComponents, QJsonObject
      {
        { JsonTagNames::Counts, PickedComponentToJson(w_setting.Counts)},
        { JsonTagNames::Loads, PickedComponentToJson(w_setting.Loads)},
      }
      }
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::PickedComponentToJson(const PickedComponent& comp) const
  {
    return
    {
      { JsonTagNames::XComponentNum, comp.XComponentNum },
      { JsonTagNames::YComponentNum, comp.YComponentNum },
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::AnalysisEntityModelToJson(QPointer<AnalysisEntityModel> modelPtr) const
  {
    if (!modelPtr)
      return QJsonObject();
    using namespace AnalysisEntity;
    QMetaEnum modelDataRoleEnum = QMetaEnum::fromType<AnalysisModelDataRoles>();
    QMetaEnum entityRootRolesEnum = QMetaEnum::fromType<AbstractEntityDataModel::EntityDataRoles>();
    QMetaEnum entityDataRoleEnum = QMetaEnum::fromType<AnalysisEntity::EntityDataRoles>();
    QMetaEnum entityTypes = QMetaEnum::fromType<EntityTypes>();
    QHash<int, QString> entityDataRolesMap;
    for (int i = 0; i < entityRootRolesEnum.keyCount(); i++)
      entityDataRolesMap[entityRootRolesEnum.value(i)] = entityRootRolesEnum.key(i);
    for (int i = 0; i < entityDataRoleEnum.keyCount(); i++)
      entityDataRolesMap[entityDataRoleEnum.value(i)] = entityDataRoleEnum.key(i);

    QHash<int, QString> entityTypeMap;
    for (int i = 0; i < entityTypes.keyCount(); i++)
      entityTypeMap[entityTypes.value(i)] = entityTypes.key(i);

    QJsonObject root;
    QJsonObject jchromaData;
    QJsonArray jchromaEntities;
    QJsonObject jmodelData;
    auto data = modelPtr->getData();
    for (auto entityIter : data.asKeyValueRange())
    {
      const auto& entityUid = entityIter.first;
      const auto& variantMap = entityIter.second;

      QJsonObject jentity;
      jentity["uid"] = entityUid.toString();
      QJsonObject jvariantMap;
      for (auto roleIter : variantMap->asKeyValueRange())
      {
        const auto& dataRole = roleIter.first;
        const auto& value = roleIter.second;
        QString stringKey = entityDataRolesMap.value(dataRole, "");
        if (!stringKey.isEmpty())
        {
          switch (dataRole)
          {
          case AbstractEntityDataModel::RoleEntityType:
            jentity[stringKey] = entityTypeMap.value(value.toInt(), entityTypeMap[RoleLast]);
            break;
          case AbstractEntityDataModel::RoleEntityUid://skip
            break;
          case RoleShape:
          {
            QByteArray path_bin;
            QDataStream stream(&path_bin, QIODeviceBase::WriteOnly);
            stream << value.value<TShape>();
            jentity[stringKey] = QString(path_bin.toBase64());
            break;
          }
          case RoleShapePixelSize:
          case RoleIntId:
          case RoleOrderNum:
            jentity[stringKey] = value.toInt();
            break;
          case RoleForceShowName:
          case RoleForceShowPassport:
          case RoleGroupUserCreated:
          case RoleExcluded:
            jentity[stringKey] = value.toBool();
            break;
          case RoleTitle:
            jentity[stringKey] = value.toString();
            break;
          case RoleColor:
          {
            auto color = value.value<TColor>();
            QJsonObject obj;
            obj["red"] = color.red();
            obj["green"] = color.green();
            obj["blue"] = color.blue();
            obj["alpha"] = color.alpha();
            jentity[stringKey] = obj;
            break;
          }
          case RoleGroupsUidSet:
          {
            auto set = value.value<TGroupsUidSet>();
            QJsonArray arr;
            for (auto& uid : set)
              arr << uid.toString();
            jentity[stringKey] = arr;
            break;
          }
          case RolePassport:
          {
            auto passport = value.value<TPassport>();
            QJsonObject jpassport = QJsonObject::fromVariantMap(passport);
            jentity[stringKey] = jpassport;
            break;
          }
          case RolePassportFilter:
          {
            auto filter = value.value<TPassportFilter>();
            QJsonObject jfilter = QJsonObject::fromVariantMap(filter);
            jentity[stringKey] = jfilter;
            break;
          }
          case RolePeakData:
          {
            auto peakData = value.value<TPeakData>();
            QJsonObject jpeak;
            jpeak[JsonTagNames::Title] = peakData.title;
            jpeak["order_num"] = peakData.orderNum;
            jentity[stringKey] = jpeak;
            break;
          }
          case RoleComponentsData:
          {
            auto compData = value.value<TComponentsData>();
            QJsonArray jcompArr;
            for (auto iter = compData.constKeyValueBegin(); iter != compData.constKeyValueEnd(); iter++)
            {
              QJsonObject jcomp;
              jcomp[JsonTagNames::Key] = iter->first;
              jcomp[JsonTagNames::Value] = iter->second;
              jcompArr << jcomp;
            }
            jentity[stringKey] = jcompArr;
            break;
          }
          case AbstractEntityDataModel::RoleEntityLast:
          case RoleLast:
          default:
            break;
          }
        }
      }
      jchromaEntities << jentity;
    }
    jchromaData["entities"] = jchromaEntities;
    root[JsonTagNames::ChromatogrammData] = jchromaData;
    auto modelData = modelPtr->modelData();
    for (auto& key : modelData.keys())
    {
      auto dataRole = AnalysisModelDataRoles(key);
      auto stringKey = modelDataRoleEnum.valueToKey(key);
      switch (dataRole)
      {
      case ModelRoleIsSpectral:
        jmodelData[stringKey] = modelData[key].toBool();
        break;
      case ModelRoleSpectralRenderType:
        jmodelData[stringKey] = modelData[key].toInt();
        break;
      case ModelRoleCurrentXComponentNum:
      case ModelRoleCurrentYComponentNum:
      case ModelRoleCurrentSpectralComponentNum:
        jmodelData[stringKey] = modelData[key].toInt();
        break;
      case ModelRoleComponentsExplVarianceMap:
      {
        auto cevMap = modelData[dataRole].value<TComponentsExplVarianceMap>();
        QJsonArray cevArr;
        for (auto iter = cevMap.constKeyValueBegin(); iter != cevMap.constKeyValueEnd(); iter++)
        {
          QJsonObject cevElem;
          cevElem[JsonTagNames::Key] = iter->first;
          cevElem[JsonTagNames::Value] = iter->second;
          cevArr << cevElem;
        }
        jmodelData[stringKey] = cevArr;
        break;
      }
      case ModelRoleColorFilter:
      case ModelRoleShapeFilter:
      {
        auto filter = modelData[key].value<TPassportFilter>();
        QJsonObject jfilter = QJsonObject::fromVariantMap(filter);
        jmodelData[stringKey] = jfilter;
        break;
      }
      case ModelRoleLast:
      default:
        break;
      }
    }
    root["modelData"] = jmodelData;
    return root;
  }

  PredictAnalysisMetaInfoList AnalysisIdentityModelJsonSerializer::ParsePredictAnalysisMetaInfoList(const QJsonObject& json, std::string_view tagName) const
  {
    PredictAnalysisMetaInfoList list;
    const auto infoArray = json.value(tagName.data()).toArray();
    std::transform(infoArray.begin(), infoArray.end(), std::back_inserter(list),
#ifdef QT_6_4_0
      [&](QJsonValueConstRef value)
#else
      [&](QJsonValueRef value)
#endif
    { return ParsePredictAnalysisMetaInfo(value.toObject()); });
    return list;
  }

  PredictAnalysisMetaInfo AnalysisIdentityModelJsonSerializer::ParsePredictAnalysisMetaInfo(const QJsonObject& json) const
  {
    PredictAnalysisMetaInfo info(ParseAnalysisMetaInfo(json));
    info.YPredConcentration = std::nullopt;
    const auto predConcentration = json.value(JsonTagNames::YPredConcentration);
    if (!predConcentration.isNull() && predConcentration.isDouble())
    {
      info.YPredConcentration = predConcentration.toDouble();
    }
    return info;
  }

  PlsPredictParameter AnalysisIdentityModelJsonSerializer::ParsePlsPredictParameter(const QJsonObject& json, std::string_view tagName) const
  {
    const auto params = json.value(tagName.data()).toObject();
    return
    {
      ParseBool(json, JsonTagNames::Autoscale.toStdString()),
      params.value(JsonTagNames::CV).toInt(),
      ParseString(json, JsonTagNames::LayerName.toStdString()),
      params.value(JsonTagNames::NMaxLV).toInt(),
      ParseBool(json, JsonTagNames::Normdata.toStdString()),
      params.value(JsonTagNames::ParentAnalysisId).toInt(),
      ParseString(json, JsonTagNames::Predict.toStdString()),
      params.value(JsonTagNames::TableId).toInt()
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::PlsPredictParamsToJson(const PlsPredictParameter& data) const
  {
    return
    {
      { JsonTagNames::Autoscale, data.autoscale },
      { JsonTagNames::CV, data.cv },
      { JsonTagNames::LayerName, QString::fromStdString(data.layer_name) },
      { JsonTagNames::NMaxLV, data.nMaxLV },
      { JsonTagNames::Normdata, data.normdata },
      { JsonTagNames::ParentAnalysisId, data.parent_analysis_id },
      { JsonTagNames::Predict, QString::fromStdString(data.predict) },
      { JsonTagNames::TableId, data.table_id }
    };
  }

  QJsonObject AnalysisIdentityModelJsonSerializer::PredictAnalysisMetaInfoToJson(const PredictAnalysisMetaInfo& data) const
  {
    auto result = AnalysisMetaInfoToJson(data);
    if (data.YPredConcentration.has_value())
    {
      JSonCombiner combiner;
      result = combiner.Combine(result, { { JsonTagNames::YPredConcentration, data.YPredConcentration.value()} });
    }

    return result;
  }
  QJsonArray AnalysisIdentityModelJsonSerializer::PredictAnalysisMetaInfoListToJson(const PredictAnalysisMetaInfoList& dataList) const
  {
    QJsonArray infoArray;
    std::transform(dataList.begin(), dataList.end(), std::back_inserter(infoArray),
      [&](const PredictAnalysisMetaInfo& metaList) { return PredictAnalysisMetaInfoToJson(metaList); });
    return infoArray;
  }
}
