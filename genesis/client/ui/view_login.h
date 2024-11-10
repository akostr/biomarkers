#pragma once

#include "view.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QActionGroup>
#include <QSettings>

////////////////////////////////////////////////////
//// Login view class
namespace Views
{
class ViewLogin : public View
{
  Q_OBJECT

public:
  explicit ViewLogin(QWidget* parent = 0);
  ~ViewLogin();

  void SetupUi();

  //// Handle login context
  virtual void ApplyContextUser(const QString &dataId, const QVariant &data);

private:
  void reconfigure(QAction* action);
  void handleInput();
  void returnPressed();
  void TryToLogin();
  //// Ui
  QPointer<QVBoxLayout>       Layout;
  QPointer<QGridLayout>       LayoutGrid;

  QPointer<QWidget>           Login;
  QPointer<QVBoxLayout>       LoginLayout;

  QPointer<QLineEdit>         Email;
  QPointer<QLineEdit>         Password;

  QPointer<QDialogButtonBox>  Accept;
  QPointer<QPushButton>       ActionSettings;
  QPointer<QActionGroup>      ConfigSelector;
  QSettings                   AppSettings;
};
}//namespace Views
