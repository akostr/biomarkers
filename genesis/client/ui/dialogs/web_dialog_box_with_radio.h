#ifndef CHROMOTOGRAMTESTDIALOG_H
#define CHROMOTOGRAMTESTDIALOG_H

#include "../dialogs/web_dialog.h"

#include <QObject>

#include <QDialog>
#include <QMap>
#include <QHash>

class QRadioButton;

class WebDialogBoxWithRadio : public WebDialog
{
    Q_OBJECT
public:
    explicit WebDialogBoxWithRadio(QWidget* parent = nullptr, const QString &captionLable = QString());
    void init(const QString &captionLable);

    QList<QRadioButton*> makeRadioButtonsWithHandle(QList< QPair< QString, std::function<void ()> >> onClickedButtons,
                                                    const QString &lableForRadioButons = QString(),
                                                     QList<std::function<void ()> > handlesAccept = {},
                                                     std::function<void ()> handleReject = {});

    void Accept() override;
    void Reject() override;




    QWidget* body();
    QDialogButtonBox* buttonBox();

    static QWidget* makeChromotogramRadioBox(QList<uint> gkList,
                                             QStringList listCriterions,
                                             const std::function<void (uint currentGK, QString criterion)> &handleCalcHotRestApi,
                                             const std::function<void (uint)> &handleCalcHotManual);

protected:
    QPointer<QWidget>     m_body;
    QPointer<QVBoxLayout> m_bodyLayout;
    QHash<QString, QRadioButton*> m_hashNameButtonPtr;
    bool                  skipOnAccept = false;
    std::function<void()> onReject;


    // WebDialog interface
public slots:
};



#endif // CHROMOTOGRAMTESTDIALOG_H
