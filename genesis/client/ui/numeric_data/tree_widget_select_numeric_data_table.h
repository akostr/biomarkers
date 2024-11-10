#ifndef TREEWIDGETSELECTNUMERICDATATABLE_H
#define TREEWIDGETSELECTNUMERICDATATABLE_H
#include <QLabel>
#include <QWidget>
#include <QPointer>
#include <QCheckBox>

class TreeModelDynamicNumericTables;
class TreeModelPresentation;
class QTreeView;
class QVBoxLayout;
class QDialogButtonBox;
class QPushButton;
class QAction;

class TreeWidgetSelectNumericDataTable: public QWidget
{
    Q_OBJECT
  public:

    explicit TreeWidgetSelectNumericDataTable(QWidget *parent = 0);

  protected:
    void SetupUi();
    void ConnectActions();

    void UpdateCountLabel();
    void UpdatePickedCountLabel();
    void Export(QList<int> idList);
    void Invalidate(bool toModel);
    void OpenTable(int id);
    void RemoveTable(const QPair<int, QString>& data);
    void EditTable(int id, const QString& title, const QString& comment);
    void HideMenuButtons(bool hide);
    void disableCheckableNotInParent(const QModelIndex& topLeft, const QModelIndex& bottomRight,
      const QList<int>& roles = QList<int>());

    QPointer<QVBoxLayout>           mLayout;
    QPointer<QTreeView>             mTreeView;
    QPointer<TreeModelDynamicNumericTables>  mModel;
    QPointer<TreeModelPresentation> mPresentationModel;
    QPointer<QDialogButtonBox>      mActionButtons;
    QPointer<QLabel>                mCountLabel;
    QPointer<QLabel>                m_pickedCountLabel;
    QPointer<QTimer>                refreshTimer;
    QPointer<QCheckBox>             m_pickAllCheckbox;

    QPointer<QPushButton>           m_ActionMenuButton;
    QPointer<QAction>               m_JoinAction;
    QPointer<QAction>               m_ExportAction;
    QPointer<QAction>               m_RemoveAction;
};

#endif // TREEWIDGETSELECTNUMERICDATATABLE_H
