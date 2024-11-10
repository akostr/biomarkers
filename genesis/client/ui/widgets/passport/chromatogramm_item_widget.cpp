#include "chromatogramm_item_widget.h"
#include <QHBoxLayout>
#include <QLabel>

PassportChromatogrammItemWidget::PassportChromatogrammItemWidget(QString chromaName, int percents, QWidget *parent)
  : QWidget{parent},
    mChromaName(chromaName),
    mPercents(percents)
{
  setupUi();
}

void PassportChromatogrammItemWidget::setPercents(int percents)
{
  if(percents < 100)
    mPercentsLabel->setText(QString("%1\%").arg(percents));
  else
    mPercentsLabel->setText(QString("OK"));
}

void PassportChromatogrammItemWidget::setupUi()
{
  auto mainLayout = new QHBoxLayout;
  setLayout(mainLayout);
  mPercentsLabel = new QLabel(this);
  mainLayout->addWidget(mPercentsLabel);
  setPercents(mPercents);
  auto nameLabel = new QLabel(mChromaName, this);
  mainLayout->addWidget(nameLabel, 1);
}
