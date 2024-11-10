#include "join_tables.h"

#include "ui_join_tables.h"

#include "api/api_rest.h"
#include "logic/known_context_tag_names.h"
#include "logic/context_root.h"
#include "logic/notification.h"
#include "logic/known_json_tag_names.h"

namespace
{
	int MinimumItemCount = 2;
}

using namespace Core;

namespace Widgets
{
	JoinTables::JoinTables(QWidget* parent)
		:QWidget(parent)
	{
		SetupUi();
		ConnectSignals();
	}

	JoinTables::~JoinTables()
	{
		delete ui;
	}

	int JoinTables::CurrentStep() const
	{
		return currentStep;
	}

	void JoinTables::Next()
	{
		ui->stackedWidget->setCurrentIndex(++currentStep);
		StepFunc[currentStep]();
	}

	void JoinTables::Undo()
	{
		ui->stackedWidget->setCurrentIndex(--currentStep);
		UpdateUi();
	}

	void JoinTables::AddTables(std::map<int, QString>&& items)
	{
		ui->selected->AddTables(items);
		UpdateUi();
	}

	void JoinTables::SetupUi()
	{
		ui = new Ui::JoinTables();
		ui->setupUi(this);
		StepFunc =
		{
		  std::bind(&JoinTables::SaveTitleAndComment, this),
		  std::bind(&JoinTables::SelectStorage, this),
		  std::bind(&JoinTables::Merge, this)
		};
		UpdateUi();
		ui->warningLabel->setStyleSheet("QLabel{color : #EB3333}");
	}

	void JoinTables::ConnectSignals()
	{
		connect(ui->title_comment, &TitleCommentWidget::titleTextChanged, this,
			[&]() { emit disableOkButton(ui->title_comment->Title().isEmpty()); });

		connect(ui->selected, &SelectedTables::ItemRemoved, this, &JoinTables::UpdateUi);
	}

	void JoinTables::SaveTitleAndComment()
	{
	}

	void JoinTables::SelectStorage()
	{
		auto projectId = GenesisContextRoot::Get()->ProjectId();
		API::REST::GetTableGroups(projectId, Names::Group::MergedTableIdentified,
			[&](QNetworkReply*, QJsonDocument doc)
			{
				std::map<QString, int> idByNames;
				auto jdata = doc.object()[JsonTagNames::Data].toArray();
				for (int i = 0; i < jdata.size(); i++)
				{
					auto jitem = jdata[i].toArray();
					idByNames.emplace(jitem[1].toString(), jitem[0].toInt());
				}
				ui->edit_group->SetGroupInfo(idByNames);
			},
			[](QNetworkReply*, QNetworkReply::NetworkError err)
			{
				Notification::NotifyError(tr("Failed to load file groups information"), err);
			});
	}

	void JoinTables::Merge()
	{
		QJsonObject obj;
		obj.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
        QString table_name = ui->title_comment->Title();
        obj.insert(JsonTagNames::Title, table_name);
		obj.insert(JsonTagNames::Comment, ui->title_comment->Comment());
		if (ui->edit_group->IsGroupChecked())
		{
			const auto& [id, name] = ui->edit_group->GetGroupInfo();
			if (id > 0)
				obj.insert(JsonTagNames::TableGroupId, id);
			else
			{
				obj.insert(JsonTagNames::GroupTitle, ui->edit_group->GroupTitle());
				obj.insert(JsonTagNames::CreateGroup, true);
			}
		}
		QJsonArray tableIds;
		auto items = ui->selected->GetItems();
        for (const auto& [id, name] : items)
			tableIds.append(id);
		obj.insert(JsonTagNames::ParentIds, tableIds);
		API::REST::SaveMergedTable(obj,
            [table_name](QNetworkReply*, QJsonDocument doc)
			{
				auto obj = doc.object();
				if (obj.value(JsonTagNames::Error).toBool())
					Notification::NotifyError(tr("Failed to merge tables"), obj.value(JsonTagNames::Msg).toString());
				else
                    Notification::NotifySuccess(tr("Success"), tr("Table \"%1\" is available on the \"Merged tables\" tab").arg(table_name));
			},
			[](QNetworkReply*, QNetworkReply::NetworkError err)
			{
				Notification::NotifyError(tr("Failed to merge tables"), err);
			});
	}

	void JoinTables::UpdateUi()
	{
		const auto count = ui->selected->ItemCount();
		ui->warningLabel->setVisible(count < MinimumItemCount);
		ui->selected->SetCaptionLabel(tr("Selected tables (%1)").arg(count));
		emit disableOkButton(count < MinimumItemCount
			|| ui->title_comment->Title().isEmpty());
	}
}
