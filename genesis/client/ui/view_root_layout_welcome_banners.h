#pragma once

#include "view.h"

#include <QVBoxLayout>
#include <QStackedWidget>

////////////////////////////////////////////////////
//// Root Layout Welcome Banners view class
namespace Views
{
class ViewRootLayoutWelcomeBanners : public View
{
  Q_OBJECT

public:
  ViewRootLayoutWelcomeBanners(QWidget* parent = 0);
  ~ViewRootLayoutWelcomeBanners();

  void SetupUi();

private:
  //// Ui
  QPointer<QVBoxLayout>     Layout;
  QPointer<QWidget>         Root;
  QPointer<QVBoxLayout>     RootLayout;

  QPointer<QWidget>         Content;
  QPointer<QVBoxLayout>     ContentLayout;
};
}//namespace Views
