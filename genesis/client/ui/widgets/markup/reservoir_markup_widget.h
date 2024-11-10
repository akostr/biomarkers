#ifndef RESERVOIR_MARKUP_WIDGET_H
#define RESERVOIR_MARKUP_WIDGET_H

#include <QWidget>
#include <QPointer>
#include <logic/markup/genesis_markup_forward_declare.h>
#include <logic/known_context_tag_names.h>

class QScrollBar;
class QTabWidget;
class IMarkupTab;
class QScrollArea;
class QToolBar;
class QAction;
class QUndoCommand;
class QVBoxLayout;
class QHBoxLayout;
class QSpacerItem;
class GPRange;
namespace GenesisMarkup
{
class MarkupStepController;
class MarkupDataModel;
class ChromatogramWidgetRefactor;
using ChromatogramsWidgetsMap = QHash<MarkupModelPtr, QHash<ChromaId, QPointer<ChromatogramWidgetRefactor>>>;

class ReservoirMarkupWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ReservoirMarkupWidget(QWidget *parent = nullptr);
  ~ReservoirMarkupWidget();
  void addTab(IMarkupTab* tab, const QString &title);
  void removeTab(int index);
  void removeTab(IMarkupTab* tab);
  void setMainMarkupModel(MarkupModelPtr model, const StepInfo& stepInfo);
  void setSubMarkupModel(MarkupModelPtr model, const StepInfo& stepInfo);
  void setPlotRanges(const QPair<GPRange, GPRange>& ranges);
  void setModule(Names::ModulesContextTags::Module module);
  void setChromaHeight(int height);
  int chromaHeight();
  QPair<GPRange, GPRange> wholeDataRange();
  void clear();
  // ChromatogramWidgetRefactor* getFirstChroma();

public slots:
  void onActionsStateChanged(bool enabled);
  void onUndoActionStateChanged(bool enabled);
  void onRedoActionStateChanged(bool enabled);

private:
  void setupUi();

protected:
  QPointer<QTimer> mVScrollBarTimer;
  QPointer<QTimer> mXAxisUptateTimer;
  QPointer<QTimer> mYAxisUptateTimer;

  QPointer<QWidget> mPlotsAreaWidget;
  QPointer<QWidget> mPlotsWidget;
  QPointer<QWidget> mPlotsToolBarOverlay;
  QPointer<QVBoxLayout> mPlotsAreaWidgetLayout;
  QPointer<QVBoxLayout> mPlotsScrollAreaWidgetLayout;
  QPointer<QVBoxLayout> mPlotsWidgetLayout;
  QPointer<QHBoxLayout> mMasterChromaLayout;
  QPointer<QScrollArea> mPlotsScrollArea;
  QPointer<QToolBar> mPlotsPageToolBar;
  QPointer<QAction> mUndoAction;
  QPointer<QAction> mRedoAction;
  QPointer<QTabWidget> mTabWidget;
  QPointer<QScrollBar> mCommonScrollBar;

  QSpacerItem* mMasterRightSpacer;

  QMap<MarkupModelPtr, StepInfo> mSteps;
  MarkupModelPtr mMainMarkupModel;
  MarkupModelPtr mSubMarkupModel;
  MarkupModelPtr mCurrentMarkupModel;
  ChromatogramsWidgetsMap mChromatogrammsWidgets;
  QScopedPointer<GPRange> mWholeDataRangeX;
  QScopedPointer<GPRange> mWholeDataRangeY;
  QScopedPointer<GPRange> mXAxisViewportRange;
  QScopedPointer<GPRange> mYAxisViewportRange;

  QList<IMarkupTab*> mAdditionalTabs;
  int mPlotPixelHeight;
  double mScrollNormalizeCoefficient;
  ChromatogramWidgetRefactor* mRangeUpdater;

protected:
  ChromatogramWidgetRefactor* makeChomatogramWidget(ChromaId id);
  void setMarkupModel(MarkupModelPtr &dst, MarkupModelPtr src, const StepInfo& stepInfo);

signals:
  void intervalsModeChanged(bool isIntervalsModeActive);
  void newCommand(QUndoCommand* cmd);
  void remarkupCalled(int id, const ChromaSettings& newSettings);

protected slots:
  void onOpenGlUpdateRequested();
  void onXAxisRangeChanged(const GPRange &newRange);
  void onXAxisRangeChangedTimeout();
  void onYAxisRangeChanged(const GPRange &newRange);
  void onYAxisRangeChangedTimeout();
  void onChromaVScrollBarVisibilityChanged(bool isVisible);
  void onUndo();
  void onRedo();
  void onMasterChanged();
  void updateIntervalsMode();
  void onStartupLoadPlots();
  void sortWidgets();
  void onScrollBarMoved();
  void updateCommonScrollbar();


  // QWidget interface
protected:
  void resizeEvent(QResizeEvent *event) override;
};
}//namespace GenesisMarkup

#endif // RESERVOIR_MARKUP_WIDGET_H
