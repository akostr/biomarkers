#ifndef ITABORDERLIST_H
#define ITABORDERLIST_H

#include <QWidgetList>
#include <QWidget>

class iTabOrderList
{
public:
  iTabOrderList(){};
  virtual ~iTabOrderList(){};
  virtual QWidgetList tabOrderWidgets() = 0;
};

class iTabOrderWidget : public QWidget, public iTabOrderList
{
  Q_OBJECT
public:
  iTabOrderWidget(QWidget* parent) : QWidget(parent) {};
  virtual ~iTabOrderWidget(){};

signals:
  void tabOrderChanged();
};

#endif // ITABORDERLIST_H
