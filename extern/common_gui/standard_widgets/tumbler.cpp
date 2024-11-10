#include "tumbler.h"

Tumbler::Tumbler(const QString& text, QWidget* parent)
  : QCheckBox(text, parent)
{
}

Tumbler::Tumbler(QWidget* parent)
  : QCheckBox(parent)
{
}
