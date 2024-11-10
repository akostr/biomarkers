#include "color_select_button.h"

#include <QColorDialog>
#include <QPalette>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>

#ifndef DISABLE_CYBER_FRAC_STYLE
#include <style.h>
#endif

#ifndef DISABLE_CUSTOM_WINDOWS
#include <custom_windows/custom_windows.h>
#endif

ColorSelectButton::ColorSelectButton(QWidget* parent, const QColor &color, int size)
  : QWidget(parent)
{
  mButton = new QPushButton(this);
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);
  l->addWidget(mButton);

  setFixedSize(size, size);
  SetColor(color);

  connect(mButton, &QPushButton::clicked, this, &ColorSelectButton::OnClick);
}

ColorSelectButton::~ColorSelectButton()
{
}

void ColorSelectButton::SetColor(const QColor& color)
{
  QPalette newPalette = mButton->palette();
  newPalette.setColor(QPalette::Button, color);
  mButton->setPalette(newPalette);

  QString borderQSS = "grey";

#ifndef DISABLE_CYBER_FRAC_STYLE
  borderQSS = Style::GetSASSValue("uiElementFrame");
#endif

  QString style = QString("border: 1px solid %1; background-color: %2").arg(borderQSS, color.name(QColor::HexArgb));
  mButton->setStyleSheet(style);
}

void ColorSelectButton::SetColor(const QString &color)
{
  SetColor(QColor(color));
}

QColor ColorSelectButton::GetColor() const
{
  return mButton->palette().color(QPalette::Button);
}

void ColorSelectButton::OnClick()
{
  QColorDialog* dialog = new QColorDialog(mButton->palette().color(QPalette::Button));
  dialog->setWindowTitle(tr("Select color"));

#if defined(USE_CUSTOM_WINDOWS)
  CustomWindows::CustomNativeQtDialog w(qApp->activeWindow(), dialog
                                        , CustomNativeWindow::CWTitle
                                        | CustomNativeWindow::CWCloseButton
                                        | CustomNativeWindow::CWFixed);
  if (w.exec() == QDialog::Accepted)
#else  
  if (dialog->exec() == QDialog::Accepted)
#endif
  {
    QColor oldColor = GetColor();
    QColor newColor = dialog->currentColor();
    if (!newColor.isValid() || oldColor == newColor)
      return;

    SetColor(newColor);
    emit ColorChanged();
  }
}
