#ifndef VIEW_PAGE_IDENTIFICATION_PLOT_LIST_H
#define VIEW_PAGE_IDENTIFICATION_PLOT_LIST_H

#include "view.h"

namespace Ui {
class ViewPageIdentificationPlotList;
}

class TreeModelDynamicIdentificationPlots;
class TreeModelPresentation;
class TreeModelItem;

namespace Views
{

class ViewPageIdentificationPlotList : public View
{
  Q_OBJECT

public:
  explicit ViewPageIdentificationPlotList(QWidget *parent = nullptr);
  ~ViewPageIdentificationPlotList();
  virtual void ApplyContextProject(const QString& /*dataId*/, const QVariant& /*data*/) override;

private slots:
  void showPlot(const QModelIndex &index);
  void reloadModel();

  void actionTrigger(const QString& actionId, TreeModelItem* item);
  void editItemTitle(TreeModelItem* item);
  void editGroupTitle(TreeModelItem* item);

  void beginSelectMode();
  void endSelectMode();
  void changeGroup(QPointer<TreeModelItem> selectedItem = nullptr);
  void removeItem(QPointer<TreeModelItem> selectedItem = nullptr);

private:
  void setupUi();
  void setupConnections();
  void updateTableCheckboxStates(const QModelIndex& topLeft, const QModelIndex& bottomRight,
    const QList<int>& roles = QList<int>());

private:
  Ui::ViewPageIdentificationPlotList *ui;
  QPointer<TreeModelDynamicIdentificationPlots> mModel;
  QPointer<TreeModelPresentation> mPresentationModel;
  QAction *mChangeGroup;
  QAction *mRemove;
  QMenu *mButtonMenu;
  int mProjectId = -1;
};

}
#endif // VIEW_PAGE_IDENTIFICATION_PLOT_LIST_H
