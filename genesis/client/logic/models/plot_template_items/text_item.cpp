#include "text_item.h"
#include <qapplication.h>

TextItem::TextItem(RootItem *rootItem)
  : PlotTemplateItem (rootItem)
  , mText(name())
  , mTextColor(QApplication::palette().text().color())
  , mFont(QApplication::font())
{

}

QJsonObject TextItem::saveToJson()
{
  QJsonObject json;
  json = PlotTemplateItem::saveToJson();
  QPointF pos = position();
  json["position_x"] = pos.x();
  json["position_y"] = pos.y();

  json["text"] = text();

  json["color"] = mTextColor.name();
  json["font"] = mFont.toString();

  return json;
}

void TextItem::loadFromJson(const QJsonObject &json)
{
  PlotTemplateItem::loadFromJson(json);
  QPointF pos;
  pos.setX(json["position_x"].toDouble());
  pos.setY(json["position_y"].toDouble());
  setPosition(pos);

  setText(json["text"].toString());

  setTextColor(QColor::fromString(json["color"].toString()));

  QFont font;
  font.fromString(json["font"].toString());
  setFont(font);
}

void TextItem::setText(const QString &text)
{
  if (mText != text)
  {
    mText = text;
    emit textChanged(text);
  }
}

void TextItem::setPosition(const QPointF &pos)
{
  if (mPosition != pos)
  {
    mPosition = pos;
    emit positionChanged(mPosition);
  }
}

void TextItem::setTextColor(const QColor &color)
{
  if (mTextColor != color)
  {
    mTextColor = color;
    emit textColorChanged(color);
  }
}

void TextItem::setFont(const QFont &font)
{
  if(mFont == font)
    return;
  auto oldFont = mFont;
  mFont = font;
  emit fontChanged(font);

  if(oldFont.pixelSize() != mFont.pixelSize())
    emit fontSizeChanged(mFont.pixelSize());
  if(oldFont.bold() != mFont.bold())
    emit boldChanged(mFont.bold());
  if(oldFont.italic() != mFont.italic())
    emit italicChanged(mFont.italic());
}

void TextItem::setFontSize(int size)
{
  if(mFont.pixelSize() == size)
    return;
  mFont.setPixelSize(size);
  emit fontSizeChanged(size);
  emit fontChanged(mFont);
}

void TextItem::setBold(bool isBold)
{
  if(mFont.bold() == isBold)
    return;
  mFont.setBold(isBold);
  emit boldChanged(mFont.bold());
  emit fontChanged(mFont);
}

void TextItem::setItalic(bool isItalic)
{
  if(mFont.italic() == isItalic)
    return;
  mFont.setItalic(isItalic);
  emit boldChanged(mFont.italic());
  emit fontChanged(mFont);
}
