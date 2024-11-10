#include "web_overlay.h"
#include "../genesis_window.h"

#include <common_gui/standard_widgets/wait_indicator.h>

#include <QApplication>

WebOverlay::WebOverlay(QWidget* parent)
  : QFrame(parent ? parent : GenesisWindow::Get())
{
  parent = parent ? parent : GenesisWindow::Get();
  if (auto parentW = parentWidget())
    parentW->installEventFilter(this);

  setFocusPolicy(Qt::NoFocus);
  setAutoFillBackground(false);
  setStyleSheet("WebOverlay{ background-color: #d9002033; }");

  move(0, 0);
  resize(parent->size());
  show();
  raise();
  repaint();
}

WebOverlay::WebOverlay(const QString& text, QWidget* parent)
  : WebOverlay(parent)
{
  IndicatorWidget = new QWidget(this);
  IndicatorWidget->setStyleSheet("background-color: #2a5168; color: white; font-weight: bold;");
  IndicatorWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  QHBoxLayout* box = new QHBoxLayout(IndicatorWidget);
  box->setContentsMargins(15, 5, 15, 5);
  box->setSizeConstraint(QLayout::SetMinimumSize);

  auto indicator = new WaitIndicator(QString(), this, 36);
  indicator->SetIcons({
    ":/resource/controls/icon_design_processing_1@2x.png",
    ":/resource/controls/icon_design_processing_2@2x.png",
    ":/resource/controls/icon_design_processing_3@2x.png",
    ":/resource/controls/icon_design_processing_4@2x.png",
    ":/resource/controls/icon_design_processing_5@2x.png",
    ":/resource/controls/icon_design_processing_6@2x.png",
    ":/resource/controls/icon_design_processing_7@2x.png",
    ":/resource/controls/icon_design_processing_8@2x.png",
    ":/resource/controls/icon_design_processing_9@2x.png",
    ":/resource/controls/icon_design_processing_10@2x.png",
    ":/resource/controls/icon_design_processing_11@2x.png",
    ":/resource/controls/icon_design_processing_12@2x.png",

  });
  box->addWidget(indicator, 0);

  Label = new QLabel(text, this);
  Label->setObjectName("progress");
  box->addWidget(Label, 1);

  IndicatorWidget->show();
  auto size = IndicatorWidget->size();
  IndicatorWidget->move(rect().center() - QPoint(size.width() / 2, size.height() / 2));
  IndicatorWidget->raise();
  repaint();
}

WebOverlay::~WebOverlay()
{
}

void WebOverlay::setText(const QString &text)
{
  Label->setText(text);
  repaint();
}

bool WebOverlay::eventFilter(QObject *watched, QEvent *event)
{
  if (watched && watched == parentWidget())
  {
    if (event->type() == QEvent::Resize || event->type() == QEvent::Show)
    {
      move(0, 0);
      resize(parentWidget()->size());
      if (IndicatorWidget)
      {
        auto size = IndicatorWidget->size();
        IndicatorWidget->move(rect().center() - QPoint(size.width() / 2, size.height() / 2));
      }
    }
  }
  return false;
}
