#include "tree_model_dynamic_numeric_tables.h"

#include "tree_model_item.h"
#include "api/api_rest.h"
#include "logic/notification.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_json_tag_names.h"

namespace
{
	const QString checkable = "checkable";
	const QString kParentTablesTitle = "_parent_tables_titles";
}

TreeModelDynamicNumericTables::TreeModelDynamicNumericTables(const QString& tableType, QObject* parent)
	: TreeModelDynamic{ parent }
	, TableType(tableType)
{
	//// Setup columns
	Columns[ColumnTitle].Ids[Qt::EditRole] = JsonTagNames::_title;
	Columns[ColumnTitle].Ids[Qt::DisplayRole] = JsonTagNames::_title;
	Columns[ColumnTitle].HeaderData[Qt::DisplayRole] = tr("Title");
	Columns[ColumnTitle].Filter = ColumnInfo::FilterTypeTextSubstring;

	//Columns[ColumnStatus].Ids[Qt::EditRole] = "identification_status";
	//Columns[ColumnStatus].Ids[Qt::DisplayRole] = "identification_status";
	//Columns[ColumnStatus].HeaderData[Qt::DisplayRole] = tr("Identification status");
	//Columns[ColumnStatus].Filter = ColumnInfo::FilterTypeTextSubstring;

	Columns[ColumnChromaCount].Ids[Qt::EditRole] = "_chromatograms_count";
	Columns[ColumnChromaCount].Ids[Qt::DisplayRole] = "_chromatograms_count";
	Columns[ColumnChromaCount].HeaderData[Qt::DisplayRole] = tr("Chromatogramms count");
	Columns[ColumnChromaCount].Filter = ColumnInfo::FilterTypeIntegerRange;

	Columns[ColumnComment].Ids[Qt::EditRole] = JsonTagNames::_comment;
	Columns[ColumnComment].Ids[Qt::DisplayRole] = JsonTagNames::_comment;
	Columns[ColumnComment].HeaderData[Qt::DisplayRole] = tr("Comment");
	Columns[ColumnComment].Filter = ColumnInfo::FilterTypeTextSubstring;

	Columns[ColumnDate].Ids[Qt::EditRole] = "_uploaded";
	Columns[ColumnDate].Ids[Qt::DisplayRole] = "_uploaded";
	Columns[ColumnDate].HeaderData[Qt::DisplayRole] = tr("Upload date");
	Columns[ColumnDate].Filter = ColumnInfo::FilterTypeDateRange;

	//// Setup item actions
	ItemActions =
	{
		/*{ "do_nothing_child", ":/resource/icons/icon_red_warning.png", tr("Do nothing for child") }*/
		//{"editgroup", "", tr("Change group")},
		{"edit", "",/*":/resource/icons/icon_action_edit.png",*/ tr("Edit table")},
		{"export", "",/*":/resource/icons/icon_action_export.png",*/ tr("Export")},
		{"remove", "",/*":/resource/icons/icon_action_delete.png",*/ tr("Remove")}
	};

	ParentItemNoChildsActions =
	{
	  //{"editgroup", "", tr("Change group")},
	  {"edit", "",/*":/resource/icons/icon_action_edit.png",*/ tr("Edit table")},
	  {"export", "",/*":/resource/icons/icon_action_export.png",*/ tr("Export")},
	  {"remove", "",/*":/resource/icons/icon_action_delete.png",*/ tr("Remove")}
	};
	ParentItemWithChildsActions =
	{
	  {"edit", "",/*":/resource/icons/icon_action_edit.png",*/ tr("Edit group name")},
	  //{"editgroup", "", tr("Change group")},
	  {"export", "",/*":/resource/icons/icon_action_export.png",*/ tr("Export")},
	};
	if (TableType == Names::Group::HeigthRatioMatrix)
	{
		//// Setup item actions
		ItemActions =
		{
			/*{ "do_nothing_child", ":/resource/icons/icon_red_warning.png", tr("Do nothing for child") }*/
			{ "info", "", tr("View info")},
			{"edit", "",/*":/resource/icons/icon_action_edit.png",*/ tr("Edit table")},
			{"export", "",/*":/resource/icons/icon_action_export.png",*/ tr("Export")},
			{"remove", "",/*":/resource/icons/icon_action_delete.png",*/ tr("Remove")}
		};

	}
	if (TableType == Names::Group::MergedTable
		|| TableType == Names::Group::ImportedTable)
	{
		ItemActions =
		{
			/*{ "do_nothing_child", ":/resource/icons/icon_red_warning.png", tr("Do nothing for child") }*/
			{"editgroup", "", tr("Change group")},
			{"edit", "",/*":/resource/icons/icon_action_edit.png",*/ tr("Edit table")},
			{"export", "",/*":/resource/icons/icon_action_export.png",*/ tr("Export")},
			{"remove", "",/*":/resource/icons/icon_action_delete.png",*/ tr("Remove")}
		};
		ParentItemNoChildsActions =
		{
			{"editgroup", "", tr("Change group")},
			{"edit", "",/*":/resource/icons/icon_action_edit.png",*/ tr("Edit table")},
			{"export", "",/*":/resource/icons/icon_action_export.png",*/ tr("Export")},
			{"remove", "",/*":/resource/icons/icon_action_delete.png",*/ tr("Remove")}
		};
		ParentItemWithChildsActions =
		{
			//{"edit", "",/*":/resource/icons/icon_action_edit.png",*/ tr("Edit group name")},
			{"editgroup", "", tr("Change group")},
			{"export", "",/*":/resource/icons/icon_action_export.png",*/ tr("Export")},
		};

	}
	Reset();
}

QString TreeModelDynamicNumericTables::tableType()
{
  return TableType;
}

QList<QPair<int, QString>> TreeModelDynamicNumericTables::getTableIdsWithName()
{
	const auto tempList = GetCheckedItems();
	QList<QPair<int, QString> > resultlistPairs;
	std::transform(tempList.begin(), tempList.end(), std::inserter(resultlistPairs, resultlistPairs.end()),
		[](TreeModelItem* item)
		{
			return QPair<int, QString>
			{
				item->GetData(JsonTagNames::_id).toInt(),
					item->GetData(JsonTagNames::_title).toString()
			};
		});
	return resultlistPairs;
}

QList<int> TreeModelDynamicNumericTables::getTableIds()
{
	auto items = GetCheckedItems();
	QList<int > resultlist;
	std::transform(items.begin(), items.end(), std::back_inserter(resultlist),
		[](TreeModelItem* itemInGroup) { return itemInGroup->GetData(JsonTagNames::_id).toInt(); });
	return resultlist;
}

int TreeModelDynamicNumericTables::NonGroupItemsCount(TreeModelItem* parent)
{
	if (!parent)
		parent = Root;
	int counter = 0;
	std::function<void(int&, TreeModelItem*)> walkThrough = [&walkThrough, this](int& counter, TreeModelItem* current)
		{
			if (!current->IsRoot() && !isItemIsGroup(current)) //non root and non group-item
				counter++;
			for (auto& child : current->GetChildren())
				walkThrough(counter, child);
		};
	walkThrough(counter, parent);
	return counter;
}

void TreeModelDynamicNumericTables::ApplyContextProject(const QString& dataId, const QVariant& data)
{
	bool isReset = isDataReset(dataId, data);
	if (isReset)
	{
		Reset();
	}
	else if (dataId == Names::ContextTagNames::ProjectId)
	{
		if (data.toInt())
			Reset();
		else
			Clear();
	}
}

void TreeModelDynamicNumericTables::FetchMore(QPointer<TreeModelItem> item)
{
	if (item->CanFetchMore())
	{
		item->SetFetched(true);

		if (auto projectId = Core::GenesisContextRoot::Get()->ProjectId())
		{
			API::REST::Tables::GetNumericTable(projectId, TableType,
				[item, this](QNetworkReply*, QVariantMap result)
				{
					if (!item)
						return;
					fetchPreprocess(result);

					beginResetModel();
					item->Load(result, false);
					fetchPostprocess(item);
					endResetModel();
				},
				[item](QNetworkReply*, QNetworkReply::NetworkError err)
				{
					if (item)
						item->Clear();
					Notification::NotifyError(tr("Failed to get Merged tables from database"), err);
				}
			);
		}
	}
}

QList<ActionInfo> TreeModelDynamicNumericTables::GetItemActions(const QModelIndex& index)
{
	if (!index.isValid())
		return {};
	if (GetItem(index)->GetParent() == Root)//parent index is ROOT index, top-lvl item
	{
		return GetItem(index)->GetChildren().isEmpty()
			? ParentItemNoChildsActions
			: ParentItemWithChildsActions;
	}
	else//not top-lvl item
	{
		return ItemActions;
	}
	return {};
}

void TreeModelDynamicNumericTables::fetchMore(const QModelIndex& parent)
{
	QPointer<TreeModelItem> parentItem;
	if (parent.column() > 0)
		return;

	if (!parent.isValid())
		parentItem = Root;
	else
		parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

	if (!parentItem->CanFetchMore())
		return;

	FetchMore(parentItem);
}

bool TreeModelDynamicNumericTables::canFetchMore(const QModelIndex& parent) const
{
	QPointer<TreeModelItem> parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = Root;
	else
		parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

	return parentItem->CanFetchMore();
}

QVariant TreeModelDynamicNumericTables::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::BackgroundRole:
		return Root->GetIndex() != index.parent() ? DefaultGroupMemberColor : QVariant();
	case Qt::ToolTipRole:
	{
		if (TableType == Names::Group::MergedTable
			|| TableType == Names::Group::MergedTableIdentified)
		{
			QPointer<TreeModelItem> item = static_cast<TreeModelItem*>(index.internalPointer());
			const auto parentList = item->GetData(kParentTablesTitle).toList();
			QString tooltip(tr("This table selects the following tables:"));
			for (int i = 0; i < parentList.count(); i++)
				tooltip.append(QString("\n %1. %2").arg(i+1).arg(parentList[i].toString()));
			return tooltip;
		}
		break;
	}
	default:
		return TreeModelDynamic::data(index, role);
	}
	return QVariant();
}

void TreeModelDynamicNumericTables::fetchPreprocess(QVariantMap& map)
{
	if (map.contains(JsonTagNames::Children))
	{
		QVariantList children = map[JsonTagNames::Children].toList();
		for (auto& c : children)
		{
			auto row = c.toMap();
			QString status = row["_is_Merged"].toBool() ? tr("Merged") : tr("-");
			row["identification_status"] = status;
			c = row;
		}
		map[JsonTagNames::Children] = children;
	}
}

// TODO refactor this shit
void TreeModelDynamicNumericTables::fetchPostprocess(QPointer<TreeModelItem> item)
{
	if (TableType == Names::Group::MergedTable
		|| TableType == Names::Group::MergedTableIdentified)
	{
		for (auto& child : item->GetChildren())
		{
			const auto parentIdVariant = child->GetData("_tables_group_id");
			if (parentIdVariant.isValid() && !parentIdVariant.isNull())
			{
				const auto parentFileId = parentIdVariant.toInt();
				auto parentItem = getChildItemByFileId(parentFileId, item);
				const auto titleItem = parentItem->GetData(JsonTagNames::_title);
				if (titleItem.isNull() || !titleItem.isValid())
					parentItem->SetData(JsonTagNames::_title, child->GetData("_tables_group_title"));
				const auto idItem = parentItem->GetData(JsonTagNames::_id);
				if (idItem.isNull() || !idItem.isValid())
				{
					parentItem->SetData(JsonTagNames::_id, parentFileId);
					parentItem->SetData("group", true);
				}
				item->MoveChildTo(child->GetRow(), parentItem, false);
			}
		}
	}
	else
	{
		QMultiHash<int, TreeModelItem*> groupedItems;
		for (auto& child : item->GetChildren())
		{
			const auto parentIdVariant = child->GetData("_tables_group_id");
			if (parentIdVariant.isValid() && !parentIdVariant.isNull())
			{
				auto parentFileId = parentIdVariant.toInt();
				auto parentItem = getChildItemByFileId(parentFileId, item);
				const auto titleItem = parentItem->GetData(JsonTagNames::_title);
				if (titleItem.isNull() || !titleItem.isValid())
					parentItem->SetData(JsonTagNames::_title, child->GetData("_tables_group_title"));
				const auto idItem = parentItem->GetData(JsonTagNames::_id);
				if (idItem.isNull() || !idItem.isValid())
				{
					parentItem->SetData(JsonTagNames::_id, parentFileId);
					parentItem->SetData("group", true);
				}
				item->MoveChildTo(child->GetRow(), parentItem, false);
			}
			const auto sampleIdVariant = child->GetData("_sample_id");
			if (sampleIdVariant.isValid() && !sampleIdVariant.isNull())
			{
				auto sampleId = sampleIdVariant.toInt();
				groupedItems.insert(sampleId, child);
			}
		}

		int groupCounter = 0;
		for (auto& group : groupedItems.uniqueKeys())
		{
			auto groupItem = new TreeModelItem(Root, groupCounter);
			auto listGroupedItems = groupedItems.values(group);
			groupItem->SetData(JsonTagNames::_title, listGroupedItems.first()->GetData("_sample_title"));
			groupItem->SetData("group", true);
			for (auto& item : listGroupedItems)
			{
				Root->MoveChildTo(item->GetRow(), groupItem, false);
			}
			++groupCounter;
		}
	}
}

TreeModelItem* TreeModelDynamicNumericTables::getChildItemByFileId(int fileId, TreeModelItem* item, bool recursive)
{
	if (!item)
		item = GetRoot();
	for (auto& child : item->GetChildren())
	{
		if (child->GetData(JsonTagNames::_id).toInt() == fileId)
			return child;
	}
	if (recursive)
	{
		for (auto& child : item->GetChildren())
		{
			auto foundedItem = getChildItemByFileId(fileId, child, true);
			if (foundedItem)
				return foundedItem;
		}
	}
	return new TreeModelItem(GetRoot(), item->GetRow());
}

Qt::ItemFlags TreeModelDynamicNumericTables::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = TreeModel::flags(index);
	const auto item = GetItem(index);
	if (item->GetData("group").toBool())
	{
		auto childs = item->GetChildren();
		const auto count = std::count_if(childs.begin(), childs.end(),
			[](const auto item)
			{
				auto checkableVariant = item->GetData(checkable);
				if (checkableVariant.isNull() && !checkableVariant.isValid())
					return false;
				return !checkableVariant.toBool();
			});
		if (f.testFlag(Qt::ItemIsUserCheckable) && count == childs.count())
		{
			f.setFlag(Qt::ItemIsUserCheckable, false);
		}
	}
	else
	{
		auto checkableVariant = item->GetData(checkable);
		if (checkableVariant.isValid() && !checkableVariant.isNull())
		{
			const auto checkableValue = checkableVariant.toBool();
			if (f.testFlag(Qt::ItemIsUserCheckable) && !checkableValue)
			{
				f.setFlag(Qt::ItemIsUserCheckable, false);
			}
		}
	}
	return f;
}

void TreeModelDynamicNumericTables::SetChildrenCheckState(Qt::CheckState state, TreeModelItem* parent)
{
	beginResetModel();
	std::function<void(TreeModelItem*)> walkThrough = [this, &walkThrough, state](TreeModelItem* current)
		{
			auto index = current->GetIndex();
			if (index.isValid() && flags(index).testFlags(Qt::ItemIsUserCheckable))
				current->SetCheckState(state);
			for (auto& child : current->GetChildren())
				walkThrough(child);
		};
	walkThrough(parent);

	endResetModel();
}

int TreeModelDynamicNumericTables::GetCheckableCount()
{
	if (!Checkable)
		return 0;
	int count = 0;
	for (auto child : GetRoot()->GetChildrenRecursive())
	{
		if (child->GetData(checkable).toBool() && child->IsEmpty())
			count++;
	}
	return count;
}

void TreeModelDynamicNumericTables::SetItemsCheckable(bool state)
{
	if (mMode == joinMode)
	{
		for (auto group : Root->GetChildren())
		{
			group->SetData(checkable, state);
			group->SetCheckState(Qt::Unchecked);
			for (auto child : group->GetChildren())
			{
				child->SetData(checkable, state);
				child->SetCheckState(Qt::Unchecked);
			}
		}
	}
}

void TreeModelDynamicNumericTables::enterDefaultMode()
{
	mMode = viewDefault;
	SetCheckable(false);
	auto list = Root->GetChildrenRecursive();
	for (auto& item : list)
	{
		item->SetCheckState(Qt::Unchecked);
	}
}

void TreeModelDynamicNumericTables::enterJoinMode()
{
	SetCheckable(true);

	auto topLeft = TreeModel::index(0, 0);
	auto bottomRight = TreeModel::index(rowCount(), columnCount());
	emit dataChanged(topLeft, bottomRight, { Qt::CheckStateRole });

	mMode = joinMode;
}

void TreeModelDynamicNumericTables::enterGroupMode()
{
	SetCheckable(true);

	auto topLeft = TreeModel::index(0, 0);
	auto bottomRight = TreeModel::index(rowCount(), columnCount());
	emit dataChanged(topLeft, bottomRight, { Qt::CheckStateRole });

	mMode = groupMode;
}

void TreeModelDynamicNumericTables::enterExportMode()
{
	SetCheckable(true);
	auto topLeft = TreeModel::index(0, 0);
	auto bottomRight = TreeModel::index(rowCount(), columnCount());
	emit dataChanged(topLeft, bottomRight, { Qt::CheckStateRole });

	mMode = exportMode;
}

void TreeModelDynamicNumericTables::enterRemoveMode()
{
	SetCheckable(true);
	mMode = removeMode;
}

TreeModelDynamicNumericTables::ModelMode TreeModelDynamicNumericTables::mode() const
{
	return mMode;
}

QList<TreeModelItem*> TreeModelDynamicNumericTables::GetCheckedItems(TreeModelItem* parent)
{
	if (!parent)
		parent = Root;
	std::function<void(QList<TreeModelItem*>&, TreeModelItem*)> getCheckedChildren = [this, &getCheckedChildren](QList<TreeModelItem*>& list, TreeModelItem* item)
		{
			if (!item)
				return;
			const auto isGroup = isItemIsGroup(item);
			if (item != Root && item->GetCheckState() == Qt::Checked
				&&  (!isGroup))
				list.append(item);
			for (auto& child : item->GetChildren())
				getCheckedChildren(list, child);
		};
	QList<TreeModelItem*> ret;
	getCheckedChildren(ret, parent);
	return ret;
}

int TreeModelDynamicNumericTables::GetItemsCount()
{
	int count = 0;
	for (auto& item : Root->GetChildrenRecursive())
		count++;
	return count;
}

TreeModelItem* TreeModelDynamicNumericTables::getChildItemBySampleId(int sampleId, TreeModelItem* item, bool recursive)
{
	if (!item)
		item = GetRoot();
	for (auto& child : item->GetChildren())
	{
		if (child->GetData("_sample_id").toInt() == sampleId)
			return child;
	}
	if (recursive)
	{
		for (auto& child : item->GetChildren())
		{
			auto foundedItem = getChildItemBySampleId(sampleId, child, true);
			if (foundedItem)
				return foundedItem;
		}
	}
	return nullptr;
}

bool TreeModelDynamicNumericTables::isItemIsGroup(const TreeModelItem* item)
{
	if (!item)
		return false;
	const auto isGroup = item->GetData("group");
	return isGroup.isValid() && !isGroup.isNull() && isGroup.toBool();
}