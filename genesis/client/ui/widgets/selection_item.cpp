#include "selection_item.h"

#include "ui_selection_item.h"

namespace Widgets
{

	SelectionItem::SelectionItem(QWidget* parent)
		:QWidget(parent)
	{
		SetupUi();
		ConnectSignals();
	}

	SelectionItem::~SelectionItem()
	{
		delete ui;
	}

	void SelectionItem::SetName(const QString& name)
	{
		ui->ItemNameLabel->setText(name);
	}

	void SelectionItem::SetId(int id)
	{
		ItemId = id;
	}

	int SelectionItem::GetId() const
	{
		return ItemId;
	}

	QString SelectionItem::GetName() const
	{
		return ui->ItemNameLabel->text();
	}

	void SelectionItem::SetupUi()
	{
		ui = new Ui::SelectionItem();
		ui->setupUi(this);
		ui->horizontalLayout->setContentsMargins(0, 0, 0, 0);
	}

	void SelectionItem::ConnectSignals()
	{
		connect(ui->closeButton, &QPushButton::clicked, this, [&]() { CloseClicked(ItemId); });
	}
}