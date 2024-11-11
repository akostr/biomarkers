#ifndef GP_RICH_TEXT_ITEM_H
#define GP_RICH_TEXT_ITEM_H

#include <graphicsplot/graphicsplot.h>

class GPRichTextItem : public GPItemRect
{
  //To set the position correctly, set the top left corner of the rectangle,
  //because the bottom right corner will be recalculated after SetText()
  //relative to the top left position for TextSize.
public:
  GPRichTextItem(GraphicsPlot *parentPlot);
  void SetText(const QString& htmlText);
  void SetFont(const QFont& font);
  QString GetText();
  QRectF pixelRect() const;
  QPointF centerPixelPos() const;
  void SetMargins(int left, int top, int right, int bottom);
  void SetMinimumHeight(int newMinimumHeight);
  void SetMinimumWidth(int newMinimumWidth);
  QRectF BoundingRect() const;
  void RotateText(qreal angle);  // Добавляем функцию для поворота текста
  void SetStretch(float scaleX, float scaleY);  // Функция для растяжения текста по осям X и Y

  double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details = nullptr) const override;

protected:
  void draw (GPPainter *painter) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;  // Переопределим метод для двойного клика
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;  // Для захвата позиции текста
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;   // Для перемещения текста
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override; // Для завершения перемещения

private:
  QScopedPointer<QStaticText> Text;
  QFont Font;

  int LeftMargin,
      RightMargin,
      TopMargin,
      BottomMargin;
  int MinimumHeight;
  int MinimumWidth;
};

#endif // GP_RICH_TEXT_ITEM_H
