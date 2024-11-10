#include "view_page_interpretation.h"

#include <genesis_style/style.h>

#include <QLabel>

////////////////////////////////////////////////////
//// Interpretation
namespace Views
{
ViewPageInterpretation::ViewPageInterpretation(QWidget* parent)
  : View(parent)
{
  SetupUi();
}

ViewPageInterpretation::~ViewPageInterpretation()
{
}

void ViewPageInterpretation::SetupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
  Layout->setSpacing(0);

  //// Caption
  {
    QLabel* catpion = new QLabel(tr("Interpretation"), this);
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
