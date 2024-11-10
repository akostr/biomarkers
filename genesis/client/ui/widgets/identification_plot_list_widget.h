#ifndef IDENTIFICATION_PLOT_LIST_WIDGET_H
#define IDENTIFICATION_PLOT_LIST_WIDGET_H

#include <QWidget>
#include "logic/structures/common_structures.h"
#include "edit_group_widget.h"
#include "ui/dialogs/templates/dialog.h"

namespace Ui {
class IdentificationPlotListWidget;
}

using namespace Structures;

class TreeModelDynamicLibraryTemplates;
class IdentificationPlotWidget;
class DialogLineEditEx;
class DialogTextEditEx;

namespace Widgets {
class EditGroupWidget;
}

namespace Models
{
class AnalysisDataTableModel;
}


class IdentificationPlotListWidget : public QWidget
{
  Q_OBJECT

public:
  enum Mode
  {
    CreateMode,
    OpenMode
  };

  explicit IdentificationPlotListWidget(QWidget *parent = nullptr);
  ~IdentificationPlotListWidget();

  void createPlots(QList<int> plotTemplateIds,
                   QPointer<TreeModelDynamicLibraryTemplates> templateListModel,
                   int tableId,
                   QString dataType);

  void openPlot(int plotId);
  void openPlot(const QJsonObject &obj);
  void clear();

  int plotCount();

signals:
  void showPreviousPage();
  void needToReloadTable();
  void loadedPlot(int plotId, QJsonObject obj);

protected:
  virtual void resizeEvent(QResizeEvent *event) override;

private slots:
  void saveChanges();
  void beginSaveMode();
  void savePlots();
  void beginExportMode();
  void exportPlots();
  void leaveMode();

  void remove();
  void openEditTitleDialog();
  void openSaveAsNewDialog();
  void selectAll(bool);
  void selectedItemChanged();
  void updateHeight(int newHeight, bool applyForAll);
  void updateSplitter(int pos, int index);
  void updateSizes();
  void checkSizes();

private:
  void setupUi();
  void setupConnections();
  QJsonObject generateJson(bool ignorePlotId = false);
  void showCheckBoxes(bool isShow);
  void setMode(Mode mode);
  void loadEntity(int tableId, QList<int> libraryGroupIds);

  QWidget* createTitleWidgetForSaveDialog(QList<std::tuple <IdentificationPlotWidget*, DialogLineEditEx*, DialogTextEditEx*>> &controls);
  Widgets::EditGroupWidget *createGroupWidgetForSaveDialog();

private:
  Ui::IdentificationPlotListWidget *ui;
  QAction *mSaveAction;
  QAction *mSaveAsNewAction;
  QAction *mEditTitleAction;
  QAction *mRemoveAction;
  QAction *mCreateAction;
  QAction *mExportAction;
  QMenu *mMenu;
  QList <IdentificationPlotWidget*> mPlotList;
  QList <int> mSplitterSized;
  int mTableId = -1;
  Mode mMode;
  int mGroupId = -1;
  QString mGroupTitle;
  bool mSaveAsNew;
};


#endif // IDENTIFICATION_PLOT_LIST_WIDGET_H
