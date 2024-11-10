#ifndef GPSHAPE_WITH_LABEL_ITEM_H
#define GPSHAPE_WITH_LABEL_ITEM_H
#include "gpshape_item.h"

class GPShapeWithLabelItem : public GPShapeItem
{
  Q_OBJECT
public:
  GPShapeWithLabelItem(GraphicsPlot *parentPlot,
                       QPointF pos = QPointF(0,0),
                       GPShape::ShapeType shape = GPShape::Circle,
                       QSizeF size = QSizeF(5,5),
                       QString labelText = "",
                       QFont font = QFont());
  GPShapeWithLabelItem(GraphicsPlot *parentPlot,
                       QPointF pos = QPointF(0,0),
                       const QPainterPath& shape = QPainterPath(),
                       int pixelSize = 5,
                       QString labelText = "",
                       QFont font = QFont());

  //  virtual ~GPShapeWithLabelItem() =default;

  void SetSize(QSizeF size);
  QString getLabelText() const;
  const QFont &getLabelFont() const;

  void setLabelText(const QString &newLabelText);
  void setLabelFont(const QFont &newLabelFont);
  void setMargins(int left, int right, int top, int bottom);
  void setLabelSpacing(int spacing);
  QRectF boundingRect() const;
  void setShapePen(const QPen &pen);
  void setShapeSelectedPen(const QPen &pen);
  void setPlatePen(const QPen &pen);
  void setPlateSelectedPen(const QPen &pen);
  void setShapeBrush(const QBrush &brush);
  void setShapeSelectedBrush(const QBrush &brush);
  void setPlateBrush(const QBrush &brush);
  void setPlateSelectedBrush(const QBrush &brush);
  void setColor(unsigned int color);
  void setColor(QColor color);
  virtual void setInactive(bool isInactive);
  bool isInactive();
  unsigned int getColor();

  void setLabelHidden(bool hidden);

  double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details = nullptr) const;
  void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) override;
  void deselectEvent(bool *selectionStateChanged) override;

protected:
  void draw(GPPainter *painter) override;

  QFont LabelFont;
  int LeftMargin,
      RightMargin,
      TopMargin,
      BottomMargin;
  int LabelSpacing;
  bool isLabelHidden;
  bool IsInactive;
  GPShapeItem::DefaultColor Color;

private:
  using GPItemRect::setPen;
  using GPItemRect::setBrush;
  using GPItemRect::setSelectedPen;
  using GPItemRect::setSelectedBrush;

  const QPen& plateMainPen();
  const QBrush& plateMainBrush();
  QColor activeShapeColor;
  QColor inactiveShapeColor;

  QScopedPointer<QStaticText> Text;
  QPen PlatePen, PlateSelectedPen;
  QBrush PlateBrush, PlateSelectedBrush;

};

#endif // GPSHAPE_WITH_LABEL_ITEM_H
