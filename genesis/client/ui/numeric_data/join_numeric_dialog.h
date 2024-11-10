#ifndef JOINNUMERICDIALOG_H
#define JOINNUMERICDIALOG_H

#include "ui/dialogs/web_dialog.h"
#include <ui/dialogs/templates/dialog.h>
#include <QStackedWidget>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include <QItemSelection>
#include <QStandardItem>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

class FlowLayout;
class QComboBox;
class DialogLineEditEx;
class DialogTextEditEx;

namespace Dialogs
{
class JoinNumericDialog: public Templates::Dialog
{
  Q_OBJECT
public:
  explicit JoinNumericDialog(const QList<QPair<int, QString>> &joinIdsAndName, QWidget *parent = nullptr);


//  void switch
  void updateView();


  QList<int> selectedIds() const;

  protected:

  void Accept() override;


  QSet<int>                 mSelectedIds;
  QHash<int, QString>       mData; // ids, name

  QPointer<QWidget>         mBody;
  QPointer<QVBoxLayout>     mBodyLayout;
  QPointer<QVBoxLayout>     mInteractiveLayout;
  QPointer<DialogLineEditEx>mLineEditJoinTableName;
  QPointer<DialogTextEditEx>mTextEditJoinTableComment;
  QList<QPointer<QWidget>>  mplates;

};
}//namespace Dialogs
#endif // JOINNUMERICDIALOG_H
