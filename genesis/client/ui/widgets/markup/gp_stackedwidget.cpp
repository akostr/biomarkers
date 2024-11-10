#include "gp_stackedwidget.h"
#include <ui/plots/chromatogram_plot.h>
#include <QPixmap>
#include <QLabel>

GenesisMarkup::GPStackedWidget::GPStackedWidget(QWidget *parent)
  : QStackedWidget(parent)
  , mPlot(nullptr)
  , mLoadingWidget(new QWidget(this))
{
  addWidget(mLoadingWidget);
  if(mPlot)
    addWidget(mPlot);

  mLabel = new QLabel(this);
  mLoadingImage.reset( new QPixmap(":/resource/controls/icon_design_processing_10@2x.png"));

  mLabel->setPixmap(*mLoadingImage);

  QVBoxLayout *vLayout = new QVBoxLayout;
  vLayout->setSpacing(0);
  vLayout->setContentsMargins(0,0,0,0);
  QHBoxLayout *hLayout = new QHBoxLayout;
  hLayout->setSpacing(0);
  hLayout->setContentsMargins(0,0,0,0);
  vLayout->addStretch();
  vLayout->addLayout(hLayout);
  vLayout->addStretch();
  hLayout->addStretch();
  hLayout->addWidget(mLabel);
  hLayout->addStretch();

  mLoadingWidget->setLayout(vLayout);
  mHiddenPlot = true;
}

void GenesisMarkup::GPStackedWidget::setLoadingImage(QPixmap *image)
{
  mLoadingImage.reset(image);
}

void GenesisMarkup::GPStackedWidget::setPlot(ChromatogramPlot *plot)
{
  if(plot)
  {
    if(mPlot)
      removeWidget(mPlot);
    mPlot = plot;
    addWidget(plot);
  }
}

void GenesisMarkup::GPStackedWidget::showPlot()
{
  if(!mHiddenPlot)
    return;
  setCurrentWidget(mPlot);
  mHiddenPlot = false;
}

void GenesisMarkup::GPStackedWidget::hidePlot()
{
  if(mHiddenPlot)
    return;
  setCurrentWidget(mLoadingWidget);
  mHiddenPlot = true;
}

void GenesisMarkup::GPStackedWidget::paintEvent(QPaintEvent *event)
{
  QStackedWidget::paintEvent(event);
}


namespace GenesisMarkup {
bool GPStackedWidget::hiddenPlot() const
{
  return mHiddenPlot;
}

}
