#include "warnings_frame.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

#ifndef DISABLE_CYBER_FRAC_STYLE
#include <style.h>
#endif

WarningsFrame::WarningsFrame(QWidget* parent)
  : QFrame(parent)
{
  setAutoFillBackground(false);

#ifndef DISABLE_CYBER_FRAC_STYLE
  setStyleSheet(Style::GetInputErrorWarningFrameStyle());
#endif

  Layout = new QHBoxLayout(this);
  Layout->setContentsMargins(5, 3, 5, 3);

  Icon = new QLabel(this);
  Icon->setObjectName("WarningFrameIcon");
#ifndef DISABLE_CYBER_FRAC_STYLE
  QIcon warningIcon(Style::GetInputErrorWarningIconPath());
  Icon->setPixmap(warningIcon.pixmap(16, 16));
#endif
  Layout->addWidget(Icon, 0, Qt::AlignTop);

  Warnings = new QLabel(this);
  Warnings->setWordWrap(true);
  Layout->addWidget(Warnings, 1);

  setVisible(false);
}

WarningsFrame::~WarningsFrame()
{
}

void WarningsFrame::SetErrors(const QString &text)
{
  setVisible(!text.isEmpty());
  if (text != Warnings->text())
  {
    Warnings->setText(text);
    emit WarningsChanged(text);
  }
}

void WarningsFrame::SetErrors(const QStringList &text)
{
  SetErrors(text.join(" "));
}

void WarningsFrame::ClearErrors()
{
  SetErrors(QString());
}

bool WarningsFrame::HasErrors() const
{
  return !Warnings->text().isEmpty();
}
