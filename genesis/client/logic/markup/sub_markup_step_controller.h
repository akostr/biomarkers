#ifndef SUBMARKUPSTEPCONTROLLER_H
#define SUBMARKUPSTEPCONTROLLER_H

#include "markup_step_controller.h"

namespace GenesisMarkup
{
class SubMarkupStepController : public MarkupStepController
{
  Q_OBJECT
public:
  explicit SubMarkupStepController(Steps targetStep, MarkupStepController *parent = nullptr);
  Steps targetStep();

protected:
  void setupSteps() override;

private:
  MarkupStepController* mParent;
  Steps mTargetStep;

  // MarkupStepController interface
public slots:
  Steps StepForward(MarkupModelPtr modelPlaceholder, const QVariant &details) override;
};
}//namespace GenesisMarkup
#endif // SUBMARKUPSTEPCONTROLLER_H
