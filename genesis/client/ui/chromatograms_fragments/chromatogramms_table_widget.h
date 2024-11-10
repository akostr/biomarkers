#ifndef CHROMATOGRAMMSTABLEWIDGET_H
#define CHROMATOGRAMMSTABLEWIDGET_H

#include <QWidget>
#include <QPointer>

class QCheckBox;
class QAbstractItemModel;
class QLabel;
class TreeModelDynamicProjectChromatogramms;
class QDialogButtonBox;
class QSortFilterProxyModel;
class TreeModelItem;
class QPushButton;
class QTreeView;

class ChromatogrammsTableWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ChromatogrammsTableWidget(QWidget *parent = nullptr);
  void Reload();

  void setScrollPosition(int pos);
  void ExitAction();

private:
  void SetupUi();

  void toggleCollapsing();

  //ui update callbacks:
  void UpdateChromatogrammCountLabel();
  void UpdatePickedChromatogramsCountLabel();

  //picked actions callbacks:
  void PickedMarkupGroupAction();
  void PickedIonsExtractGroupAction();
  void PickedChangeGroupingAction();
  void PickedCopyToOtherProjectGroupAction();
  void PickedExportGroupAction();
  void PickedRemoveGroupAction();
  void PickedPassportsGroupAction();

  void ActionAssignAndEnter(QString buttonText, QString forWhatAction,
                            std::function<void()> func,
                            std::function<void()> prepare = [](){},
                            std::function<void()> done = [](){});


  //items actions functions:
  void ItemExtractIons(TreeModelItem *item);
  void ItemChangeGrouping(TreeModelItem *item);
  void ItemCopyToOtherProject(TreeModelItem *item);
  void ItemViewSamplePassport(TreeModelItem *item);
  void ItemExport(TreeModelItem *item);
  void ItemDelete(TreeModelItem *item);

  //group-items actions functions:
  void ItemGroupDelete(TreeModelItem *item);
  void ItemGroupRename(TreeModelItem *item);
  void ItemGroupExport(TreeModelItem *item);
  void ItemGroupEdit(TreeModelItem* item);

  void activatePassportsDialogAction(QList<int> filesIds, bool isInGroupAction = false);

  void addMemoryScrollbarPosition();
  void updateScrollPosition();
  void ShowDialogForGrouping(const QList<int>& ids, const std::map<QString, int>& groupInfo);

  QPointer<QPushButton>                           m_actionsMenuButton;
  QPointer<QTreeView>                             m_treeView;
  QPointer<TreeModelDynamicProjectChromatogramms> m_model;
  QPointer<QSortFilterProxyModel>                 m_sortProxyModel;
  QPointer<QDialogButtonBox>                      m_actionButtons;
  QPointer<QLabel>                                m_chromatogrammsCountLabel;
  QPointer<QLabel>                                m_pickedChromatogrammsCountLabel;
  QPointer<QCheckBox>                             m_pickAllCheckbox;
  QMetaObject::Connection                         m_actionConnection;
  QString                                         m_forWhatActionText;
  std::function<void()>                           m_doneAction;
  bool mCollapsed = false;
  int  m_scrollbarPosition = 0;
};

#endif // CHROMATOGRAMMSTABLEWIDGET_H
