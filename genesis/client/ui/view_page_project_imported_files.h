#pragma once

#include "view.h"

#include "../logic/tree_model_dynamic_project_imported_files.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QTableView>

////////////////////////////////////////////////////
//// Projects
namespace Views
{
class ViewPageProjectImportedFiles : public View
{
  Q_OBJECT

public:
  ViewPageProjectImportedFiles(QWidget* parent = 0);
  ~ViewPageProjectImportedFiles();

  void SetupModels();
  void SetupUi();

private:
  //// Logic
  QPointer<TreeModelDynamicProjectImportedFiles> Files;

  //// Ui
  QPointer<QVBoxLayout>               Layout;
  QPointer<QFrame>                    Content;
  QPointer<QVBoxLayout>               ContentLayout;
  QPointer<QTableView>                Table;
};
}//namespace Views
