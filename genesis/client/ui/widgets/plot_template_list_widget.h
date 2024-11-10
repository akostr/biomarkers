#pragma once

#include <QWidget>
#include <QPointer>
#include "logic/tree_model_dynamic_library_templates.h"
#include "logic\structures\library\library_structures.h"

class PlotTemplateModel;
class TreeModelDynamicLibraryCompoundsAdditional;
class TreeModelPresentation;
namespace Views {
class ViewPageLibrary;
}

namespace Ui {
class PlotTemplateListWidget;
}

class PlotTemplateListWidget : public QWidget
{
  Q_OBJECT

public:
enum class Mode
{
  NoneMode,
  ViewMode,
  EditMode,
  AddExistToGroup,
  AddMode,
  CreationPlotMode
};
  explicit PlotTemplateListWidget(QWidget *parent = nullptr);
  ~PlotTemplateListWidget();
  void setPageLibrary(Views::ViewPageLibrary *);
  void setAxesList(QList<Structures::KeyString> keys);
  void setMode(Mode mode);
  void showTemplatesForTable(int tableId);
  QPointer<TreeModelDynamicLibraryTemplates> getPlotListModel();
  Mode getMode() { return m_mode; }
  int getPlotType() const;
  PlotTemplateModel *getPlotTemplateModel() { return m_itemModel; }

signals:
  void nextStep();
  void saveChanges();
  void canceled();
  void disableFiltes(bool);
  void ActionModeTriggered();
  void CancelAccess();

public slots:
  void FilterByState(Structures::LibraryFilterState);
  void disableUiForOpenShare(bool disable);
  void saveEditableTemplate();
  void leaveModifying();

private slots:
  void beginAdding();
  void beginModifying();
  void addExisted();
  void beginSelectedSharing();
  void beginSelectedRemoving();
  void makeSystemic();
  void onPicked(const QVariantMap& item);
  void updateTemplateObjects(int libraryGroupId, bool isOk);

private:
  void setupUi();
  void setupConnections();
  bool checkInputValidity(const QVariantMap& data, QString* msg);
  void sendRequestTemplateObject(int libraryGroupId);

private:
  Ui::PlotTemplateListWidget *ui;
  PlotTemplateModel *m_itemModel;
  TreeModelDynamicLibraryCompoundsAdditional *m_model;
  QPointer<TreeModelPresentation> m_existTemplateProxyModel;
  Mode m_mode;
  qlonglong m_selectedId;
  qlonglong m_tableId;
  QString m_selectedName;
  QAction* m_makeSystemAction;
  QAction* m_shareAccessAction;
  Views::ViewPageLibrary *m_library;
  QString m_groupName;
};
