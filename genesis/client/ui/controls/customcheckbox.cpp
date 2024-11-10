#include "customcheckbox.h"

CustomCheckBox::CustomCheckBox(const QString &text, QWidget *parent)
  : QCheckBox(text, parent)
{

}

CustomCheckBox::CustomCheckBox(QWidget *parent)
  : QCheckBox(parent)
{

}

void CustomCheckBox::nextCheckState()
{
  switch(checkState())
  {
  case Qt::Unchecked:
    setCheckState(Qt::Checked);
    break;
  case Qt::PartiallyChecked:
    setCheckState(Qt::Unchecked);
    break;
  case Qt::Checked:
    setCheckState(Qt::Unchecked);
    break;
  }
}
