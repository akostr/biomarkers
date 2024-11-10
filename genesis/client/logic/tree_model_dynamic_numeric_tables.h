#pragma once

#ifndef TREE_MODEL_DYNAMIC_MERGED_TABLES_H
#define TREE_MODEL_DYNAMIC_MERGED_TABLES_H

#include "tree_model_dynamic.h"

class TreeModelDynamicNumericTables : public TreeModelDynamic
{
	Q_OBJECT
public:
	enum ModelMode {
		viewDefault,
		joinMode,
		exportMode,
		removeMode,
		groupMode
	};
	enum Column
	{
		ColumnTitle,
        //ColumnStatus,
		ColumnChromaCount,
		ColumnComment,
		ColumnDate,

		ColumnLast
	};

	explicit TreeModelDynamicNumericTables(const QString& tableType, QObject* parent = nullptr);
	~TreeModelDynamicNumericTables() = default;

    QString tableType();

	QList<QPair<int, QString>> getTableIdsWithName();
	QList<int> getTableIds();
	int NonGroupItemsCount(TreeModelItem* parent = nullptr);
	QList<TreeModelItem*> GetCheckedItems(TreeModelItem* parent = nullptr);
	void SetChildrenCheckState(Qt::CheckState state, TreeModelItem* parent);
	int GetCheckableCount();
	void SetItemsCheckable(bool state = false);
	int GetItemsCount();

	void enterDefaultMode();
	void enterJoinMode();
	void enterGroupMode();
	void enterExportMode();
	void enterRemoveMode();
	ModelMode mode() const;

public:
	// TreeModelDynamic interface:
	void ApplyContextProject(const QString& dataId, const QVariant& data) override;
	void FetchMore(QPointer<TreeModelItem> item) override;

	// TreeModel interface:
	QList<ActionInfo> GetItemActions(const QModelIndex& index) override;

	// QAbstractItemModel interface
	void fetchMore(const QModelIndex& parent) override;
	bool canFetchMore(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	TreeModelItem* getChildItemBySampleId(int sampleId, TreeModelItem* item, bool recursive = false);
	static bool isItemIsGroup(const TreeModelItem* item);

private:
	void fetchPreprocess(QVariantMap& map);
	void fetchPostprocess(QPointer<TreeModelItem> item);

	TreeModelItem* getChildItemByFileId(int fileId, TreeModelItem* item, bool recursive = false);

private:
	QString TableType;
	QList<ActionInfo> ParentItemNoChildsActions;
	QList<ActionInfo> ParentItemWithChildsActions;
	static inline const QColor DefaultGroupMemberColor = QColor(0, 39, 59, 13);
	QList<ActionInfo> mGroupItemActions;
	ModelMode mMode = viewDefault;
};

#endif
