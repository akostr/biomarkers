#include "tree_widget_joined_tables.h"
#include "genesis_style/style.h"
#include "logic/tree_model_dynamic_numeric_tables.h"
#include "ui/numeric_data/join_numeric_dialog.h"
#include "ui/itemviews/tableview_headerview.h"
#include "ui/itemviews/datatable_item_delegate.h"
#include "ui/itemviews/tableview_model_actions_column_delegate.h"
#include "ui/dialogs/templates/tree_confirmation.h"
#include "ui/dialogs/templates/save_edit.h"
#include "ui/genesis_window.h"
#include "logic/notification.h"
#include "logic/context_root.h"
#include "logic/tree_model_item.h"
#include "logic/known_context_tag_names.h"
#include "ui/known_view_names.h"
#include "api/api_rest.h"
#include "ui/dialogs/web_dialog_group_editor.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QTreeView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QRegExp>
#include <QFileDialog>

using namespace Core;

using ModelClass = TreeModelDynamicNumericTables;

TreeWidgetJoinedTables::TreeWidgetJoinedTables(QWidget* parent)
	: QWidget{ parent }
{
	SetupUi();
}

void TreeWidgetJoinedTables::SetupUi()
{
	setStyleSheet(Style::Genesis::GetUiStyle());
	auto layout = new QVBoxLayout(this);

	mPickedCountLabel = new QLabel();
	mPickedCountLabel->setVisible(false);

	mModel = new TreeModelDynamicNumericTables(Names::Group::MergedTable, this);
	mTreeView = new QTreeView(this);

	mTreeView->setStyleSheet(Style::Genesis::GetTreeViewStyle());
	mTreeView->setProperty("dataDrivenColors", true);
	mTreeView->setHeader(new TableViewHeaderView(Qt::Horizontal, mTreeView));
	mTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	mTreeView->header()->setStretchLastSection(true);

	mTreeView->setModel(mModel->GetPresentationModel());
	mTreeView->setSortingEnabled(true);

  auto ActionsMenuButton = new QPushButton(tr("Actions with tables"), this);
  ActionsMenuButton->setProperty("menu_secondary", true);
	auto actionsMenu = new QMenu(ActionsMenuButton);
	auto joinAction = actionsMenu->addAction(tr("Join included tables"));
	auto groupAction = actionsMenu->addAction(tr("Group tables"));
	auto exportAction = actionsMenu->addAction(tr("Export"));
	auto removeAction = actionsMenu->addAction(tr("Remove"));
	auto d = mTreeView->itemDelegate();
	mTreeView->setItemDelegate(new DataTableItemDelegate(mTreeView));
	d->deleteLater();
	mTreeView->setItemDelegateForColumn(ModelClass::ColumnLast, new TableViewModelActionsColumnDelegate(mTreeView));

	mTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mTreeView, &QTreeView::customContextMenuRequested, this, [&]()
		{
			auto model = mTreeView->selectionModel();
			auto index = model->currentIndex();
			auto originIndex = mModel->GetPresentationModel()->mapToSource(index);
			mTreeView->clicked(mModel->index(originIndex.row(), ModelClass::ColumnLast, originIndex.parent()));
		});
	auto internalMenuLayout = new QHBoxLayout();

	mActionButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	auto button = mActionButtons->button(QDialogButtonBox::Ok);
	button->setFocusPolicy(Qt::NoFocus);


	connect(mActionButtons, &QDialogButtonBox::rejected, this, [this, ActionsMenuButton]()
		{
			mModel->enterDefaultMode();
			mActionButtons->setVisible(false);
			ActionsMenuButton->setVisible(true);
			mPickedCountLabel->setVisible(false);
			mPickAllCheckbox->setVisible(false);
		});

	connect(mActionButtons, &QDialogButtonBox::accepted, this, [this, ActionsMenuButton]()
		{
			if (mModel->mode() == ModelClass::joinMode)
			{
				auto list = mModel->getTableIdsWithName();
				if (list.size() < 2)
				{
					Notification::NotifyError(tr("Select two or more tables"));
					return;
				}
				auto joinDialog = new Dialogs::JoinNumericDialog(list, this);
				joinDialog->Open();
				connect(joinDialog, &WebDialog::Accepted, this, [this, ActionsMenuButton]()
					{
						ActionsMenuButton->setVisible(true);
						mActionButtons->setVisible(false);
						mModel->enterDefaultMode();
						mRefreshTimer = new QTimer(this);
						mRefreshTimer->setSingleShot(true);
						mRefreshTimer->setInterval(500);
						connect(mRefreshTimer, &QTimer::timeout, this, []()
							{
								TreeModel::ResetInstances("TreeModelDynamicNumericTables");
							}, Qt::QueuedConnection);
						mRefreshTimer->start();
						mPickedCountLabel->setVisible(false);
						mPickAllCheckbox->setVisible(false);
					});

				connect(joinDialog, &WebDialog::Rejected, this, [this, ActionsMenuButton]()
					{
						ActionsMenuButton->setVisible(true);
						mActionButtons->setVisible(false);
						mPickedCountLabel->setVisible(false);
						mPickAllCheckbox->setVisible(false);
						mModel->enterDefaultMode();
						TreeModel::ResetInstances("TreeModelDynamicNumericTables");
					});
				return;
			}

			else if (mModel->mode() == ModelClass::exportMode)
			{
				auto list = mModel->getTableIdsWithName();
				if (list.isEmpty())
				{
					return;
				}
				auto idsList = mModel->getTableIds();
				Export(idsList);
				ActionsMenuButton->setVisible(true);
				mActionButtons->setVisible(false);
				mPickedCountLabel->setVisible(false);
				mPickAllCheckbox->setVisible(false);
				mModel->enterDefaultMode();
			}
			else if (mModel->mode() == ModelClass::removeMode)
			{
				auto list = mModel->getTableIdsWithName();
				if (list.isEmpty())
					return;
				Dialogs::Templates::TreeConfirmation::DeleteTablesWithDependencies(list, this, "TreeModelNumericData");
				ActionsMenuButton->setVisible(true);
				mActionButtons->setVisible(false);
				mPickedCountLabel->setVisible(false);
				mPickAllCheckbox->setVisible(false);
				mModel->enterDefaultMode();
			}
			else if (mModel->mode() == ModelClass::groupMode)
			{
				auto projectId = GenesisContextRoot::Get()->ProjectId();
				API::REST::GetTableGroups(projectId, Names::Group::MergedTable,
					[&](QNetworkReply* r, QJsonDocument doc)
					{
						qDebug() << doc;
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
						ActionsMenuButton->setVisible(true);
						mActionButtons->setVisible(false);
						mPickedCountLabel->setVisible(false);
						mPickAllCheckbox->setVisible(false);
						mModel->enterDefaultMode();
					},
					[&](QNetworkReply* r, QNetworkReply::NetworkError err)
					{
						Notification::NotifyError(tr("Failed to load file groups information"), err);
						ActionsMenuButton->setVisible(true);
						mActionButtons->setVisible(false);
						mPickedCountLabel->setVisible(false);
						mPickAllCheckbox->setVisible(false);
						mModel->enterDefaultMode();
					});
			}
		});

	mActionButtons->setVisible(false);
	internalMenuLayout->setDirection(QHBoxLayout::Direction::RightToLeft);
	internalMenuLayout->addWidget(ActionsMenuButton);
	internalMenuLayout->addWidget(mActionButtons);

	internalMenuLayout->addStretch();
	internalMenuLayout->addWidget(mPickedCountLabel);

	{//pick all checkbox
		mPickAllCheckbox = new QCheckBox("");
		mPickAllCheckbox->setTristate(true);
		mPickAllCheckbox->setCheckState(Qt::Unchecked);
		mPickAllCheckbox->setProperty("invalidation_to_model_in_progress", false);
		internalMenuLayout->addWidget(mPickAllCheckbox);
		auto invalidate = [this](bool toModel)
			{
				if (toModel)
				{
					mPickAllCheckbox->setProperty("invalidation_to_model_in_progress", true);
					mModel->SetChildrenCheckState(mPickAllCheckbox->checkState(), mModel->GetRoot());
					mPickAllCheckbox->setProperty("invalidation_to_model_in_progress", false);
				}
				else
				{
					auto sb = QSignalBlocker(mPickAllCheckbox);
					auto checkedSize = mModel->GetCheckedItems().size();
					auto checkableCount = mModel->GetCheckableCount();
					if (checkableCount == 0)
					{
						mPickAllCheckbox->setCheckState(Qt::Unchecked);
						if (mModel->mode() == ModelClass::ModelMode::joinMode)
						{
							mPickAllCheckbox->setDisabled(true);
						}
					}
					else if (checkedSize < checkableCount)
					{
						if (checkedSize == 0)
						{
							mPickAllCheckbox->setCheckState(Qt::Unchecked);
							if (mModel->mode() == ModelClass::ModelMode::joinMode)
							{
								mPickAllCheckbox->setDisabled(true);
							}
						}
						else
						{
							mPickAllCheckbox->setCheckState(Qt::PartiallyChecked);
							mPickAllCheckbox->setDisabled(false);
						}
					}
					else
					{
						mPickAllCheckbox->setCheckState(Qt::Checked);
						mPickAllCheckbox->setDisabled(false);
					}
				}
			};
		connect(mModel, &ModelClass::dataChanged, this,
			[this, invalidate](const QModelIndex&, const QModelIndex&, const QVector<int>& roles = QVector<int>())
			{
				if (mPickAllCheckbox->property("invalidation_to_model_in_progress").toBool())
					return;
				if (roles.contains(Qt::CheckStateRole))
					invalidate(false);
			});
		connect(mModel, &ModelClass::modelReset, this,
			[this, invalidate]()
			{
				if (mPickAllCheckbox->property("invalidation_to_model_in_progress").toBool())
					return;
				invalidate(false);
			});

		connect(mPickAllCheckbox, &QCheckBox::stateChanged, this,
			[this, invalidate](int state)
			{
				if (state == Qt::PartiallyChecked)
				{
					auto sb = QSignalBlocker(mPickAllCheckbox);
					mPickAllCheckbox->setCheckState(Qt::Checked);
					invalidate(true);
				}
				else
				{
					if (state == Qt::Unchecked)
					{
						mModel->SetItemsCheckable(true);
						invalidate(true);
					}
					else
					{
						invalidate(true);
					}
				}
			});
		mPickAllCheckbox->setVisible(false);
	}

	ActionsMenuButton->setVisible(true);
	ActionsMenuButton->setMenu(actionsMenu);

	layout->addLayout(internalMenuLayout);
	layout->addWidget(mTreeView);

	mCountLabel = new QLabel(this);
	layout->addWidget(mCountLabel);
	connect(mCountLabel, &QLabel::linkActivated, this, [this, ActionsMenuButton]()
		{
			TreeModel::ResetInstances("TreeModelDynamicNumericTables");
			mModel->enterDefaultMode();
			mActionButtons->setVisible(false);
			ActionsMenuButton->setVisible(true);
			mPickedCountLabel->setVisible(false);
			mPickAllCheckbox->setVisible(false);
		});

	connect(mModel, &ModelClass::rowsInserted, this, &TreeWidgetJoinedTables::UpdateCountLabel);
	connect(mModel, &ModelClass::rowsRemoved, this, &TreeWidgetJoinedTables::UpdateCountLabel);

	connect(mModel, &ModelClass::dataChanged, this, [this](const QModelIndex&, const QModelIndex&, const QVector<int>&)
		{
			UpdatePickedCountLabel();
		});

	connect(mModel, &ModelClass::modelReset, this, [this]()
		{
			UpdateCountLabel();
			UpdatePickedCountLabel();
			mTreeView->expandAll();
		});
	connect(groupAction, &QAction::triggered, this, [this, button, ActionsMenuButton]()
		{
			button->setText(tr("Group tables"));
			mActionButtons->setVisible(true);
			ActionsMenuButton->setVisible(false);
			mPickedCountLabel->setVisible(true);
			mPickAllCheckbox->setVisible(true);
			mPickAllCheckbox->setDisabled(true);
			mModel->enterGroupMode();
		});

	connect(joinAction, &QAction::triggered, this, [this, button, ActionsMenuButton]()
		{
			button->setText(tr("Join tables"));
			mActionButtons->setVisible(true);
			ActionsMenuButton->setVisible(false);
			mPickedCountLabel->setVisible(true);
			mPickAllCheckbox->setVisible(true);
			mPickAllCheckbox->setDisabled(true);
			mModel->enterJoinMode();
		});

	connect(exportAction, &QAction::triggered, this, [this, button, ActionsMenuButton]()
		{
			button->setText(tr("Export tables"));
			mActionButtons->setVisible(true);
			ActionsMenuButton->setVisible(false);
			mPickedCountLabel->setVisible(true);
			mPickAllCheckbox->setVisible(true);
			mPickAllCheckbox->setDisabled(false);
			mModel->enterExportMode();
		});
	connect(removeAction, &QAction::triggered, this, [this, button, ActionsMenuButton]()
		{
			button->setText(tr("Remove tables"));
			mActionButtons->setVisible(true);
			ActionsMenuButton->setVisible(false);
			mPickedCountLabel->setVisible(true);
			mPickAllCheckbox->setVisible(true);
			mPickAllCheckbox->setDisabled(false);
			mModel->enterRemoveMode();
		});

	auto openTable = [](int id)
		{
			if (id)
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
		};

	auto removeTable = [this](QPair<int, QString> data)
		{
			Dialogs::Templates::TreeConfirmation::DeleteTablesWithDependencies({ data }, this, "TreeModelHeightRatio");
		};

	auto editTable = [this](int id, const QString& title, const QString& comment)
		{
			if (id)
			{
				Dialogs::Templates::SaveEdit::Settings s;
				s.header = tr("Editing");
				s.titleHeader = tr("Table title");
				s.commentHeader = tr("Comment");
				s.titlePlaceholder = tr("Enter title");
				s.commentPlaceholder = tr("Enter comment");
				s.defaultTitle = title;
				s.defaultComment = comment;
				s.titleTooltipsSet.empty = tr("Title should be not empty");
				s.titleTooltipsSet.forbidden = tr("Title is alredy in use");
				s.titleTooltipsSet.notChecked = tr("Title is not checked yet");
				s.titleTooltipsSet.valid = tr("Title is valid");

				auto dial = new Dialogs::Templates::SaveEdit(this, s);
				connect(dial, &WebDialog::Accepted, this, [dial, id, title, comment, this]()
					{
						auto newTitle = dial->getTitle();
						auto newComment = dial->getComment();

						auto callback = [newTitle, id, title, this](QNetworkReply*, QJsonDocument doc)
							{

								auto root = doc.object();
								if (root["error"].toBool())
								{
									Notification::NotifyError(root["msg"].toString(), tr("Server error"));
									return;
								}
								if (newTitle != title)
								{
									Notification::NotifyHeaderless(tr("Table(%1) name is renamed from [%2] to [%3]")
										.arg(id)
										.arg(title)
										.arg(newTitle));
								}
								else
								{
									Notification::NotifyHeaderless(tr("Table(%1) is changed")
										.arg(newTitle));
								}
								QTimer::singleShot(0, this, [this]()
									{
										mModel->Reset();
									});
							};
						auto errorCallback = [](QNetworkReply*, QNetworkReply::NetworkError e)
							{
								Notification::NotifyError("", e);
							};
						if (newTitle == title && newComment == comment)
						{
							Notification::NotifyInfo(tr("There were no changes"));
						}
						else
						{
							API::REST::EditTable(id, newTitle, newComment, callback, errorCallback);
						}

					});
				dial->Open();
			}
		};

	connect(mTreeView, &QAbstractItemView::doubleClicked, this, [this, openTable](const QModelIndex& indexPresentation)
		{
			auto index = mModel->GetPresentationModel()->mapToSource(indexPresentation);
			TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer());
			int id = item->GetData("_id").toInt();
			auto mode = mModel->mode();
			if (indexPresentation.column() == 0 && mode == ModelClass::ModelMode::viewDefault && item->IsEmpty())
			{
				openTable(id);
			}
		});

	connect(mModel, &TreeModel::ActionTriggered, this, [this, openTable, removeTable, editTable](const QString& actionId, TreeModelItem* item)
		{
			int id = item->GetData("_id").toInt();
			QString title = item->GetData("_title").toString();
			QString comment = item->GetData("_comment").toString();
			if (actionId == "open")
			{
				//disabled for now
				openTable(id);
			}
			else if (actionId == "remove")
			{
				removeTable({ id, title });
			}
			else if (actionId == "edit")
			{
				editTable(id, title, comment);
			}
			else if (actionId == "export")
			{
				QList<int> idsList;
				idsList.append(item->GetData("_id").toInt());
				Export(idsList);
			}
			else if (actionId == "export_group")
			{
				QList<int> idsList;
				for (auto& child : item->GetChildren())
				{
					idsList.append(child->GetData("_id").toInt());
				}
				Export(idsList);
			}
		});
	mTreeView->expandAll();
}

void TreeWidgetJoinedTables::UpdateCountLabel()
{
	QString text = tr("Results total: ") + QString::number(mModel->NonGroupItemsCount());
	QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
	mCountLabel->setText(tmpl);
}

void TreeWidgetJoinedTables::UpdatePickedCountLabel()
{
	mPickedCountLabel->setText(tr("Picked %n tables ", "", mModel->GetCheckedItems().size()));
}

void TreeWidgetJoinedTables::Export(QList<int> idList)
{
	Notification::NotifyInfo(tr("Starting process of table export"), tr("Preparing export"));
	const auto projectId = GenesisContextRoot::Get()->ProjectId();
	API::REST::ExportDataTables(projectId, idList,
		[](QNetworkReply* reply, QByteArray buffer)
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

			Notification::NotifySuccess(tr("Choose export path"), tr("Table is ready for export"));
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
		[](QNetworkReply*, QNetworkReply::NetworkError)
		{
			Notification::NotifyError(tr("Export data tables error"));
		});
}

void TreeWidgetJoinedTables::ShowEditGroupDialog(const QList<int>& ids, const std::map<QString, int>& groupInfo)
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
