#ifndef TREE_WIDGET_IMPORTED_TABLES_H
#define TREE_WIDGET_IMPORTED_TABLES_H

#include <QWidget>
#include <QPointer>

namespace Ui {
class TreeWidgetImportedTables;
}
class TreeModelDynamicNumericTables;
class TreeModelItem;
class QLabel;

class TreeWidgetImportedTables : public QWidget
{
  Q_OBJECT

public:
  explicit TreeWidgetImportedTables(QWidget *parent = nullptr);
  ~TreeWidgetImportedTables();

private:
  void SetupUi();

  //group actions:
  void ActionAssignAndEnter(QString buttonText, QString forWhatAction,
                            std::function<void()> func,
                            std::function<void()> prepare = [](){},
                            std::function<void()> done = [](){});
  void ExitAction();

  void PickedEditGroupAction();
  void ShowEditGroupDialog(const QList<int>& ids, const std::map<QString, int>& groupInfo);
  void PickedExportGroupAction();
  void PickedRemoveGroupAction();
  void UpdateCountLabel();


  //items actions functions:
  void ItemEdit(TreeModelItem* item);
  void ItemExport(TreeModelItem* item);
  void ItemRemove(TreeModelItem* item);
  void EditGroup(TreeModelItem* item);

  void EditTable(TreeModelItem* item);
  void EditGroupTitle(TreeModelItem* item);

  void UpdateMainCheckBox(QString forWhat);
  void OnDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>());
  void UpdateLabelAndCheckbox();


private:
  Ui::TreeWidgetImportedTables             *ui;
  QPointer<TreeModelDynamicNumericTables> mModel;
  QMetaObject::Connection                  mActionConnection;
  std::function<void()>                    mDoneAction;
};

#endif // TREE_WIDGET_IMPORTED_TABLES_H