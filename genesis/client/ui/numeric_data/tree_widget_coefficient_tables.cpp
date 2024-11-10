#include "tree_widget_coefficient_tables.h"

#include "logic/tree_model_dynamic_numeric_tables.h"
#include "logic/tree_model_item.h"
#include "logic/notification.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "api/api_rest.h"
#include "ui/genesis_window.h"
#include "ui/known_view_names.h"
#include "ui/itemviews/tableview_model_actions_column_delegate.h"
#include "ui/itemviews/tableview_headerview.h"
#include "ui/itemviews/datatable_item_delegate.h"
#include "ui/dialogs/templates/tree_confirmation.h"
#include "ui/dialogs/templates/save_edit.h"
#include "ui/dialogs/web_dialog_import_xls_file.h"
#include "ui/dialogs/web_dialog_group_editor.h"
#include "ui/dialogs/web_dialog_join_tables.h"
#include "genesis_style/style.h"

#include "ui_tree_widget_coefficient_tables.h"

#include "ui/dialogs/table_rename_dialog.h"

#include <QMenu>
#include <QFileDialog>
#include <QRegExp>

using namespace Core;
using namespace Dialogs;

using ModelClass = TreeModelDynamicNumericTables;

TreeWidgetCoefficientTables::TreeWidgetCoefficientTables(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::TreeWidgetCoefficientTables)
{
	SetupUi();
	ConnectSignals();
}

TreeWidgetCoefficientTables::~TreeWidgetCoefficientTables()
{
	delete ui;
}

void TreeWidgetCoefficientTables::SetupUi()
{
	ui->setupUi(this);
	setStyleSheet(Style::Genesis::GetUiStyle());
	mModel = new TreeModelDynamicNumericTables(Names::Group::CoefficientTableIdentified, this);

	ui->treeView->setModel(mModel->GetPresentationModel());
	ui->treeView->setProperty("dataDrivenColors", true);
	ui->treeView->setHeader(new TableViewHeaderView(Qt::Horizontal, ui->treeView));
	ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->treeView->header()->setStretchLastSection(true);
	ui->treeView->setSortingEnabled(true);

	auto d = ui->treeView->itemDelegate();
	ui->treeView->setItemDelegate(new DataTableItemDelegate(ui->treeView));
	d->deleteLater();
	ui->treeView->setItemDelegateForColumn(ModelClass::ColumnLast, new TableViewModelActionsColumnDelegate(ui->treeView));
	ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->menuButton->setMenu(new QMenu(ui->menuButton));
    ui->menuButton->setProperty("menu_secondary", true);
	ui->acceptActionBtn->setProperty("blue", true);
	ui->rejectActionBtn->setProperty("secondary", true);
	ui->rejectActionBtn->hide();
	ui->acceptActionBtn->hide();
	ui->pickAllCheckBox->hide();
}

void TreeWidgetCoefficientTables::ConnectSignals()
{
	connect(ui->pickAllCheckBox, &QCheckBox::stateChanged, this, [this]()
		{
			mModel->SetChildrenCheckState(ui->pickAllCheckBox->checkState(), mModel->GetRoot());
		});
	connect(mModel, &ModelClass::ActionTriggered, this, [this](const QString& actionId, TreeModelItem* item)
		{
			if (actionId == "edit")
				ItemEdit(item);
			else if (actionId == "export")
				ItemExport(item);
			else if (actionId == "remove")
				ItemRemove(item);
			if (actionId == "editgroup")
				EditGroup(item);
		});
	connect(ui->rejectActionBtn, &QPushButton::clicked, this, &TreeWidgetCoefficientTables::ExitAction);

	connect(ui->countLabel, &QLabel::linkActivated, this, [this]()
		{
			TreeModel::ResetInstances("TreeModelDynamicNumericTables");
		});

	connect(mModel, &ModelClass::rowsInserted, this, &TreeWidgetCoefficientTables::UpdateLabelAndCheckbox);
	connect(mModel, &ModelClass::rowsRemoved, this, &TreeWidgetCoefficientTables::UpdateLabelAndCheckbox);
	connect(mModel, &ModelClass::modelReset, this, &TreeWidgetCoefficientTables::UpdateLabelAndCheckbox);
	connect(mModel, &ModelClass::dataChanged, this, &TreeWidgetCoefficientTables::OnDataChanged);

	connect(ui->treeView, &QAbstractItemView::doubleClicked, [this](const QModelIndex& indexPresentation)
		{
			auto index = mModel->GetPresentationModel()->mapToSource(indexPresentation);
			TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer());
			int id = item->GetData("_id").toInt();
			auto mode = mModel->mode();
			if (indexPresentation.column() == 0 && mode == ModelClass::ModelMode::viewDefault && item->IsEmpty())
			{
				if (auto contextRoot = Core::GenesisContextRoot::Get())
				{
					const auto context = contextRoot->GetChildContextStepBasedMarkup();
					if (!context)
						return;
					// should always set qvariant map for erase selected checkboxes
					// when picked before
					context->SetData({ {Names::ContextTagNames::ReferenceId, id} });
					GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageAnalysisDataTablePageName);
				}
			}
		});
	connect(ui->treeView, &QTreeView::customContextMenuRequested, this, [&]()
		{
			auto model = ui->treeView->selectionModel();
			auto index = model->currentIndex();
			ui->treeView->clicked(mModel->GetPresentationModel()->index(index.row(), ModelClass::ColumnLast, index.parent()));
		});
	if (auto menu = ui->menuButton->menu())
	{
		connect(menu->addAction(tr("Merge table")), &QAction::triggered, this, &TreeWidgetCoefficientTables::PickedMergeTableAction);
		connect(menu->addAction(tr("Edit group")), &QAction::triggered, this, &TreeWidgetCoefficientTables::PickedEditGroupAction);
		connect(menu->addAction(tr("Export")), &QAction::triggered, this, &TreeWidgetCoefficientTables::PickedExportGroupAction);
		connect(menu->addAction(tr("Remove")), &QAction::triggered, this, &TreeWidgetCoefficientTables::PickedRemoveGroupAction);
	}

}

void TreeWidgetCoefficientTables::ActionAssignAndEnter(QString buttonText, QString forWhatAction, std::function<void()> func, std::function<void()> prepare, std::function<void()> done)
{
	prepare();
	mDoneAction = done;
	ui->acceptActionBtn->setText(buttonText);
	if (mActionConnection)
		disconnect(mActionConnection);
	mActionConnection = connect(ui->acceptActionBtn, &QPushButton::clicked, this, func);
	ui->acceptActionBtn->show();
	ui->rejectActionBtn->show();
	ui->pickAllCheckBox->show();
	UpdateMainCheckBox(forWhatAction);
	ui->menuButton->hide();

	mModel->SetCheckable(true);
	ui->treeView->update();
	ui->treeView->resizeColumnToContents(0);
}

void TreeWidgetCoefficientTables::ExitAction()
{
	if (mActionConnection)
		disconnect(mActionConnection);
	ui->rejectActionBtn->hide();
	ui->acceptActionBtn->hide();
	ui->pickAllCheckBox->hide();
	ui->menuButton->show();

	mModel->SetChildrenCheckState(Qt::Unchecked, mModel->GetRoot());
	mModel->SetCheckable(false);
	mDoneAction();
	mDoneAction = []() {};
}

void TreeWidgetCoefficientTables::PickedEditGroupAction()
{
	ActionAssignAndEnter(tr("Edit group"), tr("for groups"),
		[&]()
		{
			auto projectId = GenesisContextRoot::Get()->ProjectId();
            API::REST::GetTableGroups(projectId, Names::Group::CoefficientTableIdentified, [this](QNetworkReply* r, QJsonDocument doc)
				{
					std::map<QString, int> idByNames;
					auto jdata = doc.object()["data"].toArray();
					for (int i = 0; i < jdata.size(); i++)
					{
						auto jitem = jdata[i].toArray();
						idByNames.emplace(jitem[1].toString(), jitem[0].toInt());
					}

					QList<int> checkedIds;
					auto checkedItems = mModel->GetCheckedItems(mModel->GetRoot());
					for (auto& item : checkedItems)
						checkedIds << item->GetData("_id").toInt();
					ShowEditGroupDialog(checkedIds, idByNames);
					ExitAction();
				},
				[&](QNetworkReply* r, QNetworkReply::NetworkError err)
				{
					Notification::NotifyError(tr("Failed to load file groups information"), err);
					ExitAction();
				});
		});
}

void TreeWidgetCoefficientTables::PickedMergeTableAction()
{
	ActionAssignAndEnter(tr("Merge"), tr("for tables"),
		[&]()
		{
			auto dialog = new WebDialogJoinTables(this);
			std::map<int, QString> checkedIds;
			auto checkedItems = mModel->GetCheckedItems(mModel->GetRoot());
			for (auto& item : checkedItems)
				checkedIds.try_emplace(item->GetData("_id").toInt(), item->GetData("_title").toString());

			dialog->AddTables(std::move(checkedIds));
			connect(dialog, &WebDialogJoinTables::Accepted, this, &TreeWidgetCoefficientTables::ExitAction);
			dialog->Open();
		});
}

void TreeWidgetCoefficientTables::ShowEditGroupDialog(const QList<int>& ids, const std::map<QString, int>& groupInfo)
{
	if (ids.isEmpty())
	{
		Notification::NotifyError(tr("For grouping changing pick one or more chromatogramms in table"));
		return;
	}

	auto dialog = new Dialogs::WebDialogGroupEditor(this);
	dialog->SetCaptionLabel(tr("Move chosen tables"));
	dialog->SetUngroupRadioButtonText(tr("To table list"));
	dialog->SetGroupRadioButtonText(tr("To table group"));
	dialog->SetGroupInfo(groupInfo);
	connect(dialog, &WebDialog::Accepted, this, [this, dialog, ids]()
		{
			if (dialog->IsAddToGroup())
			{
				auto groupInfo = dialog->GetGroupInfo();
                API::REST::AddTablesToGroup(GenesisContextRoot::Get()->ProjectId(),
					ids,
					groupInfo.first,
					groupInfo.second,
					[this, ids](QNetworkReply* r, QJsonDocument doc)
					{
						mModel->Reset();
						Notification::NotifySuccess(tr("%n imported matrix successfully changed their grouping", "", ids.size()));
					},
					[&](QNetworkReply* r, QNetworkReply::NetworkError err)
					{
						Notification::NotifyError(tr("Failed to add file to group"), err);
					});
			}
			else
			{
                API::REST::UngroupTables(GenesisContextRoot::Get()->ProjectId(),
					ids,
					[this, ids](QNetworkReply* r, QJsonDocument doc)
					{
						mModel->Reset();
						Notification::NotifySuccess(tr("%n imported matrix successfully changed their grouping", "", ids.size()));
					},
					[&](QNetworkReply* r, QNetworkReply::NetworkError err)
					{
						Notification::NotifyError(tr("Failed to ungroup file"), err);
					});
			}
		});
	dialog->Open();
}

void TreeWidgetCoefficientTables::PickedExportGroupAction()
{
	ActionAssignAndEnter(tr("Export"), tr("for export"), [this]()
		{
			QList<int> checkedIds;
			{
				auto checkedItems = mModel->GetCheckedItems(mModel->GetRoot());
				for (auto& item : checkedItems)
					checkedIds << item->GetData("_id").toInt();
			}
			if (checkedIds.isEmpty())
			{
				Notification::NotifyError(tr("To export, pick one or more tables in table"));
				return;
			}
			const auto projectId = GenesisContextRoot::Get()->ProjectId();
			API::REST::ExportDataTables(projectId, checkedIds, [](QNetworkReply* reply, QByteArray buffer)
				{
					auto fileName = QString::fromUtf8(reply->rawHeader("file-title"));
					QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
					int pos = 0;
					while ((pos = rx.indexIn(fileName, pos)) != -1) {
						fileName.replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
					}
					fileName.replace("\"", "");

					auto fileExtension = QString::fromUtf8(reply->rawHeader("file-extension"));
					fileExtension.replace("\"", "");
					Notification::NotifySuccess(tr("Choose export path"), tr("Tables are ready for export"));
					fileName = QFileDialog::getSaveFileName(nullptr, tr("Choose export path"), fileName, QString("*.%1;;All files (*.*)").arg(fileExtension));

					if (!fileName.isEmpty())
					{
						QFile file(fileName);
						if (file.open(QFile::WriteOnly))
						{
							file.write(buffer);
							file.close();
						}
						else
						{
							Notification::NotifyError(tr("Can't open file: %1").arg(fileName));
						}
					}
				},
				[](QNetworkReply*, QNetworkReply::NetworkError e)
				{
					Notification::NotifyError(tr("Export data tables network error"), e);
				});
			ExitAction();
		});
}

void TreeWidgetCoefficientTables::PickedRemoveGroupAction()
{
	ActionAssignAndEnter(tr("Remove"), tr("for removing"), [this]()
		{
			QList<QPair<int, QString>> infos;
			{
				auto checkedItems = mModel->GetCheckedItems(mModel->GetRoot());
				for (auto& item : checkedItems)
					infos.append({ item->GetData("_id").toInt(), item->GetData("_title").toString() });
			}
			if (infos.isEmpty())
			{
				Notification::NotifyError(tr("To remove, pick one or more tables in table"));
				return;
			}
			Dialogs::Templates::TreeConfirmation::DeleteTablesWithDependencies(infos, this, "TreeModelDynamicCoefficientTables");
			ExitAction();
		});
}

void TreeWidgetCoefficientTables::UpdateCountLabel()
{
	QString text = tr("Results total: ") + QString::number(mModel->GetItemsCount());
	QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
	ui->countLabel->setText(tmpl);
}

void TreeWidgetCoefficientTables::ItemEdit(TreeModelItem* item)
{
	if (item->GetChildren().isEmpty())
		EditTable(item);
	else
		EditGroupTitle(item);
}

void TreeWidgetCoefficientTables::ItemExport(TreeModelItem* item)
{
	const auto childs = item->GetChildren();
	QList<int> exportedTables;
	if (childs.isEmpty())
	{
		exportedTables.append(item->GetData("_id").toInt());
	}
	else
	{
		for (const auto& child : childs)
			exportedTables.append(child->GetData("_id").toInt());
	}
	const auto projectId = GenesisContextRoot::Get()->ProjectId();
	API::REST::ExportDataTables(projectId, exportedTables, [](QNetworkReply* reply, QByteArray buffer)
		{
			auto fileName = QString::fromUtf8(reply->rawHeader("file-title"));
			QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
			int pos = 0;
			while ((pos = rx.indexIn(fileName, pos)) != -1) {
				fileName.replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
			}
			fileName.replace("\"", "");

			auto fileExtension = QString::fromUtf8(reply->rawHeader("file-extension"));
			fileExtension.replace("\"", "");
			Notification::NotifySuccess(tr("Choose export path"), tr("Tables are ready for export"));
			fileName = QFileDialog::getSaveFileName(nullptr, tr("Choose export path"), fileName, QString("*.%1;;All files (*.*)").arg(fileExtension));

			if (!fileName.isEmpty())
			{
				QFile file(fileName);
				if (file.open(QFile::WriteOnly))
				{
					file.write(buffer);
					file.close();
				}
				else
				{
					Notification::NotifyError(tr("Can't open file: %1").arg(fileName));
				}
			}
		},
		[](QNetworkReply*, QNetworkReply::NetworkError e)
		{
			Notification::NotifyError(tr("Export data tables network error"), e);
		});
}

void TreeWidgetCoefficientTables::ItemRemove(TreeModelItem* item)
{
	Dialogs::Templates::TreeConfirmation::DeleteTablesWithDependencies({ {item->GetData("_id").toInt(), item->GetData("_title").toString()} }, this, "TreeModelDynamicCoefficientTables");
}

void TreeWidgetCoefficientTables::EditGroup(TreeModelItem* item)
{
	auto projectId = GenesisContextRoot::Get()->ProjectId();
    API::REST::GetTableGroups(projectId, Names::Group::CoefficientTableIdentified, [this, item](QNetworkReply* r, QJsonDocument doc)
		{
			std::map<QString, int> idByNames;
			auto jdata = doc.object()["data"].toArray();
			for (int i = 0; i < jdata.size(); i++)
			{
				auto jitem = jdata[i].toArray();
				idByNames.emplace(jitem[1].toString(), jitem[0].toInt());
			}

			auto childs = item->GetChildren();
			QList<int> ids;
			if (!childs.empty())
			{
				for (const auto& child : childs)
					ids.append(child->GetData("_id").toInt());
			}
			else
			{
				ids.append(item->GetData("_id").toInt());
			}
			ShowEditGroupDialog(ids, idByNames);
		},
		[](QNetworkReply* r, QNetworkReply::NetworkError err)
		{
			Notification::NotifyError(tr("Failed to load file groups information"), err);
		});
}

void TreeWidgetCoefficientTables::EditTable(TreeModelItem* item)
{
	auto s = Dialogs::Templates::SaveEdit::Settings();
	s.header = tr("Rename table");
	s.titleHeader = tr("Table title");
	s.commentHeader = tr("Table comment");
	s.titleRequired = true;
	s.commentRequired = false;
	s.titlePlaceholder = tr("Enter table title here");
	s.commentPlaceholder = tr("Enter table comment here");
	s.titleMaxLen = 50;
	s.commentMaxLen = 70;
	s.okBtnText = tr("Save");
	s.cancelBtnText = tr("Cancel");
	s.defaultComment = item->GetData("_comment").toString();
	s.defaultTitle = item->GetData("_title").toString();
	s.titleTooltipsSet.empty = tr("Title should be non-empty");
	s.titleTooltipsSet.forbidden = "";
	s.titleTooltipsSet.notChecked = tr("Not checked");
	s.titleTooltipsSet.valid = tr("Title is valid");

	auto dial = new Dialogs::Templates::SaveEdit(this, s);
	int tableId = item->GetData("_id").toInt();
	QString oldTitle = item->GetData("_title").toString();
	QString oldComment = item->GetData("_comment").toString();
	connect(dial, &WebDialog::Accepted, this, [dial, tableId, oldTitle, oldComment]()
		{
			QString title = dial->getTitle();
			QString comment = dial->getComment();
			API::REST::RenameTable(tableId, title, comment,
				[oldTitle, title, oldComment, comment](QNetworkReply*, QJsonDocument doc)
				{
					auto obj = doc.object();
					if (obj["error"].toBool())
					{
						Notification::NotifyError(tr("Rename table server error: %1").arg(obj["msg"].toString()));
						return;
					}
					if (title != oldTitle)
						Notification::NotifySuccess(tr("Table \"%1\" renamed to \"%2\"").arg(oldTitle).arg(title));
					else if (comment != oldComment)
						Notification::NotifySuccess(tr("Table \"%1\" changed").arg(title));
					else
						Notification::NotifyInfo(tr("There were no changes"));
					TreeModel::ResetInstances("TreeModelDynamicNumericTables");
				},
				[](QNetworkReply*, QNetworkReply::NetworkError e)
				{
					Notification::NotifyError(tr("Rename table network error"), e);
				});
		});
	dial->Open();
}

void TreeWidgetCoefficientTables::EditGroupTitle(TreeModelItem* item)
{
	const auto oldTitle = item->GetData("_title").toString();
	auto dialog = new TableRenameDialog(this, oldTitle);
	dialog->SetLineEditCaption(tr("Group name"));
	connect(dialog, &WebDialog::Accepted, this,
		[&, groupId = item->GetData("_id").toInt(), dialog, oldTitle]()
		{
			auto newTitle = dialog->getTitle();
			auto projectId = GenesisContextRoot::Get()->ProjectId();
			API::REST::RenameFilesGroup(projectId, groupId, newTitle,
				[newTitle, oldTitle](QNetworkReply*, QJsonDocument doc)
				{
					auto obj = doc.object();
					if (obj["error"].toBool())
					{
						Notification::NotifyError(tr("Rename table server error: %1").arg(obj["msg"].toString()));
						return;
					}
					if (newTitle != oldTitle)
						Notification::NotifySuccess(tr("Table \"%1\" renamed to \"%2\"").arg(oldTitle).arg(newTitle));
					else
						Notification::NotifyInfo(tr("There were no changes"));
					TreeModel::ResetInstances("TreeModelDynamicNumericTables");
				},
				[](QNetworkReply*, QNetworkReply::NetworkError e) { Notification::NotifyError(tr("Rename table network error"), e); });
		});
	dialog->Open();
}

void TreeWidgetCoefficientTables::UpdateMainCheckBox(QString forWhat)
{
	auto checkedList = mModel->GetCheckedItems();
	auto oldText = ui->pickAllCheckBox->property("for_what").toString();
	if (!oldText.isEmpty() && oldText != forWhat)
		ui->pickAllCheckBox->setProperty("for_what", forWhat);
	ui->pickAllCheckBox->setText(tr("Picked %n table(s)", "", checkedList.size()) + " " + ui->pickAllCheckBox->property("for_what").toString());
	bool checked = mModel->GetRoot()->GetChildrenRecursive().size() == checkedList.size();
	{
		QSignalBlocker s(ui->pickAllCheckBox);
		ui->pickAllCheckBox->setChecked(checked);
	}
}

void TreeWidgetCoefficientTables::OnDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles)
{
	if (roles.contains(Qt::CheckStateRole))
	{
		UpdateMainCheckBox(ui->pickAllCheckBox->property("for_what").toString());
	}
}

void TreeWidgetCoefficientTables::UpdateLabelAndCheckbox()
{
	UpdateCountLabel();
	UpdateMainCheckBox(ui->pickAllCheckBox->property("for_what").toString());
}
