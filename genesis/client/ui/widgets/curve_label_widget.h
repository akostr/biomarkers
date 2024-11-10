#pragma once
#ifndef CURVE_LABEL_WIDGET_H
#define CURVE_LABEL_WIDGET_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QPointer>

class CurveLabelWidget : public QWidget
{
public:
  CurveLabelWidget(const QString& text, const QColor& color, QWidget* parent = nullptr);
  ~CurveLabelWidget();
  void setText(const QString& text);
  void setColor(const QColor& color);
  QString text() const;
  QColor color() const;

private:
  void setupUi();
  QPixmap createPixmap(QColor color);

  void mouseDoubleClickEvent(QMouseEvent* event) override;

  QPointer<QLabel> m_colorLabel;
  QPointer<QLabel> m_textLabel;

  QString m_text;
  QColor m_color;
};

#endif