#ifndef PEN_STYLE_COMBO_BOX_H
#define PEN_STYLE_COMBO_BOX_H

#include <QComboBox>

enum PenStyleComboDataRoles
{
  RolePenStyle = Qt::UserRole + 1,
  RolePen
};

class PenStyleComboBox : public QComboBox
{
  Q_OBJECT
public:
  PenStyleComboBox(QWidget *parent = nullptr);
  void setCurrentPenStyle(Qt::PenStyle style);
  Qt::PenStyle currentPenStyle();
  QPen currentPen();
  static void drawPenStyle(QPainter *painter, const QRect &drawRect, const QPen &pen, const QColor &background);

signals:
  void penStyleChanged(Qt::PenStyle style);

  // QWidget interface
protected:
  void paintEvent(QPaintEvent *event) override;
};



#endif // PEN_STYLE_COMBO_BOX_H
