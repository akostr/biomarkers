#ifndef CHROMATOGRAMPLOTCONCEPT_H
#define CHROMATOGRAMPLOTCONCEPT_H

#include "graphicsplot/graphicsplot.h"
#include "logic/markup/markup_step_controller.h"
#include <logic/markup/genesis_markup_forward_declare.h>
#include <logic/markup/chromatogram_data_model.h>

#include <graphicsplot/graphicsplot_extended.h>


class IPlotController;

class LayerableContainer : public QObject
{//class for protection our QHash, that has not owned by itself object pointers
 //and cleaned automatically, when the objects destroyed
  Q_OBJECT
public:
  explicit LayerableContainer(QObject* parent = nullptr) : QObject(parent){};
  ~LayerableContainer() = default;

  bool add(const QUuid& uid, GPLayerable* ptr);
  bool hasSelected();
  GPLayerable* get(const QUuid& uid) const;
  void remove(const QUuid& uid);
  void removeAndDelete(const QUuid& uid);
  void clearAndDelete();
  bool contains(const QUuid& uid) const {return mHash.contains(uid);}
  const QHash<QUuid, GPLayerable*>& getHash() const {return mHash;}

private:
  void onLayerableDeleted();
  QHash<QUuid, GPLayerable*> mHash;
};

class ChromatogramPlot: public GraphicsPlot

{
  Q_OBJECT
public:
  ChromatogramPlot(QWidget *parent = nullptr);

  void setInteractionsLocked(bool locked);

  void setChromatogrammModel(GenesisMarkup::ChromatogrammModelPtr model, const GenesisMarkup::StepInfo& modelStepInteractions);

  QPair<GPRange, GPRange> boundingRange();
  const GPRange &xDataRange() const;
  const GPRange &yDataRange() const;

  void replotOnVisibleViewPort(GraphicsPlot::RefreshPriority rp = GraphicsPlot::rpQueuedReplot);

  const GenesisMarkup::StepInfo &stepInteractions() const;
  bool isMaster() const;

  GenesisMarkup::ChromatogrammModelPtr model() const;

  bool setBLSelectionMode(bool enabled);

public slots:
  void ensureAllCurvesVisible();
  void onEntityChanged(EntityType type,
                     TEntityUid eId,
                     DataRoleType role,
                     const QVariant& value);

  void onEntityResetted(EntityType type,
                      TEntityUid eId,
                      AbstractEntityDataModel::ConstDataPtr peakData);

  void onEntityAdded(EntityType type,
                   TEntityUid eId,
                   AbstractEntityDataModel::ConstDataPtr peakData);

  void onEntityAboutToRemove(EntityType,
                     TEntityUid eId);
  void onMarkupDataChanged(uint role, const QVariant& data);
  void onModelAboutToReset();
  void onModelReset();
  void clearLayerables();

signals:
  void newCommand(QUndoCommand* command);
  void repaintedVisible();
  void intervalsApplyingRequested(QList<GenesisMarkup::ChromaId> chromaIds,
                                  QMap<QUuid, GenesisMarkup::ChromaSettings> intervalsSettings,
                                  QMap<QUuid, QPair<double, double>> deprecationIntervals,
                                  GenesisMarkup::ChromaSettings chromaSettings,
                                  GenesisMarkup::Steps desiredStep);

protected slots:
  void customUpdate(GraphicsPlot::RefreshPriority rp = GraphicsPlot::rpQueuedReplot);
protected:
  // using GraphicsPlot::replot;
  void setupUiSettings();
  void connectModelSignals(GenesisMarkup::ChromatogrammModelPtr model);

  Qt::CursorShape GetCursorAt(const QPointF &pos);

  GenesisMarkup::ChromatogrammModelPtr mModel;
  GenesisMarkup::StepInfo mStepInteractions;
  GPRange mXDataRange;
  GPRange mYDataRange;
  LayerableContainer mLayerables;
  GPEXDraggingInterface* DraggingObject;

  QPointer<QMenu>           contextMenu;

  GenesisMarkup::ChromatogrammModes mMode;
  bool contextMenuLocked;
  QUuid mTemporaryPeakUid;
  bool baseLineInteractionsMode;

  // QWidget interface
protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;
  virtual void FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions);
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void saveImage();
  void copyImage();
  void handleStepInteractions(const GenesisMarkup::StepInfo& newModelStepInteractions);
  void handleStepInteractions();
  
  void paintEvent(QPaintEvent *event) override;

};



#endif // CHROMATOGRAMPLOTCONCEPT_H
