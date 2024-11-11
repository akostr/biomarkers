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
  void SetScale(float scaleX, float scaleY);  // Масштабирование
  void RotateText(double angle);  // Поворот текста
  void SetTextPosition(QPointF newPos);  // Перемещение текста

  double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details = nullptr) const override;

protected:
  void draw (GPPainter *painter) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;  // Переопределим метод для двойного клика
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;  // Начало действий с текстом
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;   // Масштабирование, перемещение, поворот
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override; // Завершение действий

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
