#pragma once

#include "view.h"


////////////////////////////////////////////////////
//// Root toolbar view class
class QHBoxLayout;
class QPushButton;
class QMenu;
namespace Views
{
class ViewRootToolbarAccountMenu : public View
{
  Q_OBJECT

public:
  ViewRootToolbarAccountMenu(QWidget* parent = 0);
  ~ViewRootToolbarAccountMenu();

  void SetupUi();
  void initStatisticButtonForRootUser();
  //// Handle user changes
  virtual void ApplyContextUser(const QString& dataId, const QVariant& data) override;

private:
  //// Ui
  QPointer<QHBoxLayout> Layout;
  QPointer<QPushButton> MenuButton;
  QPointer<QMenu> Menu;

  // QObject interface
public:
  bool eventFilter(QObject *watched, QEvent *event) override;
};
}//namespace Views
