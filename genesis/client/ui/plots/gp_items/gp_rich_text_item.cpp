#include "gp_rich_text_item.h"

GPRichTextItem::GPRichTextItem(GraphicsPlot *parentPlot)
  : GPItemRect(parentPlot)
  , Text(new QStaticText())
  , Font()
  , LeftMargin(0)
  , RightMargin(0)
  , TopMargin(0)
  , BottomMargin(0)
  , MinimumHeight(0)
{
  setPen(QPen(Qt::NoPen));
  setBrush(Qt::NoBrush);
  Text->setTextFormat(Qt::RichText);
}

void GPRichTextItem::SetText(const QString &htmlText)
{
  Text->setText(htmlText);
  Text->prepare(QTransform(), Font);

  auto br = QPointF(Text->size().width(), Text->size().height());

  bottomRight->setType(GPItemPosition::ptAbsolute);
  bottomRight->setParentAnchor(topLeft);
  bottomRight->setCoords(br);
}

void GPRichTextItem::SetFont(const QFont &font)
{
  Font = font;
}
//поворот текста
void GPRichTextItem::RotateText(qreal angle) {
    QTransform transform;
    transform.rotate(angle);  // Поворачиваем текст на заданный угол
    Text->prepare(transform, Font);  // Применяем трансформацию к тексту
}
//функция редактирования текста, которая будет вызываться при двойном клике мыши
void GPRichTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    Q_UNUSED(event);
    // Открываем диалоговое окно для редактирования текста
    bool ok;
    QString newText = QInputDialog::getText(nullptr, "Редактирование текста", 
                                              "Введите новый текст:", 
                                              QLineEdit::Normal, Text->text(), &ok);
    if (ok && !newText.isEmpty()) {
        SetText(newText);  // Обновляем текст, если введен новый
    }
}
//функция масштабирования текста
void GPRichTextItem::SetStretch(float scaleX, float scaleY) {
    QTransform transform;  // Создаем объект QTransform
    transform.scale(scaleX, scaleY);  // Применяем растяжение по осям X и Y

    Text->prepare(transform, Font);  // Применяем трансформацию к тексту
}

QString GPRichTextItem::GetText()
{
  return Text->text();
}

QRectF GPRichTextItem::pixelRect() const
{
  return QRectF(topLeft->pixelPosition(), bottomRight->pixelPosition());
}

QPointF GPRichTextItem::centerPixelPos() const
{
  return pixelRect().center();
}

void GPRichTextItem::SetMargins(int left, int top, int right, int bottom)
{
  LeftMargin = left;
  TopMargin = top;
  RightMargin = right;
  BottomMargin = bottom;
}

double GPRichTextItem::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if(onlySelectable && !selectable())
    return -1;

  if(BoundingRect().contains(pos))
    return 0.5;
  else return -1;
}

void GPRichTextItem::draw(GPPainter *painter)
{
  auto tmpRect = BoundingRect();
  if (tmpRect.topLeft().toPoint() == tmpRect.bottomRight().toPoint())
    return;
  QRectF rect = tmpRect.normalized();
  double clipPad = mainPen().widthF();
  QRectF boundingRect = rect.adjusted(-clipPad, -clipPad, clipPad, clipPad);
  if (boundingRect.intersects(clipRect())) // only draw if bounding rect of rect item is visible in cliprect
  {
    painter->save();
    painter->setPen(mainPen());
    painter->setBrush(mainBrush());
    painter->drawRoundedRect(rect, 10, 10);
    painter->restore();

    painter->drawStaticText(topLeft->pixelPosition(), *Text);
  }
}

void GPRichTextItem::SetMinimumWidth(int newMinimumWidth)
{
  MinimumWidth = newMinimumWidth;
}

void GPRichTextItem::SetMinimumHeight(int newMinimumHeight)
{
  MinimumHeight = newMinimumHeight;
}

QRectF GPRichTextItem::BoundingRect() const
{
  QPointF p1 = topLeft->pixelPosition() - QPointF(LeftMargin, TopMargin) ;
  QPointF p2 = bottomRight->pixelPosition() + QPointF(RightMargin, BottomMargin);
  if(p2.y() - p1.y() < MinimumHeight)
  {
    p2.setY(p1.y() + MinimumHeight);
  }
  if(p2.x() - p1.x() < MinimumWidth)
  {
    p2.setX(p1.x() + MinimumWidth);
  }
  return QRectF(p1, p2);
}
