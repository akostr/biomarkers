#include "markup_step_controller.h"
#include "curve_data_model.h"
#include "genesis_markup_forward_declare.h"
#include "logic/markup/baseline_data_model.h"
#include "logic/markup/chromatogram_data_model.h"
#include "logic/markup/commands.h"
#include "logic/markup/genesis_markup_enums.h"
#include "logic/known_context_tag_names.h"
#include "logic/tree_model.h"
#include <api/api_rest.h>
#include <logic/context_root.h>
#include <ui/genesis_window.h>
#include <ui/dialogs/web_dialog_markers_transfer.h>
#include <ui/dialogs/web_dialog_intermarkers_transfer.h>
#include <QMetaEnum>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QUndoStack>

//#define DEBUG
using namespace Core;
//you SHOULD call the initialize() method by yourself after object creation

#define MARKUP_ID GenesisContextRoot::Get()->GetContextMarkup()->GetData(Names::MarkupContextTags::kMarkupId).toInt()

namespace GenesisMarkup
{
MarkupStepController::MarkupStepController(QObject *parent)
  : QObject{parent}
  , mPassportsModel(new ChromatogramPassportModel())
  , mCurrentStepIndex(0)
  , mUploadCounter(0)
  , mUndoStack(new QUndoStack(this))//will be deleted by QObject
{
  mUndoStack->setUndoLimit(50);
  //you SHOULD call the initialize() method by yourself after object creation
}

MarkupStepController::~MarkupStepController()
{
}

bool MarkupStepController::CanStepForward(QString *message, const QVariant& details)
{
  if(mCurrentStepIndex == mStepsInfo.size() - 1)
  {
    if(message) *message = tr("There is last step, no steps after");
    return false;
  }
  auto currStep = mStepsInfo[mCurrentStepIndex].step;
  auto currModel = mStepModels[currStep];
  if(!currModel)
    return false;
  switch(currStep)
  {
  case Step0DataLoad:
    return true;
  case Step1PeakDetection:
    if(currModel->hasMaster())
    {
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
  case Step2MasterMarkersMarkup:
    //TODO: 3 or more markers, and their intervals table data
  {
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
      if(markers.size() < 2)
      {
        if(message)
          *message = tr("Markers count should be more or equal to 2");
        return false;
      }
      QStringList invalidMarkers;
      for(auto& marker : markers)
      {
        if(!marker->hasDataAndItsValid(PeakCovatsIndex) ||
           marker->getData(PeakCovatsIndex).toDouble() <= 0)
        {
          invalidMarkers << marker->getData(PeakTitle).toString();
          break;
        }
      }
      if(invalidMarkers.isEmpty())
      {
        if(message)
          *message = tr("no error");
        return true;
      }
      else
      {
        QString badMarkers;
        for(auto& M : invalidMarkers)
          badMarkers += M + "; ";

        if(message)
          *message = tr("invalid marker covats index value:") + " " + badMarkers;
        return false;
      }
    }
    else
    {
      if(message)
        *message = tr("No reference picked, pick reference first");
      return false;
    }
    break;
  }
  case Step3MasterInterMarkersMarkup:
    return true;
  case Step4SlavePeaksEditing:
    if(!details.isValid() || details.isNull() || !details.canConvert(QMetaType::Int))
      return false;
    return true;
  case Step5MarkersTransferToSlave:
  {
    if(!details.isValid() || details.isNull())
    {
      if(message)
        *message = tr("Missed transfer parameters");
      return false;
    }
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
  case Step6InterMarkersTransferToSlave:
    if(details.value<QPair<QString, QString>>().first.isEmpty())
    {
      if(message)
        *message = tr("empty new data table name");
      return false;
    }
    return true;
    //    if(message)
//      *message = tr("Not realized yet");
//    return false;
  case Step7Complete:
  default:
    return false;
  }
}

Steps MarkupStepController::StepForward(MarkupModelPtr modelPlaceholder, const QVariant& details)
{
  if(mCurrentStepIndex >= mStepsInfo.size() - 1)
  {
//    emit MarkupComplete();
    return mStepsInfo[mCurrentStepIndex].step;
  }
  QString message;
  if(!CanStepForward(&message, details))
  {
    Notification::NotifyError(message);

    //deferred call
    QMetaObject::invokeMethod(this, &MarkupStepController::undoAndRemoveLastCommand, Qt::QueuedConnection);
    return mStepsInfo[mCurrentStepIndex].step;
  }

  mLoadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  loadDataForStep(mStepsInfo[mCurrentStepIndex + 1].step, modelPlaceholder, details);
  return mStepsInfo[mCurrentStepIndex + 1].step;
}

QPair<Steps, MarkupModelPtr> MarkupStepController::StepBack()
{
  if(mCurrentStepIndex <= 0)
    return {mStepsInfo[mCurrentStepIndex].step, mStepModels[mStepsInfo[mCurrentStepIndex].step]};
  mLoadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  // qApp->processEvents();
  mCurrentStepIndex--;

  if(auto model = mStepModels.value(mStepsInfo[mCurrentStepIndex].step, nullptr))
    model->updateTitlesDisplayMode();

  emit ModelChanged(mStepModels[mStepsInfo[mCurrentStepIndex].step], mStepsInfo[mCurrentStepIndex]);
  GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
  mLoadingOverlayId = QUuid();
  return {mStepsInfo[mCurrentStepIndex].step, mStepModels[mStepsInfo[mCurrentStepIndex].step]};
}

void MarkupStepController::SaveMarkup(const QVariant& details)
{
  QString title;
  QString comment;
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
    auto dataPair = details.value<QPair<QString, QString>>();
    title = dataPair.first;
    comment = dataPair.second;
  }
  auto prepareOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Markup preparing"));
  QSet<int> filesIds;
  {
    auto model = mStepModels[currentStep];
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
  API::REST::SaveMarkupState(mCurrentStepIndex,
    filesIds, MARKUP_ID, markup, projectId,
    referenceId,
    title, comment,
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
        TreeModel::ResetInstances("TreeModelDynamicNumericTables");
      }
      qInfo() << "Markup save success";
      emit markupSavingFinished(false);
      TreeModel::ResetInstances("TreeModelDynamicProjectMarkupsList");
    }
    GenesisWindow::Get()->RemoveOverlay(overlayId);
  },
  [overlayId, markup, this](QNetworkReply*, QNetworkReply::NetworkError e)
  {
    QString errStr = QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(e);
    Notification::NotifyError( errStr, tr("Markup loading FAILED"));
#ifdef DEBUG
      saveToFile(markup);
#endif
    GenesisWindow::Get()->RemoveOverlay(overlayId);
    emit markupSavingFinished(true);
  }/*,//strange behaviour. 100% instantly and then loading...
  [overlayId, loadingStr](qint64 r, qint64 t)
  {
      GenesisWindow::Get()->UpdateOverlay(overlayId, QString("%1 (%2%) (%3)").arg(loadingStr).arg((double)r * 100.0 / (double)t).arg(QLocale::system().formattedDataSize(t)));
  }*/);
}

void MarkupStepController::SaveMarkupAs(const QString &markupName, const QString &markupComment, bool instantLoad)
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
  API::REST::SaveMarkupAs(Core::GenesisContextRoot::Get()->ProjectId(),
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
    Notification::NotifyError((QMetaEnum::fromType<QNetworkReply::NetworkError>().key(e)), tr("Reference saving network error"));
    GenesisWindow::Get()->RemoveOverlay(overlayId);
    emit markupSavingFinished(true);
  });
}

void MarkupStepController::LoadMarkup(int markupId)
{
  //// Здесь загрузка данных хроматограмм комбинируется с загрузкой данных моделей и самого контроллера

  //включаем спиннер
  mLoadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  //загрузаем версию разметки
  //грузим файл разметки с сервера
    API::REST::LoadMarkupState(markupId,
    [this, markupId](QNetworkReply*, QJsonDocument data)
    {
//      qDebug().noquote() << data.toJson();
      auto root = data.object();
      //файл пустой?
      if(root["error"].toBool())
      { //просто файл пустой?
        if(root["msg"].toString().contains("No data"))
        { //да, файл пустой
          createNewMarkup(markupId);
          //Do NOT hide loading overlay here, it must be hidden by createNewMarkup()
          auto result = root["result"].toObject();
          if(result.contains("m_z"))
          {
              Core::GenesisContextRoot::Get()->GetContextMarkup()->SetData("m_z", result["m_z"].toString());
          }
          if(result.contains("markup_files_types"))
          {
              auto jtypes = result["markup_files_types"].toArray();
              QStringList fileTypes;
              for(auto type : jtypes)
              {
                  fileTypes.append(type.toString());
              }
              Core::GenesisContextRoot::Get()->GetContextMarkup()->SetData("fileTypes", fileTypes);
          }
          return;
        }
        else
        {
          Notification::NotifyError(root["msg"].toString(), tr("Loading error"));
          GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
          emit markupLoadingFinished();
          return;
        }
      }
      else
      { //нет, файл не пустой:
        auto result = root["result"].toObject();
        int stepInd = result["match_step"].toInt();
        Steps step = mStepsInfo[stepInd].step;
        auto matchData = result["match_data"].toObject();
        loadExistedMarkup(matchData, step);
        if(matchData.contains("submarkup"))
        {
          emit submarkupLoadingRequired(matchData["submarkup"].toObject());
        }
        //Do NOT hide loading overlay here, it must be hidden by loadExistedMarkup()
        if (step == Step7Complete && result.contains("table_id"))
        {
          //qDebug() << result["table_id"].toInt();
          mStepModels[step]->setData(MarkupLastTableId, result["table_id"].toInt());
        }
        if(result.contains("m_z"))
        {
            Core::GenesisContextRoot::Get()->GetContextMarkup()->SetData("m_z", result["m_z"].toString());
        }
        if(result.contains("markup_files_types"))
        {
            auto jtypes = result["markup_files_types"].toArray();
            QStringList fileTypes;
            for(auto type : jtypes)
            {
                fileTypes.append(type.toString());
            }
            Core::GenesisContextRoot::Get()->GetContextMarkup()->SetData("fileTypes", fileTypes);
        }

        return;
      }
    },
    [this](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(err), tr("Server error"));
      GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
      emit markupLoadingFinished();
    },
    [this](qint64 r, qint64 t)
    {
      GenesisWindow::Get()->UpdateOverlay(mLoadingOverlayId,
                                          QString("%1 (%2%) (%3)").arg(tr("Loading")).arg(
                                          (double)r * 100.0 / (double)t, 0, 'g', 3)
                                            .arg(QLocale::system().formattedDataSize(t)));
    });

      //Грузим данные пасспорта (не обязательно)
//      API::REST::Tables::GetFileInfo(id,
//        [](QNetworkReply*, QVariantMap result)
//        {
//        },
//        [](QNetworkReply*, QNetworkReply::NetworkError err)
//        {
//          //Notification::Notify("core/db", Notification::StatusError, tr("Failed to get file information from database"), err);
//        }
//      );

//    }

    //по окончании эмитим сигнал ModelChanged
  //нет
    //load(dataFromServer);//грузим все разметки
    //потом грузим данные хроматограмм
    //и когда все закончится
    //по окончании эмитим сигнал ModelChanged
  //убираем спиннер

}

void MarkupStepController::SetCurrentStepAndSwitchHisModel(Steps step, MarkupModelPtr newMarkupModel, bool force)
{
  auto uid = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  // qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
  if(newMarkupModel != mStepModels[step])
  {
    mStepModels[step] = newMarkupModel;
    setCurrentStep(step);
  }
  else if(getCurrentStepInfo().step != step)
  {
    setCurrentStep(step);
  }
  else if(force)
  {
    setCurrentStep(step);
  }
  GenesisWindow::Get()->RemoveOverlay(uid);
}

void MarkupStepController::MergePassportDataWith(const MarkupStepController &other)
{
  auto oldIdList = mPassportsModel->getIdList();
  auto otherPassportModel = other.passportsModel();
  for(auto& id : otherPassportModel.getIdList())
  {
    if(!oldIdList.contains(id))
      mPassportsModel->addNewPassport(id);

    mPassportsModel->resetPassportData(id, *otherPassportModel.getPassport(id));
  }
}

void MarkupStepController::applyIntervals(QList<int> chromaIds, const QMap<QUuid, ChromaSettings>& intervalsData, const QMap<QUuid, QPair<double, double> > &deprecationIntervals, ChromaSettings initialSettings, Steps desiredStep)
{
  Steps currentStep = mStepsInfo[mCurrentStepIndex].step;
  auto currModel = mStepModels[currentStep];
  auto preparedModel = getFullCopyWithoutMarkup(currentStep);
  auto calcIntervals = [currModel, &intervalsData, &deprecationIntervals, &initialSettings](MarkupModelPtr newModel, bool intervalsFromMaster = false)
  {
    auto createIntervalCopyAndSetSettings = [](AbstractEntityDataModel::ConstDataPtr interval, const ChromaSettings& settings)->AbstractEntityDataModel::DataPtr
    {
      AbstractEntityDataModel::DataPtr newInterval = AbstractEntityDataModel::DataPtr::create(*interval);
      auto& is = settings;
      (*newInterval)[Coelution] = is.Coel;
      (*newInterval)[DegreeOfApproximation] = is.Doug_peuck;
      (*newInterval)[BoundsByMedian] = is.Med_bounds;
      (*newInterval)[MinimalPeakHeight] = is.Min_h;
      (*newInterval)[Noisy] = is.Noisy;
      (*newInterval)[SignalToMedianRatio] = is.Sign_to_med;
      (*newInterval)[ViewSmooth] = is.View_smooth;
      (*newInterval)[BaseLineSmoothLvl] = is.BLineSmoothLvl;
      (*newInterval)[MedianFilterWindowSize] = is.Window_size;
      (*newInterval)[IntervalIsApplied] = true;
      return newInterval;
    };
    auto createDeprecationInterval = [](AbstractEntityDataModel::ConstDataPtr interval, const QPair<double, double> &bounds)->AbstractEntityDataModel::DataPtr
    {
      AbstractEntityDataModel::DataPtr newInterval = AbstractEntityDataModel::DataPtr::create(*interval);
      (*newInterval)[IntervalType] = ITMarkupDeprecation;
      (*newInterval)[IntervalLeft] = bounds.first;
      (*newInterval)[IntervalRight] = bounds.second;
      return newInterval;
    };
    QList<AbstractEntityDataModel::DataPtr> masterIntervalsEntities;
    if(intervalsFromMaster)
    {
      if(!currModel->hasMaster())
        intervalsFromMaster = false;
      else
      {
        auto mi = currModel->getMaster()->getEntities(TypeInterval);
        for(auto& interval : mi)
        {
          auto uid = interval->getUid();
          if(interval->getData(IntervalType).toInt() == ITMarkupDeprecation)
          {
            if(deprecationIntervals.contains(uid))
              masterIntervalsEntities.append(createDeprecationInterval(interval, deprecationIntervals[uid]));
          }
          else if(intervalsData.contains(uid))
          {
            masterIntervalsEntities.append(createIntervalCopyAndSetSettings(interval, intervalsData[uid]));
          }
        }
      }
    }
    for(auto& id : newModel->getChromatogrammsIdList())
    {
      auto currChroma = currModel->getChromatogramm(id);
      if(!currChroma)
        continue;
      auto newChroma = newModel->getChromatogramm(id);
      newChroma->setChromatogramValue(ChromatogrammMode, CMMarkup);
      if(newChroma->isMaster() || intervalsFromMaster)
      {
        newChroma->clearEntities(TypeInterval);
        for(auto& interval : masterIntervalsEntities)
        {
          auto uid = interval->getUid();
          newChroma->addNewEntity(TypeInterval, *interval, uid);
        }
      }
      else
      {
        for(auto& interval : currChroma->getEntities(TypeInterval))
        {
          auto uid = interval->getUid();

          if(interval->getData(IntervalType).toInt() == ITMarkupDeprecation)
          {
            if(deprecationIntervals.contains(uid))
              newChroma->addNewEntity(TypeInterval, *createDeprecationInterval(interval, deprecationIntervals[uid]), uid);
          }
          else if(intervalsData.contains(uid))
          {
            newChroma->addNewEntity(TypeInterval, *createIntervalCopyAndSetSettings(interval, intervalsData[uid]), uid);
          }
        }
      }
      newChroma->setChromaSettings(initialSettings);
    }
  };

  if(currModel->hasMaster() && chromaIds.contains(currModel->getMaster()->id()))
  {//update all chromatogramms independent from chromaIds list
    //TODO: set loading overlay
    MarkupModelPtr newModel(new MarkupDataModel(*this, *preparedModel));
    newModel->setMaster(currModel->getMaster()->id());
    calcIntervals(newModel, true);
    MarkupModelPtr calculatedModel(new MarkupDataModel(*this));//should be filled inside postLoadingAction lambda:
    applyStepForModel(Step1PeakDetection, newModel, calculatedModel, QVariant(), [calculatedModel, currModel, currentStep, desiredStep, this](bool error)
    {
      if(error)
      {
        Notification::NotifyError(tr("Failed to apply step 1 for model"));
        return;
      }
      auto cmd = new SetStepCommand(this, currModel, currentStep, desiredStep, calculatedModel);
      mUndoStack->push(cmd);
//      emit newCommand(cmd);
    });
  }
  else
  {//update only selected chromatogramms in chromaIds list
    auto newModel = getSelectiveCopyWithExternalMaster(Step0DataLoad, chromaIds);
    calcIntervals(newModel);
    MarkupModelPtr calculatedModel(new MarkupDataModel(*this));//should be filled inside postLoadingAction lambda:
    applyStepForModel(Step1PeakDetection, newModel, calculatedModel, QVariant(), [calculatedModel, currModel, this](bool error)
    {
      if(error)
      {
        Notification::NotifyError(tr("Failed to apply step 1 for model"));
        return;
      }
      auto cmd = new SwapChromasCommand(this, currModel, calculatedModel->getChromatogrammsListModels());
      mUndoStack->push(cmd);
//      emit newCommand(cmd);
    });
  }
}

void MarkupStepController::StepBackUiSlot()
{
  auto cmd = new PrevStepCommand(this, getCurrentModel(), getCurrentStepInfo().step);
  pushCommand(cmd);
}

void MarkupStepController::updateTitlesDisplayMode()
{
  auto curr = getCurrentModel();
  if(curr)
    getCurrentModel()->updateTitlesDisplayMode();
}

MarkupModelPtr MarkupStepController::getCurrentModel() const
{
  return mStepModels.value(mStepsInfo[mCurrentStepIndex].step, nullptr);
}

StepInfo MarkupStepController::getCurrentStepInfo()
{
  return mStepsInfo[mCurrentStepIndex];
}

MarkupModelPtr MarkupStepController::getSelectiveCopyWithExternalMaster(Steps step, const QList<int> &ids)
{
  if(!mStepModels.contains(step))
    return nullptr;
  auto currModel = mStepModels[step];
  MarkupModelPtr markup(new MarkupDataModel(*this));
  markup->setData(currModel->data());
  for(auto& id : ids)
  {
    if(auto chromaPtr = currModel->getChromatogramm(id))
    {
      markup->addChromatogramm(id, *chromaPtr);
    }
    else
    {
      Q_ASSERT(false);//error: no chroma while copying by id list
      ;
    }
  }

  //we should set as master - our current step master independent to new model step
  if(auto master = mStepModels[mStepsInfo[mCurrentStepIndex].step]->getMaster())
  {
    markup->setData(MarkupMasterIdRole, master->id());
    markup->setData(MarkupExternalMasterPtr, QVariant::fromValue(master));
  }

  return markup;
}

MarkupModelPtr MarkupStepController::getFullCopyWithoutMarkup(Steps step)
{
  if(!mStepModels.contains(step))
    return nullptr;
  auto currModel = mStepModels[step];
  MarkupModelPtr markup(new MarkupDataModel(*this));
  markup->setData(currModel->data());
  for(auto& id : currModel->getChromatogrammsIdList())
  {
    if(auto chromaPtr = currModel->getChromatogramm(id))
    {
      ChromatogrammModelPtr newChromaPtr(new ChromatogramDataModel(*chromaPtr));
      newChromaPtr->removeEntitiesIf([](AbstractEntityDataModel::ConstDataPtr entity)->bool
      {
        auto eType = entity->getType();
        return eType != TypeCurve && eType != TypeBaseline;
      });

      markup->addChromatogramm(id, *newChromaPtr);
    }
    else
    {
      Q_ASSERT(false);//error: no chroma while copying by id list
      ;
    }
  }
  return markup;
}

MarkupModelPtr MarkupStepController::getCopyWithoutMarkupWithExternalMasterAndIntervals(Steps step, QList<int> chromaIdToCopy)
{
  if(!mStepModels.contains(step))
    return nullptr;
  auto currModel = mStepModels[step];
  MarkupModelPtr markup(new MarkupDataModel(*this));
  markup->setData(currModel->data());
  auto master = currModel->getMaster();
  for(auto& id : chromaIdToCopy)
  {
    if(master && master->id() == id)
      continue;
    if(auto chromaPtr = currModel->getChromatogramm(id))
    {
      ChromatogrammModelPtr newChromaPtr(new ChromatogramDataModel(*chromaPtr));
      newChromaPtr->removeEntitiesIf([](AbstractEntityDataModel::ConstDataPtr entity)->bool
      {
        auto eType = entity->getType();
        return eType != TypeCurve && eType != TypeBaseline && eType != TypeInterval;
      });

      markup->addChromatogramm(id, newChromaPtr);
    }
    else
    {
      Q_ASSERT(false);//error: no chroma while copying by id list
      ;
    }
  }

  //we should set as master - our current step master independent to new model step
  if(master)
  {
    markup->setData(MarkupMasterIdRole, master->id());
    markup->setData(MarkupExternalMasterPtr, QVariant::fromValue(master));
  }

  return markup;
}

void MarkupStepController::pushCommand(QUndoCommand *cmd)
{
  mUndoStack->push(cmd);
}

void MarkupStepController::initialize()
{
  //you SHOULD call the initialize() method by yourself after object creation
  setupSteps();
}

const ChromatogramPassportModel& MarkupStepController::passportsModel() const
{
  return *mPassportsModel;
}

QSharedPointer<ChromatogramPassportModel> MarkupStepController::passportsModelPtr()
{
  return mPassportsModel;
}

const QList<StepInfo> &MarkupStepController::stepsInfo() const
{
  return mStepsInfo;
}

const QMap<Steps, MarkupModelPtr> &MarkupStepController::stepModels() const
{
  return mStepModels;
}

void MarkupStepController::setPassportsModel(const ChromatogramPassportModel &newPassportsModel)
{
  mPassportsModel.reset(new ChromatogramPassportModel(newPassportsModel));
}

void MarkupStepController::setExternalPassportsModel(QSharedPointer<ChromatogramPassportModel> newPassportsModel)
{
  mPassportsModel = newPassportsModel;
}

int MarkupStepController::currentStepIndex() const
{
  return mCurrentStepIndex;
}

bool MarkupStepController::extractStep(Steps step, MarkupModelPtr &retModel, StepInfo &retStepInfo)
{
  if(mStepsInfo[mCurrentStepIndex].step == step)
    return false;
  int i = 0;
  for(i = 0; i < mStepsInfo.size(); i++)
  {
    auto& info = mStepsInfo[i];
    if(info.step == step)
    {
      retStepInfo = info;
      break;
    }
  }
  if(i >= mStepsInfo.size())
    return false;

  if(mStepModels.contains(step))
    retModel = mStepModels[step];
  else
    retModel.reset(nullptr);

  mStepModels.remove(step);
  mStepsInfo.removeAt(i);
  emit stepsChanged();
  return true;
}

bool MarkupStepController::insertStep(Steps step, MarkupModelPtr retModel, const StepInfo &retStepInfo)
{
  if(mStepsInfo[mCurrentStepIndex].step == step)
    return false;
  int i = 0;
  for(i = 0; i < mStepsInfo.size(); i++)
  {
    if(mStepsInfo[i].step == step)
      break;
  }
  if(i < mStepsInfo.size())
    return false;
  auto iterToInsert = std::lower_bound(mStepsInfo.begin(), mStepsInfo.end(), step, [](const StepInfo& elem, const Steps val)->bool
  {
    return elem.step < val;
  });
  mStepsInfo.insert(iterToInsert, retStepInfo);
  mStepModels[step] = retModel;
  emit stepsChanged();
  return true;
}

void MarkupStepController::setupSteps()
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

  {
    StepInfo step3;
    step3.stepName = tr("Reference intermarkers markup");
    step3.toolTip = tr(
"Working with the reference:\n\
- Marking of intermarker peaks\n\
- Markers table available");
//"Работа с эталоном:\n\
//- Разметка межмаркерных пиков\n\
//- Доступна таблица маркеров";
    step3.stepNumber = 3;
    step3.slaveInteractions = {
      SISExportAsImage
    };
    step3.masterInteractions = {
      SIMAddRemoveBaseLinePoint    |
      SIMBaseLinePointInteractions |
      SIMAddRemoveInterMarker      |
      SIMAddRemovePeak             |
      SIMSetUnsetInterMarker       |
      SIMExportAsImage             |
      SIMModifyPeaks               |
      SIMModifyInterMarkers        |
      SIMEnabledStyle        |
      SIMSetupParameters
    };
    step3.guiInteractions = {
      SIGImportChromatogramms |
      SIGViewPassport         |
      SIGMarkersTable         |
      SIGNextStep             |
      SIGSaveMarkup |
      SIGSaveMarkupAs |
      SIGPrevStep             |
      SIGDigitalDataTable     |
      SIGRemarkupSlaves|
      SIGRemoveSlaveChromatogramm
    };
    step3.step = Step3MasterInterMarkersMarkup;
    mStepsInfo.append(step3);
  }

  {
    StepInfo step4;
    step4.stepName = tr("Additional chromatogramms peaks editing");
    step4.toolTip = tr(
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
    step4.stepNumber = 4;
    step4.slaveInteractions = {
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
    step4.masterInteractions = {
      SIMExportAsImage
    };
    step4.guiInteractions = {
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
    step4.step = Step4SlavePeaksEditing;
    mStepsInfo.append(step4);
  }

  {
    StepInfo step5;
    step5.stepName = tr("Markers transfer");
    step5.toolTip = tr(
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
    step5.stepNumber = 5;
    step5.slaveInteractions = {
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
    step5.masterInteractions = {
      SIMExportAsImage
    };
    step5.guiInteractions = {
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
    step5.step = Step5MarkersTransferToSlave;
    mStepsInfo.append(step5);
  }

  StepInfo step6;
  step6.stepName = tr("Intermarkers transfer");
  step6.toolTip =tr(
"Working with additional\n\
chromatograms:\n\
- Transfer and marking of intermarker peaks\n\
- Numerical data table available");
//"Работа с сопроводительными\n\
//хроматограмми:\n\
//- Перенос и разметка межмаркерных пиков\n\
//- Доступна таблица числовых данных";
  step6.stepNumber = 6;
  step6.slaveInteractions = {
    SISAddRemoveBaseLinePoint    |
    SISBaseLinePointInteractions |
    SISAddRemoveInterMarker      |
    SISAddRemovePeak             |
    SISSetUnsetInterMarker       |
    SISExportAsImage             |
    SISModifyPeaks               |
    SISModifyInterMarkers        |
    SISEnabledStyle       |
    SISSetupParameters
  };
  step6.masterInteractions = {
    SIMExportAsImage
  };
  step6.guiInteractions = {
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
    SIGRemoveSlaveChromatogramm
  };
  step6.step = Step6InterMarkersTransferToSlave;
  mStepsInfo.append(step6);

  StepInfo step7;
  step7.stepName = tr("Comlpetion");
  step7.toolTip = tr(
"Marking complete.\n\
You can come back if necessary\n\
at any of the steps and edit it");
//"Разметка завершена.\n\
//При необходимости можно вернуться\n\
//на любой из шагов и отредактировать ее";
  step7.stepNumber = 7;
  step7.slaveInteractions = {
    SISExportAsImage
  };
  step7.masterInteractions = {
    SIMExportAsImage
  };
  step7.guiInteractions = {
    SIGViewPassport         |
    SIGSaveMaster           |
    SIGMatrix               |
    SIGSaveMarkupAs |
    SIGEditNameAndComment   |
    SIGEditCopy             |
    SIGPrevStep             |
    SIGSwitchToTable
  };
  step7.step = Step7Complete;
  mStepsInfo.append(step7);
}

void MarkupStepController::setCurrentStep(Steps newStep)
{
  auto iter = std::find_if(mStepsInfo.begin(), mStepsInfo.end(), [newStep](const StepInfo& info)->bool
  {
    return info.step == newStep;
  });
  if(iter == mStepsInfo.end())
  {
    Notification::NotifyError(tr("No such step in list"));
    return;
  }
  for(auto& chroma : mStepModels[iter->step]->getChromatogrammsListModels())
  {
    auto bline = chroma->getBaseLine();
    if(bline)
      bline->setBaseLine(chroma->getChromaSettings().BLineSmoothLvl);
  }
  mCurrentStepIndex = std::distance(mStepsInfo.begin(), iter);

  if(auto model = mStepModels.value(iter->step, nullptr))
    model->updateTitlesDisplayMode();
  emit ModelChanged(mStepModels[iter->step], mStepsInfo[mCurrentStepIndex]);
}

void MarkupStepController::loadDataForStep(Steps newStep, MarkupModelPtr modelPlaceholder, const QVariant& details)
{
  auto previousStepModel = mStepModels[mStepsInfo[mCurrentStepIndex].step];

  switch(newStep)
  {
  case Step1PeakDetection:
  {
//    qDebug().noquote() << QJsonDocument(dataToServer).toJson();
    loadNextStepModel(newStep,
                      mStepsInfo[mCurrentStepIndex].step,
                      MarkupDataProcessing::Reservoir::formDataForStep1(previousStepModel),
                      API::REST::DetectPeaks,
                      qOverload<Steps, QJsonObject, MarkupModelPtr, QUuid&>(MarkupDataProcessing::Reservoir::parseResponseForStep1),
                      modelPlaceholder);
  }
    break;
  case Step2MasterMarkersMarkup:
  {
    QJsonObject dataToServer = MarkupDataProcessing::Reservoir::formDataForSteps234(previousStepModel);
    loadNextStepModel(newStep,
                      mStepsInfo[mCurrentStepIndex].step,
                      dataToServer,
                      API::REST::MarkupStep2,
                      qOverload<Steps, QJsonObject, MarkupModelPtr, QUuid&>(MarkupDataProcessing::Reservoir::parseResponseForSteps234),
                      modelPlaceholder);
  }
    break;
  case Step3MasterInterMarkersMarkup:
  {
    QJsonObject dataToServer = MarkupDataProcessing::Reservoir::formDataForSteps234(previousStepModel);
//    qDebug().noquote() << QJsonDocument(dataToServer).toJson();
    loadNextStepModel(newStep,
                      mStepsInfo[mCurrentStepIndex].step,
                      dataToServer,
                      API::REST::DetectIntermarkers,
                      qOverload<Steps, QJsonObject, MarkupModelPtr, QUuid&>(MarkupDataProcessing::Reservoir::parseResponseForSteps234),
                      modelPlaceholder);
  }
    break;
  case Step4SlavePeaksEditing:
  {
    QJsonObject dataToServer = MarkupDataProcessing::Reservoir::formDataForSteps234(previousStepModel);
    loadNextStepModel(newStep,
                      mStepsInfo[mCurrentStepIndex].step,
                      dataToServer,
                      API::REST::TransferPrepare,
                      qOverload<Steps, QJsonObject, MarkupModelPtr, QUuid&>(MarkupDataProcessing::Reservoir::parseResponseForSteps234),
                      modelPlaceholder);
  }
    break;
  case Step5MarkersTransferToSlave:
  {

    //qDebug().noquote() << QJsonDocument(dataToServer).toJson();
    loadNextStepModel(newStep,
                      mStepsInfo[mCurrentStepIndex].step,
                      MarkupDataProcessing::Reservoir::formDataForStep5(previousStepModel, details),
                      API::REST::TransferMarkers,
                      qOverload<Steps, QJsonObject, MarkupModelPtr, QUuid&>(MarkupDataProcessing::Reservoir::parseResponseForStep567),
                      modelPlaceholder);
  }
    break;
  case Step6InterMarkersTransferToSlave:
  {
    if(!previousStepModel->hasMaster())
    {
      Notification::NotifyError(tr("No reference plot"));
      return;
    }


    loadNextStepModel(newStep,
                      mStepsInfo[mCurrentStepIndex].step,
                      MarkupDataProcessing::Reservoir::formDataForStep6(previousStepModel, details),
                      API::REST::IntermarkersTransfer,
                      qOverload<Steps, QJsonObject, MarkupModelPtr, QUuid&>(MarkupDataProcessing::Reservoir::parseResponseForStep567),
                      modelPlaceholder);

  }
    break;
  case Step7Complete:
  {

    loadNextStepModel(newStep,
                      mStepsInfo[mCurrentStepIndex].step,
                      MarkupDataProcessing::Reservoir::formDataForStep7(previousStepModel),
                      API::REST::TransferCheck,
                      qOverload<Steps, QJsonObject, MarkupModelPtr, QUuid&>(MarkupDataProcessing::Reservoir::parseResponseForStep567),
                      modelPlaceholder,
                      [this, details]()
    {
      SaveMarkup(details);
    });
  }
    break;
  default:
    break;
  }
}

void MarkupStepController::applyStepForModel(Steps newStep, MarkupModelPtr model, MarkupModelPtr modelPlaceholder, const QVariant &details, std::function<void (bool)> postLoadingAction)
{
  auto previousStepModel = model;

  switch(newStep)
  {
  case Step1PeakDetection:
  {
    loadStepForModel(previousStepModel,
                     newStep,
                     MarkupDataProcessing::Reservoir::formDataForStep1(previousStepModel),
                     API::REST::DetectPeaks,
                     qOverload<Steps, QJsonObject, MarkupModelPtr>(MarkupDataProcessing::Reservoir::parseResponseForStep1),
                     modelPlaceholder,
                     postLoadingAction);
  }
    break;
  case Step2MasterMarkersMarkup:
  {
    QJsonObject dataToServer = MarkupDataProcessing::Reservoir::formDataForSteps234(previousStepModel);
    loadStepForModel(previousStepModel,
                     newStep,
                     dataToServer,
                     API::REST::MarkupStep2,
                     qOverload<Steps, QJsonObject, MarkupModelPtr>(MarkupDataProcessing::Reservoir::parseResponseForSteps234),
                     modelPlaceholder,
                     postLoadingAction);
  }
    break;
  case Step3MasterInterMarkersMarkup:
  {
    QJsonObject dataToServer = MarkupDataProcessing::Reservoir::formDataForSteps234(previousStepModel);
//    qDebug().noquote() << QJsonDocument(dataToServer).toJson();
    loadStepForModel(previousStepModel,
                     newStep,
                     dataToServer,
                     API::REST::DetectIntermarkers,
                     qOverload<Steps, QJsonObject, MarkupModelPtr>(MarkupDataProcessing::Reservoir::parseResponseForSteps234),
                     modelPlaceholder,
                     postLoadingAction);
  }
    break;
  case Step4SlavePeaksEditing:
  {
    QJsonObject dataToServer = MarkupDataProcessing::Reservoir::formDataForSteps234(previousStepModel);
    loadStepForModel(previousStepModel,
                     newStep,
                     dataToServer,
                     API::REST::TransferPrepare,
                     qOverload<Steps, QJsonObject, MarkupModelPtr>(MarkupDataProcessing::Reservoir::parseResponseForSteps234),
                     modelPlaceholder,
                     postLoadingAction);
  }
    break;
  case Step5MarkersTransferToSlave:
  {
    QJsonObject dataToServer = MarkupDataProcessing::Reservoir::formDataForStep5(previousStepModel, details);
    //qDebug().noquote() << QJsonDocument(dataToServer).toJson();
    loadStepForModel(previousStepModel,
                     newStep,
                     dataToServer,
                     API::REST::TransferMarkers,
                     qOverload<Steps, QJsonObject, MarkupModelPtr>(MarkupDataProcessing::Reservoir::parseResponseForStep567),
                     modelPlaceholder,
                     postLoadingAction);
  }
    break;
  case Step6InterMarkersTransferToSlave:
  {
    if(!previousStepModel->hasMaster())
    {
      Notification::NotifyError(tr("No reference plot"));
      return;
    }
    QJsonObject dataToServer = MarkupDataProcessing::Reservoir::formDataForStep6(previousStepModel, details);
    loadStepForModel(previousStepModel,
                     newStep,
                     dataToServer,
                     API::REST::IntermarkersTransfer,
                     qOverload<Steps, QJsonObject, MarkupModelPtr>(MarkupDataProcessing::Reservoir::parseResponseForStep567),
                     modelPlaceholder,
                     postLoadingAction);

  }
    break;
  case Step7Complete:
  {
    QJsonObject dataToServer = MarkupDataProcessing::Reservoir::formDataForStep7(previousStepModel);
    loadStepForModel(previousStepModel,
                     newStep,
                     dataToServer,
                     API::REST::TransferCheck,
                     qOverload<Steps, QJsonObject, MarkupModelPtr>(MarkupDataProcessing::Reservoir::parseResponseForStep567),
                     modelPlaceholder);
  }
    break;
  default:
    break;
  }
}

void MarkupStepController::setModelsPackAndStep(Steps newStep, const QMap<Steps, MarkupModelPtr> &newModelsPack)
{
  mStepModels = newModelsPack;
  setCurrentStep(newStep);
}

QUndoStack *MarkupStepController::getUndoStack()
{
  return mUndoStack;
}

QString MarkupStepController::getMZ()
{
    return Core::GenesisContextRoot::Get()->GetContextMarkup()->GetData("m_z").toString();
}

QStringList MarkupStepController::getFileTypes()
{
  return Core::GenesisContextRoot::Get()->GetContextMarkup()->GetData("fileTypes").toStringList();
}

void MarkupStepController::merge(const MarkupStepController &otherController)
{
  MarkupModelPtr mergedModel(new MarkupDataModel(*this, *this->getCurrentModel()));
  auto oldModel = getCurrentModel();
  auto newModel = otherController.getCurrentModel();
  for(auto& id : newModel->getChromatogrammsIdList())
  {
    if(mergedModel->hasChromatogram(id))
      mergedModel->swapChromatogramm(id, newModel->takeChromatogramm(id));
    else
      mergedModel->addChromatogramm(id, newModel->takeChromatogramm(id));
  }
  auto cmd = new SetStepCommand(this,
                                getCurrentModel(),
                                getCurrentStepInfo().step,
                                getCurrentStepInfo().step,
                                mergedModel);
  pushCommand(cmd);
}

void MarkupStepController::importReference(int newRefId, int newChromaId)
{
  //function of loading reference markup and applying it by command (2-nd step of import)
  auto loadChroma = [this](int newRefId, int newChromaId)
  {
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
    API::REST::GetReference(newRefId,
      [newChromaId, newRefId, overlayId, this](QNetworkReply*, QJsonDocument jreferencedoc)
      {
        auto jref = jreferencedoc.object();
        auto error = jref["error"].toBool();
        if(error)
        {
          Notification::NotifyError(jref["msg"].toString(), tr("Can't load reference data by server internal"));
        }
        auto result = jref["result"].toObject();
        auto referenceData = result["etalon_data"].toObject();
        ChromatogrammModelPtr newReferenceChroma(new ChromatogramDataModel(newChromaId));

        newReferenceChroma->load(referenceData);

        //curve SHOULD BE loaded in memory.
        auto curve = MarkupDataModel::getCurve(newChromaId);
        auto curveUid = newReferenceChroma->addNewEntity(TypeCurve);
        newReferenceChroma->setEntityData(curveUid, CurveData, QVariant::fromValue(curve));
        newReferenceChroma->setChromatogramValue(ChromatogrammReferenceId, newRefId);

        MarkupModelPtr step4Model(new MarkupDataModel(*this));
        MarkupModelPtr step0Model(new MarkupDataModel(*this, *stepModels()[Step0DataLoad]));
        ChromatogrammModelPtr newMasterChroma(new ChromatogramDataModel(*newReferenceChroma));
        newMasterChroma->clearEntities(TypePeak);

        if(step0Model->hasChromatogram(newMasterChroma->id()))
          step0Model->swapChromatogramm(newMasterChroma->id(), newMasterChroma);
        else
          step0Model->addChromatogramm(newMasterChroma->id(), newMasterChroma);
        step0Model->setMaster(newMasterChroma->id());
        auto masterSettings = newMasterChroma->getChromaSettings();
        for(auto& chroma : step0Model->getChromatogrammsListModels())
        {
          if(chroma->id() == step0Model->getMaster()->id())
            continue;
          chroma->clearEntities(TypeInterval);
          chroma->setChromaSettings(masterSettings);
          for(auto& interval : step0Model->getMaster()->getEntities(TypeInterval))
          {
            chroma->addNewEntity(TypeInterval, *interval, interval->getUid());
          }
        }

        applyStepForModel(Step1PeakDetection, step0Model, step4Model, QVariant(),
                                           [step0Model, step4Model, newReferenceChroma, this](bool error)
                                           {
                                             if(error)
                                             {
                                               Notification::NotifyError(tr("Failed to detect peaks on model"));
                                               return;
                                             }
                                             else
                                             {
                                               if(auto master = step4Model->getMaster())
                                               {
                                                 for(auto& marker : master->getEntities(
                                                        [](AbstractEntityDataModel::ConstDataPtr ent)->bool
                                                        {
                                                          if(ent->getType() == TypePeak && ent->hasDataAndItsValid(PeakType))
                                                          {
                                                            auto peakType = ent->getData(PeakType).toInt();
                                                            return peakType == PTMarker || peakType == PTInterMarker;
                                                          }
                                                          return false;
                                                        })
                                                      )
                                                 {
                                                   master->setEntityData(marker->getUid(), PeakType, PTPeak);
                                                 }
                                               }
                                               step4Model->swapChromatogramm(newReferenceChroma->id(), newReferenceChroma);

                                               //generation of [0|1]-3 step markup models
                                               auto modelsMap = generateModelsDownFromStep4(step4Model, step0Model);

                                               auto cmd = new ReferenceImportCommand(this, modelsMap);
                                               pushCommand(cmd);
                                             }
                                           });
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      },
      [overlayId](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Can't load reference data"), err);
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      });
  };

  //function of loading passport data and curve data for new referense (1-st step of import), it calls 2-nd step by itself
  auto loadFileData = [this, loadChroma](int newRefId, int newChromaId)
  {
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
    API::REST::GetFileChromatogramAndPassport(newChromaId,
      [this, newRefId, overlayId, newChromaId, loadChroma](QNetworkReply*, QJsonDocument jchromadoc)
      {
        //load passport and curve
        auto jchroma = jchromadoc.object();
        if(!jchroma.contains("file_info"))
        {
          GenesisWindow::Get()->RemoveOverlay(overlayId);
          return;
        }
        auto jfileInfo = jchroma["file_info"].toObject();

        //if that curve alredy loaded, skip it
        if(!MarkupDataModel::hasCurve(newChromaId))
        {
          auto jx = jchroma["x"].toArray();
          auto jy = jchroma["y"].toArray();
          QPair<QVector<double>, QVector<double>> curveData;
          auto& keys = curveData.first;
          auto& values = curveData.second;
          for (int i = 0; i < jx.size() && i < jy.size(); ++i)
          {
            keys << jx[i].toDouble();
            values << jy[i].toDouble();
          }
          CurveDataModelPtr curve;
          curve.reset(new CurveDataModel(curveData.first, curveData.second));
          MarkupDataModel::setCurve(newChromaId, curve);
        }

        auto passportModel = passportsModelPtr();
        //if this passport alredy loaded, skip it
        if(!passportModel->getPassport(newChromaId))
        {
          int id = jfileInfo["project_file_id"].toInt();
          QStringList filePath = jfileInfo["filename"].toString().split("/");
          QString title = jfileInfo["title"].toString();
          QString fileName;
          if (!filePath.empty())
            fileName = filePath.last();
          if(!title.isEmpty())
            fileName = title + " (" + fileName + ")";
          QString field = jfileInfo["field_title"].toString();
          QString layer = jfileInfo["layer_title"].toString();
          QString sampleDate = jfileInfo["sample_date_time"].toString();
          QString well = jfileInfo["well_title"].toString();

          passportModel->addNewPassport(newChromaId);
          passportModel->setPassportData(newChromaId, PassportFilename, fileName);
          passportModel->setPassportData(newChromaId, PassportTitle, title);
          passportModel->setPassportData(newChromaId, PassportFileId, id);
          passportModel->setPassportData(newChromaId, PassportField, field);
          passportModel->setPassportData(newChromaId, PassportDate, sampleDate);
          passportModel->setPassportData(newChromaId, PassportLayer, layer);
          passportModel->setPassportData(newChromaId, PassportWell, well);
        }
        //call 2-nd step of import
        loadChroma(newRefId, newChromaId);

        GenesisWindow::Get()->RemoveOverlay(overlayId);
      },
      [overlayId](QNetworkReply*, QNetworkReply::NetworkError err)
      {

        GenesisWindow::Get()->RemoveOverlay(overlayId);
        Notification::NotifyError(tr("Can't load chromatogramm"), err);
      });
  };

  //if that curve and passport alredy loaded, skip 1-st step of import
  if(MarkupDataModel::hasCurve(newChromaId) &&
      passportsModelPtr()->getPassport(newChromaId))
    loadChroma(newRefId, newChromaId);//call directly 2-nd import step
  else //else call 1-st import step
    loadFileData(newRefId, newChromaId);
}


void MarkupStepController::load(const QJsonObject &data)
{
  mStepModels.clear();
  QJsonObject root = data;
  QJsonObject jsteps = root["steps"].toObject();
  auto stepsMetaEnum = QMetaEnum::fromType<Steps>();
  for(auto& key : jsteps.keys())
  {
    MarkupModelPtr newModel(new MarkupDataModel(*this, nullptr));
    newModel->load(jsteps[key].toObject());
    mStepModels[(Steps)stepsMetaEnum.keyToValue(key.toStdString().c_str())] = newModel;
//    qDebug().noquote() << QJsonDocument(newModel->print()).toJson();
  }
}

void MarkupStepController::createNewMarkup(int markupId)
{
  mPassportsModel->clearData();
  mStepModels.clear();
  //создаем модель нулевого шага разметки
  mStepModels[Step0DataLoad] = MarkupModelPtr(new MarkupDataModel(*this));
  //загружаем список хроматограмм с сервера
  API::REST::Tables::GetMarkupFiles(markupId,
  [this](QNetworkReply*, QVariantMap map) {
//    qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(map)).toJson();
    auto fileList = map["children"].toList();
    //на основе этого списка создаем список хроматограмм
    for (int i = 0; i < fileList.size(); ++i)
    {
      auto fileData = fileList[i].toMap();
      int id = fileData["project_file_id"].toInt();
      QStringList filePath = fileData["filename"].toString().split("/");
      QString title = fileData["title"].toString();
      QString fileName;
      if (!filePath.empty())
        fileName = filePath.last();
      if(!title.isEmpty())
        fileName = title + " (" + fileName + ")";
      QString field = fileData["field_title"].toString();
      QString layer = fileData["layer_title"].toString();
      QString sampleDate = fileData["sample_date_time"].toString();
      QString well = fileData["well_title"].toString();

      mPassportsModel->addNewPassport(id);
      mPassportsModel->setPassportData(id, PassportFilename, fileName);
      mPassportsModel->setPassportData(id, PassportTitle, title);
      mPassportsModel->setPassportData(id, PassportFileId, id);
      mPassportsModel->setPassportData(id, PassportField, field);
      mPassportsModel->setPassportData(id, PassportDate, sampleDate);
      mPassportsModel->setPassportData(id, PassportLayer, layer);
      mPassportsModel->setPassportData(id, PassportWell, well);

      //по мере загрузки данных создаем для них модели хроматограм, и засовываем в модель нулевого шага
      mStepModels[Step0DataLoad]->addChromatogramm(id);
    }

    //грузим данные всех хроматограмм
    auto idList = mPassportsModel->getIdList();
    GenesisWindow::Get()->UpdateOverlay(mLoadingOverlayId, tr("%n chromatogramm(s) remaining", "", idList.size()));
    for(auto& id : idList)
    {
      //Грузим данные хроматограммы
      mUploadCounter++;
      API::REST::GetProjectFileCromotogramData(id,
      [this, id](QNetworkReply*, QJsonDocument doc)
      {
        auto data = doc.object();
        auto jbaseLines = data["baselines"].toObject();
        auto xArr = data["x"].toArray();
        auto yArr = data["y"].toArray();
//          auto ion_mass = data["ions"].toArray();

        if (!xArr.isEmpty() && !yArr.isEmpty())
        {
          QPair<QVector<double>, QVector<double>> curveData;
          auto& keys = curveData.first;
          auto& values = curveData.second;
          for (int i = 0; i < xArr.size() && i < yArr.size(); ++i)
          {
            keys << xArr[i].toDouble();
            values << yArr[i].toDouble();
          }
          auto chroma = mStepModels[Step0DataLoad]->getChromatogramm(id);
          CurveDataModelPtr curve;
          if(MarkupDataModel::hasCurve(id))
          {
            curve = MarkupDataModel::getCurve(id);
          }
          else
          {
            curve.reset(new CurveDataModel(curveData.first, curveData.second));
            MarkupDataModel::setCurve(id, curve);
          }
          auto curveUid = chroma->addNewEntity(TypeCurve);
          chroma->setEntityData(curveUid, CurveData, QVariant::fromValue(curve));
          chroma->parseJBaseLines(jbaseLines, chroma->getChromatogramValue(ChromatogrammSettingsBaseLineSmoothLvl).toInt());


//          for (int i = 0; i < ion_mass.size() && i < ion_mass.size(); ++i)
//          {
//            AllIons << ion_mass[i].toInt();
//          }
        }
        mUploadCounter--;
        GenesisWindow::Get()->UpdateOverlay(mLoadingOverlayId, tr("%n chromatogramm(s) remaining", "", mUploadCounter));
        if(mUploadCounter == 0)
        {
          GenesisWindow::Get()->UpdateOverlay(mLoadingOverlayId, tr("setting up controllers"));
          setCurrentStep(GenesisMarkup::Step0DataLoad);
          GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
          mLoadingOverlayId = QUuid();
          emit markupLoadingFinished();
        }
      },
      [this](QNetworkReply*, QNetworkReply::NetworkError)
      {
        mUploadCounter--;
        if(mUploadCounter == 0)
        {
          setCurrentStep(GenesisMarkup::Step0DataLoad);
          qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
          GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
          mLoadingOverlayId = QUuid();
          emit markupLoadingFinished();
        }

      });
    }
  },
  [this](QNetworkReply*, QNetworkReply::NetworkError err) {
    qDebug() << err;
    GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
    mLoadingOverlayId = QUuid();
    emit markupLoadingFinished();
  });
}

void MarkupStepController::createNewMarkupForImport(const QList<int> &chromaIds, const ChromaSettings& initialSettings, MarkupModelPtr oldModel)
{
  mPassportsModel->clearData();
  mStepModels.clear();
  //создаем модель нулевого шага разметки
  mStepModels[Step0DataLoad] = MarkupModelPtr(new MarkupDataModel(*this));
  //загружаем список хроматограмм с сервера
  API::REST::Tables::GetProjectFiles(GenesisContextRoot::Get()->ProjectId(),
  [oldModel, initialSettings, chromaIds, this](QNetworkReply*, QVariantMap map) {
//    qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(map)).toJson();
    auto fileList = map["children"].toList();
    //на основе этого списка создаем список хроматограмм
    for (int i = 0; i < fileList.size(); ++i)
    {
      auto fileData = fileList[i].toMap();
      int id = fileData["project_file_id"].toInt();
      if(!chromaIds.contains(id))
        continue;
      QStringList filePath = fileData["filename"].toString().split("/");
      QString title = fileData["title"].toString();
      QString fileName;
      if (!filePath.empty())
        fileName = filePath.last();
      if(!title.isEmpty())
        fileName = title + " (" + fileName + ")";
      QString field = fileData["field_title"].toString();
      QString layer = fileData["layer_title"].toString();
      QString sampleDate = fileData["sample_date_time"].toString();
      QString well = fileData["well_title"].toString();

      mPassportsModel->addNewPassport(id);
      mPassportsModel->setPassportData(id, PassportFilename, fileName);
      mPassportsModel->setPassportData(id, PassportTitle, title);
      mPassportsModel->setPassportData(id, PassportFileId, id);
      mPassportsModel->setPassportData(id, PassportField, field);
      mPassportsModel->setPassportData(id, PassportDate, sampleDate);
      mPassportsModel->setPassportData(id, PassportLayer, layer);
      mPassportsModel->setPassportData(id, PassportWell, well);

      //по мере загрузки данных создаем для них модели хроматограм, и засовываем в модель нулевого шага
      mStepModels[Step0DataLoad]->addChromatogramm(id);
    }
    if(mPassportsModel->getIdList().isEmpty())
    {
      setCurrentStep(GenesisMarkup::Step0DataLoad);
      Q_ASSERT(false);//wrong chromatogramms id list
      return;
    }
    //грузим данные всех хроматограмм
    for(auto& id : chromaIds)
    {
      //Грузим данные хроматограммы
      mUploadCounter++;
      API::REST::GetProjectFileCromotogramData(id,
      [this, id, oldModel, initialSettings](QNetworkReply*, QJsonDocument doc)
      {
        auto data = doc.object();
        auto jbaseLines = data["baselines"].toObject();
        auto xArr = data["x"].toArray();
        auto yArr = data["y"].toArray();

        if (!xArr.isEmpty() && !yArr.isEmpty())
        {
          QPair<QVector<double>, QVector<double>> curveData;
          auto& keys = curveData.first;
          auto& values = curveData.second;
          for (int i = 0; i < xArr.size() && i < yArr.size(); ++i)
          {
            keys << xArr[i].toDouble();
            values << yArr[i].toDouble();
          }

          auto chroma = mStepModels[Step0DataLoad]->getChromatogramm(id);
          CurveDataModelPtr curve;
          if(MarkupDataModel::hasCurve(id))
          {
            curve = MarkupDataModel::getCurve(id);
          }
          else
          {
            curve.reset(new CurveDataModel(curveData.first, curveData.second));
            MarkupDataModel::setCurve(id, curve);
          }
          auto curveUid = chroma->addNewEntity(TypeCurve);
          chroma->setEntityData(curveUid, CurveData, QVariant::fromValue(curve));
          chroma->parseJBaseLines(jbaseLines, chroma->getChromatogramValue(ChromatogrammSettingsBaseLineSmoothLvl).toInt());
        }
        mUploadCounter--;
        if(mUploadCounter == 0)
          setupImportModel(mStepModels[Step0DataLoad], oldModel, initialSettings);
      },
      [this, oldModel, initialSettings](QNetworkReply*, QNetworkReply::NetworkError)
      {
        mUploadCounter--;
        if(mUploadCounter == 0)
        setupImportModel(mStepModels[Step0DataLoad], oldModel, initialSettings);
      });
    }
  },
  [](QNetworkReply*, QNetworkReply::NetworkError err) {
      Notification::NotifyError(tr("Network error"), err);
  });
}

void MarkupStepController::loadExistedMarkup(QJsonObject data, Steps currentStep)
{
  load(data);

  //no passport data and curves data yet, but curves uids should be loaded
  mPassportsModel->clearData();
  //загружаем список хроматограмм с сервера
  API::REST::Tables::GetMarkupFiles(MARKUP_ID,
  [this, currentStep](QNetworkReply*, QVariantMap map) {
//    qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(map)).toJson();
    auto fileList = map["children"].toList();
    //на основе этого списка создаем список хроматограмм
    for (int i = 0; i < fileList.size(); ++i)
    {
      auto fileData = fileList[i].toMap();
      int id = fileData["project_file_id"].toInt();
      QStringList filePath = fileData["filename"].toString().split("/");
      QString title = fileData["title"].toString();
      QString fileName;
      if (!filePath.empty())
        fileName = filePath.last();
      if(!title.isEmpty())
        fileName = title + " (" + fileName + ")";
      QString field = fileData["field_title"].toString();
      QString layer = fileData["layer_title"].toString();
      QString sampleDate = fileData["sample_date_time"].toString();
      QString well = fileData["well_title"].toString();

      mPassportsModel->addNewPassport(id);
      mPassportsModel->setPassportData(id, PassportFilename, fileName);
      mPassportsModel->setPassportData(id, PassportTitle, title);
      mPassportsModel->setPassportData(id, PassportFileId, id);
      mPassportsModel->setPassportData(id, PassportField, field);
      mPassportsModel->setPassportData(id, PassportDate, sampleDate);
      mPassportsModel->setPassportData(id, PassportLayer, layer);
      mPassportsModel->setPassportData(id, PassportWell, well);
    }

    //грузим данные всех хроматограмм
    auto idList = mPassportsModel->getIdList();
    GenesisWindow::Get()->UpdateOverlay(mLoadingOverlayId, tr("%n chromatogramm(s) remaining", "", idList.size()));
    for(auto& id : idList)
    {
      //Грузим данные хроматограммы
      mUploadCounter++;
      API::REST::GetProjectFileCromotogramData(id,
      [this, id, currentStep](QNetworkReply*, QJsonDocument doc)
      {
        auto data = doc.object();
        auto xArr = data["x"].toArray();
        auto yArr = data["y"].toArray();
//          auto ion_mass = data["ions"].toArray();

        if (!xArr.isEmpty() && !yArr.isEmpty())
        {
          QPair<QVector<double>, QVector<double>> curveData;
          auto& keys = curveData.first;
          auto& values = curveData.second;
          for (int i = 0; i < xArr.size() && i < yArr.size(); ++i)
          {
            keys << xArr[i].toDouble();
            values << yArr[i].toDouble();
          }
          CurveDataModelPtr curve;
          if(MarkupDataModel::hasCurve(id))
          {
            curve = MarkupDataModel::getCurve(id);
          }
          else
          {
            curve.reset(new CurveDataModel(curveData.first, curveData.second));
            MarkupDataModel::setCurve(id, curve);
          }
          for(auto& model : mStepModels)
          {
            auto chroma = model->getChromatogramm(id);
            if(!chroma)
              continue;
            auto curveUid = chroma->addNewEntity(TypeCurve);
            chroma->setEntityData(curveUid, CurveData, QVariant::fromValue(curve));
          }

//          for (int i = 0; i < ion_mass.size() && i < ion_mass.size(); ++i)
//          {
//            AllIons << ion_mass[i].toInt();
//          }
        }
        mUploadCounter--;
        GenesisWindow::Get()->UpdateOverlay(mLoadingOverlayId, tr("%n chromatogramm(s) remaining", "", mUploadCounter));
        if(mUploadCounter == 0)
        {
          GenesisWindow::Get()->UpdateOverlay(mLoadingOverlayId, tr("setting up controllers"));
          setCurrentStep(currentStep);
          GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
          mLoadingOverlayId = QUuid();
          emit markupLoadingFinished();
        }
      },
      [this, currentStep](QNetworkReply*, QNetworkReply::NetworkError)
      {
        mUploadCounter--;
        if(mUploadCounter == 0)
        {
          setCurrentStep(currentStep);
          GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
          mLoadingOverlayId = QUuid();
          emit markupLoadingFinished();
        }
      });
    }
  },
  [this](QNetworkReply*, QNetworkReply::NetworkError err) {
    qDebug() << err;
    GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
    mLoadingOverlayId = QUuid();
    emit markupLoadingFinished();
  });
}

QJsonObject MarkupStepController::save() const
{
  QJsonObject root;
  QJsonObject jsteps;
  auto stepsMetaEnum = QMetaEnum::fromType<Steps>();
  for(auto& key : mStepModels.keys())
  {
    jsteps[stepsMetaEnum.valueToKey(key)] = mStepModels[key]->save();
  }
  root["steps"] = jsteps;
  root["passports"] = mPassportsModel->save();
  return root;
}

void MarkupStepController::loadNextStepModel(Steps newStep,
                                             Steps currentStep,
                                             const QJsonObject& dataToServer,
                                             std::function<void (int, const QJsonObject&, API::REST::ReplyHandleFunc, API::REST::ReplyErrorFunc)> apiRequest,
                                             std::function<void (Steps, QJsonObject, MarkupModelPtr, QUuid &)> parseResponse,
                                             MarkupModelPtr modelPlaceholder,
                                             std::function<void ()> postLoadingAction)
{
  if(dataToServer.isEmpty())
  {
    if(!mLoadingOverlayId.isNull())
    {
      GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
      mLoadingOverlayId = QUuid();
    }

    //deferred call
    QMetaObject::invokeMethod(this, &MarkupStepController::undoAndRemoveLastCommand, Qt::QueuedConnection);
    return;
  }
  if(mLoadingOverlayId.isNull())
    mLoadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Next step calculation"));
//  qDebug().noquote() << QJsonDocument(dataToServer).toJson();

  apiRequest(MARKUP_ID,
             dataToServer,
             [this, newStep, parseResponse, currentStep, modelPlaceholder, postLoadingAction](QNetworkReply*, QJsonDocument doc)
  {
//    qDebug().noquote() << doc.toJson();

    if(doc.object()["error"].toBool())
    {
      if(!mLoadingOverlayId.isNull())
      {
        GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
        mLoadingOverlayId = QUuid();
      }
      Notification::NotifyError((doc.object()["msg"].toString()), QString("Server error"));

      //deferred call
      QMetaObject::invokeMethod(this, &MarkupStepController::undoAndRemoveLastCommand, Qt::QueuedConnection);
      return;
    }
    ////copy valuable data
    MarkupModelPtr newModel;
    if(modelPlaceholder)
      newModel = modelPlaceholder;
    else
      newModel = MarkupModelPtr(new MarkupDataModel(*this));

    //qDebug() << currentStep;
    auto previousStepModel = mStepModels[currentStep];
    for(auto& id : previousStepModel->getChromatogrammsIdList())
    {
      ////need to move this to the appropriate classes

      //COPY VALUABLE DATA FROM PREV STEP
      if(newStep == Step6Identification)//BIOMARKERS
      {
        ChromatogrammModelPtr chroma(new ChromatogramDataModel(*previousStepModel->getChromatogramm(id)));
        newModel->addChromatogramm(id, chroma);
      }
      //OTHER IS FOR RESERVOIR & same steps on BIOMARKERS
      else if(newStep <= Step5MarkersTransferToSlave)
      {
        if(previousStepModel->getMaster() && previousStepModel->getMaster()->id() == id)
        {
          ChromatogrammModelPtr chroma(new ChromatogramDataModel(id));
          auto prevChroma = previousStepModel->getChromatogramm(id);

          {//copy chroma-settings from prev:
            //const_cast because we not need any preprocessing of this values, or some reaction to they changing
            Q_ASSERT(prevChroma->id() == chroma->id());//just for safety
            chroma->setData(prevChroma->internalDataReference());
          }

          for(auto& curve : prevChroma->getEntities(TypeCurve))
            //(*curve) here - is shared pointer to CurveDataModel, so we just copy it
            //CurveDataModel has signals, so, receivers can subscribe to data changing of curve model
            chroma->addNewEntity(TypeCurve, *curve);

          for(auto& baseLine : prevChroma->getEntities(TypeBaseline))
            //copy whole base line data, there is not so much
            chroma->addNewEntity(TypeBaseline, *baseLine);
          for(auto& interval : prevChroma->getEntities(TypeInterval))
            //copy intervals data, i think, we can have different intervals on different steps
            chroma->addNewEntity(TypeInterval, *interval);
          newModel->addChromatogramm(id, chroma);
        }
        else
        {
          ChromatogrammModelPtr chroma(new ChromatogramDataModel(*previousStepModel->getChromatogramm(id)));
          newModel->addChromatogramm(id, chroma);
        }
      }
      else
      {
//        if(previousStepModel->getMaster() && previousStepModel->getMaster()->id() == id)
        if(!previousStepModel->hasInternalMaster() && previousStepModel->hasMaster() && previousStepModel->getMaster()->id() == id)
        {
          ChromatogrammModelPtr chroma(new ChromatogramDataModel(*previousStepModel->getChromatogramm(id)));
          newModel->addChromatogramm(id, chroma);
        }
        else
        {
          ChromatogrammModelPtr chroma(new ChromatogramDataModel(id));
          auto prevChroma = previousStepModel->getChromatogramm(id);
          {//copy chroma-settings from prev:
            Q_ASSERT(prevChroma->id() == chroma->id());//just for safety
            chroma->setData(prevChroma->internalDataReference());
          }
          for(auto& curve : prevChroma->getEntities(TypeCurve))
            //(*curve) here - is shared pointer to CurveDataModel, so we just copy it
            //CurveDataModel has signals, so, receivers can subscribe to data changing of curve model
            chroma->addNewEntity(TypeCurve, *curve);

          for(auto& baseLine : prevChroma->getEntities(TypeBaseline))
            //copy whole base line data, there is not so much
            chroma->addNewEntity(TypeBaseline, *baseLine);
          for(auto& interval : prevChroma->getEntities(TypeInterval))
            //copy intervals data, i think, we can have different intervals on different steps
            chroma->addNewEntity(TypeInterval, *interval);
          newModel->addChromatogramm(id, chroma);
        }
      }
    }
    newModel->setData(previousStepModel->data());
//    if(previousStepModel->hasMaster())
//      newModel->setMaster(previousStepModel->getMaster()->id());
//    newModel->setData(MarkupValuablePeaksSortedList, previousStepModel->getData(MarkupValuablePeaksSortedList));
    //// end copy
    parseResponse(newStep, doc.object(), newModel, mLoadingOverlayId);
    SetCurrentStepAndSwitchHisModel(newStep, newModel);
    if(postLoadingAction)
      postLoadingAction();
    if(!mLoadingOverlayId.isNull())
    {
      GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
      mLoadingOverlayId = QUuid();
    }
  },
  [this](QNetworkReply*, QNetworkReply::NetworkError e)
  {
    if(!mLoadingOverlayId.isNull())
    {
      GenesisWindow::Get()->RemoveOverlay(mLoadingOverlayId);
      mLoadingOverlayId = QUuid();
    }
    QMetaEnum errMeta = QMetaEnum::fromType<QNetworkReply::NetworkError>();

    //deferred call
    Notification::NotifyError(errMeta.valueToKey(e), tr("Network error"));

    QMetaObject::invokeMethod(this, &MarkupStepController::undoAndRemoveLastCommand, Qt::QueuedConnection);
  });
}

void MarkupStepController::loadStepForModel(MarkupModelPtr model, Steps newStep, const QJsonObject &dataToServer, std::function<void (int, const QJsonObject &, API::REST::ReplyHandleFunc, API::REST::ReplyErrorFunc)> apiRequest, std::function<void (Steps, QJsonObject, MarkupModelPtr)> parseResponse, MarkupModelPtr modelPlaceholder, std::function<void (bool)> postLoadingAction)
{
  {
    if(dataToServer.isEmpty())
    {
      qCritical() << "Markup data to server is empty";
      if(postLoadingAction)
        postLoadingAction(true);
      return;
    }
    QUuid overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
    apiRequest(MARKUP_ID,
               dataToServer,
               [this, newStep, parseResponse, overlayId, modelPlaceholder, postLoadingAction, model](QNetworkReply*, QJsonDocument doc)
    {
      if(doc.object()["error"].toBool())
      {
        qCritical() << "Error markup data response from server";
        if(postLoadingAction)
          postLoadingAction(true);
        GenesisWindow::Get()->RemoveOverlay(overlayId);
        return;
      }
      ////copy valuable data
      MarkupModelPtr newModel;
      if(modelPlaceholder)
        newModel = modelPlaceholder;
      else
        newModel = MarkupModelPtr(new MarkupDataModel(*this));

      auto previousStepModel = model;
      for(auto& id : previousStepModel->getChromatogrammsIdList())
      {
        //COPY VALUABLE DATA FROM PREV STEP
        if(newStep <= Step5MarkersTransferToSlave)
        {
          if(previousStepModel->getMaster() && previousStepModel->getMaster()->id() == id)
          {
            ChromatogrammModelPtr chroma(new ChromatogramDataModel(id));
            auto prevChroma = previousStepModel->getChromatogramm(id);

            {//copy chroma-settings from prev:
              //const_cast because we not need any preprocessing of this values, or some reaction to they changing
              Q_ASSERT(prevChroma->id() == chroma->id());//just for safety
              chroma->setData(prevChroma->internalDataReference());
            }

            for(auto& curve : prevChroma->getEntities(TypeCurve))
              //(*curve) here - is shared pointer to CurveDataModel, so we just copy it
              //CurveDataModel has signals, so, receivers can subscribe to data changing of curve model
              chroma->addNewEntity(TypeCurve, *curve);

            for(auto& baseLine : prevChroma->getEntities(TypeBaseline))
              //copy whole base line data, there is not so much
              chroma->addNewEntity(TypeBaseline, *baseLine);
            for(auto& interval : prevChroma->getEntities(TypeInterval))
              //copy intervals data, i think, we can have different intervals on different steps
              chroma->addNewEntity(TypeInterval, *interval);
            newModel->addChromatogramm(id, chroma);
          }
          else
          {
            ChromatogrammModelPtr chroma(new ChromatogramDataModel(*previousStepModel->getChromatogramm(id)));
            newModel->addChromatogramm(id, chroma);
          }
        }
        else
        {
          if(!previousStepModel->hasInternalMaster() && previousStepModel->hasMaster() && previousStepModel->getMaster()->id() == id)
          {
            ChromatogrammModelPtr chroma(new ChromatogramDataModel(*previousStepModel->getChromatogramm(id)));
            newModel->addChromatogramm(id, chroma);
          }
          else
          {
            ChromatogrammModelPtr chroma(new ChromatogramDataModel(id));
            auto prevChroma = previousStepModel->getChromatogramm(id);
            {//copy chroma-settings from prev:
              Q_ASSERT(prevChroma->id() == chroma->id());//just for safety
              chroma->setData(prevChroma->internalDataReference());
            }
            for(auto& curve : prevChroma->getEntities(TypeCurve))
              //(*curve) here - is shared pointer to CurveDataModel, so we just copy it
              //CurveDataModel has signals, so, receivers can subscribe to data changing of curve model
              chroma->addNewEntity(TypeCurve, *curve);

            for(auto& baseLine : prevChroma->getEntities(TypeBaseline))
              //copy whole base line data, there is not so much
              chroma->addNewEntity(TypeBaseline, *baseLine);
            for(auto& interval : prevChroma->getEntities(TypeInterval))
              //copy intervals data, i think, we can have different intervals on different steps
              chroma->addNewEntity(TypeInterval, *interval);
            newModel->addChromatogramm(id, chroma);
          }
        }
      }
      newModel->setData(previousStepModel->data());
      //// end copy
      parseResponse(newStep, doc.object(), newModel);
      if(postLoadingAction)
        postLoadingAction(false);
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    },
    [overlayId, postLoadingAction](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      Notification::NotifyError(tr("Network error"), e);
      if(postLoadingAction)
        postLoadingAction(true);
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    });
  }
}

void MarkupStepController::saveToFile(QJsonObject object)
{
  QString currentPath =  QDir().currentPath();
  QString filename = QString("%1/%2/Markup_%3.log").arg(currentPath).arg("logs").arg(QDateTime::currentDateTime().toString("yyyy-mm-dd hh-mm-ss"));

  QFile *file = new QFile(filename);
  if (file->open( QIODevice::ReadWrite | QIODevice::Append |  QIODevice::Text))
  {
    file->write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    file->close();
  }
}

void MarkupStepController::setupImportModel(MarkupModelPtr newModel, MarkupModelPtr oldModel, const ChromaSettings &settings)
{
  auto oldMaster = oldModel->getMaster();
  if(oldMaster)
  {
    newModel->setData(MarkupMasterIdRole, oldMaster->id());
    newModel->setData(MarkupExternalMasterPtr, QVariant::fromValue(oldMaster));
  }
  if(settings != ChromaSettings())
  {
    for(auto& id : newModel->getChromatogrammsIdList())
    {
      //silently update chroma settings to initial settings
      auto& chromaData = const_cast<QHash<int, QVariant>&>(newModel->getChromatogramm(id)->internalDataReference());
      chromaData[ChromatogrammSettingsBaseLineSmoothLvl] = settings.BLineSmoothLvl;
      chromaData[ChromatogrammSettingsMinimalPeakHeight] = settings.Min_h;
      chromaData[ChromatogrammSettingsNoisy] = settings.Noisy;
      chromaData[ChromatogrammSettingsMedianFilterWindowSize] = settings.Window_size;
      chromaData[ChromatogrammSettingsSignalToMedianRatio] = settings.Sign_to_med;
      chromaData[ChromatogrammSettingsDegreeOfApproximation] = settings.Doug_peuck;
      chromaData[ChromatogrammSettingsBoundsByMedian] = settings.Med_bounds;
      chromaData[ChromatogrammSettingsCoelution] = settings.Coel;
      chromaData[ChromatogrammSettingsViewSmooth] = settings.View_smooth;
      auto blineEntities = newModel->getChromatogramm(id)->getEntities(TypeBaseline);
      if(!blineEntities.isEmpty())
      {
        auto bline = blineEntities.first();
        if(bline->hasDataAndItsValid(BaseLineData))
        {
          auto blineModelPtr = blineEntities.first()->getData(BaseLineData).value<TBaseLineDataModel>();
          if(blineModelPtr)
            blineModelPtr->setBaseLine(settings.BLineSmoothLvl);
        }
      }
    }
  }
  else
  {
    for(auto& id : newModel->getChromatogrammsIdList())
    {
      //silently update chroma settings to initial settings
      if(!newModel->getChromatogramm(id))
        continue;

      auto blineEntities = newModel->getChromatogramm(id)->getEntities(TypeBaseline);
      if(!blineEntities.isEmpty())
      {
        auto bline = blineEntities.first();
        if(bline->hasDataAndItsValid(BaseLineData))
        {
          auto blineModelPtr = blineEntities.first()->getData(BaseLineData).value<TBaseLineDataModel>();
          if(blineModelPtr)
            blineModelPtr->setBaseLine(newModel->getChromatogramm(id)->getChromaSettings().BLineSmoothLvl);
        }
      }
    }
  }
  if(oldMaster)
  {
    auto intervalEntities = oldMaster->getEntities(TypeInterval);
    for(auto& id : newModel->getChromatogrammsIdList())
    {
      auto chroma = newModel->getChromatogramm(id);
      for(auto& interval : intervalEntities)
      {
        chroma->addNewEntity(TypeInterval, *interval, interval->getUid());
      }
    }
  }
  SetCurrentStepAndSwitchHisModel(Step0DataLoad, newModel, true);
}

void MarkupStepController::undoAndRemoveLastCommand()
{
  auto* cmd = const_cast<QUndoCommand*>(mUndoStack->command(mUndoStack->count()-1));
  cmd->undo();             // must be called explicitly
  cmd->setObsolete(true);
  mUndoStack->undo();
}

QMap<Steps, MarkupModelPtr> MarkupStepController::generateModelsDownFromStep4(MarkupModelPtr step4model, MarkupModelPtr step0model)
{
  MarkupModelPtr step3Model(new MarkupDataModel(*this, *step4model));
  MarkupModelPtr step2Model(new MarkupDataModel(*this, *step4model));

  auto step2Master = step2Model->getMaster();
  for(auto& intermarker : step2Master->getEntities({{PeakType, PeakTypes::PTInterMarker}}))
  {
    step2Master->setEntityData(intermarker->getUid(), PeakType, PTPeak);
  };

  MarkupModelPtr step1Model(new MarkupDataModel(*this, *step2Model));
  auto step1Master = step1Model->getMaster();

  for(auto& marker : step1Master->getEntities({{PeakType, PTMarker}}))
  {
    step1Master->setEntityData(marker->getUid(), PeakType, PTPeak);
  }
  if(!step0model)
  {
    step0model.reset(new MarkupDataModel(*this, *step1Model));
    for(auto& chroma : step0model->getChromatogrammsListModels())
    {
      chroma->clearEntities(TypePeak);
    }
  }
  QMap<Steps, MarkupModelPtr> ret;
  ret[Step0DataLoad] = step0model;
  ret[Step1PeakDetection] = step1Model;
  ret[Step2MasterMarkersMarkup] = step2Model;
  ret[Step3MasterInterMarkersMarkup] = step3Model;
  ret[Step4SlavePeaksEditing] = step4model;

  return ret;
}

namespace MarkupDataProcessing
{
QJsonObject formLaying(ChromatogrammModelPtr chroma)
{
  QJsonObject layingRef;
  layingRef["baseline"] = chroma->getJBaseLineForStepCalculations();
  layingRef["peaks"] = chroma->getJPeaksForStepCalculations();
  layingRef["id"] = chroma->id();
  layingRef["x_cut_point"] = chroma->hasCutPoint() ? QJsonValue() : chroma->cutPoint();
  return layingRef;
}

namespace Reservoir
{
void parseResponseForStep1(Steps, QJsonObject json, MarkupModelPtr model)
{
  auto jresult = json["result"].toObject();
  for(auto& key : jresult.keys())
  {
    auto id = key.toInt();
    auto chroma = model->getChromatogramm(id);
    if(chroma)
    {
      auto dataObj = jresult[key].toObject();
      auto jparameters = dataObj["parameters"].toObject();

      chroma->parseJPeaks(dataObj["peaks"].toArray());
      chroma->parseJBaseLines(dataObj["baselines"].toObject(),
                              jparameters["default_baseline"].toInt());
    }
  }
}

void parseResponseForSteps234(Steps, QJsonObject json, MarkupModelPtr model)
{
  auto jresult = json["result"].toObject();
  auto chroma = model->getChromatogramm(jresult["id"].toInt());

  if(chroma)
  {
    chroma->parseJPeaks(jresult["peaks"].toArray());
    chroma->parseJBaseLine(jresult["baseline"].toArray());
    auto cutPt = jresult["x_cut_point"].toVariant();
    if(cutPt.isValid() && !cutPt.isNull())
      chroma->setCutPoint(cutPt.toDouble());
    else
      chroma->resetCutPoint();
  }
}

void parseResponseForStep567(Steps, QJsonObject json, MarkupModelPtr model)
{
  auto result = json["result"].toObject();
  for(auto& stringKey : result.keys())
  {
    int id = stringKey.toInt();
    if(model->hasMaster()
        && !model->hasInternalMaster()
        && id == model->getMaster()->id())
      continue;

    auto jchroma = result[stringKey].toObject();
    auto chroma = model->getChromatogramm(id);
    if(chroma)
    {
      chroma->parseJBaseLine(jchroma["baseline"].toArray());
      chroma->parseJPeaks(jchroma["peaks"].toArray());
      auto cutPt = jchroma["x_cut_point"].toVariant();
      if(cutPt.isValid() && !cutPt.isNull())
        chroma->setCutPoint(cutPt.toDouble());
      else
        chroma->resetCutPoint();
    }
  }
}
void parseResponseForStep1(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid& overlayId)
{
  parseResponseForStep1(newStep, json, model);
  GenesisWindow::Get()->RemoveOverlay(overlayId);
  overlayId = QUuid();
}

void parseResponseForSteps234(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid& overlayId)
{
  parseResponseForSteps234(newStep, json, model);
  GenesisWindow::Get()->RemoveOverlay(overlayId);
  overlayId = QUuid();
}

void parseResponseForStep567(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid& overlayId)
{
  parseResponseForStep567(newStep, json, model);
  GenesisWindow::Get()->RemoveOverlay(overlayId);
  overlayId = QUuid();
}

QJsonObject formDataForStep1(MarkupModelPtr prevStepModel)
{
  QJsonObject dataToServer;
  {
    QJsonArray files;
    for(auto& chroma : prevStepModel->getChromatogrammsListModels())
    {
      QJsonObject jfile;
      jfile["project_file_id"] = (int)chroma->id();
      jfile["file_uuid"]       = "";
      QJsonObject jparameters;
      jparameters["coel"]             = chroma->getChromatogramValue(ChromatogrammSettingsCoelution).toInt();
      jparameters["doug_peuck"]       = chroma->getChromatogramValue(ChromatogrammSettingsDegreeOfApproximation).toInt();
      jparameters["med_bounds"]       = chroma->getChromatogramValue(ChromatogrammSettingsBoundsByMedian).toBool();
      jparameters["min_h"]            = chroma->getChromatogramValue(ChromatogrammSettingsMinimalPeakHeight).toDouble();
      jparameters["noisy"]            = chroma->getChromatogramValue(ChromatogrammSettingsNoisy).toBool();
      jparameters["sign_to_med"]      = chroma->getChromatogramValue(ChromatogrammSettingsSignalToMedianRatio).toDouble();
      jparameters["view_smooth"]      = chroma->getChromatogramValue(ChromatogrammSettingsViewSmooth).toBool();

      bool findProperBLine = false;
      auto blines = chroma->getEntities(TypeBaseline);
      if(!blines.isEmpty())
      {
        auto blineEntity = blines.first();
        if(blineEntity->hasDataAndItsValid(BaseLineData))
        {
          auto blineModelPtr = blineEntity->getData(BaseLineData).value<TBaseLineDataModel>();
          if(blineModelPtr)
          {
            jparameters["default_baseline"] = blineModelPtr->getSmoothFactor();
            findProperBLine = true;
          }
        }
      }

      if(!findProperBLine)
        jparameters["default_baseline"] = chroma->getChromatogramValue(ChromatogrammSettingsBaseLineSmoothLvl).toInt();


      jparameters["window_size"]      = chroma->getChromatogramValue(ChromatogrammSettingsMedianFilterWindowSize).toInt();
      jfile["parameters"] = jparameters;
      jfile["baseline"] = chroma->getJBaseLineForStepCalculations();
      QJsonArray intervals;
      QList<QUuid> uids = chroma->getUIdListOfEntities(GenesisMarkup::TypeInterval);
      std::sort(uids.begin(), uids.end(), [&chroma](const QUuid& a, const QUuid& b) -> bool {
        auto aData = chroma->getEntity(a);
        auto bData = chroma->getEntity(b);
        auto aIter = aData->constFind(GenesisMarkup::IntervalLeft);
        auto bIter = bData->constFind(GenesisMarkup::IntervalLeft);
        if (aIter == aData->constEnd() || !aIter.value().isValid())
        {
          return false;  // a >= b
        }
        else
        {
          if (bIter == bData->constEnd() || !bIter.value().isValid())
            return true;  // a < b
          else
          {
            return aIter.value().value<TIntervalLeft>() < bIter.value().value<TIntervalLeft>();
          }
        }
      });
      int id = 0;
      for (int i = 0; i < uids.size(); i++)
      {
        QJsonObject interval;
        auto& uid = uids[i];
        auto entityPtr = chroma->getEntity(uid);
        if (!entityPtr)
          continue;
        else
        {
          interval["id"] = id++;
          interval["start_ret_time"] = entityPtr->value(GenesisMarkup::IntervalLeft).value<TIntervalLeft>();
          interval["end_ret_time"] = entityPtr->value(GenesisMarkup::IntervalRight).value<TIntervalRight>();
          switch ((IntervalTypes)entityPtr->value(GenesisMarkup::IntervalType).toInt()) {
          case ITNone:
            interval["type"] = "";
            break;
          case ITCustomParameters:
          {
            interval["type"] = "custom";
            QJsonObject interval_parameters;
            interval_parameters["coel"]            = entityPtr->value(GenesisMarkup::Coelution).toInt();
            interval_parameters["doug_peuck"]      = entityPtr->value(GenesisMarkup::DegreeOfApproximation).toInt();
            interval_parameters["med_bounds"]      = entityPtr->value(GenesisMarkup::BoundsByMedian).toBool();
            interval_parameters["min_h"]           = entityPtr->value(GenesisMarkup::MinimalPeakHeight).toDouble();
            interval_parameters["noisy"]           = entityPtr->value(GenesisMarkup::Noisy).toBool();
            interval_parameters["sign_to_med"]     = entityPtr->value(GenesisMarkup::SignalToMedianRatio).toDouble();
            interval_parameters["view_smooth"]     = entityPtr->value(GenesisMarkup::ViewSmooth).toBool();
            interval_parameters["default_baseline"]= entityPtr->value(GenesisMarkup::BaseLineSmoothLvl).toInt();
            interval_parameters["window_size"]     = entityPtr->value(GenesisMarkup::MedianFilterWindowSize).toInt();
            interval["parameters"] = interval_parameters;
          }
          break;
          case ITMarkupDeprecation:
            interval["type"] = "locked";
            break;
          }
        }
        if(!interval.isEmpty())
        {
          intervals << interval;
        }
      }
      jfile["intervals"] = intervals;
      files << jfile;
    }
    dataToServer["files"] = files;
  }
  return dataToServer;
}

QJsonObject formDataForSteps234(MarkupModelPtr prevStepModel)
{
  QJsonObject dataToServer;
  {
    auto master = prevStepModel->getMaster();
    if(!master)
    {
      Notification::NotifyError(MarkupStepController::tr("No reference plot selected, pick one first"));
      return dataToServer;
    }
    dataToServer["laying_reference"] = formLaying(master);
    dataToServer["project_file_id"] = master->id();
  }
  return dataToServer;
}

QJsonObject formDataForStep5(MarkupModelPtr prevStepModel, const QVariant& details)
{
  QJsonObject dataToServer;
  {
    auto master = prevStepModel->getMaster();
    QJsonArray files;
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
      files << file;

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
        files << file;
      }
    }
    dataToServer["files"] = files;
    dataToServer["laying"] = laying;
    dataToServer["laying_reference"] = MarkupDataProcessing::formLaying(master);
  }
  dataToServer["match"] = details.toInt();
  return dataToServer;
}

QJsonObject formDataForStep6(MarkupModelPtr prevStepModel, const QVariant &details)
{
  GenesisMarkup::IntermakrerTransferDialogDataPair pair = details.value<GenesisMarkup::IntermakrerTransferDialogDataPair>();
  QJsonObject dataToServer;
  {
    auto master = prevStepModel->getMaster();
    QJsonArray files;
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
      files << file;

      if(!isMaster)
        laying[QString::number(id)] = MarkupDataProcessing::formLaying(prevStepModel->getChromatogramm(id));
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
        files << file;
      }
    }
    dataToServer["files"] = files;
    dataToServer["laying"] = laying;
    dataToServer["laying_reference"] = MarkupDataProcessing::formLaying(master);
    QJsonArray blocks;
    for(auto& interval : pair.second)
    {
      QJsonArray block;//order is Valuable and Important!!! 1)window, 2)start, 3)end
      block << interval.window;
      block << interval.start;
      block << interval.end;
      blocks << block;
    }
    dataToServer["blocks"] = blocks;
  }
  dataToServer["match"] = pair.first;
  return dataToServer;
}

QJsonObject formDataForStep7(MarkupModelPtr prevStepModel)
{
  QJsonObject dataToServer;
  {
    QJsonObject layingOther;
    for(auto& id : prevStepModel->getChromatogrammsIdList())
    {
      if(id == prevStepModel->getMaster()->id())
        continue;
      auto chroma = prevStepModel->getChromatogramm(id);
      QJsonObject jchroma;
      jchroma["baseline"] = chroma->getJBaseLineForStepCalculations();
      jchroma["peaks"] = chroma->getJPeaksForStepCalculations();
      jchroma["id"] = chroma->id();
      jchroma["x_cut_point"] = chroma->hasCutPoint() ? QJsonValue() : chroma->cutPoint();
      layingOther[QString::number(id)] = jchroma;
    }
    dataToServer["laying_other"] = layingOther;

    QJsonObject reference;
    auto master = prevStepModel->getMaster();
    reference["baseline"] = master->getJBaseLineForStepCalculations();
    reference["peaks"] = master->getJPeaksForStepCalculations();
    reference["id"] = master->id();
    reference["x_cut_point"] = master->hasCutPoint() ? QJsonValue() : master->cutPoint();
    dataToServer["reference"] = reference;
  }
  return dataToServer;
}

}//namespace Reservoir
}//namespace MarkupDataProcessing

}//GenesisMarkup
