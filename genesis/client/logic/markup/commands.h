#ifndef NEXTSTEPCOMMAND_H
#define NEXTSTEPCOMMAND_H

#include "quuid.h"
#include "qvariant.h"
#include <QUndoCommand>
#include <logic/markup/genesis_markup_enums.h>
#include <logic/markup/abstract_data_model.h>
#include <logic/markup/genesis_markup_forward_declare.h>

namespace GenesisMarkup {

class MarkupStepController;

//class CommandSequence takes ownership of commands from input commands list
class CommandSequence : public QUndoCommand
{
public:
  CommandSequence(const QList<QUndoCommand*>& sequence);
  ~CommandSequence();
  void undo();
  void redo();

private:
  QList<QUndoCommand*> mSequence;
};

class SetMasterCommand : public QUndoCommand
{
public:
  SetMasterCommand(ChromaId newMasterId, MarkupModelPtr model);
  void undo() override;
  void redo() override;

private:
  MarkupModelPtr mModel;
  ChromaId mNewId;
  ChromaId mOldId;
};

class NextStepCommand : public QUndoCommand
{
public:
  NextStepCommand(MarkupStepController* controller,
                  MarkupModelPtr oldModel,
                  Steps oldStep,
                  const QVariant& details = QVariant());
  //details here - is variant variable for forwarding data from dialogs. use it as you want.
  //for step next from step 4 to step 5 details - is int (enum) of marker transfer match type;

private:
  MarkupModelPtr mOldModel;
  MarkupModelPtr mNewModel;
  Steps mOldStep;
  Steps mNewStep;
  MarkupStepController* mController;
  QVariant mDetails;

  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class PrevStepCommand : public QUndoCommand
{
public:
  PrevStepCommand(MarkupStepController* controller,
                  MarkupModelPtr oldModel,
                  Steps oldStep);

private:
  MarkupModelPtr mOldModel;
  MarkupModelPtr mNewModel;
  Steps mOldStep;
  Steps mNewStep;
  MarkupStepController* mController;

  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class SetStepCommand : public QUndoCommand
{
public:
  SetStepCommand(MarkupStepController* controller,
                  MarkupModelPtr oldModel,
                  Steps oldStep,
                  Steps newStep, MarkupModelPtr newModel);

private:
  MarkupModelPtr mOldModel;
  MarkupModelPtr mNewModel;
  Steps mOldStep;
  Steps mNewStep;
  MarkupStepController* mController;

  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class SwapChromasCommand : public QUndoCommand
{
public:
  SwapChromasCommand(MarkupStepController* controller,
                 MarkupModelPtr model,
                 QList<ChromatogrammModelPtr> newChromas);

private:
  MarkupStepController* mController;
  MarkupModelPtr mModel;
  QList<ChromatogrammModelPtr> mOldChromas;
  QList<ChromatogrammModelPtr> mNewChromas;

  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class SetPeakTypeCommand : public QUndoCommand
{
public:
  SetPeakTypeCommand(ChromatogrammModelPtr model,
                     QUuid peakUid,
                     GenesisMarkup::PeakTypes oldType,
                     GenesisMarkup::PeakTypes newType);

private:
  ChromatogrammModelPtr mModel;
  GenesisMarkup::PeakTypes mOldType;
  GenesisMarkup::PeakTypes mNewType;
  QUuid mPeakUid;

  void handlePeaksTypeDependencies(bool redo);
  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class MovePeakBorderCommand : public QUndoCommand
{
public:
  MovePeakBorderCommand(ChromatogrammModelPtr model,
                     QUuid peakUid,
                     double oldBorderValue,
                     double newBorderValue,
                     double newRetTime,
                     bool isLeftBorder);

private:
  void assignValue(double value, double retTime);

  ChromatogrammModelPtr mModel;
  QUuid mPeakUid;
  double mOldBorderValue;
  double mNewBorderValue;
  bool mIsLeftBorder;
  double mOldRetTime;
  double mNewRetTime;

  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class ChangeBaseLineCommand : public QUndoCommand
{
public:
  ChangeBaseLineCommand(BaseLineDataModelPtr model,
                        const QPair<QVector<double>, QVector<double>> &oldBaseLine,
                        const QPair<QVector<double>, QVector<double>> &newBaseLine);

private:
  BaseLineDataModelPtr mModel;
  QPair<QVector<double>, QVector<double>> mOldBaseLine;
  QPair<QVector<double>, QVector<double>> mNewBaseLine;
  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class ChangeEntityDataCommand : public QUndoCommand
{
public:
  ChangeEntityDataCommand(ChromatogrammModelPtr model,
                          QUuid entityUid,
                          int dataRole,
                          const QVariant& newData);

private:
  ChromatogrammModelPtr mModel;
  QUuid   mEntityUid;
  int mDataRole;
  QVariant mOldData;
  QVariant mNewData;
  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class RemoveEntityCommand : public QUndoCommand
{
public:
  RemoveEntityCommand(ChromatogrammModelPtr model,
                      QUuid entityId);

private:
  ChromatogrammModelPtr mModel;
  DataModel mEntity;
  QUuid mUid;
  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};
class RemoveEntityListCommand : public QUndoCommand
{
public:
  RemoveEntityListCommand(ChromatogrammModelPtr model,
                      const QList<QUuid> &entityIds);

private:
  ChromatogrammModelPtr mModel;
  QList<QPair<QUuid, DataModel>> mEntities;
  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class AddEntityCommand : public QUndoCommand
{
public:
  AddEntityCommand(ChromatogrammModelPtr model,
                   AbstractEntityDataModel::DataPtr data,
                   MarkupEntityTypes type);
  AddEntityCommand(ChromatogrammModelPtr model,
                   const DataModel &data,
                   MarkupEntityTypes type);

private:
  ChromatogrammModelPtr mModel;
  AbstractEntityDataModel::DataPtr mData;
  MarkupEntityTypes mType;
  QUuid mUid;
  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class ResetEntityDataCommand : public QUndoCommand
{

public:
  ResetEntityDataCommand(ChromatogrammModelPtr model,
                         QUuid entityUid,
                   AbstractEntityDataModel::DataPtr data);

  ResetEntityDataCommand(ChromatogrammModelPtr model, QUuid entityUid, const DataModel &data);
private:
  ChromatogrammModelPtr mModel;
  DataModel mOldData;
  DataModel mNewData;
  QUuid mEntityUid;

public:
  void undo() override;
  void redo() override;
};

class SetChromatogramDataCommand : public QUndoCommand
{

public:
  SetChromatogramDataCommand(ChromatogrammModelPtr model,
                             int dataRole,
                             QVariant newData);
private:
  ChromatogrammModelPtr mModel;
  int mDataRole;
  QVariant mNewData;
  QVariant mOldData;

public:
  void undo() override;
  void redo() override;
  bool mergeWith(const QUndoCommand *other) override;

  friend class SetChromatogramSettingsCommand;
};

class SetChromatogramSettingsCommand : public QUndoCommand
{

public:
  SetChromatogramSettingsCommand(ChromatogrammModelPtr model,
                                 double Min_h,
                                 bool Noisy,
                                 bool View_smooth,
                                 int Window_size,
                                 double Sign_to_med,
                                 int Doug_peuck,
                                 bool Med_bounds,
                                 int Coel);
  SetChromatogramSettingsCommand(ChromatogrammModelPtr model, const ChromaSettings& settings);
private:
//  struct Settings
//  {
//    double Min_h;
//    bool Noisy;
//    bool View_smooth;
//    int Window_size;
//    double Sign_to_med;
//    int Doug_peuck;
//    bool Med_bounds;
//    int Coel;
//  };
  ChromatogrammModelPtr mModel;
  int mDataRole;
  ChromaSettings mOldSettings;
  ChromaSettings mNewSettings;

public:
  void undo() override;
  void redo() override;
};

class SetMasterChromatogramSettingsCommand : public QUndoCommand
{

public:
  SetMasterChromatogramSettingsCommand(MarkupModelPtr markup, GenesisMarkup::ChromaSettings settings);
private:
  MarkupModelPtr mModel;
  QMap<int, ChromaSettings> mOldSettings;
  ChromaSettings mNewSettings;

public:
  void undo() override;
  void redo() override;
};

class MoveIntervalBorderCommand : public QUndoCommand
{
public:
  MoveIntervalBorderCommand(ChromatogrammModelPtr model,
                     QUuid Uid,
                     double oldBorderValue,
                     double newBorderValue,
                     bool isLeftBorder);

private:
  void assignValue(double value);

  ChromatogrammModelPtr mModel;
  QUuid mUid;
  double mOldBorderValue;
  double mNewBorderValue;
  bool mIsLeftBorder;

  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class RemoveChromatogramComand : public QUndoCommand
{
public:
  RemoveChromatogramComand(MarkupModelPtr model,
                            ChromaId chromaId);

private:
  MarkupModelPtr mModel;
  ChromaId mChromaId;
  ChromatogrammModelPtr mChroma;

  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class LambdaSequenceCommand : public QUndoCommand
{
public:
  LambdaSequenceCommand(std::function<void()> redoSequence,
                        std::function<void()> undoSequence);

private:
  std::function<void()> mRedoSequence;
  std::function<void()> mUndoSequence;

  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

class ReferenceImportCommand : public QUndoCommand
{
public:
  ReferenceImportCommand(MarkupStepController *controller,
                         QMap<Steps, MarkupModelPtr> postImportModelsPack1to4);
private:
  QMap<Steps, MarkupModelPtr> mPreImportModelsPack;
  QMap<Steps, MarkupModelPtr> mPostImportModelsPack;
  Steps mOldStep;
  MarkupStepController* mController;

  // QUndoCommand interface
public:
  void undo() override;
  void redo() override;
};

}//GenesisMarkup
#endif // NEXTSTEPCOMMAND_H
