#pragma once

#include <QSlider>

////////////////////////////////////////////////////
//// Fwds
class QStylePainter;
class RangeSliderPrivate;

////////////////////////////////////////////////////
//// Range Slider widget
class RangeSlider : public QSlider
{
  Q_OBJECT

  Q_PROPERTY(int minimumValue READ minimumValue WRITE setMinimumValue)
  Q_PROPERTY(int maximumValue READ maximumValue WRITE setMaximumValue)
  Q_PROPERTY(int minimumPosition READ minimumPosition WRITE setMinimumPosition)
  Q_PROPERTY(int maximumPosition READ maximumPosition WRITE setMaximumPosition)
  Q_PROPERTY(bool symmetricMoves READ symmetricMoves WRITE setSymmetricMoves)
  Q_PROPERTY(QString handleToolTip READ handleToolTip WRITE setHandleToolTip)

public:  
  // Range slider
  explicit RangeSlider(Qt::Orientation o, QWidget* parent = nullptr);
  explicit RangeSlider(QWidget* parent = nullptr);
  virtual ~RangeSlider();

  // Values
  int minimumValue() const;
  int maximumValue() const;

  // Positions
  int minimumPosition() const;
  void setMinimumPosition(int min);
  int maximumPosition() const;
  void setMaximumPosition(int max);
  void setPositions(int min, int max);
  
  bool symmetricMoves() const; 
  void setSymmetricMoves(bool symmetry);

  QString handleToolTip()const;
  void setHandleToolTip(const QString& toolTip);

  bool isMinimumSliderDown()const;
  bool isMaximumSliderDown()const;

signals:
  // Values
  void minimumValueChanged(int min);
  void maximumValueChanged(int max);
  void valuesChanged(int min, int max);

  // Positions
  void minimumPositionChanged(int min);
  void maximumPositionChanged(int max);
  void positionsChanged(int min, int max);

public slots:
  void setMinimumValue(int min);
  void setMaximumValue(int max);
  void setValues(int min, int max);

protected Q_SLOTS:
  void OnRangeChanged(int minimum, int maximum);

protected:
  RangeSlider(RangeSliderPrivate* impl, Qt::Orientation o, QWidget* parent = nullptr);
  RangeSlider(RangeSliderPrivate* impl, QWidget* parent = nullptr);

  //// Overriden
  virtual void mousePressEvent(QMouseEvent* ev) override;
  virtual void mouseMoveEvent(QMouseEvent* ev) override;
  virtual void mouseReleaseEvent(QMouseEvent* ev) override;
  virtual void paintEvent(QPaintEvent* ev) override;
  virtual void initMinimumSliderStyleOption(QStyleOptionSlider* option) const;
  virtual void initMaximumSliderStyleOption(QStyleOptionSlider* option) const;
  virtual bool event(QEvent* event) override;

protected:
  QScopedPointer<RangeSliderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(RangeSlider);
  Q_DISABLE_COPY(RangeSlider);
};
