#ifndef SPINBOXACTION_H
#define SPINBOXACTION_H

#include <QWidgetAction>
#include <QPointer>

class QSpinBox;
class QLabel;
class SpinBoxAction : public QWidgetAction
{
  Q_OBJECT
public:
  explicit SpinBoxAction(const QString& title, QWidget *parent = nullptr);

  QPointer<QSpinBox> SpinBox;
  QPointer<QLabel> Label;
};

#endif // SPINBOXACTION_H
