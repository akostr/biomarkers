#include "biomarkers_sub_markup_step_controller.h"
namespace GenesisMarkup
{
BiomarkersSubMarkupStepController::BiomarkersSubMarkupStepController(Steps targetStep, MarkupStepController *parent)
  : BiomarkersMarkupStepController(parent)
  , mParent(parent)
  , mTargetStep(targetStep)
{}

Steps BiomarkersSubMarkupStepController::StepForward(MarkupModelPtr modelPlaceholder, const QVariant &details)
{
  if(getCurrentStepInfo().step == mStepsInfo.last().step)
  {
    emit MarkupComplete();
    return getCurrentStepInfo().step;
  }
  auto ret = MarkupStepController::StepForward(modelPlaceholder, details);
  return ret;
}

void BiomarkersSubMarkupStepController::setupSteps()
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
    StepInfo step1;
    step1.stepName = tr("Initial detected peaks setup");
    step1.toolTip = tr("- Selecting a referense\n"
                       "- Configuring general and individual settings\n"
                       "- Tail trimming/restoration");

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

    StepInfo step3;
    step3.stepName = tr("Additional chromatogramms peaks editing");
    step3.toolTip = tr("Working with additional\nchromatograms:\n"
                       "- Configuring general and individual settings\n"
                       "- Tail trimming/restoration\n"
                       "- Markers table available\n"
                       "- Numerical data table available");

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

    StepInfo step4;
    step4.stepName = tr("Markers transfer");
    step4.toolTip = tr("Working with additional\n"
                       "chromatograms:\n"
                       "- Transferring and marking markers\n"
                       "- Correction of time windows in the marker table\n"
                       "- Numerical data table available");

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
    }; step4.step = Step5MarkersTransferToSlave;
    mStepsInfo.append(step4);

    StepInfo step6;
    step6.stepName = tr("Comlpetion");
    step6.toolTip = tr("Marking complete.\n"
                       "You can come back if necessary\n"
                       "at any of the steps and edit it");
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
  QList<Steps> deprecatedSteps{Step2MasterMarkersMarkup, Step6Identification, Step7Complete};
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
}//namespace GenesisMarkup
