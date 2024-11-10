#pragma once

#include <QFrame>
#include <QPointer>

class QLabel;
/**
 * @brief The WebOverlay class
 * used for WebDialogs, and for loading overlays.
 * in case of WebDialog, the overlay "parent" should be nullptr,
 * inside it will be setted to GenesisWindow::Get(). otherwise
 * dialog will be shown on top of "parent", not on top of window.
 * The WebDialog parent may be any, i suppose
 */
class WebOverlay : public QFrame
{
  Q_OBJECT

public:
  WebOverlay(QWidget* parent = nullptr);
  WebOverlay(const QString& text, QWidget* parent = nullptr);
  ~WebOverlay();
  void setText(const QString& text);

  bool eventFilter(QObject* watched, QEvent* event);

protected:
  QPointer<QWidget> IndicatorWidget;
  QPointer<QLabel>  Label;
};
