#include "spinbox_action.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <genesis_style/style.h>

SpinBoxAction::SpinBoxAction(const QString &title, QWidget* parent)
  : QWidgetAction(parent)
{
  QWidget* body = new QWidget(parent);
  QHBoxLayout* pLayout = new QHBoxLayout();
  pLayout->setContentsMargins(7,0,0,0);
  Label = new QLabel (title);  //bug fixed here, pointer was missing
  Label->setStyleSheet("QLabel:disabled {color: rgb(166,177,184) }");
  pLayout->addWidget (Label);
  SpinBox = new QSpinBox(parent);
  SpinBox->setProperty("tiny", true);
  pLayout->addWidget(SpinBox);
  body->setLayout (pLayout);
  setDefaultWidget(body);
  connect(this, &QAction::enabledChanged, this, [this](bool enabled)
  {
    Label->setEnabled(enabled);
    SpinBox->setVisible(enabled);
  });
}
