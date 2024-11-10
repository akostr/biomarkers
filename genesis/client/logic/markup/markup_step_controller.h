#ifndef MARKUPSTEPCONTROLLER_H
#define MARKUPSTEPCONTROLLER_H

#include <QObject>
#include <api/api_rest.h>
#include "markup_data_model.h"
#include "chromatogram_passport_model.h"
#include "json_serializable.h"
#include "genesis_markup_enums.h"

class QUndoCommand;
class QUndoStack;
namespace GenesisMarkup
{
//you SHOULD call the initialize() method by yourself after object creation


namespace MarkupDataProcessing
{
QJsonObject formLaying(ChromatogrammModelPtr chroma);
namespace Reservoir
{
void parseResponseForStep1(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid &overlayId);
void parseResponseForSteps234(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid &overlayId);
void parseResponseForStep567(Steps newStep, QJsonObject json, MarkupModelPtr model, QUuid &overlayId);
void parseResponseForStep1(Steps newStep, QJsonObject json, MarkupModelPtr model);
void parseResponseForSteps234(Steps newStep, QJsonObject json, MarkupModelPtr model);
void parseResponseForStep567(Steps newStep, QJsonObject json, MarkupModelPtr model);
QJsonObject formDataForStep1(MarkupModelPtr prevStepModel);
QJsonObject formDataForSteps234(MarkupModelPtr prevStepModel);
QJsonObject formDataForStep5(MarkupModelPtr prevStepModel, const QVariant &details);
QJsonObject formDataForStep6(MarkupModelPtr prevStepModel, const QVariant &details);
QJsonObject formDataForStep7(MarkupModelPtr prevStepModel);

};
};

class MarkupStepController : public QObject, public JsonSerializable
{
  //you SHOULD call the initialize() method by yourself after object creation
  Q_OBJECT
public:
  explicit MarkupStepController(QObject *parent = nullptr);
  virtual ~MarkupStepController();
  virtual bool CanStepForward(QString *message = nullptr, const QVariant &details = QVariant());

public slots:
  //StepForward should be called only by commands from undo stack
  virtual Steps StepForward(MarkupModelPtr modelPlaceholder = nullptr, const QVariant &details = QVariant());
  virtual QPair<Steps, MarkupModelPtr> StepBack();
  virtual void SaveMarkup(const QVariant &details);
  virtual void SaveMarkupAs(const QString& markupName, const QString& markupComment, bool instantLoad = true);
  virtual void LoadMarkup(int markupId);
  virtual void SetCurrentStepAndSwitchHisModel(Steps step, GenesisMarkup::MarkupModelPtr newMarkupModel, bool force = false);
//  virtual void SetCurrentStepAndSwitchHisModelSilent(Steps step, GenesisMarkup::MarkupModelPtr newMarkupModel);
  void MergePassportDataWith(const MarkupStepController& other);

  void applyIntervals(QList<int> chromaIds,
                      const QMap<QUuid, ChromaSettings> &intervalsData,
                      const QMap<QUuid, QPair<double, double>> &deprecationIntervals,
                      ChromaSettings initialSettings,
                      Steps desiredStep);
  void StepBackUiSlot();
  void updateTitlesDisplayMode();

public:
  MarkupModelPtr getCurrentModel() const;
  void createNewMarkupForImport(const QList<int>& chromaIds, const ChromaSettings &initialSettings, MarkupModelPtr oldModel = nullptr);
  void createNewMarkup(int markupId);
//  GenesisMarkup::StepInteractionsFlags getInteractionsFlags();
  StepInfo getCurrentStepInfo();
  MarkupModelPtr getSelectiveCopyWithExternalMaster(Steps step, const QList<int>& ids);
  MarkupModelPtr getFullCopyWithoutMarkup(Steps step);
  MarkupModelPtr getCopyWithoutMarkupWithExternalMasterAndIntervals(Steps step, QList<int> chromaIdToCopy);
  void pushCommand(QUndoCommand* cmd);

  //you SHOULD call the initialize() method by yourself after object creation
  void initialize();

  const ChromatogramPassportModel &passportsModel() const;
  QSharedPointer<ChromatogramPassportModel> passportsModelPtr();

  const QList<StepInfo> &stepsInfo() const;

  const QMap<Steps, MarkupModelPtr> &stepModels() const;

  void setPassportsModel(const ChromatogramPassportModel &newPassportsModel);
  void setExternalPassportsModel(QSharedPointer<ChromatogramPassportModel> newPassportsModel);

  int currentStepIndex() const;

  bool extractStep(Steps step, MarkupModelPtr& retModel, StepInfo& retStepInfo);
  bool insertStep(Steps step, MarkupModelPtr retModel, const StepInfo& retStepInfo);

  virtual void applyStepForModel(Steps newStep, MarkupModelPtr model, MarkupModelPtr modelPlaceholder, const QVariant &details = QVariant(), std::function<void (bool)> postLoadingAction = nullptr);
  void setModelsPackAndStep(Steps newStep, const QMap<Steps, MarkupModelPtr>& newModelsPack);
  QUndoStack* getUndoStack();

  QString getMZ();
  QStringList getFileTypes();
  void merge(const MarkupStepController& otherController);
  void importReference(int newRefId, int newChromaId);
  QJsonObject save() const override;
  void load(const QJsonObject &data) override;
  void setCurrentStep(Steps newStep);

signals:
  void ModelChanged(MarkupModelPtr newModel, const StepInfo& modelStepInteractions);
  void MarkupComplete();
  void LastCommandFailure();
//  void newCommand(QUndoCommand* cmd);
  void stepsChanged();
  void markupSavingFinished(bool error);
  void markupTableIdReceived(int tableId);
  void markupLoadingFinished();
  void submarkupLoadingRequired(QJsonObject submarkupData);

protected:
  QMap<Steps, MarkupModelPtr> mStepModels;
  QList<StepInfo> mStepsInfo;
  QSharedPointer<ChromatogramPassportModel> mPassportsModel;

  QUuid mLoadingOverlayId;
  int mCurrentStepIndex;
  int mUploadCounter;
  QUndoStack* mUndoStack;

  virtual void setupSteps();
  //loadDataForStep should be called only by commands from undo stack (from StepForward)
  virtual void loadDataForStep(Steps newStep, MarkupModelPtr modelPlaceholder, const QVariant &details = QVariant());
  // JsonSerializable interface
  void loadExistedMarkup(QJsonObject data, Steps currentStep) ;
  //loadNextStepModel should be called only by commands from undo stack (from loadDataForStep)
  void loadNextStepModel(Steps newStep, Steps currentStep, const QJsonObject &dataToServer,
                         std::function<void (int, const QJsonObject &, API::REST::ReplyHandleFunc, API::REST::ReplyErrorFunc)> apiRequest,
                         std::function<void (Steps newStep, QJsonObject, MarkupModelPtr, QUuid& overlayId)> parseResponse,
                         MarkupModelPtr modelPlaceholder = nullptr,
                         std::function<void ()> postLoadingAction = nullptr);
  void loadStepForModel(MarkupModelPtr model,
                        Steps newStep,
                        const QJsonObject &dataToServer,
                        std::function<void (int, const QJsonObject &, API::REST::ReplyHandleFunc, API::REST::ReplyErrorFunc)> apiRequest,
                        std::function<void (Steps newStep, QJsonObject, MarkupModelPtr)> parseResponse,
                        MarkupModelPtr modelPlaceholder = nullptr,
                        std::function<void (bool)> postLoadingAction = nullptr);
  void saveToFile(QJsonObject object);
  void setupImportModel(MarkupModelPtr newModel, MarkupModelPtr oldModel, const ChromaSettings& settings);

protected slots:
  void undoAndRemoveLastCommand();

private:
  QMap<Steps, MarkupModelPtr> generateModelsDownFromStep4(MarkupModelPtr step4model, MarkupModelPtr step0model);
};


}//GenesisMarkup
#endif // MARKUPSTEPCONTROLLER_H
