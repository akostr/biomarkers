#pragma once

#include <QWidget>
#include <QDateTime>
#include <functional>

class QDoubleSpinBox;
class QDateTimeEdit;
class QLabel;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Base template interface for custom range widgets
template <class T>
class RangeHandler
{
public:
  RangeHandler(int stepsCount, T defaultMin, T defaultMax)
    : mStepsCount(stepsCount)
    , mRangeMin(defaultMin)
    , mRangeMax(defaultMax)
  {}

  T Start()
  {
    return qBound(mRangeMin, GetValue(0), mRangeMax);
  }
  T Stop()
  {
    return qBound(mRangeMin, GetValue(mStepsCount - 1), mRangeMax);
  }
  T StepAt(int index) const
  {
    return qBound(mRangeMin, GetValue(index), mRangeMax);
  }
  QVector<T> Steps() const
  {
    QVector<T> steps;
    for (int i = 0; i < mStepsCount; ++i)
      steps << qBound(mRangeMin, GetValue(i), mRangeMax);
    return std::move(steps);
  }

  QPair<T, T> RangeMinMax() const
  {
    return { mRangeMin, mRangeMax };
  }
  T RangeMin() const
  {
    return mRangeMin;
  }
  T RangeMax() const
  {
    return mRangeMax;
  }
  bool RangeIsNull() const
  {
    return GetValue(0) == GetValue(mStepsCount - 1);
  }

  void SetStart(T start)
  {
    SetStepAt(0, start);
  }
  void SetStop(T stop)
  {
    SetStepAt(mStepsCount - 1, stop);
  }
  void SetStepAt(int index, const T& value)
  {
    SetValue(index, value);
  }
  void SetSteps(const QVector<T>& steps)
  {
    for (int i = 0; i < mStepsCount && i < steps.size(); ++i)
      SetValue(i, steps[i]);
  }

  void SetRangeMinMax(T min, T max)
  {
    mRangeMin = min;
    mRangeMax = max;
  }
  void SetRangeMinMax(QPair<T, T> range)
  {
    mRangeMin = range.first;
    mRangeMax = range.second;
  }

protected:
  virtual void CheckNewStepValue(int index)
  {
    T value = GetValue(index);
    T left = mRangeMin;
    T right = mRangeMax;
    if (index > 0)
      left = qMax(left, GetValue(index - 1));
    if (index < mStepsCount - 1)
      right = qMin(right, GetValue(index + 1));

    T newVal = qBound(left, value, right);
    if (newVal != value)
      SetValue(index, newVal);
  }

  // Reimplimenting interface
  virtual T GetValue(int index) const = 0;
  virtual void SetValue(int index, const T& value) = 0;

protected:
  int mStepsCount;
  T mRangeMin;
  T mRangeMax;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Base range widget interface
class BaseRangeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit BaseRangeWidget(QWidget* parent = nullptr);
  ~BaseRangeWidget();

  void SetLabels(const QStringList& labels);

  void SetMaximumEditorWidth(const int& width);
  void SetMaximumLabelWidth(const int& width);

signals:
  void RangeChanged();

protected slots:
  void OnStepEdited();

protected:
  virtual void OnEditorValueChanged(int index) = 0;

protected:
  QVector<QWidget*> mEditors;
  QVector<QLabel*> mLabels;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Range widget implementation with QDoubleSpinBox
class DoubleRangeWidget : public BaseRangeWidget, public RangeHandler<double>
{
  Q_OBJECT

public:
  DoubleRangeWidget(QWidget* parent = nullptr, int stepCount = 2, Qt::Orientation orientation = Qt::Horizontal);
  virtual ~DoubleRangeWidget();

  void SetSuffix(const QString& suffix);

protected:
  void OnEditorValueChanged(int index);
  virtual double GetValue(int index) const;
  virtual void SetValue(int index, const double& value);
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Range widget implementation with QDateTimeEdit
class DateTimeRangeWidget : public BaseRangeWidget , public RangeHandler<QDateTime>
{
  Q_OBJECT

public:
  DateTimeRangeWidget(QWidget* parent = nullptr, int stepCount = 2);
  virtual ~DateTimeRangeWidget();

  void SetDisplayFormat(const QString& format);

protected:
  void OnEditorValueChanged(int index);
  virtual QDateTime GetValue(int index) const;
  virtual void SetValue(int index, const QDateTime& value);
};
