#ifndef WEB_DIALOG_CREATE_PROJECT_REFACTOR_H
#define WEB_DIALOG_CREATE_PROJECT_REFACTOR_H

#include <ui/dialogs/templates/dialog.h>

#include <QWidget>

namespace Ui {
class WebDialogCreateProjectRefactor;
}

namespace Dialogs
{
class WebDialogCreateProjectRefactor : public Templates::Dialog
{
  Q_OBJECT

public:
  explicit WebDialogCreateProjectRefactor(QWidget* parent = nullptr);
  ~WebDialogCreateProjectRefactor();

  void Accept() override;

private:
  void SetupModels();
  void SetupUi();
  void setDialogDataValidity();

private:
  Ui::WebDialogCreateProjectRefactor* ui;
  QWidget* mContent;

  // WebDialog interface
public slots:
  void Open() override;
};
}

#endif // WEB_DIALOG_CREATE_PROJECT_REFACTOR_H
