#pragma once

#include <QLabel>
#include <QString>
#include <QWidget>

class ElidedLabel : public QLabel
{
  Q_OBJECT
  Q_PROPERTY(bool Elided READ IsElided);

public:
  explicit ElidedLabel(const QString& text, QWidget* parent = nullptr);
  explicit ElidedLabel(QWidget* parent = nullptr);
  ~ElidedLabel();

  bool IsElided() const;

protected:
  void paintEvent(QPaintEvent *event) override;

signals:
  void ElisionChanged(bool elided);

private:
  bool Elided;
};
