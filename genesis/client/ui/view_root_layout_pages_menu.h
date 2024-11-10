#pragma once

#include "view.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QLabel>

////////////////////////////////////////////////////
//// Root Layout Pages Menu view class
namespace Ui {
    class ViewRootLayoutPagesMenuUi;
}
class QTreeWidgetItem;
namespace Views
{
//class QTreeWidgetItem;
class ViewRootLayoutPagesMenu : public View
{
  Q_OBJECT

public:
  enum DataRoles
  {
    ViewPageNameRole = Qt::UserRole + 1,
    ViewPageLibTabIndRole
  };

  ViewRootLayoutPagesMenu(QWidget* parent = 0);
  ~ViewRootLayoutPagesMenu();

  void setupUi();
  void loadReservoirMenu();
  void loadBiomarkersMenu();
  void loadPlotsMenu();
  void clearMenu();
  void topLevelItemsInit();
  void loadCommonMenu();

  //// Handle
  virtual void ApplyContextUi(const QString& dataId, const QVariant& data) override;
  virtual void ApplyContextProject(const QString& dataId, const QVariant& data) override;
  virtual void ApplyContextUser(const QString &dataId, const QVariant &data) override;
  virtual void ApplyContextModules(const QString &dataId, const QVariant &data) override;
  void ApplyContextLibrary(const QString &dataId, const QVariant &data) override;

  void setCurrentModule(int module);

public:
  static QIcon nullIcon();


private:
  Ui::ViewRootLayoutPagesMenuUi* ui;
  //// Ui
  QPointer<QHBoxLayout>   mLayout;
  QPointer<QWidget>       mRoot;
  QPointer<View>          mUserToolbar;
  //main menu
  QTreeWidgetItem* mChromasAndFragmentsItem;
  QTreeWidgetItem* mIndexationItem;
  QTreeWidgetItem* mIdentificationItem;
  QTreeWidgetItem* mInterpretationItem;
  QTreeWidgetItem* mEventLogItem;
  QMap<QString, QTreeWidgetItem*> mItemsMap;
  //common menu
  QTreeWidgetItem* mProjectsItem;
  QTreeWidgetItem* mLibraryItem;
  QTreeWidgetItem* mCompoundsItem;
  QTreeWidgetItem* mCoefficientsItem;
  QTreeWidgetItem* mTemplatesItem;
  QMap<QString, QTreeWidgetItem*> mCommonItemsMap;

  QIcon mCollapseDwnIcon;
  QIcon mCollapseUpIcon;
};
}//namespace Views
