#ifndef PROJECT_INFO_MENU_WIDGET_H
#define PROJECT_INFO_MENU_WIDGET_H

#include <QWidget>

namespace Ui {
class ProjectInfoMenuWidget;
}

class ProjectInfoMenuWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ProjectInfoMenuWidget(QWidget *parent = nullptr);
  ~ProjectInfoMenuWidget();
  void setTitle(const QString& title);
  void setGroup(const QString& group);
  void setPixmap(const QPixmap &pixmap);

  bool isChecked() const;
  void setCheckState(bool isChecked);

signals:
  void checkStateChanged(bool isChecked);

private:
  Ui::ProjectInfoMenuWidget *ui;
  bool mIsChecked = false;

  // QWidget interface
protected:
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
};

#endif // PROJECT_INFO_MENU_WIDGET_H
