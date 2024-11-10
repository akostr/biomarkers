#pragma once

#include <QWidget>

class QColor;
class QPushButton;

class ColorSelectButton : public QWidget
{
  Q_OBJECT

public:
  explicit ColorSelectButton(QWidget* parent, const QColor& color = QColor(), int size = 21);
  ~ColorSelectButton();

  void SetColor(const QColor& color);
  void SetColor(const QString& color);
  QColor GetColor() const;

signals:
  void ColorChanged();

protected slots:
  void OnClick();

protected:
  // Button as child object for applying default button .QPushButton stylesheet
  QPushButton* mButton;
};
