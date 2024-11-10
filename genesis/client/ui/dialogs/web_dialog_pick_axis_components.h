#ifndef WEBDIALOGPICKAXISCOMPONENTS_H
#define WEBDIALOGPICKAXISCOMPONENTS_H

#include "logic/structures/pca_data_structures.h"
#include "web_dialog.h"

namespace Ui
{
  class WebDialogPickAxisComponents;
}

class WebDialogPickAxisComponents : public WebDialog
{
  Q_OBJECT

public:
  WebDialogPickAxisComponents(QWidget *parent, const QList<double> &explPCsVariance);
  QPair<int, int> GetComponents();

  void setCurrentXComponent(int index);
  void setCurrentYComponent(int index);

private:
  void SetupUi(const QList<double> &explPCsVariance);

private:
  Ui::WebDialogPickAxisComponents* Ui = nullptr;
};

#endif // WEBDIALOGPICKAXISCOMPONENTS_H
