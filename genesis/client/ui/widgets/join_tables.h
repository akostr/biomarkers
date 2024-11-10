#pragma once
#ifndef JOIN_TABLES_H
#define JOIN_TABLES_H

#include <QWidget>

namespace Ui
{
	class JoinTables;
}

namespace Widgets
{
	class JoinTables : public QWidget
	{
		Q_OBJECT
	public:
		explicit JoinTables(QWidget* parent = nullptr);
		~JoinTables();

		int CurrentStep() const;
		void Next();
		void Undo();

		void AddTables(std::map<int, QString>&& items);

	signals:
		void disableOkButton(bool disable);

	private:
		Ui::JoinTables* ui = nullptr;
		std::vector<std::function<void()>> StepFunc;
		int currentStep = 0;

		void SetupUi();
		void ConnectSignals();

		void SaveTitleAndComment();
		void SelectStorage();
		void Merge();

		void UpdateUi();
	};
}
#endif