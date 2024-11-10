#ifndef MARKUP_WIDGET_H
#define MARKUP_WIDGET_H

#include <QWidget>

class MarkupWidget : public QWidget
{
  Q_OBJECT

public:
  explicit MarkupWidget(QWidget *parent = nullptr);
  ~MarkupWidget();
};

#endif // MARKUP_WIDGET_H
