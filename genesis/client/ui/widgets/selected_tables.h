#pragma once
#ifndef SELECTED_TABLES_H
#define SELECTED_TABLES_H

#include <QWidget>

namespace Ui
{
	class SelectedTables;
}

namespace Widgets
{
	class SelectionItem;

	class SelectedTables : public QWidget
	{
		Q_OBJECT
	public:
		explicit SelectedTables(QWidget* parent = nullptr);
		~SelectedTables();

		void SetCaptionLabel(const QString& text);
		void AddTables(const std::map<int, QString>& items);
		std::map<int, QString> GetItems() const;
		int ItemCount() const;

	signals :
		void ItemRemoved();

	private:
		Ui::SelectedTables* ui = nullptr;
		std::map<int, SelectionItem*> ItemsById;

		void SetupUi();
		void RemoveItem(int id);
	};
}

#endif