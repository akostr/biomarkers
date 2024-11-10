#pragma once

#include "view.h"

#include <QVBoxLayout>

////////////////////////////////////////////////////
//// Root toolbar view class
namespace Views
{
class ViewRootToolbar : public View
{
  Q_OBJECT

public:
  ViewRootToolbar(QWidget* parent = 0);
  ~ViewRootToolbar();

  void SetupUi();

private:
  //// Ui
  QPointer<QVBoxLayout> Layout;
};
}//namespace Views
