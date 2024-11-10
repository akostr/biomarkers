#pragma once

#include <QCheckBox>

class Tumbler : public QCheckBox
{
  Q_OBJECT

public:
  explicit Tumbler(const QString& text, QWidget* parent = nullptr);
  explicit Tumbler(QWidget* parent = nullptr);
};
