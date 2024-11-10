#ifndef CUSTOMCHECKBOX_H
#define CUSTOMCHECKBOX_H

#include <QCheckBox>

class CustomCheckBox : public QCheckBox
{
  Q_OBJECT
public:
  CustomCheckBox(const QString &text, QWidget *parent = nullptr);
  CustomCheckBox(QWidget *parent = nullptr);


  // QAbstractButton interface
protected:
  void nextCheckState() override;
};

#endif // CUSTOMCHECKBOX_H
