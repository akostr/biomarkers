#include "main_menu_splitter.h"
#include "ui/controls/circle_button.h"
#include <QDebug>

MainMenuSplitter::MainMenuSplitter(QWidget *parent)
  :QSplitter(Qt::Horizontal, parent)
{
  mCollapseLeftButton = new CircleButton(this);
  connect(this, &MainMenuSplitter::splitterMoved, this, [](int pos, int index)
          {
  });
}
void MainMenuSplitter::mouseMoveEvent(QMouseEvent *event)
{

}
