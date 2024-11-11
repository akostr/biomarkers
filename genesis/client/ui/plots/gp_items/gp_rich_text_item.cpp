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
//функция масштабирования, переноса и поворота текста
private:
    QPointF mCenter;  // Центр текста
    bool mIsScaling = false;  // Флаг для отслеживания масштабирования
    bool mIsMoving = false;   // Флаг для отслеживания перемещения
    bool mIsRotating = false; // Флаг для отслеживания поворота
    double mInitialDistance = 0.0; // Начальное расстояние для масштабирования
    double mInitialAngle = 0.0;    // Начальный угол для поворота

void GPRichTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        mCenter = this->boundingRect().center();  // Вычисляем центр текста

        // Вычисляем начальную дистанцию для масштабирования
        mInitialDistance = std::sqrt(std::pow(event->scenePos().x() - mCenter.x(), 2) +
                                     std::pow(event->scenePos().y() - mCenter.y(), 2));

        // Вычисляем начальный угол для поворота
        mInitialAngle = std::atan2(event->scenePos().y() - mCenter.y(), event->scenePos().x() - mCenter.x());

        mIsScaling = true;  // Начинаем масштабирование или поворот
    }
    QGraphicsItem::mousePressEvent(event);  // Стандартное поведение для нажатия
}

void GPRichTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (mIsScaling) {
        double currentDistance = std::sqrt(std::pow(event->scenePos().x() - mCenter.x(), 2) +
                                           std::pow(event->scenePos().y() - mCenter.y(), 2));

        double scaleFactor = currentDistance / mInitialDistance;  // Коэффициент масштабирования
        SetScale(scaleFactor, scaleFactor);  // Масштабируем текст
    }

    if (mIsMoving) {
        // Перемещение текста
        QPointF delta = event->scenePos() - mCenter;
        SetTextPosition(mCenter + delta);
    }

    if (mIsRotating) {
        double currentAngle = std::atan2(event->scenePos().y() - mCenter.y(), event->scenePos().x() - mCenter.x());
        double deltaAngle = currentAngle - mInitialAngle;
        RotateText(deltaAngle);  // Поворот текста
    }

    QGraphicsItem::mouseMoveEvent(event);  // Стандартное поведение для перемещения
}

void GPRichTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (mIsScaling || mIsMoving || mIsRotating) {
        mIsScaling = false;
        mIsMoving = false;
        mIsRotating = false;  // Завершаем действия
    }
    QGraphicsItem::mouseReleaseEvent(event);  // Стандартное поведение для отпускания кнопки
}

void GPRichTextItem::SetScale(float scaleX, float scaleY) {
    QTransform transform;
    transform.scale(scaleX, scaleY);  // Масштабируем текст
    Text->prepare(transform, Font);    // Применяем масштабирование
}

void GPRichTextItem::SetTextPosition(QPointF newPos) {
    this->setPos(newPos);  // Перемещаем текст
}

void GPRichTextItem::RotateText(double angle) {
    QTransform transform;
    transform.translate(mCenter.x(), mCenter.y());  // Сдвигаем в центр
    transform.rotate(qRadiansToDegrees(angle));  // Поворачиваем текст
    transform.translate(-mCenter.x(), -mCenter.y());  // Возвращаем текст на место
    Text->prepare(transform, Font);  // Применяем поворот
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
