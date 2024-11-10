#pragma once

#include "view.h"

#include <QVBoxLayout>
#include <QFrame>

////////////////////////////////////////////////////
//// ProjectLog
namespace Views
{
class ViewPageProjectLog : public View
{
  Q_OBJECT

public:
  ViewPageProjectLog(QWidget* parent = 0);
  ~ViewPageProjectLog();

  void SetupUi();

private:
  //// Ui
  QPointer<QVBoxLayout>       Layout;
  QPointer<QFrame>            Content;
};
}//namespace Views
