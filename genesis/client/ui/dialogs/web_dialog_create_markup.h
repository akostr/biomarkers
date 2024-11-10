#pragma once

#include "web_dialog_create.h"

#include "../../logic/tree_model_dynamic_project_markups_list.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>

class WebDialogCreateMarkup : public WebDialogCreate
{
  Q_OBJECT

public:
  WebDialogCreateMarkup(QWidget* parent);
  ~WebDialogCreateMarkup();

  virtual void Accept();

private:
  void SetupModels();
  void SetupUi();

  QString GetCurrentName() const;

private:
  //// Models
  QPointer<TreeModelDynamicProjectMarkupsList> Markups;

  //// Ui
  QPointer<QWidget>       Body;
  QPointer<QVBoxLayout>   BodyLayout;

  //// Input
  QPointer<QLineEdit>     MarkupName;
};
