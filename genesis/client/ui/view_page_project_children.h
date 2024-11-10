#pragma once

#include "view.h"

#include "../logic/tree_model_dynamic_project_children.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QTableView>

////////////////////////////////////////////////////
//// Projects
namespace Views
{
class ViewPageProjectChildren : public View
{
  Q_OBJECT

public:
  ViewPageProjectChildren(QWidget* parent = 0);
  ~ViewPageProjectChildren();

  void SetupModels();
  void SetupUi();

private:
  //// Logic
  QPointer<TreeModelDynamicProjectChildren> Projects;

  //// Ui
  QPointer<QVBoxLayout>               Layout;
  QPointer<QFrame>                    Content;
  QPointer<QVBoxLayout>               ContentLayout;
  QPointer<QTableView>                Table;
};
}//namespace Views
