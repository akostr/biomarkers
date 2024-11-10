#include "biomarkers_markup_step_controller.h"
#include <logic/notification.h>
#include <ui/genesis_window.h>
#include <logic/context_root.h>
#include <QApplication>
#include <logic/known_context_tag_names.h>
// #include <QFile>

using namespace Core;
#define MARKUP_ID GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kMarkupId).toInt()

namespace GenesisMarkup
{
BiomarkersMarkupStepController::BiomarkersMarkupStepController(QObject *parent)
  : MarkupStepController(parent)
{}

void BiomarkersMarkupStepController::setupSteps()
{
  mStepsInfo.clear();
  {
    StepInfo step0;
    step0.stepName = tr("Data loading");
    step0.stepNumber = 0;
    step0.slaveInteractions = {
      SISExportAsImage
    };
    step0.masterInteractions = {
      SIMSetupParameters            |
      SIMEnabledStyle            |
      SIMCutTail                    |
      SIMAddRemoveBaseLinePoint     |
      SIMBaseLinePointInteractions  |
      SIMModifyParamIntervals       |
      SIMModifyMarkupIntervals      |
      SIMExportAsImage
    };
    step0.guiInteractions = {
      SIGPickMaster           |
      SIGImportChromatogramms |
      SIGSaveMarkup           |
      SIGSaveMarkupAs         |
      SIGViewPassport         |
      SIGDetectPeaks          |
      SIGRemoveSlaveChromatogramm
    };
    step0.step = Step0DataLoad;
    mStepsInfo.append(step0);
  }

  {
    StepInfo step1;
    step1.stepName = tr("Initial detected peaks setup");
    step1.toolTip = tr(
"- Selecting a referense\n\
- Configuring general and individual settings\n\
- Tail trimming/restoration");
//"-Выбор эталона\n\
//-Настройка общих и индивидуальных параметров\n\
//-Обрезка/восстановление хвоста";
    step1.stepNumber = 1;
    step1.slaveInteractions = {
      SISExportAsImage
    };
    step1.masterInteractions = {
      SIMCutTail                   |
      SIMAddRemoveBaseLinePoint    |
      SIMBaseLinePointInteractions |
      SIMAddRemovePeak             |
      SIMExportAsImage             |
      SIMModifyPeaks               |
      SIMSetupParameters           |
      SIMEnabledStyle           |
      SIMAddRemoveInterval         |
      SIMModifyParamIntervals      |
      SIMModifyMarkupIntervals
    };
    step1.guiInteractions = {
      SIGPickMaster           |
      SIGImportChromatogramms |
      SIGViewPassport         |
      SIGNextStep             |
      SIGSaveMarkup |
      SIGSaveMarkupAs |
      SIGPrevStep             |
      SIGRemarkupSlaves|
      SIGRemoveSlaveChromatogramm
    };
    step1.step = Step1PeakDetection;
    mStepsInfo.append(step1);
  }

  {
    StepInfo step2;
    step2.stepName = tr("Reference markers markup");
    step2.toolTip = tr(
"Working with the reference:\n\
-Marker marking\n\
-Assigning indexes\n\
 in the marker table");
//"Работа с эталоном:\n\
// -Разметка маркеров\n\
// -Присвоение индексов\n\
//  в таблице маркеров";
    step2.stepNumber = 2;
    step2.slaveInteractions = {
      SISExportAsImage
    };
    step2.masterInteractions = {
      SIMAddRemoveBaseLinePoint    |
      SIMBaseLinePointInteractions |
      SIMAddRemoveMarker           |
      SIMAddRemovePeak             |
      SIMSetUnsetMarker            |
      SIMExportAsImage             |
      SIMModifyPeaks               |
      SIMModifyMarkers             |
      SIMEnabledStyle             |
      SIMSetupParameters
    };
    step2.guiInteractions = {
      SIGImportChromatogramms |
      SIGViewPassport         |
      SIGMarkersTable         |
      SIGSaveMarkup |
      SIGSaveMarkupAs |
      SIGNextStep             |
      SIGPrevStep             |
      SIGRemarkupSlaves|
      SIGRemoveSlaveChromatogramm
    };
    step2.step = Step2MasterMarkersMarkup;
    mStepsInfo.append(step2);
  }

//   {
//     StepInfo step3;
//     step3.stepName = tr("Reference intermarkers markup");
//     step3.toolTip = tr(
// "Working with the reference:\n\
// - Marking of intermarker peaks\n\
// - Markers table available");
// //"Работа с эталоном:\n\
// //- Разметка межмаркерных пиков\n\
// //- Доступна таблица маркеров";
//     step3.stepNumber = 3;
//     step3.slaveInteractions = {
//       SISExportAsImage
//     };
//     step3.masterInteractions = {
//       SIMAddRemoveBaseLinePoint    |
//       SIMBaseLinePointInteractions |
//       SIMAddRemoveInterMarker      |
//       SIMAddRemovePeak             |
//       SIMSetUnsetInterMarker       |
//       SIMExportAsImage             |
//       SIMModifyPeaks               |
//       SIMModifyInterMarkers        |
//       SIMEnabledStyle        |
//       SIMSetupParameters
//     };
//     step3.guiInteractions = {
//       SIGImportChromatogramms |
//       SIGViewPassport         |
//       SIGMarkersTable         |
//       SIGNextStep             |
//       SIGSaveMarkup |
//       SIGSaveMarkupAs |
//       SIGPrevStep             |
//       SIGDigitalDataTable     |
//       SIGRemarkupSlaves|
//       SIGRemoveSlaveChromatogramm
//     };
//     step3.step = Step3MasterInterMarkersMarkup;
//     mStepsInfo.append(step3);
//   }

  {
    StepInfo step3;
    step3.stepName = tr("Additional chromatogramms peaks editing");
    step3.toolTip = tr(
"Working with additional\nchromatograms:\n\
- Configuring general and individual settings\n\
- Tail trimming/restoration\n\
- Markers table available\n\
- Numerical data table available");
//"Работа с сопроводительными\n хроматограмми:\n\
//- Настройка общих и индивидуальных параметров\n\
//- Обрезка/восстановление хвоста\n\
//- Доступна таблица маркеров\n\
//- Доступна таблица числовых данных";
    step3.stepNumber = 3;
    step3.slaveInteractions = {
      SISCutTail                   |
      SISAddRemoveBaseLinePoint    |
      SISBaseLinePointInteractions |
      SISAddRemovePeak             |
      SISExportAsImage             |
      SISModifyPeaks               |
      SISAddRemoveInterval         |
      SISModifyParamIntervals      |
      SISModifyMarkupIntervals     |
      SISEnabledStyle     |
      SISSetupParameters
    };
    step3.masterInteractions = {
      SIMExportAsImage
    };
    step3.guiInteractions = {
      SIGImportChromatogramms      |
      SIGViewPassport              |
      SIGMarkersTable              |
      SIGSaveMaster                |
      SIGSaveMarkup                |
      SIGSaveMarkupAs              |
      SIGNextStep                  |
      SIGPrevStep                  |
      SIGDigitalDataTable          |
      SIGRemarkupSlaves            |
      SIGRemoveSlaveChromatogramm
    };
    step3.step = Step4SlavePeaksEditing;
    mStepsInfo.append(step3);
  }

  {
    StepInfo step4;
    step4.stepName = tr("Markers transfer");
    step4.toolTip = tr(
"Working with additional\n\
chromatograms:\n\
- Transferring and marking markers\n\
- Correction of time windows in the marker table\n\
- Numerical data table available");
//"Работа с сопроводительными\n\
//хроматограмми:\n\
//- Перенос и разметка маркеров\n\
//- Коррекция окон времени в таблице маркеров\n\
//- Доступна таблица числовых данных";
    step4.stepNumber = 4;
    step4.slaveInteractions = {
      SISAddRemoveBaseLinePoint    |
      SISBaseLinePointInteractions |
      SISAddRemoveMarker           |
      SISAddRemovePeak             |
      SISSetUnsetMarker            |
      SISExportAsImage             |
      SISModifyPeaks               |
      SISModifyMarkers             |
      SISEnabledStyle            |
      SISSetupParameters
    };
    step4.masterInteractions = {
      SIMExportAsImage
    };
    step4.guiInteractions = {
      SIGImportChromatogramms |
      SIGViewPassport         |
      SIGMarkersTable         |
      SIGSaveMaster           |
      SIGNextStep             |
      SIGSaveMarkup |
      SIGSaveMarkupAs |
      SIGPrevStep             |
      SIGDigitalDataTable     |
      SIGRepeatMarkerTransfer |
      SIGRemarkupSlaves|
      SIGRemoveSlaveChromatogramm
    };
    step4.step = Step5MarkersTransferToSlave;
    mStepsInfo.append(step4);
  }

  StepInfo step5;
  step5.stepName = tr("Identification");
  step5.toolTip =tr("Compounds identification");
  step5.stepNumber = 5;
  step5.slaveInteractions = {
    SISExportAsImage
  };
  step5.masterInteractions = {
    SIMExportAsImage /*|
    SIMKeySelectorInteractions*/
  };
  step5.guiInteractions = {
    SIGImportChromatogramms |
    SIGViewPassport         |
    SIGMarkersTable         |
    SIGSaveMaster           |
    SIGSaveMarkup |
    SIGSaveMarkupAs |
    SIGComplete             |
    SIGPrevStep             |
    SIGDigitalDataTable     |
    SIGRepeatInterMarkerTransfer |
    SIGRemarkupSlaves|
    SIGRemoveSlaveChromatogramm |
    SIGIdentification
  };
  step5.step = Step6Identification;
  mStepsInfo.append(step5);

  StepInfo step6;
  step6.stepName = tr("Comlpetion");
  step6.toolTip = tr(
"Marking complete.\n\
You can come back if necessary\n\
at any of the steps and edit it");
//"Разметка завершена.\n\
//При необходимости можно вернуться\n\
//на любой из шагов и отредактировать ее";
  step6.stepNumber = 6;
  step6.slaveInteractions = {
    SISExportAsImage
  };
  step6.masterInteractions = {
    SIMExportAsImage
  };
  step6.guiInteractions = {
    SIGViewPassport         |
    SIGSaveMaster           |
    SIGMatrix               |
    SIGSaveMarkupAs |
    SIGEditNameAndComment   |
    SIGEditCopy             |
    SIGPrevStep             |
    SIGSwitchToTable
  };
  step6.step = Step7Complete;
  mStepsInfo.append(step6);
}


void BiomarkersMarkupStepController::loadDataForStep(Steps newStep, MarkupModelPtr modelPlaceholder, const QVariant &details)
{
  switch(newStep)
  {
  case Step6Identification:
  {
    auto previousStepModel = mStepModels[mStepsInfo[mCurrentStepIndex].step];
    if(!previousStepModel->hasMaster())
    {
      Notification::NotifyError(tr("No reference plot"));
      return;
    }

    auto dataToServer = MarkupDataProcessing::Reservoir::formDataForStep5(previousStepModel, details);
    loadNextStepModel(newStep,
                      mStepsInfo[mCurrentStepIndex].step,
                      dataToServer,
                      API::REST::Identification,
                      qOverload<Steps, QJsonObject, MarkupModelPtr, QUuid&>(MarkupDataProcessing::Biomarkers::parseResponseForStep6Identification),
                      modelPlaceholder);
    break;
  }
  case Step7Complete:
  {
    auto previousStepModel = mStepModels[mStepsInfo[mCurrentStepIndex].step];
    if(!previousStepModel->hasMaster())
    {
      Notification::NotifyError(tr("No reference plot"));
      return;
    }
    auto dataToServer = MarkupDataProcessing::Biomarkers::formDataForStep6(previousStepModel);
    loadNextStepModel(newStep,
                      mStepsInfo[mCurrentStepIndex].step,
                      dataToServer,
                      API::REST::TransferBiomarkers,
                      qOverload<Steps, QJsonObject, MarkupModelPtr, QUuid&>(MarkupDataProcessing::Biomarkers::parseResponseForStep7Complete),
                      modelPlaceholder,
                      [this, details]()
                      {
                        SaveMarkup(details);
                      });
    break;
  }
  default:
    MarkupStepController::loadDataForStep(newStep, modelPlaceholder, details);
  }
}

void GenesisMarkup::BiomarkersMarkupStepController::applyStepForModel(Steps newStep, MarkupModelPtr model, MarkupModelPtr modelPlaceholder, const QVariant &details, std::function<void (bool)> postLoadingAction)
{
  switch(newStep)
  {
  case Step6Identification:
  {
    auto previousStepModel = mStepModels[mStepsInfo[mCurrentStepIndex].step];
    if(!previousStepModel->hasMaster())
    {
      Notification::NotifyError(tr("No reference plot"));
      return;
    }

    auto dataToServer = MarkupDataProcessing::Reservoir::formDataForStep5(previousStepModel, details);
    loadStepForModel(previousStepModel,
                     newStep,
                     dataToServer,
                     API::REST::Identification,
                     qOverload<Steps, QJsonObject, MarkupModelPtr>(MarkupDataProcessing::Biomarkers::parseResponseForStep6Identification),
                     modelPlaceholder,
                     postLoadingAction);
    break;
  }
  case Step7Complete:
  {
    auto previousStepModel = mStepModels[mStepsInfo[mCurrentStepIndex].step];
    if(!previousStepModel->hasMaster())
    {
      Notification::NotifyError(tr("No reference plot"));
      return;
    }

    auto dataToServer = MarkupDataProcessing::Biomarkers::formDataForStep6(previousStepModel);
    loadStepForModel(previousStepModel,
                     newStep,
                     dataToServer,
                     API::REST::Identification,
                     qOverload<Steps, QJsonObject, MarkupModelPtr>(MarkupDataProcessing::Biomarkers::parseResponseForStep7Complete),
                     modelPlaceholder,
                     postLoadingAction);
    break;
  }

  default:
    MarkupStepController::applyStepForModel(newStep, model, modelPlaceholder, details, postLoadingAction);
  }
}

bool BiomarkersMarkupStepController::CanStepForward(QString *message, const QVariant &details)
{
  auto currStep = mStepsInfo[mCurrentStepIndex].step;
  qDebug() << (Steps) currStep;
  switch(currStep)
  {
  case Step0DataLoad:
  case Step1PeakDetection:
  case Step3MasterInterMarkersMarkup:
  case Step4SlavePeaksEditing:
  case Step7Complete:
  case StepEnumLast:
  default:
    return MarkupStepController::CanStepForward(message, details);
    break;
  case Step6Identification:
  {
    auto model = mStepModels[mStepsInfo[mCurrentStepIndex].step];
    if(!model->hasMaster())
    {
      if(message)
        *message = tr("No reference");
      return false;
    }
    int identifiedPeaksCount = 0;
    for(auto& peak : model->getMaster()->getEntities(TypePeak))
    {
      if(peak->hasData(PeakLibraryGroupId) && peak->getData(PeakLibraryGroupId).isValid())
        identifiedPeaksCount++;
      if(identifiedPeaksCount >= 1)
        return true;
    }
    if(message)
      *message = tr("To complete the marking it is necessary to identify at least one peak");
    return false;
  }
  case Step5MarkersTransferToSlave:
  {
    auto model = mStepModels[mStepsInfo[mCurrentStepIndex].step];
    if(!model->hasMaster())
    {
      if(message)
        *message = tr("Missed master chromatogramm");
      return false;
    }
    auto masterMarkers = model->getMaster()->getEntities(TypePeak);
    masterMarkers.removeIf([](const AbstractEntityDataModel::ConstDataPtr& value)->bool
                           {
                             return value->getData(PeakType).toInt() != PTMarker;
                           });

    for(auto& chroma : model->getChromatogrammsList())
    {
      auto chromaMarkers = chroma.second->getEntities(TypePeak);
      chromaMarkers.removeIf([](const AbstractEntityDataModel::ConstDataPtr& value)->bool
                             {
                               return value->getData(PeakType).toInt() != PTMarker;
                             });
      if(chromaMarkers.size() != masterMarkers.size())
      {
        if(message)
        {
          auto passport = mPassportsModel->getPassport(chroma.first);
          if(passport)
            *message = tr("Invalid markers count on %1").arg(passport->getData(PassportFilename).toString());
          else
            *message = tr("Invalid markers count on %1").arg(tr("#$%@!NAME ERROR! NO PASSPORT DATA!@%$#"));
        }
        return false;
      }
    }
    return true;
  }
  case Step2MasterMarkersMarkup:
  {
    auto currModel = mStepModels[currStep];
    if(auto master = currModel->getMaster())
    {
      QList<AbstractEntityDataModel::ConstDataPtr> markers;
      for(auto& peakId : master->getUIdListOfEntities(TypePeak))
      {
        auto peak = master->getEntity(peakId);
        if(peak->hasDataAndItsValid(PeakType) &&
            peak->getData(PeakType).toInt() == PTMarker)
          markers << peak;
      }
      if(markers.size() < 1)
      {
        if(message)
          *message = tr("Markers count should be more or equal to 1");
        return false;
      }
      if(message)
        *message = tr("no error");
      return true;
    }
    else
    {
      if(message)
        *message = tr("No reference picked, pick reference first");
      return false;
    }
    break;
  }
  }
}

void MarkupDataProcessing::Biomarkers::parseResponseForStep6Identification(Steps newStep, QJsonObject json, MarkupModelPtr model)
{
  model->setData(MarkupIdentificationData, QVariant::fromValue(json));
  // QFile f("identification_output.json");
  // f.open(QIODevice::WriteOnly);
  // f.write(QJsonDocument(json).toJson());
  // f.close();
}

void MarkupDataProcessing::Biomarkers::parseResponseForStep6Identification(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid &overlayId)
{
  parseResponseForStep6Identification(newStep, json, model);
  GenesisWindow::Get()->RemoveOverlay(overlayId);
  overlayId = QUuid();
}

QJsonObject MarkupDataProcessing::Biomarkers::formDataForStep6(MarkupModelPtr prevStepModel)
{
  QJsonObject dataToServer;
  {
    auto master = prevStepModel->getMaster();
    // QJsonArray files;
    QJsonObject laying;
    bool masterFound = false;
    for(auto& id : prevStepModel->getChromatogrammsIdList())
    {
      bool isMaster = id == master->id();
      if(isMaster)
        masterFound = true;
      QJsonObject file;
      file["master"] = isMaster;
      file["project_file_id"] = id;
      // files << file;

      if(!isMaster)
        laying[QString::number(id)] =
            MarkupDataProcessing::formLaying(prevStepModel->getChromatogramm(id));
    }
    if(!masterFound)
    {
      if(!prevStepModel->hasInternalMaster() &&
          prevStepModel->hasMaster())
      {
        auto master = prevStepModel->getMaster();
        QJsonObject file;
        file["master"] = true;
        file["project_file_id"] = master->id();
        // files << file;
      }
    }
    // dataToServer["files"] = files;
    dataToServer["laying"] = laying;
    dataToServer["laying_reference"] = MarkupDataProcessing::formLaying(master);
  }
  // dataToServer["match"] = details.toInt();
  return dataToServer;
}

void MarkupDataProcessing::Biomarkers::parseResponseForStep7Complete(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid &overlayId)
{
  parseResponseForStep7Complete(newStep, json, model);
  GenesisWindow::Get()->RemoveOverlay(overlayId);
  overlayId = QUuid();
}

void MarkupDataProcessing::Biomarkers::parseResponseForStep7Complete(Steps newStep, QJsonObject json, MarkupModelPtr model)
{
  auto result = json["result"].toObject();
  for(auto& stringKey : result.keys())
  {
    int id = stringKey.toInt();
    auto jchroma = result[stringKey].toObject();
    auto chroma = model->getChromatogramm(id);
    if(chroma)
    {
      chroma->parseJBaseLine(jchroma["baseline"].toArray());
      chroma->parseJPeaks(jchroma["peaks"].toArray());
      for(auto peak : chroma->getEntities(TypePeak))
      {
        chroma->setEntityData(peak->getUid(),
                              PeakTitle,
                              peak->getData(PeakCompoundTitle).toString());
      }
      auto cutPt = jchroma["x_cut_point"].toVariant();
      if(cutPt.isValid() && !cutPt.isNull())
        chroma->setCutPoint(cutPt.toDouble());
      else
        chroma->resetCutPoint();
    }
  }
}


}//namespace GenesisMarkup



void GenesisMarkup::BiomarkersMarkupStepController::SaveMarkup(const QVariant &details)
{
  auto currentStep = mStepsInfo[mCurrentStepIndex].step;
  bool remarkupCase = false;
  if(currentStep != Step7Complete && details.isValid() && details.typeId() == QMetaType::fromType<QVariantMap>().id())
  {
    //remarkup or addmarkup is active, details contains second controller data
    //and not contains title and comment wich should be stay empty
    remarkupCase = true;
  }
  else if(details.isValid())
  {
    //any other situation
  }
  auto prepareOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Markup preparing"));
  QSet<int> filesIds;
  {
    auto model = mStepModels[mStepsInfo[mCurrentStepIndex].step];
    for(auto& markup : mStepModels)
    {
      auto idList = markup->getChromatogrammsIdList();
      filesIds.unite({idList.begin(), idList.end()});
    }
  }
  QJsonObject markup = this->save();
  if(remarkupCase)
  {
    QJsonObject jsubMarkup;
    auto subdata = details.toMap()["sub_controller_data"].toMap();
    jsubMarkup["step_ind"] = subdata["step_ind"].toInt();
    jsubMarkup["target_step_ind"] = subdata["target_step_ind"].toInt();
    jsubMarkup["markup"] = subdata["json"].toJsonObject();
    filesIds.unite(subdata["files_ids"].value<QSet<int>>());
    markup["submarkup"] = jsubMarkup;
  }
  auto projectId = GenesisContextRoot::Get()->ProjectId();
  //  auto userId = GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kUserId).toInt();
  auto currModel = getCurrentModel();
  int referenceId = -1;
  if (auto master = currModel->getMaster())
  {
    auto variant = master->getChromatogramValue(ChromatogrammReferenceId);
    if(variant.isValid() && !variant.isNull())
    {
      referenceId = variant.toInt();
    }
  }
  GenesisWindow::Get()->RemoveOverlay(prepareOverlayId);
  QString loadingStr = tr("Markup saving");
  auto overlayId = GenesisWindow::Get()->ShowOverlay(loadingStr);
  auto step = mStepsInfo[mCurrentStepIndex].step;
  API::REST::SaveIdentificationMarkupState(mCurrentStepIndex,
      filesIds, MARKUP_ID, markup, projectId,
      referenceId,
      details.toMap(),
      [this, currModel, step, overlayId, markup](QNetworkReply*, QJsonDocument doc)
      {
        auto root = doc.object();
        if(root["error"].toBool())
        {
          Notification::NotifyError(root["msg"].toString(), tr("Save error"));
          emit markupSavingFinished(true);
        }
        else
        {
          Notification::NotifySuccess(tr("Markup saved"));
          if(step == Step7Complete)
          {
            int tableId = root["table_id"].toInt();
            currModel->setData(MarkupLastTableId, tableId);
            emit markupTableIdReceived(tableId);
          }
          qInfo() << "Markup save success";
          emit markupSavingFinished(false);
        }
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      },
      [overlayId, markup, this](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError(tr("Network error"), e);
// #ifdef DEBUG
//         saveToFile(markup);
// #endif
        GenesisWindow::Get()->RemoveOverlay(overlayId);
        emit markupSavingFinished(true);
      }/*,//strange behaviour. 100% instantly and then loading...
  [overlayId, loadingStr](qint64 r, qint64 t)
  {
      GenesisWindow::Get()->UpdateOverlay(overlayId, QString("%1 (%2%) (%3)").arg(loadingStr).arg((double)r * 100.0 / (double)t).arg(QLocale::system().formattedDataSize(t)));
  }*/);
}

void GenesisMarkup::BiomarkersMarkupStepController::SaveMarkupAs(const QString &markupName, const QString &markupComment, bool instantLoad)
{
  QSet<int> filesIds;
  {
    auto model = mStepModels[mStepsInfo[mCurrentStepIndex].step];
    for(auto& markup : mStepModels)
    {
      auto idList = markup->getChromatogrammsIdList();
      filesIds.unite({idList.begin(), idList.end()});
    }
  }

  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Markup saving as"));
  API::REST::SaveIdentificationAs(Core::GenesisContextRoot::Get()->ProjectId(),
      mCurrentStepIndex,
      Core::GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kMarkupId).toInt(),
      filesIds,
      markupName,
      markupComment,
      this->save(),
      [overlayId, markupName, this, instantLoad](QNetworkReply*, QJsonDocument doc)
      {
        auto obj = doc.object();
        if(obj["error"].toBool())
        {
          Notification::NotifyError(obj["msg"].toString(), tr("Markup saving error"));
          emit markupSavingFinished(true);
        }
        else
        {
          int newMarkupId = obj["indexing_id"].toInt();
          int newMarkupVersionId = obj["version_id"].toInt();
          auto contextData = GenesisContextRoot::Get()->GetContextMarkup()->GetData();
          contextData[Names::MarkupContextTags::kMarkupId] = newMarkupId;
          contextData[Names::MarkupContextTags::kMarkupTitle] = markupName;
          Notification::NotifySuccess(tr("Markup \"%1\"(%2) saved").arg(markupName).arg(newMarkupId));
          if(instantLoad)
          {
            GenesisContextRoot::Get()->GetContextMarkupVersion()->SetData(Names::MarkupContextTags::kMarkupId, newMarkupVersionId);
            GenesisContextRoot::Get()->GetContextMarkup()->SetData(Names::MarkupContextTags::kMarkupTitle, markupName);
            GenesisContextRoot::Get()->GetContextMarkup()->SetData(Names::MarkupContextTags::kMarkupId, newMarkupId);
          }
          emit markupSavingFinished(false);
        }
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      },
      [this, overlayId](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError(tr("Reference saving network error"), e);
        GenesisWindow::Get()->RemoveOverlay(overlayId);
        emit markupSavingFinished(true);
      });
}
