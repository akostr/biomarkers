#ifndef TREEWIDGETJOINEDTABLES_H
#define TREEWIDGETJOINEDTABLES_H

#include <QWidget>
#include <QPointer>

class TreeModelDynamicNumericTables;
class QTreeView;
class QLabel;
class QDialogButtonBox;
class QCheckBox;
class QTimer;

class TreeWidgetJoinedTables : public QWidget
{
	Q_OBJECT
public:
	explicit TreeWidgetJoinedTables(QWidget* parent = nullptr);

protected:
	void SetupUi();
	void UpdateCountLabel();
	void UpdatePickedCountLabel();
	void Export(QList<int> idList);
	void ShowEditGroupDialog(const QList<int>& ids, const std::map<QString, int>& groupInfo);

	QPointer<QTreeView>             mTreeView;
	QPointer<TreeModelDynamicNumericTables> mModel;
	QPointer<QDialogButtonBox>      mActionButtons;
	QPointer<QLabel>                mCountLabel;
	QPointer<QLabel>                mPickedCountLabel;
	QPointer<QTimer>                mRefreshTimer;
	QPointer<QCheckBox>             mPickAllCheckbox;

};

#endif // TREEWIDGETJOINEDTABLES_H
