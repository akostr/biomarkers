#pragma once

#include "view.h"

#include "../logic/tree_model_dynamic_project_info.h"
#include "../logic/tree_model_dynamic_project_users.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QTableView>
#include <QLabel>
#include <QComboBox>
#include <QGridLayout>

////////////////////////////////////////////////////
//// ProjectInfo
namespace Views
{
class ViewPageProjectInfo : public View
{
  Q_OBJECT

public:
  ViewPageProjectInfo(QWidget* parent = 0);
  ~ViewPageProjectInfo();

  void SetupModels();
  void SetupUi();

private:
  //// Logic
  QPointer<TreeModelDynamicProjectInfo>   ProjectInfo;
//  QPointer<TreeModelDynamicProjectUsers>  ProjectUsers;

  //// Ui
  QPointer<QVBoxLayout>                   Layout;
  QPointer<QFrame>                        Content;
  QPointer<QVBoxLayout>                   ContentLayout;

  QPointer<QWidget>                       ProjectInfoArea;
  QPointer<QVBoxLayout>                   ProjectInfoAreaLayout;
  QPointer<QLabel>                        ProjectInfoAreaCaption;
  QPointer<QGridLayout>                   ProjectInfoAreaContentLayout;
  
  QPointer<QLabel>                        mCaption;
  QPointer<QLabel>                        ProjectName;
  QPointer<QLabel>                        ProjectId;
  QPointer<QLabel>                        ProjectParent;
  QPointer<QLabel>                        ProjectDateCreated;
  QPointer<QLabel>                        mGroup;
  QPointer<QLabel>                        mStatusPix;
  QPointer<QLabel>                        mStatus;
  QPointer<QLabel>                        mChromatogramCount;
  QPointer<QLabel>                        mCodeWord;
  QPointer<QLabel>                        mComment;
//  QPointer<QLabel>                        ProjectAuthor;

//  QPointer<QLabel>                        ProjectContributorsCaption;
//  QPointer<QTableView>                    ProjectContributorsView;

  QPixmap createCircleIcon(QSize size, QColor color);
  void editProject();
  void manageProjectBaseSettings();
};
}//namespace Views
