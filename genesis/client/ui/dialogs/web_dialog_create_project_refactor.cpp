#include "web_dialog_create_project_refactor.h"
#include "ui_web_dialog_create_project_refactor.h"

#include <genesis_style/style.h>
#include <ui/genesis_window.h>
#include "ui/known_view_names.h"
#include "../../api/api_rest.h"
#include "../../logic/context_root.h"
#include "../../logic/notification.h"
#include "../../logic/tree_model_dynamic_projects.h"
#include <logic/known_context_tag_names.h>
#include <logic/context_root.h>

#include <QPushButton>
#include <QPainter>

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

namespace Dialogs
{
WebDialogCreateProjectRefactor::WebDialogCreateProjectRefactor(QWidget* parent) :
  Dialogs::Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  , ui(new Ui::WebDialogCreateProjectRefactor)
  , mContent(new QWidget(nullptr))
{
  SetupUi();
}

WebDialogCreateProjectRefactor::~WebDialogCreateProjectRefactor()
{
  delete ui;
}

void WebDialogCreateProjectRefactor::Accept()
{
  QString projectName = ui->widget_title->text();

  using namespace Names::ModulesContextTags;
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Creating project"));
  auto module = Module(GenesisContextRoot::Get()->GetContextModules()->GetData(kModule).toInt());
  QString licence;
  for(auto it = kLicenseToModule.constKeyValueBegin(); it != kLicenseToModule.keyValueEnd(); it++)
  {
    if(it->second == module)
    {
      licence = it->first;
      break;
    }
  }
  if(licence.isEmpty())
    licence = "licence-unknown_licence";

  API::REST::Tables::AddProjectNew(
    GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kCurrentGroup).toString(),
    projectName,
    ui->widget_comment->text(),
    ui->checkBox_codeWord->isChecked() ? ui->widget_codeWord->text() : "",
    licence,
    [overlayId, projectName](QNetworkReply*, QVariantMap result)
    {
      // qDebug().noquote() << QJsonDocument(QJsonObject::fromVariantMap(result)).toJson();
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
                        auto map = children[0].toMap();
                        //// Validate
                        if (int pid = map["id"].toInt())
                        {
                          //// Apply project
                          map["group"] = map["group_title"];
                          auto color = map["status"].toList()[2].value<QColor>();
                          QPixmap pixmap(48,48);
                          pixmap.fill(QColor(0,0,0,0));
                          QPainter p(&pixmap);
                          p.setPen(Qt::NoPen);
                          p.setBrush(color);
                          p.drawEllipse(pixmap.rect());
                          pixmap.setDevicePixelRatio(2);
                          map[ContextTagNames::ProjectStatusIcon] = QIcon(pixmap);
                          contextProject->SetData(map);

                          //// Apply  page
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

        Notification::NotifySuccess(tr("New project \"%1\" is successfully created").arg(projectName), tr("Project is successfully created"));
      }
      else
      {
        Notification::NotifyError(tr("The project \"%1\" was not created. Try creating the project again.").arg(projectName), tr("Error creating project"));
      }

      TreeModel::ResetInstances("TreeModelDynamicProjects");
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    },
    [overlayId, projectName](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      GenesisWindow::Get()->RemoveOverlay(overlayId);
      Notification::NotifyError(tr("The project \"%1\" was not created. Try creating the project again.").arg(projectName), tr("Error creating project"), err);
    });

  WebDialog::Accept();
}

void WebDialogCreateProjectRefactor::SetupModels()
{
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  auto callback = [overlayId, this](QNetworkReply*, QJsonDocument doc)
  {
    auto root = doc.object();
    if (root["error"].toBool() == true)
    {
      Notification::NotifyError(root["msg"].toString(), tr("Server error"));
      GenesisWindow::Get()->RemoveOverlay(overlayId);
      Reject();
      return;
    }
    auto jforbiddenArray = root["project_titles"].toArray();

    auto s = ui->widget_title->settings();
    s.forbiddenStrings.clear();
    for (const auto& v : jforbiddenArray)
    {
      s.forbiddenStrings.insert(v.toString());
    }
    if (!s.defaultText.isEmpty() && s.forbiddenStrings.contains(s.defaultText))
    {
      QString original = s.defaultText;
      QString m;
      for (uint i = 0; i < (uint)(-1); i++)
      {
        m = QString("%1 %2").arg(original).arg(i);
        if (!s.forbiddenStrings.contains(m))
        {
          s.defaultText = m;
          break;
        }
      }
    }
    ui->widget_title->applySettings(s);

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

void WebDialogCreateProjectRefactor::SetupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Project creation");
  s.buttonsNames = { {QDialogButtonBox::Ok, tr("Create")},
                   {QDialogButtonBox::Cancel, tr("Cancel")} };
  applySettings(s);

  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);

  //// Title widget settings
  {
    auto settings = ui->widget_title->settings();
    settings.defaultText = tr("New project");
    settings.textHeader = tr("Project name");
    settings.textPlaceholder = tr("Enter name");
    settings.controlSymbols = true;
    settings.textTooltipsSet = {
      tr("Name is empty"),
      tr("Name alredy in use"),
      tr("Not checked"),
      tr("Name is valid"),
      tr("Name must not contain the characters '\', '/', '\"', '*', '<', '|', '>'")
    };
    ui->widget_title->applySettings(settings);
  }

  //// Comment widget settings
  {
    auto settings = ui->widget_comment->settings();
    settings.textHeader = tr("Comment");
    settings.textMaxLen = 70;
    settings.textPlaceholder = tr("Enter comment");
    settings.textRequired = false;
    ui->widget_comment->applySettings(settings);
  }

  //// Code word widget settings
  {
    auto settings = ui->widget_codeWord->settings();
    settings.textHeader = tr("Code word");
    settings.textPlaceholder = tr("Enter code word");
    settings.textRequired = true;
    ui->widget_codeWord->applySettings(settings);
  }

  ui->checkBox_codeWord->setChecked(false);
  setDialogDataValidity();

  connect(ui->widget_title, &DialogLineEditEx::validityChanged, this, &WebDialogCreateProjectRefactor::setDialogDataValidity);
  connect(ui->widget_codeWord, &DialogLineEditEx::validityChanged, this, &WebDialogCreateProjectRefactor::setDialogDataValidity);
  connect(ui->checkBox_codeWord, &QCheckBox::stateChanged, this, [&]
  {
    setDialogDataValidity();
  });
}

void WebDialogCreateProjectRefactor::setDialogDataValidity()
{
  ui->widget_codeWord->setEnabled(ui->checkBox_codeWord->isChecked());

  if (!ui->widget_title->checkValidity() || (ui->checkBox_codeWord->isChecked() && !ui->widget_codeWord->checkValidity()))
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
  else
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  }
}

void WebDialogCreateProjectRefactor::Open()
{
  WebDialog::Open();
  SetupModels();
}
}
