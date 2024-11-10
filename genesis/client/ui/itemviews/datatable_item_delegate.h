#ifndef DATATABLEITEMDELEGATE_H
#define DATATABLEITEMDELEGATE_H


#include <QStyledItemDelegate>


class DataTableItemDelegate : public virtual QStyledItemDelegate
{
  Q_OBJECT
public:
  DataTableItemDelegate(QObject *parent = nullptr);

  void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

signals:

};

#endif // DATATABLEITEMDELEGATE_H
