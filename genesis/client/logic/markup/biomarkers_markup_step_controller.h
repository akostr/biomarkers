#ifndef BIOMARKERSMARKUPSTEPCONTROLLER_H
#define BIOMARKERSMARKUPSTEPCONTROLLER_H

#include "markup_step_controller.h"
namespace GenesisMarkup
{
namespace MarkupDataProcessing
{
namespace Biomarkers
{
void parseResponseForStep6Identification(Steps newStep, QJsonObject json, MarkupModelPtr model);
void parseResponseForStep6Identification(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid &overlayId);
void parseResponseForStep7Complete(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid &overlayId);
void parseResponseForStep7Complete(Steps newStep, QJsonObject json, MarkupModelPtr model);
QJsonObject formDataForStep6(MarkupModelPtr prevStepModel);

}
};
class BiomarkersMarkupStepController : public MarkupStepController
{
  Q_OBJECT
public:
  BiomarkersMarkupStepController(QObject *parent = nullptr);


  // MarkupStepController interface
protected:
  void setupSteps() override;

  // MarkupStepController interface
  protected:
  void loadDataForStep(Steps newStep, MarkupModelPtr modelPlaceholder, const QVariant &details) override;

  public:
  void applyStepForModel(Steps newStep, MarkupModelPtr model, MarkupModelPtr modelPlaceholder, const QVariant &details, std::function<void (bool)> postLoadingAction) override;
  bool CanStepForward(QString *message, const QVariant &details) override;

  // MarkupStepController interface
public slots:
  void SaveMarkup(const QVariant &details) override;
  void SaveMarkupAs(const QString &markupName, const QString &markupComment, bool instantLoad) override;
};
}//namespace GenesisMarkup

#endif // BIOMARKERSMARKUPSTEPCONTROLLER_H
