#include "commands.h"
#include "logic/markup/genesis_markup_enums.h"
#include "logic/markup/markup_step_controller.h"
#include <logic/markup/baseline_data_model.h>
#include <logic/notification.h>

namespace GenesisMarkup {

NextStepCommand::NextStepCommand(MarkupStepController *controller,
                                 MarkupModelPtr oldModel,
                                 Steps oldStep,
                                 const QVariant &details)
{
  mOldModel = oldModel;
  mOldStep = oldStep;
  mController = controller;
  mNewModel = nullptr;
  mDetails = details;
}

void NextStepCommand::undo()
{
  if(!mController)
    return;
  mController->SetCurrentStepAndSwitchHisModel(mOldStep, mOldModel);
}

void NextStepCommand::redo()
{
  if(!mController)
    return;
  if(mNewModel)
    mController->SetCurrentStepAndSwitchHisModel(mNewStep, mNewModel);
  else
  {
    mNewModel.reset(new MarkupDataModel(*mController));
    mNewStep = mController->StepForward(mNewModel, mDetails);
  }
}

SetStepCommand::SetStepCommand(MarkupStepController *controller, MarkupModelPtr oldModel, Steps oldStep, Steps newStep, MarkupModelPtr newModel)
{
  mOldModel = oldModel;
  mOldStep = oldStep;
  mController = controller;
  mNewStep = newStep;
  mNewModel = newModel;
}

void SetStepCommand::undo()
{
  if(!mController)
    return;
  mController->SetCurrentStepAndSwitchHisModel(mOldStep, mOldModel);
}

void SetStepCommand::redo()
{
  if(!mController)
    return;
  mController->SetCurrentStepAndSwitchHisModel(mNewStep, mNewModel);
}

PrevStepCommand::PrevStepCommand(MarkupStepController *controller,
                                 MarkupModelPtr oldModel,
                                 Steps oldStep)
  : mController(controller),
    mOldModel(oldModel),
    mOldStep(oldStep)
{

}

void PrevStepCommand::undo()
{
  if(!mController)
    return;
  mController->SetCurrentStepAndSwitchHisModel(mOldStep, mOldModel);
}

void PrevStepCommand::redo()
{
  if(!mController)
    return;
  if(!mNewModel)
  {
    auto ret = mController->StepBack();
    mNewStep = ret.first;
    mNewModel = ret.second;
  }
  else
  {
    mController->SetCurrentStepAndSwitchHisModel(mNewStep, mNewModel);
  }
}

SetPeakTypeCommand::SetPeakTypeCommand(ChromatogrammModelPtr model,
                                       QUuid peakUid,
                                       PeakTypes oldType,
                                       PeakTypes newType)
  : QUndoCommand(),
    mModel(model),
    mOldType(oldType),
    mNewType(newType),
    mPeakUid(peakUid)
{
}

void SetPeakTypeCommand::handlePeaksTypeDependencies(bool redo)
{


}

void SetPeakTypeCommand::undo()
{
  mModel->setEntityData(mPeakUid, GenesisMarkup::PeakType, mOldType);
  handlePeaksTypeDependencies(false);
}

void SetPeakTypeCommand::redo()
{
  mModel->setEntityData(mPeakUid, GenesisMarkup::PeakType, mNewType);
  handlePeaksTypeDependencies(true);
}

MovePeakBorderCommand::MovePeakBorderCommand(ChromatogrammModelPtr model,
                                             QUuid peakUid,
                                             double oldBorderValue,
                                             double newBorderValue,
                                             double newRetTime,
                                             bool isLeftBorder)
  : QUndoCommand(),
    mModel(model),
    mPeakUid(peakUid),
    mOldBorderValue(oldBorderValue),
    mNewBorderValue(newBorderValue),
    mIsLeftBorder(isLeftBorder),
    mNewRetTime(newRetTime)
{
  mOldRetTime = mModel->getEntity(mPeakUid)->getData(PeakRetentionTime).toDouble();
}

void MovePeakBorderCommand::assignValue(double value, double retTime)
{
  int dataRole = 0;
  if(mIsLeftBorder)
    dataRole = GenesisMarkup::PeakStart;
  else//rightBorder
    dataRole = GenesisMarkup::PeakEnd;

  QPointF pt;
  auto peakEntity = mModel->getEntity(mPeakUid);
  if(peakEntity->hasDataAndItsValid(dataRole))
    pt = peakEntity->getData(dataRole).toPointF();
  pt.setX(value);
  mModel->setEntityData(mPeakUid, dataRole, pt);
  mModel->setEntityData(mPeakUid, GenesisMarkup::PeakRetentionTime, retTime);
}

void MovePeakBorderCommand::undo()
{
  assignValue(mOldBorderValue, mOldRetTime);
}

void MovePeakBorderCommand::redo()
{
  assignValue(mNewBorderValue, mNewRetTime);
}

ChangeBaseLineCommand::ChangeBaseLineCommand(BaseLineDataModelPtr model,
                                                   const QPair<QVector<double>, QVector<double> > &oldBaseLine,
                                                   const QPair<QVector<double>, QVector<double> > &newBaseLine)
  : mModel(model),
    mOldBaseLine(oldBaseLine),
    mNewBaseLine(newBaseLine)
{}

void ChangeBaseLineCommand::undo()
{
  mModel->setBaseLine(mModel->getSmoothFactor(), mOldBaseLine.first, mOldBaseLine.second);
}

void ChangeBaseLineCommand::redo()
{
  mModel->setBaseLine(mModel->getSmoothFactor(), mNewBaseLine.first, mNewBaseLine.second);
}

ChangeEntityDataCommand::ChangeEntityDataCommand(ChromatogrammModelPtr model, QUuid entityUid, int dataRole, const QVariant &newData)
  : QUndoCommand(),
    mModel(model),
    mEntityUid(entityUid),
    mDataRole(dataRole),
    mOldData(mModel->getEntity(entityUid)->getData(dataRole)),
    mNewData(newData)
{}

void ChangeEntityDataCommand::undo()
{
    mModel->setEntityData(mEntityUid, mDataRole, mOldData);
}

void ChangeEntityDataCommand::redo()
{
  mModel->setEntityData(mEntityUid, mDataRole, mNewData);
}

RemoveEntityCommand::RemoveEntityCommand(ChromatogrammModelPtr model, QUuid entityId)
  : mModel(model)
  , mUid(entityId)
{
  mEntity = *mModel->getEntity(entityId);
}

void RemoveEntityCommand::undo()
{
  mModel->addNewEntity(mEntity[AbstractEntityDataModel::RoleEntityType].toInt(), mEntity, mUid);
}

void RemoveEntityCommand::redo()
{
  mModel->removeEntity(mUid);
}

AddEntityCommand::AddEntityCommand(ChromatogrammModelPtr model,
                                   AbstractEntityDataModel::DataPtr data,
                                   MarkupEntityTypes type)
  : mModel(model),
    mData(data),
    mType(type)
{}

AddEntityCommand::AddEntityCommand(ChromatogrammModelPtr model,
                                   const DataModel &data,
                                   MarkupEntityTypes type)
  : mModel(model),
    mType(type)
{
  mData.reset(new DataModel(data));
}

void AddEntityCommand::undo()
{
  mModel->removeEntity(mUid);
}

void AddEntityCommand::redo()
{
  if(mUid.isNull())
    mUid = mModel->addNewEntity(mType, *mData);
  else
    mModel->addNewEntity(mType, *mData, mUid);
}



ResetEntityDataCommand::ResetEntityDataCommand(ChromatogrammModelPtr model, QUuid entityUid, AbstractEntityDataModel::DataPtr data)
  : mModel(model),
  mEntityUid(entityUid)
{
  if(data)
  {
    mNewData = *data;
  }

  if(mModel->getEntity(entityUid))
  {
    mOldData = *mModel->getEntity(entityUid);
  }
}

ResetEntityDataCommand::ResetEntityDataCommand(ChromatogrammModelPtr model, QUuid entityUid, const DataModel &data)
  : mModel(model),
  mEntityUid(entityUid),
  mNewData(data)
{
  if(mModel->getEntity(entityUid))
  {
    mOldData = *mModel->getEntity(entityUid);
  }
}


void ResetEntityDataCommand::undo()
{
  mModel->resetEntityData(mEntityUid, mOldData);
}

void ResetEntityDataCommand::redo()
{
  mModel->resetEntityData(mEntityUid, mNewData);
}

SetChromatogramDataCommand::SetChromatogramDataCommand(ChromatogrammModelPtr model,
                                                       int dataRole,
                                                       QVariant newData)
  : mModel(model),
    mDataRole(dataRole),
    mNewData(newData)
{
  mOldData = mModel->getChromatogramValue(dataRole);
}

void SetChromatogramDataCommand::undo()
{
  mModel->setChromatogramValue(mDataRole, mOldData);
}

void SetChromatogramDataCommand::redo()
{
  mModel->setChromatogramValue(mDataRole, mNewData);
}

bool SetChromatogramDataCommand::mergeWith(const QUndoCommand *other)
{
  auto otherCmd = dynamic_cast<const SetChromatogramDataCommand*>(other);
  if(otherCmd)
  {
    if(otherCmd->mModel == mModel && otherCmd->mDataRole == mDataRole)
    {
      mNewData = otherCmd->mNewData;
      return true;
    }
  }
  return false;
}

SetChromatogramSettingsCommand::SetChromatogramSettingsCommand(ChromatogrammModelPtr model,
                                                               double Min_h,
                                                               bool Noisy,
                                                               bool View_smooth,
                                                               int Window_size,
                                                               double Sign_to_med,
                                                               int Doug_peuck,
                                                               bool Med_bounds,
                                                               int Coel)
  : SetChromatogramSettingsCommand(model,
    ChromaSettings{Min_h, Noisy, View_smooth, Window_size, Sign_to_med, Doug_peuck, Med_bounds, Coel})
{
}

SetChromatogramSettingsCommand::SetChromatogramSettingsCommand(ChromatogrammModelPtr model, const ChromaSettings& settings)
  : mModel(model),
    mNewSettings(settings)
{
  mOldSettings.Min_h = mModel->getChromatogramValue(ChromatogrammSettingsMinimalPeakHeight).toDouble();
  mOldSettings.Noisy = mModel->getChromatogramValue(ChromatogrammSettingsNoisy).toBool();
  mOldSettings.View_smooth = mModel->getChromatogramValue(ChromatogrammSettingsViewSmooth).toBool();
  mOldSettings.Window_size = mModel->getChromatogramValue(ChromatogrammSettingsMedianFilterWindowSize).toInt();
  mOldSettings.Sign_to_med = mModel->getChromatogramValue(ChromatogrammSettingsSignalToMedianRatio).toDouble();
  mOldSettings.Doug_peuck = mModel->getChromatogramValue(ChromatogrammSettingsDegreeOfApproximation).toInt();
  mOldSettings.Med_bounds = mModel->getChromatogramValue(ChromatogrammSettingsBoundsByMedian).toBool();
  mOldSettings.Coel = mModel->getChromatogramValue(ChromatogrammSettingsCoelution).toInt();
  mOldSettings.BLineSmoothLvl = mModel->getChromatogramValue(ChromatogrammSettingsBaseLineSmoothLvl).toInt();
}

void SetChromatogramSettingsCommand::undo()
{
  mModel->setChromaSettings(mOldSettings);
  Notification::NotifyInfo(QObject::tr("undo %1 chromatogramm settings").arg(mModel->id()));
}

void SetChromatogramSettingsCommand::redo()
{
  mModel->setChromaSettings(mNewSettings);
  Notification::NotifySuccess(QObject::tr("set %1 chromatogramm settings").arg(mModel->id()));
}

SetMasterChromatogramSettingsCommand::SetMasterChromatogramSettingsCommand(MarkupModelPtr markup, ChromaSettings settings)
 : mModel(markup) ,
   mNewSettings(settings)
{
  for (const auto& chroma : mModel->getChromatogrammsList())
    mOldSettings[chroma.first] = chroma.second->getChromaSettings();
}

void SetMasterChromatogramSettingsCommand::undo()
{
  for(auto& chroma : mModel->getChromatogrammsList())
    chroma.second->setChromaSettings(mOldSettings[chroma.first]);
  Notification::NotifyInfo(QObject::tr("undo master(and all) chromatogramm settings"));
}

void SetMasterChromatogramSettingsCommand::redo()
{
  for(auto& chroma : mModel->getChromatogrammsList())
    chroma.second->setChromaSettings(mNewSettings);
  Notification::NotifySuccess(QObject::tr("set master(and all) chromatogramm settings"));
}

MoveIntervalBorderCommand::MoveIntervalBorderCommand(ChromatogrammModelPtr model, QUuid Uid, double oldBorderValue, double newBorderValue,  bool isLeftBorder)
  : QUndoCommand(),
    mModel(model),
    mUid(Uid),
    mOldBorderValue(oldBorderValue),
    mNewBorderValue(newBorderValue),
    mIsLeftBorder(isLeftBorder)
{
}

void MoveIntervalBorderCommand::assignValue(double value)
{
  int dataRole = 0;
  if(mIsLeftBorder)
    dataRole = GenesisMarkup::IntervalLeft;
  else//rightBorder
    dataRole = GenesisMarkup::IntervalRight;

  auto entity = mModel->getEntity(mUid);
  mModel->setEntityData(mUid, dataRole, value);
}

void MoveIntervalBorderCommand::undo()
{
  assignValue(mOldBorderValue);
}

void MoveIntervalBorderCommand::redo()
{
  assignValue(mNewBorderValue);
}

SwapChromasCommand::SwapChromasCommand(MarkupStepController *controller,
                                     MarkupModelPtr model,
                                     QList<ChromatogrammModelPtr> newChromas)
  : mController(controller),
    mModel(model),
    mNewChromas(newChromas)
{
  for(auto& chroma : mNewChromas)
    if(mModel->hasChromatogram(chroma->id()))
      mOldChromas << mModel->getChromatogramm(chroma->id());
}

void SwapChromasCommand::undo()
{
  for(auto& chroma : mOldChromas)
    mModel->swapChromatogramm(chroma->id(), chroma);
}

void SwapChromasCommand::redo()
{
  for(auto& chroma : mNewChromas)
    mModel->swapChromatogramm(chroma->id(), chroma);
}

RemoveChromatogramComand::RemoveChromatogramComand(MarkupModelPtr model, ChromaId chromaId)
  : mModel(model),
    mChromaId(chromaId)
{
  mChroma = mModel->getChromatogramm(mChromaId);
  Q_ASSERT(mChroma);
}

void RemoveChromatogramComand::undo()
{
  Q_ASSERT(mChroma);
  if(mChroma)
    mModel->addChromatogramm(mChromaId, mChroma);
}

void RemoveChromatogramComand::redo()
{
  mModel->removeChromatogramm(mChromaId);
}

LambdaSequenceCommand::LambdaSequenceCommand(std::function<void ()> redoSequence, std::function<void ()> undoSequence)
  : mRedoSequence(redoSequence),
    mUndoSequence(undoSequence)
{}

void LambdaSequenceCommand::undo()
{
  mUndoSequence();
}

void LambdaSequenceCommand::redo()
{
  mRedoSequence();
}

ReferenceImportCommand::ReferenceImportCommand(MarkupStepController* controller, QMap<Steps, MarkupModelPtr> postImportModelsPack1to4)
{
  mController = controller;
  mPostImportModelsPack = postImportModelsPack1to4;
  mOldStep = mController->getCurrentStepInfo().step;
  mPreImportModelsPack = mController->stepModels();
}

void ReferenceImportCommand::undo()
{
  mController->setModelsPackAndStep(mOldStep, mPreImportModelsPack);
}

void ReferenceImportCommand::redo()
{
  mController->setModelsPackAndStep(Step4SlavePeaksEditing, mPostImportModelsPack);
}

SetMasterCommand::SetMasterCommand(ChromaId newMasterId, MarkupModelPtr model)
{
  mModel = model;
  if(model->getMaster())
    mOldId = model->getMaster()->id();
  else
    mOldId = -1;
  mNewId = newMasterId;
}

void SetMasterCommand::undo()
{
  if(mOldId == -1)
    mModel->resetMaster();
  else
    mModel->setMaster(mOldId);
}

void SetMasterCommand::redo()
{
  if(mNewId == -1)
    mModel->resetMaster();
  else
    mModel->setMaster(mNewId);
}

RemoveEntityListCommand::RemoveEntityListCommand(ChromatogrammModelPtr model, const QList<QUuid>& entityIds)
  : mModel(model)
{
  for(auto& id : entityIds)
  {
    mEntities.append({id, *mModel->getEntity(id)});
  }
}

void RemoveEntityListCommand::undo()
{
  for(auto& e : mEntities)
    mModel->addNewEntity(e.second[AbstractEntityDataModel::RoleEntityType].toInt(), e.second, e.first);
}

void RemoveEntityListCommand::redo()
{
  for(auto& e : mEntities)
    mModel->removeEntity(e.first);
}

CommandSequence::CommandSequence(const QList<QUndoCommand *> &sequence)
  : mSequence(sequence)
{}

CommandSequence::~CommandSequence()
{
  for(auto& cmd : mSequence)
    delete cmd;
  mSequence.clear();
}

void CommandSequence::undo()
{
  for(auto backIt = mSequence.rbegin(); backIt != mSequence.rend(); backIt++)
    (*backIt)->undo();
}

void CommandSequence::redo()
{
  for(auto it = mSequence.begin(); it != mSequence.end(); it++)
    (*it)->redo();
}

}//GenesisMarkup

