#pragma once
#ifndef WEB_DIALOG_CALCULATE_COEFFICIENT_TABLE_H
#define WEB_DIALOG_CALCULATE_COEFFICIENT_TABLE_H

#include "ui/dialogs/templates/info.h"

#include <QWidget>
#include <QPointer>

class CoefficientTableCalc;
class QPushButton;

namespace Dialogs
{
	class WebDialogCalculateCoefficientTable : public Templates::Info
	{
		Q_OBJECT

	public:
		explicit WebDialogCalculateCoefficientTable(QWidget* parent = nullptr);
		~WebDialogCalculateCoefficientTable() = default;

		void Accept() override;
		void LoadCoefficientByTableId(int id);

	private:
		CoefficientTableCalc* CalcWidget = nullptr;
		QPointer<QPushButton> PrevButton = nullptr;
		QList<QSizeF> StepSizes;

		void SetupUi();
		void ConnectSignals();
		void UpdateStepUi();
		void PrevStep();
		void DisableOkButton(bool disable);

		QSize sizeHint() const override;

		QStringList Steps;
		QString NextStep;
		QString Save;
	};
}
#endif