#ifndef BIOMARKERSSUBMARKUPSTEPCONTROLLER_H
#define BIOMARKERSSUBMARKUPSTEPCONTROLLER_H

#include "biomarkers_markup_step_controller.h"

namespace GenesisMarkup
{
class BiomarkersSubMarkupStepController : public BiomarkersMarkupStepController
{
public:
  explicit BiomarkersSubMarkupStepController(Steps targetStep, MarkupStepController *parent = nullptr);

private:
  MarkupStepController* mParent;
  Steps mTargetStep;

  // MarkupStepController interface
public slots:
  Steps StepForward(MarkupModelPtr modelPlaceholder, const QVariant &details) override;

protected:
  void setupSteps() override;
};
}//namespace GenesisMarkup
#endif // BIOMARKERSSUBMARKUPSTEPCONTROLLER_H
