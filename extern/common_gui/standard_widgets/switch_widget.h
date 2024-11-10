#pragma once

#include <QCheckBox>

class QPropertyAnimation;

class Switch : public QCheckBox
{
  Q_OBJECT

  Q_PROPERTY(int offset READ offset WRITE setOffset)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)

public:
  explicit Switch(QWidget* parent = nullptr);

  QSize sizeHint() const override; 

  QBrush brush() const;
  void setBrush(const QBrush& brsh);
  void setBrushOff(const QBrush& brsh);
  void setBrushBg(const QBrush& brsh);
  void setBrushBgOff(const QBrush& brsh);

  int offset() const;
  void setOffset(int o);

public slots:
  void setChecked(bool checked);

protected:
  void paintEvent(QPaintEvent*) override;
  void mousePressEvent(QMouseEvent*) override;
  void mouseReleaseEvent(QMouseEvent*) override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  void enterEvent(QEvent*) override;
#else
  void enterEvent(QEnterEvent*) override;
#endif
  void resizeEvent(QResizeEvent *event) override;

private:
  qreal Opacity;
  int X, Y, Height, Margin;
  QBrush Thumb;
  QBrush Brush;
  QBrush BrushOff;
  QBrush BrushBg;
  QBrush BrushBgOff;
  QPropertyAnimation* Animation;
};

