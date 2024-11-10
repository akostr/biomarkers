#pragma once

#include "view.h"

#include <QStackedWidget>
#include <QVBoxLayout>

////////////////////////////////////////////////////
//// Root view class
namespace Views
{
class ViewRoot : public View
{
  Q_OBJECT

public:
  ViewRoot(QWidget* parent = 0);
  ~ViewRoot();
  QPointer<View> GetCurrentPage();
  QString GetCurrentPageId();

  void SetupUi();

  //// Handle user context
  virtual void ApplyContextUser(const QString& dataId, const QVariant& data) override;
  virtual void ApplyContextModules(const QString& dataId, const QVariant& data) override;

private:
  //// Ui
  QPointer<QVBoxLayout>     Layout;
  QPointer<QStackedWidget>  LayoutStack;
  QPointer<View>            LayoutWelcome;
  QPointer<View>            LayoutModulesPage;
  QPointer<View>            LayoutPages;
  QString                   LastUserLogin;

private:
  void applyModule(int module);
};
}//namespace Views
