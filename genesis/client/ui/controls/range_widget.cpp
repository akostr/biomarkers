#include "range_widget.h"
#include "ui_range_widget.h"

RangeWidget::RangeWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::RangeWidget)
{
  ui->setupUi(this);
  connect(ui->lowerBound, &QSpinBox::valueChanged, this, &RangeWidget::onLowerBoundChanged);
  connect(ui->upperBound, &QSpinBox::valueChanged, this, &RangeWidget::onUpperBoundChanged);
  connect(ui->closeButton, &QPushButton::clicked, this, [this](){emit closed(this);});
}

RangeWidget::~RangeWidget()
{
  delete ui;
}

int RangeWidget::lowerBound()
{
  return ui->lowerBound->value();
}

int RangeWidget::upperBound()
{
  return ui->upperBound->value();
}

void RangeWidget::setLowerBound(int lowerBound)
{
  ui->lowerBound->setValue(lowerBound);
}

void RangeWidget::setUpperBound(int upperBound)
{
  ui->upperBound->setValue(upperBound);
}

void RangeWidget::setMinLowerBoundValue(int min)
{
  ui->lowerBound->setMinimum(min);
}

void RangeWidget::setMaxLowerBoundValue(int max)
{
  ui->lowerBound->setMaximum(max);
}

void RangeWidget::setMinUpperBoundValue(int min)
{
  ui->upperBound->setMinimum(min);
}

void RangeWidget::setMaxUpperBoundValue(int max)
{
  ui->upperBound->setMaximum(max);
}

void RangeWidget::setBounds(int min, int max)
{
  setMinLowerBoundValue(min);
  setMinUpperBoundValue(min+1);
  setMaxLowerBoundValue(max);
  setMaxUpperBoundValue(max);
}

void RangeWidget::onLowerBoundChanged(int newVal)
{
  emit lowerBoundChanged(newVal);
  setMinUpperBoundValue(newVal+1);
}

void RangeWidget::onUpperBoundChanged(int newVal)
{
  emit upperBoundChanged(newVal);
}
