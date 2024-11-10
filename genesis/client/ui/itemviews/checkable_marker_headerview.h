#ifndef CHECKABLEMARKERHEADERVIEW_H
#define CHECKABLEMARKERHEADERVIEW_H

#include "checkable_headerview.h"

class CheckableMarkerHeaderView : public CheckableHeaderView
{
  Q_OBJECT
public:
  CheckableMarkerHeaderView(Qt::Orientation orientation, QWidget *parent);

protected:
  void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
  QSize sectionSizeFromContents(int logicalIndex) const override;

private:
};

#endif // CHECKABLEMARKERHEADERVIEW_H
