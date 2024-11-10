#include "web_dialog_create_project.h"

#include "../../api/api_rest.h"
#include "../../logic/context_root.h"
#include "../../logic/notification.h"
#include <logic/known_context_tag_names.h>
#include <ui/controls/dialog_line_edit_ex.h>
#include <ui/genesis_window.h>

#include <genesis_style/style.h>

#include <logic/context_root.h>

#include <QComboBox>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QDebug>
#include "ui/known_view_names.h"

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

namespace Dialogs
{
/////////////////////////////////////////////////////
//// Web Dialog / create project
WebDialogCreateProject::WebDialogCreateProject(QWidget* parent)
  : Dialogs::Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
{
  auto s = Dialogs::Templates::Dialog::Settings();
  s.dialogHeader = tr("Project creation");
  s.buttonsNames = {{QDialogButtonBox::Ok, tr("Create project")}};
  s.buttonsProperties = {{QDialogButtonBox::Ok, {{"blue", true}}}};
  applySettings(s);
  SetupUi();
}

WebDialogCreateProject::~WebDialogCreateProject()
{
}

void WebDialogCreateProject::SetupModels()
{
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  auto callback = [overlayId, this](QNetworkReply*, QJsonDocument doc)
  {
    auto root = doc.object();
    if(root["error"].toBool() == true)
    {
      Notification::NotifyError(root["msg"].toString(), tr("Server error"));
      GenesisWindow::Get()->RemoveOverlay(overlayId);
      Reject();
      return;
    }
    auto jforbiddenArray = root["project_titles"].toArray();

    auto s = ProjectNameEdit->settings();
    s.forbiddenStrings.clear();
    for(const auto& v : jforbiddenArray)
    {
      s.forbiddenStrings.insert(v.toString());
    }
    if(!s.defaultText.isEmpty() && s.forbiddenStrings.contains(s.defaultText))
    {
      QString original = s.defaultText;
      QString m;
      for(uint i = 0; i < (uint)(-1); i++)
      {
        m = QString("%1 %2").arg(original).arg(i);
        if(!s.forbiddenStrings.contains(m))
        {
          s.defaultText = m;
          break;
        }
      }
    }
    ProjectNameEdit->applySettings(s);

    GenesisWindow::Get()->RemoveOverlay(overlayId);
  };
  auto errorCallback = [overlayId, this](QNetworkReply*, QNetworkReply::NetworkError e)
  {
    Notification::NotifyError(tr("Can't load data for proper renaming. Check your connection."), tr("Network error"), e);
    GenesisWindow::Get()->RemoveOverlay(overlayId);
    Reject();
  };

  API::REST::GetOccupiedProjectsNames(callback, errorCallback);
}

void WebDialogCreateProject::SetupUi()
{
  //// Body
  Body = new QWidget();
  getContent()->layout()->addWidget(Body);

  BodyLayout = new QVBoxLayout(Body);
  BodyLayout->setContentsMargins(0,0,0,0);
  BodyLayout->setSpacing(0);

  auto s = DialogLineEditEx::Settings();
  s.defaultText = tr("New project");
  s.textHeader = tr("Project name");
  s.textMaxLen = 50;
  s.textPlaceholder = tr("Enter project name");
  s.textRequired = true;
  s.controlSymbols = true;
  s.textTooltipsSet = {
    tr("Name is empty"),
    tr("Name alredy in use"),
    tr("Not checked"),
    tr("Name is valid"),
    tr("Name must not contain the characters '\', '/', '\"', '*', '<', '|', '>'")
  };

  ProjectNameEdit = new DialogLineEditEx(this, s);
  BodyLayout->addWidget(ProjectNameEdit);
  connect(ProjectNameEdit, &DialogLineEditEx::validityChanged, this, &WebDialogCreateProject::SetDialogValidity);
}

QString WebDialogCreateProject::GetCurrentName() const
{
  return ProjectNameEdit->text();
}

void WebDialogCreateProject::SetDialogValidity(bool isValid)
{
  ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
}

void WebDialogCreateProject::Accept()
{
  QString projectName   = ProjectNameEdit->text();

  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Creating project"));

  API::REST::Tables::AddProject(GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kCurrentGroup).toString(), projectName, /*projectParentId, propagateAccess,*/
    [overlayId](QNetworkReply*, QVariantMap result)
    {
      //qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(result)).toJson();
      auto children = result["children"].toList();
      if (children.size())
      {
        if (int pid = children[0].toMap()["add_project"].toInt())
        {
          if (auto contextRoot = GenesisContextRoot::Get())
          {
            if (auto contextProject = contextRoot->GetContextProject())
            {
              auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
              API::REST::Tables::GetProjectInfo
              (
                pid,
                [overlayId](QNetworkReply*, QVariantMap result)
                {
                  //// Transform
                  if (result.contains("children"))
                  {
                    QVariantList children = result["children"].toList();
                    for (auto& c : children)
                    {
                      auto row = c.toMap();
                      row["full_name"] = row["first_name"].toString() + " "
                        + row["last_name"].toString();
                      c = row;
                    }
                    result["children"] = children;
                  }

                  //// Update project context
                  if (auto contextRoot = GenesisContextRoot::Get())
                  {
                    if (auto contextProject = contextRoot->GetContextProject())
                    {
                      auto children = result["children"].toList();
                      if (children.size())
                      {
                        //// Validate
                        if (int pid = children[0].toMap()["id"].toInt())
                        {
                          //// Apply project
                          contextProject->SetData(children[0].toMap());

                          GenesisWindow::Get()->ShowPage(ViewProjectInfoPageName);
                        }
                      }
                    }
                  }
                  GenesisWindow::Get()->RemoveOverlay(overlayId);
                },
                [overlayId](QNetworkReply*, QNetworkReply::NetworkError err)
                {
                  GenesisWindow::Get()->RemoveOverlay(overlayId);
                  Notification::NotifyError(tr("Failed to get project information"), err);
                }
              );
            }
          }
        }
      }
      TreeModel::ResetInstances("TreeModelDynamicProjects");
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    },
    [overlayId](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      GenesisWindow::Get()->RemoveOverlay(overlayId);
      Notification::NotifyError(tr("Failed to create project"), err);
    });

  WebDialog::Accept();
  Notification::NotifySuccess(tr("New project \"%1\" is successfully created").arg(projectName), tr("Project is successfully created"));
}

void WebDialogCreateProject::Open()
{
  WebDialog::Open();
  SetupModels();
}
}//namespace Dialogs
