#include "view_root_toolbar.h"

#include "dialogs/web_dialog_create_project.h"

#include <genesis_style/style.h>
#include "ui/known_view_names.h"

#include <QDialogButtonBox>
#include <QPushButton>

using namespace ViewPageNames;

////////////////////////////////////////////////////
//// Root toolbar view class
namespace Views
{
ViewRootToolbar::ViewRootToolbar(QWidget* parent)
  : View(parent)
{
  //// Setup
  SetupUi();
}

ViewRootToolbar::~ViewRootToolbar()
{
}

void ViewRootToolbar::SetupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetToolbarStyle());

  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(0, 0, 0, 0);

  //// Content
  {
    //// Stretch
    Layout->addStretch();

    //// Login view
    {
      auto loginView = View::Create(ViewRootToolbarAccountMenuPageName, this);
      Layout->addWidget(loginView);
    }
//    //// Tools
//    {
//      QDialogButtonBox* createProject = new QDialogButtonBox(QDialogButtonBox::Ok, this);
//      Layout->addWidget(createProject);

//      if (auto ok = createProject->button(QDialogButtonBox::Ok))
//      {
//        ok->setFocusPolicy(Qt::NoFocus);
//        ok->setText(" " + tr("Create project"));
//        ok->setIcon(QIcon(":/resource/icons/icon_action_add_light.png"));
//      }

//      connect(createProject, &QDialogButtonBox::accepted, [this]()
//        {
//          auto dialog = new WebDialogCreateProject(this);
//          dialog->Open();

//          connect(dialog, &WebDialogCreateProject::Accepted, []()
//            {
//              TreeModel::ResetInstances("TreeModelDynamicProjects");
//            });
//        });
//    }
  }
}
}//namespace Views
