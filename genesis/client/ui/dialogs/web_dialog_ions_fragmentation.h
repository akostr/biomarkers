#ifndef WEB_DIALOG_IONS_FRAGMENTATION_H
#define WEB_DIALOG_IONS_FRAGMENTATION_H

#include <ui/dialogs/templates/dialog.h>
#include <QPointer>
#include <QMap>

#include <QStandardItemModel>

class QFormLayout;
class FlowLayout;
class IonsListModel;
class RangeWidget;

namespace Ui {
class WebDialogIonsFragmentation;
}

using Btns = QDialogButtonBox::StandardButton;
namespace Dialogs
{
class WebDialogIonsFragmentation : public Templates::Dialog
{
  Q_OBJECT

public:
  explicit WebDialogIonsFragmentation(QWidget *parent);
  ~WebDialogIonsFragmentation();
  void setIons(const QList<int> &ions);
  QList<int> getIons();
  bool getSum();

private slots:
  void addRange();
  void onRangeClosed(RangeWidget* selfPointer);
  void invalidateFlowItems(bool toModel);
  void checkDataValidity();

private:
  void setupUi();
  QWidget* createPlate(int ion);

  Ui::WebDialogIonsFragmentation *ui;
  QPointer<QFormLayout> m_rangesFormLayout;
  QPointer<FlowLayout> m_flowLayout;
  QPointer<IonsListModel> m_ionsModel;
  QPair<int, int> m_ionsRange;
  QMap<int, QWidget*> m_flowItemsMap;
  QMap<QWidget*, QPair<int, int>> m_ranges;


};
}//namespace Dialogs

class IonsListModel : public QStandardItemModel
{
public:
  IonsListModel(QObject *parent = nullptr);

  void setIons(const QList<int> &ions);
  QList<int> getCheckedIons() const;
  void setIonsChecked(const QList<int> &ions);

  // QAbstractItemModel interface
public:
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QVariant data(const QModelIndex &index, int role) const;
  QMap<int, QStandardItem*> m_ionsItemMap;
};
#endif // WEB_DIALOG_IONS_FRAGMENTATION_H
