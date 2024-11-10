#pragma once

#include "view.h"

#include <QVBoxLayout>
#include <QFrame>

////////////////////////////////////////////////////
//// Interpretation
namespace Views
{
class ViewPageInterpretation : public View
{
  Q_OBJECT

public:
  ViewPageInterpretation(QWidget* parent = 0);
  ~ViewPageInterpretation();

  void SetupUi();

private:
  //// Ui
  QPointer<QVBoxLayout>       Layout;
  QPointer<QFrame>            Content;
};
}//namespace Views
