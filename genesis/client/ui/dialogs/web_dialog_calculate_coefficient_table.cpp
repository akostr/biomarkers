#include "web_dialog_calculate_coefficient_table.h"

#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>

#include "ui/widgets/coefficient_table_calc.h"
#include "genesis_style/style.h"

namespace Dialogs
{
	WebDialogCalculateCoefficientTable::WebDialogCalculateCoefficientTable(QWidget* parent)
		: Templates::Info(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, new CoefficientTableCalc())
	{
		SetupUi();
		ConnectSignals();
	}

	void WebDialogCalculateCoefficientTable::Accept()
	{
		CalcWidget->Next();
		if (CalcWidget->CurrentStep() >= Steps.count())
			Done(QDialog::Accepted);
		else
			UpdateStepUi();
	}

	void WebDialogCalculateCoefficientTable::LoadCoefficientByTableId(int id)
	{
		CalcWidget->LoadCoefficient(id);
	}

	QSize WebDialogCalculateCoefficientTable::sizeHint() const
	{
		return { 1, 1 };
	}

	void WebDialogCalculateCoefficientTable::SetupUi()
	{
		Steps = {
				tr("Step 1 of 4. Coefficient choice"),
				tr("Step 2 of 4. Table preview"),
				tr("Step 3 of 4. Set name and comment"),
				tr("Step 4 of 4. Storage place choice"),
		};
		NextStep = tr("Next step");
		Save = tr("Save");
		PrevButton = new QPushButton(this);
		PrevButton->setText(tr("Previous step"));
		PrevButton->setIcon(QIcon("://resource/icons/icon_button_lt.png"));
		PrevButton->setStyleSheet(Style::Genesis::GetUiStyle());
		PrevButton->setProperty("secondary", true);
		PrevButton->setVisible(false);
		PrevButton->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qpushbutton.qss"));
		ButtonLayout->insertWidget(0, PrevButton);
		CalcWidget = findChild<CoefficientTableCalc*>();
		Settings settings;
		settings.dialogHeader = tr("Calculation coefficient table");
		settings.buttonsNames =
		{
		  { QDialogButtonBox::Ok, NextStep },
		};
		settings.buttonsProperties = { { QDialogButtonBox::Ok, { {"blue", true} } } };
        StepSizes = { {0.75, 0.85} , {0.75, 0.85}, {0.3, 0.4}, {0.3, 0.4} };
		Size = StepSizes[0];
		applySettings(settings);
		setContentHeader(Steps[CalcWidget->CurrentStep()]);
		UpdateStepUi();
		DisableOkButton(true);
	}

	void WebDialogCalculateCoefficientTable::ConnectSignals()
	{
		connect(CalcWidget, &CoefficientTableCalc::disableOkButton, this, &WebDialogCalculateCoefficientTable::DisableOkButton);
		connect(PrevButton, &QPushButton::clicked, this, &WebDialogCalculateCoefficientTable::PrevStep);
	}

	void WebDialogCalculateCoefficientTable::UpdateStepUi()
	{
		const auto current = CalcWidget->CurrentStep();
		setContentHeader(Steps[current]);
		ButtonBox->button(QDialogButtonBox::Ok)->setText((current == Steps.count() - 1) ? Save : NextStep);
		PrevButton->setVisible(current);
		Size = StepSizes[current];
	}

	void WebDialogCalculateCoefficientTable::PrevStep()
	{
		CalcWidget->Undo();
		if (CalcWidget->CurrentStep() >= 0)
			UpdateStepUi();
	}

	void WebDialogCalculateCoefficientTable::DisableOkButton(bool disable)
	{
		ButtonBox->button(QDialogButtonBox::Ok)->setDisabled(disable);
	}
}
