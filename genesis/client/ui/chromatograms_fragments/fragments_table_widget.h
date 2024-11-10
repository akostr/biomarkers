#ifndef FRAGMENTSTABLEWIDGET_H
#define FRAGMENTSTABLEWIDGET_H

#include <QWidget>
#include <QPointer>

class TreeModelDynamicProjectFragments;
class QLabel;
class TreeModelItem;
class QDialogButtonBox;
class QSortFilterProxyModel;
class QCheckBox;
class QPushButton;
class QTreeView;

class FragmentsTableWidget : public QWidget
{
  Q_OBJECT
public:
  explicit FragmentsTableWidget(QWidget *parent = nullptr);

  void setScrollPosition(int pos);
  void ExitAction();

private:
  void SetupUi();
  void connectModelAndPickAllCheckBox();
  void disconnectModelAndPickAllCheckBox();
  void UpdateFragmentsCountLabel();
  void UpdatePickedFragmentsCountLabel();

  void toggleCollapsing();

  //picked actions callbacks:
  void PickedMarkupGroupAction();
  void PickedExportGroupAction();
  void PickedPassportGroupAction();
  void PickedRemoveGroupAction();

  void ActionAssignAndEnter(QString buttonText, QString forWhatAction,
                            std::function<void()> func,
                            std::function<void()> prepare = [](){},
                            std::function<void()> done = [](){});

  //items actions functions:
  void ItemViewSamplePassport(TreeModelItem *item);
  void ItemEditMZ(TreeModelItem *item);
  void ItemExport(TreeModelItem *item);
  void ItemDelete(TreeModelItem* item);

  void activatePassportsDialogAction(QList<int> filesIds, bool isInGroupAction = false);
  void addMemoryScrollbarPosition();
  void updateScrollPosition();
  void invalidate();
  void onPickAllCheckboxStateChecked(int state);
  void onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
    const QVector<int>& roles = QVector<int>());


private:
  QPointer<QPushButton>                      m_actionsMenuButton;
  QPointer<QTreeView>                        m_treeView;
  QPointer<TreeModelDynamicProjectFragments> mModel;
  QPointer<QSortFilterProxyModel>            m_sortProxyModel;
  QPointer<QLabel>                           m_pickedFragmentsCountLabel;
  QPointer<QLabel>                           m_fragmentsCountLabel;
  QPointer<QDialogButtonBox>                 m_actionButtons;
  QPointer<QCheckBox>                        m_pickAllCheckbox;
  QMetaObject::Connection                    m_actionConnection;
  QString                                    m_forWhatActionText;
  std::function<void()>                      m_doneAction;
  bool mCollapsed = false;
  int  m_scrollbarPosition = 0;
};

#endif // FRAGMENTSTABLEWIDGET_H
