#ifndef FILESTABLEACTIONSCOLUMNDELEGATE_H
#define FILESTABLEACTIONSCOLUMNDELEGATE_H

#include <QStyledItemDelegate>
#include <QPointer>

class QMenu;
class QAbstractItemView;

class FilesTableActionsColumnDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit FilesTableActionsColumnDelegate(QAbstractItemView *parent = nullptr);

  void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  virtual void destroyEditor(QWidget* editor, const QModelIndex& index) const override;
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;

private:
  void Setup();

private:
  QPointer<QAbstractItemView>    View;
  mutable QPointer<QMenu> Menu;
  mutable QPoint          MenuPosition;
};

#endif // FILESTABLEACTIONSCOLUMNDELEGATE_H
