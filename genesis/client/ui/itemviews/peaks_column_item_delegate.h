#ifndef PEAKSCOLUMNITEMDELEGATE_H
#define PEAKSCOLUMNITEMDELEGATE_H

#include <QStyledItemDelegate>

class PeaksColumnItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override;
signals:

};

#endif // PEAKSCOLUMNITEMDELEGATE_H
