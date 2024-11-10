#ifndef VIEWPAGEPROJECTMARKUPREFACTOR_H
#define VIEWPAGEPROJECTMARKUPREFACTOR_H

#include <QObject>
#include <QScopedPointer>
#include <QMap>
#include <graphicsplot/graphicsplot.h>
#include "ui/widgets/markup/stepper_widget.h"
#include "graphicsplot/graphicsplot_data_structures.h"
#include "ui/view.h"
#include "logic/markup/markup_step_controller.h"
#include "logic/markup/genesis_markup_forward_declare.h"


#ifdef RENDERDOC_API_DEBUG
#include "renderdoc_app.h"
#endif

namespace Ui {
class MarkupPage;
}


class QVBoxLayout;
class QStackedLayout;
class PinWidget;
class QScrollArea;
class QToolBar;
class QComboBox;
class QLabel;
class QPushButton;
class QMenu;
class QHBoxLayout;
class ChromatogramWidget;
class QTableView;
class ChromatogramPlotOldLegacy;
class IPlotMasterableController;
class MarkupController;
class ChromatogrammsTabWidget;
class ChromatogramPlot;
class IdentificationWidget;

namespace Widgets
{
class TableMarkerWidget;
}

namespace GenesisMarkup
{
class MarkupDataModel;
class ReservoirMarkupWidget;
class ChromatogramWidgetRefactor;
// using ChromatogramsWidgetsMap = QHash<MarkupStepController*, QHash<ChromaId, QPointer<ChromatogramWidgetRefactor>>>;
//  using UndoStacksMap = QHash<MarkupStepController*, QUndoStack*>;

class ViewPageProjectMarkup : public Views::View
{
  Q_OBJECT
public:
  explicit ViewPageProjectMarkup(QWidget* parent = nullptr);
  ~ViewPageProjectMarkup();
  void SetupUi();
  void SaveMarkupStateOnBack();
  void SaveMarkupAs(bool instantLoad);
  bool isCommandStackClean();

signals:
  void markupStateClean();

  // View interface
public:
  void SwitchToPage(const QString &pageId) override;
  void SwitchToModule(int module) override;
  void LogOut() override;
  void ApplyContextMarkup(const QString& dataId, const QVariant& data) override;
  void ApplyContextModules(const QString &dataId, const QVariant &data) override;

private slots:
  void loadMarkup(int markupId);
  void onUserChangedTitlesMode(bool checked);
  void updateTitleMode(GenesisMarkup::ChromatogrammTitlesDisplayModes mode);
  void openRatioMatrixSettingsDialog(int tableId);

protected slots:
  void onMainModelChanged(MarkupModelPtr model, const StepInfo &stepInfo);
  void onSubModelChanged(MarkupModelPtr model, const StepInfo &stepInfo);
  void handleUiInteractions(const StepInfo &stepInfo);

  void ReMarkupChromatogramms(const QList<int> ids, const ChromaSettings* initialSettings = nullptr);
  void ReMarkupImportedChromatogramms(const QList<int> ids, const ChromaSettings initialSettings = ChromaSettings());

protected slots:
  void TryToSwitchToTable();
  void SaveReferenceOnBack(ChromatogrammModelPtr chroma);
  void ImportChromatogramms();

  void onNewCommand(QUndoCommand* command);
  void onRemarkupCalled(int id, const ChromaSettings& newSettings);

  void onIntervalsModeChanged(bool isIntervalsModeActive);
  void clearView();
  void setViewHeader(QString newHeader);
  void onSubmarkupLoadingRequired(QJsonObject submarkupData);

  // QWidget interface
protected:
  void keyPressEvent(QKeyEvent *event) override;
  void setNewController(MarkupStepController* newController);
  void setupControllerConnections(MarkupStepController* newController);
  void setStepperToController(StepperWidget* stepper, MarkupStepController* controller);

  void onStepForward();

  // void createUndoStack(MarkupStepController* controller);
  QUndoStack *getCurrentUndoStack();
  // void setCurrentUndoStack(MarkupStepController* controller);
  void connectUndoStack(MarkupStepController* controller);
  void disconnectUndoStack(MarkupStepController* controller);

private:
  void onCurrentUndoStackCanRedoChanged(bool canRedo);
  void onCurrentUndoStackCanUndoChanged(bool canUndo);
  void onCurrentUndoStackCleanChanged(bool clean);
  //for future
  // void onCurrentUndoStackIndexChanged(int idx);
  // void onCurrentUndoStackUndoTextChanged(const QString& undoText);
  // void onCurrentUndoStackRedoTextChanged(const QString& redoText);

public slots:
  void onDialogRised();
  void onDialogDestroyed();


protected:
           //debug only
#ifdef RENDERDOC_API_DEBUG
  RENDERDOC_API_1_6_0 *rdoc_api;
#endif

  Ui::MarkupPage* ui;
  //models
  MarkupStepController* mCurrentController = nullptr;
  MarkupStepController* mMainController = nullptr;
  MarkupStepController* mSubController = nullptr;
  // QList<MarkupStepController*> mControllers;

  //
  int VersionId;
  int MarkupId;
  QUuid UploadingOverlayId;

  //// Ui
  QPointer<Widgets::TableMarkerWidget>  MarkersTableWidget;
  QPointer<ReservoirMarkupWidget> MarkupWidget;
  QPointer<IdentificationWidget> IdentificationTabWidget;
  // QPointer<QAction>            UndoAction;
  // QPointer<QAction>            RedoAction;
  QPointer<QAction>            SaveAsAction;
  QPointer<QAction>            SaveAction;
  QPointer<QAction>            RemarkupAction;
  QPointer<QAction>            ImportChromatogrammsAction;
  QPointer<QAction>            ToDataTableAction;
  QPointer<QAction>            TitleModeOrderAction;
  QPointer<QAction>            TitleModeIndexAction;
  QPointer<QAction>            TitleModeRetTimeAction;
  QPointer<QAction>            TitleModeCompoundAction;
  QPointer<QMenu>              ConfigurateMarkupButtonMenu;

private:
  bool loadingIsPending = false;
  bool mFirstShow = false;
  ChromatogrammTitlesDisplayModes mTitlesMode = TitleModeOrder;

};

} //namespace GenesisMarkup

#endif // VIEWPAGEPROJECTMARKUPREFACTOR_H
