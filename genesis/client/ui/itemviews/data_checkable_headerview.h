#pragma once
#ifndef DATA_CHECKABLE_HEADERVIEW_H
#define DATA_CHECKABLE_HEADERVIEW_H

#include <QHeaderView>

namespace Control
{
  class DataCheckableHeaderView : public QHeaderView
  {
    Q_OBJECT
  public:
    DataCheckableHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);

  protected:
    const static int platesMargins = 5;
    QRect CheckBoxLocalRect;

    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
    void paintSortIcon(QPainter* painter, const QRect& rect, int logicalIndex) const;
    void mousePressEvent(QMouseEvent* event);


    void resizeEvent(QResizeEvent* event) override;
  private:
    QIcon IconSortAsc;
    QIcon IconSortDes;
    int FirstColumnIndentation = 0;
  };
}
#endif
