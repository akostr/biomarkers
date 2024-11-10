#include "view_page_pca_model.h"
#include "ui_view_page_pca_model.h"
#include <genesis_style/style.h>
#include <ui/plots/analysis_plot_refactor.h>
#include <ui/plots/gp_items/gpshape.h>
#include <ui/plots/gp_items/gpshape_item.h>
#include <logic/context_root.h>
#include <logic/known_json_tag_names.h>
#include <logic/notification.h>
#include <logic/models/analysis_plot_item_model.h>
#include <api/api_rest.h>
#include <QTreeView>
// #include <logic/service/ijson_serializer.h>
// #include <logic/service/service_locator.h>
// #include <logic/models/pca_analysis_identity_model.h>

// using namespace Model;
// using namespace Structures;
// using namespace Names;
// using namespace Service;
// using namespace Widgets;
using namespace Core;
using namespace AnalysisEntity;
// using namespace Constants;

namespace Views
{
ViewPagePcaModel::ViewPagePcaModel(QWidget *parent)
  : View(parent),
  ui(new Ui::ViewPagePcaModel)
{
  setupUi();
}

void ViewPagePcaModel::ApplyContextMarkupVersionAnalysisPCA(const QString &dataId, const QVariant &data)
{
  if(isDataReset(dataId, data))
  {
    auto context = GenesisContextRoot::Get()->GetContextMarkupVersionAnalisysPCA();
    auto id = context->GetData(JsonTagNames::analysis_id).toInt();
    if (id > 0)
    {
      loadModels(id);
    }
    else
    {
      // const auto object = context->GetData(ContextTagNames::PCAData).toJsonObject();
      // const auto jsonResponseSerializer = ServiceLocator::Instance().Resolve<PcaAnalysisResponseModel, IJsonSerializer>();
      // PcaResponseModel = jsonResponseSerializer->ToModel(object);
      // CreatePcaModel();
      // LoadGraphicsFromResponseModel();
      Notification::NotifyError(tr("not implemented yet"));
    }
  }
}

void ViewPagePcaModel::setupUi()
{
  setStyleSheet(Style::Genesis::GetUiStyle());
  ui->setupUi(this);
  auto splitter = new QSplitter(Qt::Horizontal, ui->countsTab);
  auto countsLayout = new QHBoxLayout(ui->countsTab);
  ui->countsTab->setLayout(countsLayout);
  countsLayout->addWidget(splitter);
  mCountPlot = new AnalysisPlotRefactor(ui->countsTab);
  splitter->addWidget(mCountPlot);
  // countsLayout->addWidget(mCountPlot, 1);
  mCountModelTreeView = new QTreeView(ui->countsTab);
  mCountModelTreeView->setMinimumWidth(200);
  splitter->addWidget(mCountModelTreeView);
  // countsLayout->addWidget(mCountModelTreeView);
}

void ViewPagePcaModel::loadModels(int analysisId)
{
  // API::REST::GetAnalysis(analysisId,
  //     [&, analysisId](QNetworkReply*, QJsonDocument doc)
  //     {
  //       if (const auto json = doc.object(); json.value(JsonTagNames::Error).toBool())
  //       {
  //         Notification::NotifyError(tr("Error : %1 while loading analysis.").arg(json.value(JsonTagNames::Status).toString()));
  //       }
  //       else
  //       {
  //         int currentShapeInd = 0;
  //         int currentColorInd = 0;
  //         QMap<QString, TEntityUid> fields;
  //         QMap<QString, TEntityUid> layers;
  //         QMap<QPair<QString, QString>, TEntityUid> layerField;
  //         auto entityModel = new AnalysisEntityModel;
  //         auto getShapeGroup = [&](QString field)->TEntityUid
  //         {
  //           if(fields.contains(field))
  //             return fields[field];
  //           DataModel groupData;
  //           groupData[RoleShape] = QVariant::fromValue(GPShape::GetPath(GPShapeItem::GetCycledShape(currentShapeInd++), 1, 1));
  //           groupData[RoleShapePixelSize] = 10;
  //           groupData[RoleGroupName] = "field: \"" + field + "\"";
  //           fields[field] = entityModel->addNewEntity(TypeShapeGroup, groupData);
  //           return fields[field];
  //         };
  //         auto getColorGroup = [&](QString layer)->TEntityUid
  //         {
  //           if(layers.contains(layer))
  //             return layers[layer];
  //           DataModel groupData;
  //           auto color = GPShapeItem::GetCycledColor(currentColorInd++);
  //           groupData[RoleColor] = color;
  //           groupData[RoleGroupName] = "layer: \"" + layer + "\"";
  //           layers[layer] = entityModel->addNewEntity(TypeColorGroup, groupData);
  //           return layers[layer];
  //         };
  //         auto getGroup = [&](QString layer, QString field)
  //         {
  //           QPair<QString, QString> pair = {layer, field};
  //           if(layerField.contains(pair))
  //             return layerField[pair];
  //           DataModel groupData;
  //           auto color = GPShapeItem::GetCycledColor(currentColorInd++);
  //           groupData[RoleColor] = color;
  //           groupData[RoleShape] = QVariant::fromValue(GPShape::GetPath(GPShapeItem::GetCycledShape(currentShapeInd++), 1, 1));
  //           groupData[RoleShapePixelSize] = 10;
  //           groupData[RoleGroupName] = "field: \"" + field + "\" - layer: \"" + layer + "\"";
  //           layerField[pair] = entityModel->addNewEntity(TypeGroup, groupData);
  //           return layerField[pair];
  //         };
  //         // qDebug().noquote() << doc.toJson();
  //         auto jroot = doc.object();
  //         auto jplotData = jroot["data"].toObject()["analysis_data"].toObject()["data"].toObject();
  //         auto jpeaksTitleArr = jplotData["P title"].toArray();
  //         auto jpeaksIdsArr = jplotData["P_id"].toArray();
  //         auto jsamplesPassportsArr = jplotData["T title"].toArray();
  //         auto jcomponentsArr = jplotData["components"].toArray();
  //         QVector<QHash<int, double>> peaksComponentsArrays;
  //         QVector<QHash<int, double>> samplesComponentsArrays;
  //         QHash<int, double> explPCsVariance;
  //         peaksComponentsArrays.resize(jpeaksTitleArr.size(), {});
  //         samplesComponentsArrays.resize(jsamplesPassportsArr.size(), {});

  //         for(int i = 0; i < jcomponentsArr.size(); i++)
  //         {
  //           auto jcomp = jcomponentsArr[i].toObject();
  //           auto number = jcomp["number"].toInt();
  //           explPCsVariance[number] = jcomp["ExplPCsVariance"].toDouble();
  //           auto jpeaksComp = jcomp["P"].toArray();
  //           auto jsamplesComp = jcomp["T"].toArray();
  //           for(int j = 0; j < jpeaksComp.size(); j++)
  //             peaksComponentsArrays[j][i] = jpeaksComp[j].toDouble();
  //           for(int j = 0; j < jsamplesComp.size(); j++)
  //             samplesComponentsArrays[j][i] = jsamplesComp[j].toDouble();
  //         }
  //         for(int i = 0; i < jpeaksTitleArr.size(); i++)
  //         {
  //           auto peakName = jpeaksTitleArr[i].toString();
  //           DataModel data;
  //           TPeakData peakData;
  //           peakData.title = peakName;
  //           peakData.excluded = false;
  //           peakData.id = jpeaksIdsArr[i].toInt();
  //           peakData.orderNum = i;
  //           data[RolePeakData] = QVariant::fromValue(peakData);
  //           TComponentsData components = peaksComponentsArrays[i];
  //           data[RoleComponentsData] = QVariant::fromValue(components);
  //           data[RoleShape] = QVariant::fromValue(GPShape::GetPath(GPShape::Circle, 1, 1));
  //           data[RoleShapePixelSize] = 10;
  //           data[RoleColor] = GPShapeItem::ToColor(GPShapeItem::Blue);
  //           entityModel->addNewEntity(TypePeak, data);
  //         }
  //         for(int i = 0; i < jsamplesPassportsArr.size(); i++)
  //         {
  //           auto jpassport  = jsamplesPassportsArr[i].toObject();
  //           TPassport passport;
  //           passport.date = jpassport["date"].toString();
  //           passport.field = jpassport["field_title"].toString();
  //           passport.fileId = jpassport["file_id"].toInt();
  //           passport.filename = jpassport["filename"].toString();
  //           passport.layer = jpassport["layer_title"].toString();
  //           passport.sampleId = jpassport["sample_id"].toInt();
  //           passport.well = jpassport["well_title"].toString();
  //           DataModel data;
  //           data[RolePassport] = QVariant::fromValue(passport);
  //           TComponentsData components = samplesComponentsArrays[i];
  //           data[RoleComponentsData] = QVariant::fromValue(components);
  //           data[RoleShape] = QVariant::fromValue(GPShape::GetPath(GPShape::Circle, 1, 1));
  //           data[RoleShapePixelSize] = 10;
  //           data[RoleColor] = GPShapeItem::ToColor(GPShapeItem::Blue);
  //           TGroupsUidSet groupsUids = {getGroup(passport.layer, passport.field)};
  //           data[RoleGroupsUidSet] = QVariant::fromValue(groupsUids);
  //           entityModel->addNewEntity(TypeSample, data);
  //         }
  //         entityModel->setExplVarianceMap(explPCsVariance);
  //         entityModel->setCurrentXYComponent(0,1);
  //         entityModel->setParent(mCountPlot);
  //         auto oldModel = mCountPlot->getModel();
  //         mCountPlot->setModel(entityModel);
  //         if(oldModel)
  //           oldModel->deleteLater();
  //         auto oldItemModel = mCountModelTreeView->model();
  //         auto itemModel = new AnalysisPlotItemModel(mCountModelTreeView);
  //         itemModel->setEntityModel(entityModel);
  //         mCountModelTreeView->setModel(itemModel);
  //         if(oldItemModel)
  //           oldItemModel->deleteLater();
  //       }

  //     },
  //     [](QNetworkReply*, QNetworkReply::NetworkError)
  //     {
  //       Notification::NotifyError(tr("Error : while loading analysis."));
  //     });
}

ViewPagePcaModel::~ViewPagePcaModel()
{
  delete ui;
}
}//namespace Views
