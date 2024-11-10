#ifndef LINEITEM_H
#define LINEITEM_H

#include "../plot_template_model.h"

class LineItem : public PlotTemplateItem
{
  Q_OBJECT
  Q_PROPERTY(QPointF start READ start WRITE setStart NOTIFY startChanged FINAL)
  Q_PROPERTY(QPointF end READ end WRITE setEnd NOTIFY endChanged FINAL)
  Q_PROPERTY(Qt::PenStyle penStyle READ penStyle WRITE setPenStyle NOTIFY penStyleChanged FINAL)
  Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor NOTIFY penColorChanged FINAL)
  Q_PROPERTY(int penWidth READ penWidth WRITE setPenWidth NOTIFY penWidthChanged FINAL)
  Q_PROPERTY(int lineEndingType READ lineEndingType WRITE setLineEndingType NOTIFY lineEndingTypeChanged FINAL)

public:
  explicit LineItem(RootItem *parent);

  virtual QJsonObject saveToJson() override;
  virtual void loadFromJson(const QJsonObject&json) override;
  virtual ItemType type() const override { return LineType; };

  QPointF start();
  QPointF end();
  Qt::PenStyle penStyle();
  QColor penColor();
  int penWidth();
  int lineEndingType();

  void setStart(QPointF);
  void setEnd(QPointF);
  void setPenStyle(Qt::PenStyle);
  void setPenColor(QColor);
  void setPenWidth(int width);
  void setLineEndingType(int);

signals:
  void startChanged(QPointF);
  void endChanged(QPointF);
  void penStyleChanged(Qt::PenStyle);
  void penColorChanged(QColor);
  void penWidthChanged(int);
  void lineEndingTypeChanged(int);

private:
  QPointF mStart;
  QPointF mEnd;
  Qt::PenStyle mPenStyle;
  QColor mPenColor;
  int mPenWidth;
  int mLineEndingType;


};

#endif // LINEITEM_H
