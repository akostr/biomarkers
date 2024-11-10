#ifndef CHROMATOGRAMWIDGETREFACTOR_H
#define CHROMATOGRAMWIDGETREFACTOR_H

#include <QWidget>
#include <QMap>
#include <QLabel>
#include <QToolBar>
#include <QPointer>
#include <QUndoCommand>

#include <logic/markup/genesis_markup_forward_declare.h>
#include "logic/markup/markup_step_controller.h"

class QVBoxLayout;
class QHBoxLayout;
class QScrollBar;
class ChromatogramPlot;
class GPRange;

namespace Ui{
class ChromatogrammUi;
}
namespace GenesisMarkup{

class GPStackedWidget;

class ChromatogramWidgetRefactor : public QWidget
{
  Q_OBJECT
public:
  ChromatogramWidgetRefactor(ChromaId id, int pixelHeight, QWidget* parent = nullptr);
  ~ChromatogramWidgetRefactor();
  void setMarkupModel(MarkupModelPtr markup, const StepInfo &modelStepInteractions);
  void swapChromatogramm(ChromatogrammModelPtr newPtr);
  GPRange dataRangeX();
  GPRange dataRangeY();
  GPRange xAxisRange();
  QPair<GPRange, GPRange> getBoundings();

  int id() const;

  QPointer<ChromatogramPlot> getPlot() const;
  QPointer<GPStackedWidget> getStackedPlotWidget() const;


public slots:
  void setXAxisRange(const GPRange &newRange);
  void setYAxisRange(const GPRange &newRange);
  void setName(const QString& name);
  void setInteractionDisabled(bool disabled);
  void setLocked(bool locked);

signals:
  void newCommand(QUndoCommand* command);
  void xAxisRangeChanged(const GPRange &newRange);
  void yAxisRangeChanged(const GPRange &newRange);
  void remarkupCalled(int id, const GenesisMarkup::ChromaSettings& newSettings);
  void intervalsApplyingRequested(QList<GenesisMarkup::ChromaId> chromaIds,
                                  QMap<QUuid, GenesisMarkup::ChromaSettings> intervalsSettings,
                                  QMap<QUuid, QPair<double, double>> deprecationIntervals,
                                  GenesisMarkup::ChromaSettings chromaSettings,
                                  GenesisMarkup::Steps desiredStep);
  void intervalsModeStateChanged();

  void BackgroundColorChanged();

protected:
  void setupToolBar();
  void onMarkupDataChanged(uint role, QVariant newValue);
  void paintEvent(QPaintEvent *event) override;
  void setPlotInternalToolbarHidded(bool hidden);
  void onExitIntervals();
  void onApplyIntervals();
  void onChromatogramModeChanged(ChromatogrammModes newMode);
  void onBurgerMenuCalled();
  void handleStepInteractions(MarkupModelPtr markup, const GenesisMarkup::StepInfo& newModelStepInteractions);
  void handleStepInteractions(MarkupModelPtr markup);
  void leaveEvent(QEvent* event) override;
  void enterEvent(QEnterEvent* event) override;

  int mId;
  double Scale;
  double IndividualXAxisRange;

  QPointer<QHBoxLayout>       PlotInternalToolbarLayout;
  QPointer<QAction>           RemoveAction;
  QPointer<ChromatogramPlot>  Plot;

  MarkupModelPtr mMarkupModel;

  QMetaObject::Connection connectionToMarkupModel;
  QMetaObject::Connection connectionToChromaModel;
  StepInfo mStepInteractions;

  double mNormalizeCoefficient = 1;

  Ui::ChromatogrammUi* ui;

  static QIcon mActiveMasterIcon;
  static QIcon mInactiveMasterIcon;
  static QIcon mUnlockedActionsIcon;
  static QIcon mLockedActionsIcon;
  static QIcon mSettingsIcon;
  static QIcon mInfoIcon;
  static QIcon mRemoveIcon;
  static bool  mIconsInitialized;

  QColor mBackgroundColor;

  // QObject interface
public:
  // bool eventFilter(QObject *watched, QEvent *event) override;
  QColor BackgroundColor() const;
  void setBackgroundColor(const QColor &newBackgroundColor);
private:
  Q_PROPERTY(QColor BackgroundColor READ BackgroundColor WRITE setBackgroundColor NOTIFY BackgroundColorChanged FINAL)
};

}//GenesisMarkup
#endif // CHROMATOGRAMWIDGETREFACTOR_H
