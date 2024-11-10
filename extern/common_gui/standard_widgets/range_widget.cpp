#include "range_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>

#include <float.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Base range widget interface
BaseRangeWidget::BaseRangeWidget(QWidget *parent)
  : QWidget(parent)
{
}

BaseRangeWidget::~BaseRangeWidget()
{
}

void BaseRangeWidget::SetLabels(const QStringList &labels)
{
  for (int i = 0; i < labels.size() && i < mLabels.size(); ++i)
  {
    mLabels[i]->setText(labels[i]);
    mLabels[i]->show();
  }
}

void BaseRangeWidget::SetMaximumEditorWidth(const int &width)
{
  for (const auto& sb : mEditors)
    sb->setMaximumWidth(width);
}

void BaseRangeWidget::SetMaximumLabelWidth(const int &width)
{
  for (const auto& l : mLabels)
    l->setMaximumWidth(width);
}

void BaseRangeWidget::OnStepEdited()
{
  if (auto w = qobject_cast<QWidget*>(sender()))
  {
    int index = mEditors.indexOf(w);
    if (index != -1)
    {
      OnEditorValueChanged(index);
      emit RangeChanged();
    }
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Range widget implementation with QDoubleSpinBox
DoubleRangeWidget::DoubleRangeWidget(QWidget* parent, int stepCount, Qt::Orientation orientation)
  : BaseRangeWidget(parent)
  , RangeHandler<double>(stepCount, -DBL_MAX, DBL_MAX)
{
  if (orientation == Qt::Horizontal)
  {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    for (int i = 0; i < stepCount; ++i)
    {
      auto l = new QLabel(this);
      l->setContentsMargins(0, 0, 6, 0);
      layout->addWidget(l, 0);
      l->hide();

      auto sb = new QDoubleSpinBox(this);
      sb->setRange(mRangeMin, mRangeMax);
      sb->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
      layout->addWidget(sb, 1);

      if (i != stepCount - 1)
      {
        auto delim = new QLabel("<html>&mdash;</html>", this);
        delim->setContentsMargins(6, 0, 6, 0);
        layout->addWidget(delim, 0);
      }

      mEditors << sb;
      mLabels << l;
    }
  }
  else
  {
    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setVerticalSpacing(6);
    layout->setHorizontalSpacing(0);

    int row = 0;
    for (int i = 0; i < stepCount; ++i)
    {
      auto l = new QLabel(this);
      l->setContentsMargins(0, 0, 6, 0);
      layout->addWidget(l, row, 0);
      l->hide();

      auto sb = new QDoubleSpinBox(this);
      sb->setRange(mRangeMin, mRangeMax);
      sb->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
      layout->addWidget(sb, row, 1);

      mEditors << sb;
      mLabels << l;

      row++;
    }
  }

  for (const auto& sb : mEditors)
    connect(sb, SIGNAL(editingFinished()), this, SLOT(OnStepEdited()));
}

DoubleRangeWidget::~DoubleRangeWidget()
{
}

void DoubleRangeWidget::SetSuffix(const QString& suffix)
{
  for (int i = 0; i < mEditors.size(); ++i)
    ((QDoubleSpinBox*)mEditors[i])->setSuffix(suffix);
}

void DoubleRangeWidget::OnEditorValueChanged(int index)
{
  CheckNewStepValue(index);
}

double DoubleRangeWidget::GetValue(int index) const
{
  if (auto sb = (QDoubleSpinBox*)mEditors.value(index))
    return sb->value();
  return 0;
}

void DoubleRangeWidget::SetValue(int index, const double& value)
{
  if (auto sb = (QDoubleSpinBox*)mEditors.value(index))
  {
    QSignalBlocker l(sb);
    sb->setValue(value);
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Range widget implementation with QDateTimeEdit
DateTimeRangeWidget::DateTimeRangeWidget(QWidget* parent, int stepCount)
  : BaseRangeWidget(parent)
  , RangeHandler<QDateTime>(stepCount
                            , QDateTime::fromMSecsSinceEpoch(0)
                            , QDateTime::fromString("8000-12-31", "yyyy-MM-dd"))
{
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);

  for (int i = 0; i < stepCount; ++i)
  {
    auto l = new QLabel(this);
    l->setContentsMargins(0, 0, 6, 0);
    layout->addWidget(l, 0);
    l->hide();

    auto dte = new QDateTimeEdit(this);
    dte->setDateTimeRange(mRangeMin, mRangeMax);
    dte->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    layout->addWidget(dte, 1);

    if (i != stepCount - 1)
    {
      auto delim = new QLabel("<html>&mdash;</html>", this);
      delim->setContentsMargins(6, 0, 6, 0);
      layout->addWidget(delim, 0);
    }

    mEditors << dte;
    mLabels << l;
  }

  for (const auto& sb : mEditors)
    connect(sb, SIGNAL(editingFinished()), this, SLOT(OnStepEdited()));
}

DateTimeRangeWidget::~DateTimeRangeWidget()
{
}

void DateTimeRangeWidget::SetDisplayFormat(const QString &format)
{
  for (const auto& sb : mEditors)
    ((QDateTimeEdit*)sb)->setDisplayFormat(format);
}

void DateTimeRangeWidget::OnEditorValueChanged(int index)
{
  CheckNewStepValue(index);
}

QDateTime DateTimeRangeWidget::GetValue(int index) const
{
  if (auto sb = (QDateTimeEdit*)mEditors.value(index))
    return sb->dateTime();
  return QDateTime();
}

void DateTimeRangeWidget::SetValue(int index, const QDateTime& value)
{
  if (auto sb = (QDateTimeEdit*)mEditors.value(index))
  {
    QSignalBlocker l(sb);
    return sb->setDateTime(value);
  }
}
