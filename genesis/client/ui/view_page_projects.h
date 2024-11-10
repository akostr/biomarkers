#pragma once

#include "view.h"

#include "../logic/tree_model_dynamic_projects.h"

class QVBoxLayout;
class QFrame;
class QComboBox;
class CommonTableView;

////////////////////////////////////////////////////
//// Projects
namespace Views
{
class ViewPageProjects : public View
{
  Q_OBJECT

public:
  ViewPageProjects(QWidget* parent = 0);
  ~ViewPageProjects() = default;

  void SetupModels();
  void SetupUi();

private:
  //// Logic
  QPointer<TreeModelPresentation>     ProjectsRepresentationModel;
  QPointer<TreeModelDynamicProjects>  Projects;

  //// Ui
  QPointer<QVBoxLayout>               Layout;
  QPointer<QFrame>                    Content;
  QPointer<QVBoxLayout>               ContentLayout;
  QPointer<CommonTableView>                Table;
  QPointer<QComboBox>                 GroupsCombo;

private:
  void setActiveGroup(int ind);
  void openProject(TreeModelItem* projectItem);
  void renameProject(TreeModelItem* projectItem);

  // View interface
public:
  void ApplyContextUser(const QString& dataId, const QVariant& data) override;
  void ApplyContextModules(const QString& dataId, const QVariant& data) override;
};


}//namespace Views
