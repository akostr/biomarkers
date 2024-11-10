#include "web_dialog_join_tables.h"

#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>

#include "genesis_style/style.h"
#include "ui/widgets/join_tables.h"

using namespace Widgets;

namespace Dialogs
{
	WebDialogJoinTables::WebDialogJoinTables(QWidget* parent)
		: Templates::Info(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, new JoinTables())
	{
		SetupUi();
		ConnectSignals();
	}

	void WebDialogJoinTables::Accept()
	{
		JoinTableWidget->Next();
		if (JoinTableWidget->CurrentStep() >= Steps.count())
			Done(QDialog::Accepted);
		else
			UpdateStepUi();
	}

	void WebDialogJoinTables::AddTables(std::map<int, QString>&& items)
	{
		JoinTableWidget->AddTables(std::forward<std::map<int, QString>>(items));
	}

	void WebDialogJoinTables::SetupUi()
	{
		Steps = {
		  tr("Step 1 of 2. Set name and comment"),
		  tr("Step 2 of 2. Storage place choice"),
		};
		NextStep = tr("Next step");
		Save = tr("Save");
		JoinTableWidget = findChild<JoinTables*>();
		PrevButton = new QPushButton(this);
		PrevButton->setText(tr("Previous step"));
		PrevButton->setIcon(QIcon("://resource/icons/icon_button_lt.png"));
		PrevButton->setStyleSheet(Style::Genesis::GetUiStyle());
		PrevButton->setProperty("secondary", true);
		PrevButton->setVisible(false);
		connect(PrevButton, &QPushButton::clicked, this, &WebDialogJoinTables::PrevStep);
		PrevButton->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qpushbutton.qss"));
		ButtonLayout->insertWidget(0, PrevButton);
		Settings settings;
		settings.dialogHeader = tr("Join tables");
		settings.buttonsNames =
		{
		  { QDialogButtonBox::Ok, NextStep },
		};
		settings.buttonsProperties = { { QDialogButtonBox::Ok, { {"blue", true} } } };
		applySettings(settings);
		setContentHeader(Steps[JoinTableWidget->CurrentStep()]);
        Size = { 0.3, 0.5 };
	}

	void WebDialogJoinTables::ConnectSignals()
	{
		connect(JoinTableWidget, &JoinTables::disableOkButton, this, &WebDialogJoinTables::DisableOkButton);
	}

	QSize WebDialogJoinTables::sizeHint() const
	{
		return { 1, 1 };
	}

	void WebDialogJoinTables::DisableOkButton(bool disable)
	{
		ButtonBox->button(QDialogButtonBox::Ok)->setDisabled(disable);
	}

	void WebDialogJoinTables::PrevStep()
	{
		JoinTableWidget->Undo();
		if (JoinTableWidget->CurrentStep() >= 0)
			UpdateStepUi();
	}

	void WebDialogJoinTables::UpdateStepUi()
	{
		const auto current = JoinTableWidget->CurrentStep();
		setContentHeader(Steps[current]);
		ButtonBox->button(QDialogButtonBox::Ok)->setText((current == Steps.count() - 1) ? Save : NextStep);
		PrevButton->setVisible(current);
	}
}
