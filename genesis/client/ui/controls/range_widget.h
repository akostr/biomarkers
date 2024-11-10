#ifndef RANGE_WIDGET_H
#define RANGE_WIDGET_H

#include <QWidget>

namespace Ui {
class RangeWidget;
}

class RangeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit RangeWidget(QWidget *parent = nullptr);
  ~RangeWidget();

signals:
  void lowerBoundChanged(int lowerBound);
  void upperBoundChanged(int lowerBound);
  void closed(RangeWidget* selfPtr);

public slots:
  int lowerBound();
  int upperBound();

  void setLowerBound(int lowerBound);
  void setUpperBound(int upperBound);

  void setMinLowerBoundValue(int min);
  void setMaxLowerBoundValue(int max);
  void setMinUpperBoundValue(int min);
  void setMaxUpperBoundValue(int max);

  void setBounds(int min, int max);

private slots:
  void onLowerBoundChanged(int newVal);
  void onUpperBoundChanged(int newVal);

private:
  Ui::RangeWidget *ui;
};

#endif // RANGE_WIDGET_H
