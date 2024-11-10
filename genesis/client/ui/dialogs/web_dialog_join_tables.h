#pragma once
#ifndef WEB_DIALOG_JOIN_TABLES_H
#define WEB_DIALOG_JOIN_TABLES_H

#include "ui/dialogs/templates/info.h"

#include <QWidget>
#include <QPointer>

class QPushButton;

namespace Widgets
{
    class JoinTables;
}

namespace Dialogs
{
    class WebDialogJoinTables : public Templates::Info
    {
        Q_OBJECT
    public:
        explicit WebDialogJoinTables(QWidget* parent = nullptr);
        ~WebDialogJoinTables() = default;
        void Accept() override;

        void AddTables(std::map<int, QString>&& items);

    private:
        void SetupUi();
        void ConnectSignals();
        QSize sizeHint() const override;
        void DisableOkButton(bool disable);
        void PrevStep();
        void UpdateStepUi();

        QPointer<QPushButton> PrevButton = nullptr;
        Widgets::JoinTables* JoinTableWidget = nullptr;

        QStringList Steps;
        QString NextStep;
        QString Save;
    };
}
#endif