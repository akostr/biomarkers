#ifndef DIALOGADDCHROMATOGRAMMS_H
#define DIALOGADDCHROMATOGRAMMS_H

#include <QStackedLayout>
#include "qcombobox.h"
#include "ui/dialogs/web_dialog.h"
#include "ui/dialogs/web_dialog_stacked.h"
#include <QHash>


namespace Ui {
class DialogAddSlaveChromatogramm;
}


class QStandardItemModel;
class QStandardItem;
class QItemSelection;
class QSortFilterProxyModel;
class WebDialogBoxWithRadio;

namespace GenesisMarkup{


class DialogAddReferenceWithMarkup;
class DialogAddSlaveChromatogramm;

class IDialogAddChromatogramms{
public:
    virtual void acceptDialog() = 0;
    virtual void cancelDialog() = 0;

    virtual void switchToAddReferenceDialog() = 0;
    virtual void switchToAddSlaveDialog() = 0;
};


class DialogAddChromatogramms: public WebDialogStacked,
                                public IDialogAddChromatogramms
{
    Q_OBJECT
public:
    enum SelectionMode{
        noneSelected,
        addReference,
        addSlave
    };
    DialogAddChromatogramms(QList<int> existedIds,int referenceId = -1, QWidget* parent = nullptr);

    SelectionMode currentMode() const;

    int getReferenceId();
    int getReferenceFileId();
    int getReferenceMarkupFileId();
    QList<int> getIdList();
protected:
    QList<int> mExistedIds;

    QPointer<WebDialogBoxWithRadio> mWebDialogBoxWithRadio;
    QPointer<DialogAddReferenceWithMarkup> mDialogAddReferenceWithMarkup;
    QPointer<DialogAddSlaveChromatogramm> mDialogAddSlaveChromatogramm;

    SelectionMode mCurrentMode = noneSelected;


    // IDialogAddChromatogramms interface
public:
    void acceptDialog() override;
    void cancelDialog() override;
    void switchToAddReferenceDialog() override;
    void switchToAddSlaveDialog() override;
};


class DialogAddReferenceWithMarkup: public WebDialog
{
    Q_OBJECT
public:
    explicit DialogAddReferenceWithMarkup(IDialogAddChromatogramms * interface,int referenceId, QWidget *parent = nullptr);

    int getNewReferenceId() const;
    int getNewReferenceFileId() const;

    enum ComboRoles
    {
      ReferenceIdRole = Qt::UserRole + 1,
      ReferenceFileIdRole
    };

public slots:
    void Accept();
    void Reject();

protected:
    void loadFiles();

    int mReferenceId;

    QPointer<QWidget>           mBody;
    QPointer<QVBoxLayout>       mBodyLayout;
    IDialogAddChromatogramms*   mIntrf;
    QPointer<QComboBox>         mCombobox;

    QHash<int, QString> mLoadedReferenceIds;
};




class DialogAddSlaveChromatogramm : public WebDialog
{
    Q_OBJECT

public:
    explicit DialogAddSlaveChromatogramm(IDialogAddChromatogramms * interface, QList<int> existedIds, QWidget *parent = nullptr);
    ~DialogAddSlaveChromatogramm();

    void initTreeView();
    QList<int> getIdList();
    void loadTreeModelData(QStandardItemModel *model);

    void checkboxingChildren(QStandardItem *item);

    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void checkboxingAllChildren(QStandardItem * child, Qt::CheckState state);
    void selectAllChildrenMIndex(const QModelIndex &index, Qt::CheckState state);

private:
    void getChildIdList(QStandardItem *child, QList<int>& ids);
    void setupUI();


private:
    QPointer<QStandardItemModel> mModel;
    QPointer<QSortFilterProxyModel> mProxy;
    bool onModelInvalidation = false;
    Ui::DialogAddSlaveChromatogramm *ui;
    IDialogAddChromatogramms* mIntrf;
    QList<int> mExistedIds;

    // WebDialog interface
public slots:
    void Accept();
    void Reject();
};
using Btns = QDialogButtonBox::StandardButton;


} // namespace GenesisMarkup


#endif // DIALOGADDCHROMATOGRAMMS_H
