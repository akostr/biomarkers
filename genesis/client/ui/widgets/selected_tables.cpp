#include "selected_tables.h"

#include "ui_selected_tables.h"

#include "selection_item.h"
#include "genesis_style/style.h"

namespace Widgets
{
	SelectedTables::SelectedTables(QWidget* parent)
		: QWidget(parent)
	{
		SetupUi();
	}

	SelectedTables::~SelectedTables()
	{
		delete ui;
	}

	void SelectedTables::SetCaptionLabel(const QString& text)
	{
		ui->CaptionLabel->setText(text);
	}

	void SelectedTables::AddTables(const std::map<int, QString>& items)
	{
		for (const auto& [id, name] : items)
		{
			auto item = new SelectionItem(this);
			item->SetName(QString("%1 (%2)").arg(name).arg(id));
			item->SetId(id);
			connect(item, &SelectionItem::CloseClicked, this, &SelectedTables::RemoveItem);
			ItemsById.try_emplace(id, item);
			ui->verticalLayout->addWidget(item);
		}
		adjustSize();
	}

	std::map<int, QString> SelectedTables::GetItems() const
	{
		std::map<int, QString> selected;
		for (const auto& [id, item] : ItemsById)
			selected.emplace(id, item->GetName());
		return selected;
	}

	int SelectedTables::ItemCount() const
	{
		return static_cast<int>(ItemsById.size());
	}

	void SelectedTables::SetupUi()
	{
		ui = new Ui::SelectedTables();
		ui->setupUi(this);
		ui->CaptionLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
		ui->verticalLayout->setContentsMargins(0,0,0,0);
	}

	void SelectedTables::RemoveItem(int id)
	{
		const auto finded = ItemsById.find(id);
		if (finded == ItemsById.end())
			return;
		auto item = finded->second;
		item->setParent(nullptr);
		ui->verticalLayout->removeWidget(item);
		ItemsById.erase(id);
		emit ItemRemoved();
	}
}