#include "view_page_project_log.h"

#include <genesis_style/style.h>

#include <QLabel>

////////////////////////////////////////////////////
//// ProjectLog
namespace Views
{
ViewPageProjectLog::ViewPageProjectLog(QWidget* parent)
  : View(parent)
{
  SetupUi();
}

ViewPageProjectLog::~ViewPageProjectLog()
{
}

void ViewPageProjectLog::SetupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
  Layout->setSpacing(0);

  //// Caption
  {
    QLabel* catpion = new QLabel(tr("Log"), this);
    Layout->addWidget(catpion);

    catpion->setStyleSheet(Style::Genesis::GetH1());
  }  

  //// Space
  Layout->addSpacing(Style::Scale(24));

  //// Content
  {
    Content = new QFrame(this);
    Layout->addWidget(Content, 1);

    Content->setFrameShape(QFrame::Box);
    Content->setObjectName("rounded");
    Content->setStyleSheet("QWidget { background-color: white } ");
  }
}
}//namespace Views
