#pragma once

#include <QMenu>

class PersistentMenu : public QMenu
{
  Q_OBJECT

public:
  explicit PersistentMenu(QWidget* parent = nullptr);
  explicit PersistentMenu(const QString& title, QWidget* parent = nullptr);
  
protected:
  virtual void mouseReleaseEvent(QMouseEvent *e) override;
};
