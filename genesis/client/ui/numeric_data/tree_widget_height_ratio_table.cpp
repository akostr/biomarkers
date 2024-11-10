#include "tree_widget_height_ratio_table.h"
#include <logic/tree_model_presentation.h>
#include "logic/tree_model_dynamic_numeric_tables.h"
#include <ui/genesis_window.h>
#include <genesis_style/style.h>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <ui/numeric_data/join_numeric_dialog.h>
#include <QDialogButtonBox>
#include <logic/notification.h>
#include <QHeaderView>
#include <api/api_rest.h>
#include <QFileDialog>
#include <QTimer>
#include <QRegExp>
#include <logic/known_context_tag_names.h>
#include <logic/tree_model_item.h>
#include <ui/known_view_names.h>
#include <logic/context_root.h>
#include <ui/itemviews/tableview_headerview.h>
#include <ui/itemviews/tableview_model_actions_column_delegate.h>
#include <ui/itemviews/datatable_item_delegate.h>
#include <ui/dialogs/templates/save_edit.h>
#include <ui/dialogs/templates/tree_confirmation.h>
#include <ui/dialogs/height_ratio_matrix/height_ratio_info.h>
#include <ui/genesis_window.h>

using ModelClass = TreeModelDynamicNumericTables;
using namespace Core;
using namespace Names;
using namespace ViewPageNames;

TreeWidgetHeightRatioTable::TreeWidgetHeightRatioTable(QWidget* parent)
	: QWidget(parent)
{
	SetupUi();
	ConnectSignals();
}

void TreeWidgetHeightRatioTable::SetupUi()
{
	setStyleSheet(Style::Genesis::GetUiStyle());
	mLayout = new QVBoxLayout(this);

	m_pickedCountLabel = new QLabel();
	//  "_id"
	//  "_title"
	//  "_username"
	//  "_table_type"
	//  "_uploaded"
	//  "_sample_id"
	//  "_sample_title"
	//  "_chromatograms_count"
	//  "_is_identified"

	// add sync
	mModel = new TreeModelDynamicNumericTables(Names::Group::HeigthRatioMatrix, this);
	mTreeView = new QTreeView(this);
	mTreeView->setStyleSheet(Style::Genesis::GetTreeViewStyle());
	mTreeView->setProperty("dataDrivenColors", true);
	mTreeView->setHeader(new TableViewHeaderView(Qt::Horizontal, mTreeView));
	mTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	mTreeView->header()->setStretchLastSection(true);

	mTreeView->setModel(mModel->GetPresentationModel());
	auto d = mTreeView->itemDelegate();
	mTreeView->setItemDelegate(new DataTableItemDelegate(mTreeView));
	d->deleteLater();
	mTreeView->setItemDelegateForColumn(ModelClass::ColumnLast, new TableViewModelActionsColumnDelegate(mTreeView));
	mTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	mTreeView->setSortingEnabled(true);

	ActionsMenuButton = new QPushButton(tr("Actions with tables"), this);
    ActionsMenuButton->setProperty("menu_secondary", true);
	auto actionsMenu = new QMenu(ActionsMenuButton);
	ExportAction = actionsMenu->addAction(tr("Export"));
	RemoveAction = actionsMenu->addAction(tr("Remove"));

	auto internalMenuLayout = new QHBoxLayout();

	mActionButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	auto button = mActionButtons->button(QDialogButtonBox::Ok);
	button->setFocusPolicy(Qt::NoFocus);

	internalMenuLayout->setDirection(QHBoxLayout::Direction::RightToLeft);
	internalMenuLayout->addWidget(ActionsMenuButton);
	internalMenuLayout->addWidget(mActionButtons);

	internalMenuLayout->addStretch();
	internalMenuLayout->addWidget(m_pickedCountLabel);

	//pick all checkbox
	m_pickAllCheckbox = new QCheckBox("");
	m_pickAllCheckbox->setTristate(true);
	m_pickAllCheckbox->setCheckState(Qt::Unchecked);
	m_pickAllCheckbox->setProperty("invalidation_to_model_in_progress", false);
	internalMenuLayout->addWidget(m_pickAllCheckbox);

	HideButtons(false);
	ActionsMenuButton->setMenu(actionsMenu);

	mLayout->addLayout(internalMenuLayout);
	mLayout->addWidget(mTreeView);

	mCountLabel = new QLabel(this);
	mLayout->addWidget(mCountLabel);

	mTreeView->expandAll();
}

void TreeWidgetHeightRatioTable::ConnectSignals()
{
	connect(mTreeView, &QTreeView::customContextMenuRequested, this, [&]()
		{
			const auto model = mTreeView->selectionModel();
			const auto index = model->currentIndex();
			const auto originModelIndex = mModel->GetPresentationModel()->mapToSource(index);
			mTreeView->clicked(mModel->index(originModelIndex.row(), ModelClass::ColumnLast, originModelIndex.parent()));
		});
	connect(mActionButtons, &QDialogButtonBox::rejected, [this]()
		{
			mModel->enterDefaultMode();
			HideButtons(false);
		});

	connect(mActionButtons, &QDialogButtonBox::accepted, [this]()
		{

			if (mModel->mode() == ModelClass::exportMode)
			{
				auto list = mModel->getTableIdsWithName();
				if (list.isEmpty())
				{
					return;
				}
				auto idsList = mModel->getTableIds();
				Export(idsList);
				HideButtons(false);
				mModel->enterDefaultMode();
			}
			else if (mModel->mode() == ModelClass::removeMode)
			{
				auto list = mModel->getTableIdsWithName();
				if (list.isEmpty())
					return;
				Dialogs::Templates::TreeConfirmation::DeleteTablesWithDependencies(list, this, "TreeModelHeightRatio");
				HideButtons(false);
				mModel->enterDefaultMode();
			}
		});

	connect(mModel, &ModelClass::dataChanged, this,
		[this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>())
		{
			if (m_pickAllCheckbox->property("invalidation_to_model_in_progress").toBool())
				return;
			if (roles.contains(Qt::CheckStateRole))
				Invalidate(false);
		});
	connect(mModel, &ModelClass::modelReset, this,
		[this]()
		{
			if (m_pickAllCheckbox->property("invalidation_to_model_in_progress").toBool())
				return;
			Invalidate(false);
		});

	connect(m_pickAllCheckbox, &QCheckBox::stateChanged, this,
		[this](int state)
		{
			if (state == Qt::PartiallyChecked)
			{
				auto sb = QSignalBlocker(m_pickAllCheckbox);
				m_pickAllCheckbox->setCheckState(Qt::Checked);
			}
			Invalidate(true);
		});
	connect(mCountLabel, &QLabel::linkActivated, this, [this]()
		{
			TreeModel::ResetInstances("TreeModelDynamicNumericTables");
			mModel->enterDefaultMode();
			HideButtons(false);
		});

	connect(mModel, &ModelClass::rowsInserted, this, &::TreeWidgetHeightRatioTable::UpdateCountLabel);
	connect(mModel, &ModelClass::rowsRemoved, this, &::TreeWidgetHeightRatioTable::UpdateCountLabel);
	connect(mModel, &ModelClass::dataChanged, this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
		{
			if (m_pickedCountLabel->isVisible() && roles.contains(Qt::CheckStateRole))
				UpdatePickedCountLabel();
		});

	connect(mModel, &ModelClass::modelReset, this, [this]()
		{
			UpdateCountLabel();
			UpdatePickedCountLabel();
			mTreeView->expandAll();
		});

	connect(ExportAction, &QAction::triggered, [this]()
		{
			mActionButtons->button(QDialogButtonBox::Ok)->setText(tr("Export matrixes"));
			HideButtons(true);
			mModel->enterExportMode();
		});
	connect(RemoveAction, &QAction::triggered, [this]()
		{
			mActionButtons->button(QDialogButtonBox::Ok)->setText(tr("remove matrixes"));
			HideButtons(true);
			mModel->enterRemoveMode();
		});
	connect(mTreeView, &QAbstractItemView::doubleClicked, [this](const QModelIndex& indexPresentation)
		{
			auto index = mModel->GetPresentationModel()->mapToSource(indexPresentation);
			TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer());
			auto mode = mModel->mode();
			if (indexPresentation.column() == 0 && mode == ModelClass::ModelMode::viewDefault && item->IsEmpty())
			{
				int id = item->GetData("_id").toInt();
				OpenTable(id);
			}
		});

	connect(mModel, &TreeModel::ActionTriggered, [this](const QString& actionId, TreeModelItem* item)
		{
			QModelIndex index = item->GetIndex();
			int id = item->GetData("_id").toInt();
			QString title = item->GetData("_title").toString();
			QString comment = item->GetData("_comment").toString();
			if (actionId == "open")
			{
				OpenTable(id);
			}
			else if (actionId == "remove")
			{
				Dialogs::Templates::TreeConfirmation::DeleteTablesWithDependencies({ {id, title } }, this, "TreeModelHeightRatio");
			}
			else if (actionId == "edit")
			{
				EditTable(id, title, comment);
			}
			else if (actionId == "info")
			{
				ViewTableInfo(id);
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
}

void TreeWidgetHeightRatioTable::ResetTable()
{

}

void TreeWidgetHeightRatioTable::UpdateCountLabel()
{
	QString text = tr("Results total: ") + QString::number(mModel->NonGroupItemsCount());
	QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
	mCountLabel->setText(tmpl);
}

void TreeWidgetHeightRatioTable::UpdatePickedCountLabel()
{
	m_pickedCountLabel->setText(tr("Picked %n tables ", "", mModel->GetCheckedItems().size()));
}

void TreeWidgetHeightRatioTable::Export(QList<int> idList)
{
	Notification::NotifyInfo(tr("Starting process of matrix export"), tr("Preparing export"));
	const auto projectId = GenesisContextRoot::Get()->ProjectId();
	API::REST::ExportDataTables(projectId, idList,
		[this](QNetworkReply* reply, QByteArray buffer)
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
			Notification::NotifySuccess(tr("Choose export path"), tr("Matrix is ready for export"));
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
		[this](QNetworkReply*, QNetworkReply::NetworkError)
		{
			Notification::NotifyError(tr("Export data tables error"));
		});
}

void TreeWidgetHeightRatioTable::Invalidate(bool toModel)
{
	if (toModel)
	{
		m_pickAllCheckbox->setProperty("invalidation_to_model_in_progress", true);
		mModel->SetChildrenCheckState(m_pickAllCheckbox->checkState(), mModel->GetRoot());
		m_pickAllCheckbox->setProperty("invalidation_to_model_in_progress", false);
	}
	else
	{
		auto sb = QSignalBlocker(m_pickAllCheckbox);
		auto checkedSize = mModel->GetCheckedItems().size();
		auto checkableCount = mModel->GetCheckableCount();
		if (checkableCount == 0)
			m_pickAllCheckbox->setCheckState(Qt::Unchecked);
		else if (checkedSize < checkableCount)
		{
			if (checkedSize == 0)
				m_pickAllCheckbox->setCheckState(Qt::Unchecked);
			else
				m_pickAllCheckbox->setCheckState(Qt::PartiallyChecked);
		}
		else
			m_pickAllCheckbox->setCheckState(Qt::Checked);
	}
}

void TreeWidgetHeightRatioTable::OpenTable(int id) const
{
	if (id)
	{
		if (auto contextRoot = GenesisContextRoot::Get())
		{
			if (const auto context = contextRoot->GetChildContextStepBasedMarkup())
			{
				context->SetData(ContextTagNames::ReferenceId, id);
				GenesisWindow::Get()->ShowPage(ViewPageAnalysisDataTablePageName);
			}
		}
	}
}

void TreeWidgetHeightRatioTable::EditTable(int id, const QString& title, const QString& comment)
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
								.arg(title));
						}

						QTimer::singleShot(0, [this]()
							{
								mModel->Reset();
							});
					};
				auto errorCallback = [this](QNetworkReply*, QNetworkReply::NetworkError e)
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
}

void TreeWidgetHeightRatioTable::ViewTableInfo(int id)
{
	auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading matrix info"));
	API::REST::getProjectHeightRatioInfo(id,
		[this, overlayId](QNetworkReply*, QJsonDocument doc)
		{
			auto obj = doc.object();
			if (obj["error"].toBool())
			{
				Notification::NotifyError(obj["msg"].toString(), tr("Server error"));
				GenesisWindow::Get()->RemoveOverlay(overlayId);
				return;
			}
			auto dial = new Dialogs::HeightRatioInfo(obj, this);
			dial->Open();
			GenesisWindow::Get()->RemoveOverlay(overlayId);
		},
		[overlayId](QNetworkReply*, QNetworkReply::NetworkError err)
		{
			Notification::NotifyError(tr("Network error"), err);
			GenesisWindow::Get()->RemoveOverlay(overlayId);
		});
}

void TreeWidgetHeightRatioTable::HideButtons(bool hide)
{
	mActionButtons->setVisible(hide);
	ActionsMenuButton->setVisible(!hide);
	m_pickedCountLabel->setVisible(hide);
	m_pickAllCheckbox->setVisible(hide);
}
