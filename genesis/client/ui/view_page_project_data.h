#pragma once

#include "view.h"

#include <QVBoxLayout>
#include <QFrame>

////////////////////////////////////////////////////
//// ProjectData
namespace Views
{
class ViewPageProjectData : public View
{
  Q_OBJECT

public:
  ViewPageProjectData(QWidget* parent = 0);
  ~ViewPageProjectData();

  void SetupUi();

private:
  //// Ui
  QPointer<QVBoxLayout>       Layout;
  QPointer<QFrame>            Content;
};
}//namespace Views
