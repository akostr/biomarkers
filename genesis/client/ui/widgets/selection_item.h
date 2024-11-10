#pragma once
#ifndef SELECTION_ITEM_H
#define SELECTION_ITEM_H

#include <QWidget>
#include <map>

namespace Ui
{
	class SelectionItem;
}

namespace Widgets
{
	class SelectionItem : public QWidget
	{
		Q_OBJECT
	public:
		explicit SelectionItem(QWidget* parent = nullptr);
		~SelectionItem();

		void SetName(const QString& name);
		void SetId(int id);

		int GetId() const;
		QString GetName() const;

	signals:
		void CloseClicked(int id);

	private:
		Ui::SelectionItem* ui = nullptr;
		int ItemId;

		void SetupUi();
		void ConnectSignals();
	};
}
#endif