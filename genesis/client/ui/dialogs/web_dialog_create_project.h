#pragma once

//#include "web_dialog_create.h"
#include <ui/dialogs/templates/dialog.h>

#include "../../logic/tree_model_dynamic_projects.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>

/////////////////////////////////////////////////////
//// Web Dialog / create project
class DialogLineEditEx;
namespace Dialogs
{
class WebDialogCreateProject : public Dialogs::Templates::Dialog
{
  Q_OBJECT

public:
  WebDialogCreateProject(QWidget* parent);
  ~WebDialogCreateProject();

  virtual void Accept();

private:
  void SetupModels();
  void SetupUi();

  QString GetCurrentName() const;
  void SetDialogValidity(bool isValid);

private:
  //// Ui
  QPointer<DialogLineEditEx> ProjectNameEdit;
  QPointer<QWidget>       Body;
  QPointer<QVBoxLayout>   BodyLayout;

  // WebDialog interface
public slots:
  void Open() override;
};
}//namespace Dialogs
