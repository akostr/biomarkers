#ifndef IDENTIFICATION_PLOT_WIDGET_H
#define IDENTIFICATION_PLOT_WIDGET_H

#include <QWidget>
#include <QPointer>

#include "logic/models/analysis_entity_model.h"

class PlotTemplateModel;
class Tumbler;
class WebOverlay;

namespace Ui {
class IdentificationPlotWidget;
}

class TreeModelDynamicLibraryTemplates;

namespace Models
{
class AnalysisDataTableModel;
}

class IdentificationPlotWidget : public QWidget
{
  Q_OBJECT

public:
  explicit IdentificationPlotWidget(QWidget *parent = nullptr);
  ~IdentificationPlotWidget();
  void setData(int templateId,
               QPointer<TreeModelDynamicLibraryTemplates> templateListModel,
               int tableId,
               QString dataType);
  void setData(const QJsonObject &obj);

  QJsonObject saveToJson(bool ignorePlotId);
  bool saveToFileSystem(const QString& path);

  bool isSelectable();
  void setSelectable(bool);
  bool isSelected();
  void setSelected(bool);
  QString title();
  void setTitle(const QString& title);
  QString header();
  void setHeader(const QString& header);
  QString comment();
  void setComment(const QString& comment);

  int getTemplateId() { return mGroupElementId; }
  int getPlotId() { return mPlotId; }

public slots:
  void updatePlotSize();

signals:
  void needToUpdateHeight(int height, bool updateAll);
  void titleChanged(const QString& title);
  void selectChanged(int);

protected:
  virtual void resizeEvent(QResizeEvent *event) override;

private slots:
  void onEntityChanged(EntityType type,
                       TEntityUid eId,
                       DataRoleType role,
                       const QVariant& value);
  void onGroupColorChanged(QSet<TEntityUid> affectedEntities);
  void onGroupShapeChanged(QSet<TEntityUid> affectedEntities);

  void onGroupEditRequested(const QUuid& uid);
  void removeGroupAndRegroup(const TEntityUid& groupUid);
  void onModelReset();

private:
  void setupUi();
  void setupConnections();

  void setTemplate(QPointer<TreeModelDynamicLibraryTemplates> templateListModel);
  void setEntity(QJsonObject obj, int libraryGroupIdForX, int libraryGroubIdForY);
  void openHeaderDialog();
  void openLegendDialog();
  void openHeigtDialog();
  void openLegendPositionDialog();
  void showLegendBySettings(bool applyAll, Qt::Alignment align, QInternal::DockPosition position);
  void showLegendWidgets(bool show);

private:
  Ui::IdentificationPlotWidget *ui;
  QPointer<PlotTemplateModel> mModel;
  QPointer<AnalysisEntityModel> mEntityModel;

  bool mApplyToAllPlots = false;
  QInternal::DockPosition mLegendPosition = QInternal::TopDock;
  Qt::Alignment mLegendAlign = Qt::AlignLeft;
  int mGroupElementId;
  int mElementId;
  QString mComment;
  QString mTitle;
  int mPlotId = -1;
  QMenu *mMenu;
  QAction *mEditHeader;
  QAction *mEditLegendPosition;
  QAction *mEditHeight;
  QAction *mEditLegend;

  Tumbler *mShowHeaderTumbler;
  Tumbler *mShowLegendTumbler;
  Tumbler *mShowGridTumbler;
};

#endif // IDENTIFICATION_PLOT_WIDGET_H
