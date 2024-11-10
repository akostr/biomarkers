#include "main_page_with_splitter.h"
#include "ui/controls/circle_button.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QMouseEvent>

MainPageWithSplitter::MainPageWithSplitter(QWidget *parent)
  : QWidget{parent}
{
  auto l = new QVBoxLayout(this);
  setMouseTracking(true);
  mSplitter = new QSplitter(this);
  mSplitter->setMouseTracking(true);
  mCollapseMenuButton = new CircleButton(this);
  l->addWidget(mSplitter);
  auto left = new QWidget(mSplitter);
  left->setMinimumSize({100,100});
  mSplitter->addWidget(left);
  auto right = new QWidget(mSplitter);
  right->setMinimumSize({100,100});
  mSplitter->addWidget(right);
  left->setStyleSheet("QWidget {background-color: green; border: 1px solid black;}");
  right->setStyleSheet("QWidget {background-color: yellow; border: 1px solid black;}");

  updateCollapseButtonPosition(mSplitter->sizes().first());
  connect(mSplitter, &QSplitter::splitterMoved, this, [this](int pos, int ind)
          {
    if(ind != 1)
    {
      // qDebug() << "wrong index";
      return;
    }
    updateCollapseButtonPosition(pos);
  });
}

void MainPageWithSplitter::updateCollapseButtonPosition(int pos)
{
  mCollapseMenuButton->setCenterOn(QPoint(mSplitter->geometry().left() + pos, mCollapseButtonVerticalOffset));
}


void MainPageWithSplitter::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  updateCollapseButtonPosition(mSplitter->sizes().first());
}


void MainPageWithSplitter::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  updateCollapseButtonPosition(mSplitter->sizes().first());
}


void MainPageWithSplitter::mouseMoveEvent(QMouseEvent *event)
{
  // qDebug() << event->pos();
}
