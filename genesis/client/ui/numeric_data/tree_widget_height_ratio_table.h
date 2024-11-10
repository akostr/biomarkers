#ifndef TREEWIDGETHEIGHTRATIOTABLE_H
#define TREEWIDGETHEIGHTRATIOTABLE_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QWidget>
#include <QPointer>
#include <QCheckBox>

class TreeModelDynamicNumericTables;
class QTreeView;
class QVBoxLayout;
class QDialogButtonBox;
class QPushButton;

class TreeWidgetHeightRatioTable : public QWidget
{
	Q_OBJECT
public:
	explicit TreeWidgetHeightRatioTable(QWidget* parent = nullptr);

	void ResetTable();

protected:
	void SetupUi();
	void ConnectSignals();
	void UpdateCountLabel();
	void UpdatePickedCountLabel();
	void Export(QList<int> idList);
	void Invalidate(bool toModel);
	void OpenTable(int id) const;
	void EditTable(int id, const QString& title, const QString& comment);
	void ViewTableInfo(int id);
	void HideButtons(bool hide);

	QPointer<QVBoxLayout>           mLayout;
	QPointer<QTreeView>             mTreeView;
	QPointer<TreeModelDynamicNumericTables>  mModel;
	QPointer<QDialogButtonBox>      mActionButtons;
	QPointer<QLabel>                mCountLabel;
	QPointer<QLabel>                m_pickedCountLabel;
	QPointer<QTimer>                refreshTimer;
	QPointer<QCheckBox>             m_pickAllCheckbox;
	QPointer<QPushButton>           ActionsMenuButton;
	QPointer<QAction>				RemoveAction;
	QPointer<QAction>				ExportAction;
};

#endif // TREEWIDGETHEIGHTRATIOTABLE_H
