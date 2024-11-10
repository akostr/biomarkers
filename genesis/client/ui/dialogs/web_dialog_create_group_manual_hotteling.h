#ifndef WEBDIALOGCREATEGROUPMANUALHOTTELING_H
#define WEBDIALOGCREATEGROUPMANUALHOTTELING_H

#include "ui/dialogs/templates/dialog.h"
#include <QSet>

class DialogLineEditEx;

class WebDialogCreateGroupManualHotteling: public Dialogs::Templates::Dialog
{
    Q_OBJECT
public:
    WebDialogCreateGroupManualHotteling(QWidget* parent, const QSet<QString>& occupiedNames);

    void setGroupAddingFunc(std::function<void (WebDialogCreateGroupManualHotteling *)> func);

    void Accept() override;

    const QString getNameGroup() const;

protected:
    std::function<void(WebDialogCreateGroupManualHotteling* dialog)> mGroupAddingFunc;
    QPointer<DialogLineEditEx> mGroupNameEdit;
    QSet<QString> mForbiddenStrings;

    void setupUi();
};

#endif // WEBDIALOGCREATEGROUPMANUALHOTTELING_H
