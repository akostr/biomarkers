#pragma once

#include "view.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>

////////////////////////////////////////////////////
//// Create account view class
namespace Views
{
class ViewLoginCreate : public View
{
  Q_OBJECT

public:
  ViewLoginCreate(QWidget* parent = 0);
  ~ViewLoginCreate();

  void SetupUi();

private:
  //// Ui
  QPointer<QVBoxLayout>       Layout;
  QPointer<QGridLayout>       LayoutGrid;

  QPointer<QWidget>           Login;
  QPointer<QVBoxLayout>       LoginLayout;

  QPointer<QLineEdit>         NameLast;
  QPointer<QLineEdit>         NameFirst;
  QPointer<QLineEdit>         NamePatronymic;
  QPointer<QLineEdit>         Email;
  QPointer<QLineEdit>         Password;

  QPointer<QDialogButtonBox>  Accept;
};
}//namespace Views
