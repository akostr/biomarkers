#include "sub_markup_step_controller.h"
namespace GenesisMarkup
{
SubMarkupStepController::SubMarkupStepController(Steps targetStep, MarkupStepController *parent)
  : MarkupStepController{nullptr}
  , mParent(parent)
  , mTargetStep(targetStep)
{}

Steps SubMarkupStepController::targetStep()
{
  return mTargetStep;
}

void SubMarkupStepController::setupSteps()
{
  mStepsInfo.clear();
  {
    if(mTargetStep == Step0DataLoad)
    {
      StepInfo step0;
      step0.stepName = tr("Data loading");
      step0.stepNumber = 0;
      step0.slaveInteractions = {
        SISSetupParameters |
        SISCutTail |
        SISAddRemoveBaseLinePoint |
        SISBaseLinePointInteractions |
        SISModifyParamIntervals |
        SISModifyMarkupIntervals |
        SISEnabledStyle |
        SISExportAsImage
      };
      step0.masterInteractions = {
        SIMExportAsImage
      };
      step0.guiInteractions = {
        SIGViewPassport         |
        SIGNextStep|
        SIGSaveMarkup |
        SIGRemoveSlaveChromatogramm
      };
      step0.step = Step0DataLoad;
      mStepsInfo.append(step0);
    }
    else
    {
      StepInfo step0;
      step0.stepName = tr("Data loading");
      step0.stepNumber = 0;
      step0.slaveInteractions = {
        SISSetupParameters |
        SISCutTail |
        SISAddRemoveBaseLinePoint |
        SISBaseLinePointInteractions |
        SISModifyParamIntervals |
        SISModifyMarkupIntervals |
        SISEnabledStyle |
        SISExportAsImage
      };
      step0.masterInteractions = {
        SIMExportAsImage
      };
      step0.guiInteractions = {
        SIGViewPassport         |
        SIGSaveMarkup |
        SIGDetectPeaks
      };
      step0.step = Step0DataLoad;
      mStepsInfo.append(step0);
    }
  }

  {
    StepInfo step1;
    step1.stepName = tr("Initial detected peaks setup");
    step1.toolTip = tr(
"- Selecting a referense\n\
- Configuring general and individual settings\n\
- Tail trimming/restoration");
    step1.stepNumber = 1;
    step1.slaveInteractions = {
      SISCutTail                   |
      SISAddRemoveBaseLinePoint    |
      SISBaseLinePointInteractions |
      SISAddRemovePeak             |
      SISExportAsImage             |
      SISModifyPeaks               |
      SISModifyParamIntervals      |
      SISModifyMarkupIntervals     |
      SISAddRemoveInterval         |
      SISExportAsImage|
      SISEnabledStyle
    };
    step1.masterInteractions = {
      SIMExportAsImage
    };
    step1.guiInteractions = {
      SIGViewPassport         |
      SIGNextStep             |
      SIGPrevStep|
      SIGSaveMarkup |
      SIGRemoveSlaveChromatogramm
    };
    step1.step = Step1PeakDetection;
    mStepsInfo.append(step1);
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
      SISModifyParamIntervals      |
      SISEnabledStyle      |
      SISModifyMarkupIntervals
    };
    step5.masterInteractions = {
      SIMExportAsImage
    };
    step5.guiInteractions = {
      SIGViewPassport         |
      SIGMarkersTable         |
      SIGSaveMaster           |
      SIGNextStep             |
      SIGSaveMarkup |
      SIGPrevStep             |
      SIGDigitalDataTable     |
      SIGRepeatMarkerTransfer|
      SIGRemoveSlaveChromatogramm
    };
    step5.step = Step5MarkersTransferToSlave;
    mStepsInfo.append(step5);
  }

  StepInfo step6;
  step6.stepName = tr("Intermarkers transfer");
  step6.stepNumber = 6;
  step6.toolTip = tr(
"Working with additional\n\
chromatograms:\n\
- Transfer and marking of intermarker peaks\n\
- Numerical data table available");
  step6.slaveInteractions = {
    SISAddRemoveBaseLinePoint    |
    SISBaseLinePointInteractions |
    SISAddRemoveInterMarker      |
    SISAddRemovePeak             |
    SISSetUnsetInterMarker       |
    SISExportAsImage             |
    SISModifyPeaks               |
    SISModifyInterMarkers        |
    SISModifyParamIntervals      |
    SISEnabledStyle      |
    SISModifyMarkupIntervals
  };
  step6.masterInteractions = {
    SIMExportAsImage
  };
  step6.guiInteractions = {
    SIGViewPassport         |
    SIGMarkersTable         |
    SIGSaveMaster           |
    SIGComplete             |
    SIGSaveMarkup |
    SIGPrevStep             |
    SIGDigitalDataTable     |
    SIGRepeatInterMarkerTransfer|
    SIGRemoveSlaveChromatogramm
  };
  step6.step = Step6InterMarkersTransferToSlave;
  mStepsInfo.append(step6);

  StepInfo step7;
  step7.stepName = tr("Comlpetion");
  step7.toolTip = tr(
"Marking complete.\n\
You can come back if necessary\n\
at any of the steps and edit it.");
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
    SIGMarkersTable         |
    SIGMatrix               |
    SIGEditNameAndComment   |
    SIGEditCopy             |
    SIGPrevStep             |
    SIGSwitchToTable        |
    SIGDigitalDataTable
  };
  step7.step = Step7Complete;
  mStepsInfo.append(step7);

  QList<Steps> deprecatedSteps{Step2MasterMarkersMarkup, Step3MasterInterMarkersMarkup, Step4SlavePeaksEditing, Step7Complete};
//  deprecatedSteps.removeIf([this](const Steps& v)->bool{return v == mTargetStep;});
  auto iter = mStepsInfo.constBegin();
  while(iter != mStepsInfo.constEnd())
  {
    if((iter->step > mTargetStep) ||
       deprecatedSteps.contains(iter->step))
      iter = mStepsInfo.erase(iter);
    else
      iter++;
  }
}

Steps SubMarkupStepController::StepForward(MarkupModelPtr modelPlaceholder, const QVariant &details)
{
  if(getCurrentStepInfo().step == mStepsInfo.last().step)
  {
    emit MarkupComplete();
    return getCurrentStepInfo().step;
  }
  auto ret = MarkupStepController::StepForward(modelPlaceholder, details);
  return ret;
}

}//namespace GenesisMarkup
