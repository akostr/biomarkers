#pragma once

#include "view.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>

////////////////////////////////////////////////////
//// Root Layout Welcome view class
namespace Views
{
class ViewRootLayoutWelcome : public View
{
  Q_OBJECT

public:
  ViewRootLayoutWelcome(QWidget* parent = 0);
  ~ViewRootLayoutWelcome();

  void SetupUi();

  //// Handle ui context
  virtual void ApplyContextUi(const QString& dataId, const QVariant& data) override;

private:
  //// Ui
  QPointer<QHBoxLayout>     RootLayout;
  QPointer<QVBoxLayout>     Layout;
  QPointer<QStackedWidget>  LayoutStack;
  QPointer<View>            Login;
  QPointer<View>            LoginCreate;
};
}//namespace Views
