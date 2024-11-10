#ifndef MULTI_NOTIFICATION_H
#define MULTI_NOTIFICATION_H

#include <ui/widgets/notifications/notification_widget.h>

namespace Ui {
class MultiNotification;
}

class QLabel;
class MultiChildWidget : public QWidget
{
public:
  explicit MultiChildWidget(QWidget* parent = nullptr);
  void setText(const QString& text);
  void setIcon(const QIcon& icon);
  void resetIcon();

private:
  QPointer<QLabel> mTextLabel;
  QPointer<QLabel> mIconLabel;
};

class QVBoxLayout;
class MultiNotification : public NotificationWidget
{
  Q_OBJECT

public:
  explicit MultiNotification(Notification* notification, QWidget *parent = nullptr);
  ~MultiNotification();

private:
  Ui::MultiNotification *ui;
  QMap<QString, MultiChildWidget*> mIdToChildMap;

private:
  void setupUi();
  void updateData();
  void collapse();
  void expand();
  void onCollapseBtnClicked(bool checked);

  // NotificationWidget interface
protected:
  void onNotificationUpdate() override;
  void handleChildData(Notification *child);

protected:
  QPointer<QVBoxLayout> mMultiLayout;
  QIcon* uploadSuccessIcon = nullptr;
  QIcon* uploadProgressIcon = nullptr;
  QIcon* uploadErrorIcon = nullptr;
};

#endif // MULTI_NOTIFICATION_H

