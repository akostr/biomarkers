#pragma once

#include "view.h"

#include <QVBoxLayout>
#include <QFrame>

////////////////////////////////////////////////////
//// Identification
namespace Views
{
class ViewPageIdentification : public View
{
  Q_OBJECT

public:
  ViewPageIdentification(QWidget* parent = 0);
  ~ViewPageIdentification();

  void SetupUi();

private:
  //// Ui
  QPointer<QVBoxLayout>       Layout;
  QPointer<QFrame>            Content;
};
}//namespace Views
