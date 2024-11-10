#ifndef DIALOG_ADD_SLAVE_CHROMATOGRAMM_H
#define DIALOG_ADD_SLAVE_CHROMATOGRAMM_H

//#include <QDialog>
#include "ui/dialogs/web_dialog.h"
#include <QStackedWidget>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include <QItemSelection>
#include <QStandardItem>
#include <QSortFilterProxyModel>

class DialogAddSlaveChromatogrammOld;
class WebDialogBoxWithRadio;
class DialogAddReferenceWithMarkupOld;
class QStackedLayout;
class DialogAddReferenceWithMarkupOld;
class DialogAddSlaveChromatogrammOld;

class DialogAddChromatogramm: public /*QWidget*/WebDialog
{
  Q_OBJECT
public:
  DialogAddChromatogramm(QList<int> existedIds, QWidget *parent = nullptr);

  QList<int> getIdList();

signals:
  QModelIndex selectedSlaveChromatogrammToAdd();

protected:
  void switchToPageAddReferenceWithMarkup();
  void switchToPageAddSlaveChromatogramm();

  void onAcceptedPageAddSlaveChromatogramm();

protected:

  QPointer<QWidget> mBody;
  QPointer<QLayout> mBodyLayout;
  bool test = false;
  QPointer<QStackedLayout> mStackedLayout;

  QPointer<WebDialogBoxWithRadio> mWebDialogBoxWithRadio;
  QPointer<DialogAddReferenceWithMarkupOld> mDialogAddReferenceWithMarkup;
  QPointer<DialogAddSlaveChromatogrammOld> mDialogAddSlaveChromatogramm;

  QList<int> mExistedIds;
};

#endif // DIALOG_ADD_SLAVE_CHROMATOGRAMM_H
