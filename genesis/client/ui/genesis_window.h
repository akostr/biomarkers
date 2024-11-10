#pragma once

#include "view.h"
#include "genesis_window_notifications.h"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QMap>
#include <QShortcut>

class WebOverlay;

using namespace Views;

////////////////////////////////////////////////////
//// Main window class
class GenesisWindow : public QMainWindow
{
  Q_OBJECT

public:
  GenesisWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
  virtual ~GenesisWindow();

  static GenesisWindow* Get();

  //// Show page
  void ShowPage(const QString& factoryId);
  void ShowPage(const std::string& factoryId);
  void SwitchModule(int module);
  QPointer<View> CurrentPage();
  QUuid ShowOverlay(const QString& text);
  void RemoveOverlay(QUuid id);
  void UpdateOverlay(QUuid id, const QString& text);
  void SetLogFilename(const QString filename);

  // QObject interface
public:
  bool eventFilter(QObject *watched, QEvent *event) override;

signals:
  void windowMouseMove(QPointF pos);

private:
  void SetupUi();

private:
  //// Ui
  QList<QPair<QUuid, QPointer<WebOverlay>>> Overlays;
  QPointer<QWidget>                         Root;
  QPointer<QStackedLayout>                  RootLayout;

  QPointer<View>                            RootView;
  QPointer<NotificationPresenter>           Notifications;
#ifndef PLATFORM_WASM
  // Open current log file by HotKey
  QShortcut                                *keyLogs;
  QString                                  current_log_filename;
#endif
};
