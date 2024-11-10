#ifndef TEXTITEM_H
#define TEXTITEM_H

#include "../plot_template_model.h"

class TextItem : public PlotTemplateItem
{
  Q_OBJECT
public:
  TextItem(RootItem *parent);

  virtual QJsonObject saveToJson();
  virtual void loadFromJson(const QJsonObject &);
  virtual ItemType type() const { return TextType; }

  //getters
  QString text() const { return mText; }
  QPointF position() const { return mPosition; }
  QColor textColor() const { return mTextColor; }
  int fontSize() const{ return mFont.pixelSize();}
  bool bold() const { return mFont.bold();};
  bool italic() const { return mFont.italic();}
  QFont font() const { return mFont; }

  //setters
  void setText(const QString&);
  void setPosition(const QPointF&);
  void setTextColor(const QColor&);
  void setFont(const QFont&);
  void setFontSize(int size);
  void setBold(bool isBold);
  void setItalic(bool isItalic);
  void setIsBold(bool newIsBold);
  void setIsItalic(bool newIsItalic);

signals:
  void textChanged(QString);
  void positionChanged(QPointF);
  void textColorChanged(QColor);
  void fontChanged(QFont);
  void fontSizeChanged(int);
  void boldChanged(bool);
  void italicChanged(bool);

private:
  QString mText;
  QPointF mPosition;
  QColor mTextColor;
  QFont mFont;

  Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged FINAL)
  Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged FINAL)
  Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
  Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged FINAL)
  Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY boldChanged FINAL)
  Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY italicChanged FINAL)
};

#endif // TEXTITEM_H
