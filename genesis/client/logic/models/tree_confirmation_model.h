#ifndef TREECONFIRMATIONMODEL_H
#define TREECONFIRMATIONMODEL_H

#include <QAbstractItemModel>
namespace Dialogs
{
  namespace Templates
  {
    class TreeItem;
  }
}

class TreeConfirmationModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  struct TreeItem
  {
    explicit TreeItem(TreeItem* parent) : parent(parent){}
    QList<TreeItem*> children;
    TreeItem* parent = nullptr;
    QVariant displayRoleData;
    QVariant userRoleData;
    bool isChecked = false;
    bool isRoot(){return !parent;}
    //root item has fileId == -1; fileName == "invalid"; parent == nullptr.
    //root item will not displayed
  };

  explicit TreeConfirmationModel(QObject *parent = nullptr);
  ~TreeConfirmationModel();

  // Basic functionality:

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  QModelIndex parent(TreeItem* item) const;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  // Fetch data dynamically:
  bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  // Editable:
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override;
  void setRootItem(TreeItem* newRootItem);
  void fillModel(Dialogs::Templates::TreeItem* item);

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  // Add data:
  bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

  // Remove data:
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

  int checkedCount() const;

signals:
  void checkedCountChanged(int checkedCount);

private:
  TreeItem* mRootItem;
  int mCheckedCount = 0;

private:
  TreeItem* itemFromIndex(const QModelIndex& index) const;
  void clearModel();
  void clearItem(TreeItem *item);
  void recalcCheckedCount();
};

#endif // TREECONFIRMATIONMODEL_H
