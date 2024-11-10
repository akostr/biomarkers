#include "view_page_projects.h"

#include <api/api_rest.h>

#include "controls/table_status_bar.h"
#include "itemviews/tableview_headerview.h"
#include "itemviews/tableview_model_actions_column_delegate.h"
#include "ui/dialogs/web_dialog_create_project_refactor.h"
#include "logic/tree_model_presentation.h"
#include "logic/tree_model_item.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/notification.h"

#include "genesis_style/style.h"
#include "ui/genesis_window.h"
#include "ui/known_view_names.h"
#include "ui/dialogs/project_table_rename_dialog.h"
#include "ui/dialogs/web_dialog_check_code_word.h"
#include "ui/itemviews/common_table_view.h"

#include <QLabel>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QTableView>
#include <QComboBox>

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

////////////////////////////////////////////////////
//// Projects
namespace Views
{
  ViewPageProjects::ViewPageProjects(QWidget* parent)
    : View(parent)
  {
    SetupModels();
    SetupUi();
  }

  void ViewPageProjects::SetupModels()
  {
    Projects = new TreeModelDynamicProjects(this);
    ProjectsRepresentationModel = Projects->GetPresentationModel();
  }

  void ViewPageProjects::SetupUi()
  {
    //// Self
    setStyleSheet(Style::Genesis::GetUiStyle());

    //// Layout
    Layout = new QVBoxLayout(this);
    Layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
    Layout->setSpacing(0);

    //// Caption
    {
      auto headerLayout = new QHBoxLayout;
      Layout->addLayout(headerLayout);

      headerLayout->setContentsMargins(0, 0, 0, 0);
      headerLayout->setSpacing(Style::Scale(8));

      //// Caption
      {
        QLabel* catpion = new QLabel(tr("Projects"), this);
        headerLayout->addWidget(catpion);
        catpion->setStyleSheet(Style::Genesis::GetH1());
        headerLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
        headerLayout->setStretch(1, 1);
        auto createProjectBtn = new QPushButton(tr("Create project"));
        createProjectBtn->setProperty("blue", true);
        createProjectBtn->setIcon(QIcon(":/resource/icons/icon_action_add_light.png"));
        connect(createProjectBtn, &QPushButton::clicked, [this]()
          {
            auto dialog = new Dialogs::WebDialogCreateProjectRefactor(this);
            dialog->Open();

            connect(dialog, &Dialogs::WebDialogCreateProjectRefactor::Accepted, []()
              {
                TreeModel::ResetInstances("TreeModelDynamicProjects");
              });
          });
        headerLayout->addWidget(createProjectBtn);
        GroupsCombo = new QComboBox(this);
        headerLayout->addWidget(GroupsCombo);
        connect(GroupsCombo, &QComboBox::currentIndexChanged, this, &ViewPageProjects::setActiveGroup);
      }
    }

    //// Space
    Layout->addSpacing(Style::Scale(24));

    //// Content
    {
      Content = new QFrame(this);
      Layout->addWidget(Content, 1);

      Content->setFrameShape(QFrame::Box);
      Content->setObjectName("rounded");
      Content->setStyleSheet("QWidget { background-color: white } ");

      ContentLayout = new QVBoxLayout(Content);
      ContentLayout->setContentsMargins(Style::Scale(32), Style::Scale(28), Style::Scale(32), Style::Scale(28));

      //// View
      {
        Table = new CommonTableView(Content);
        ContentLayout->addWidget(Table);

        // Table->setItemDelegateForColumn(ProjectsRepresentationModel->mapColumnFromSource(TreeModelDynamicProjects::ColumnLast), new TableViewModelActionsColumnDelegate(Table));
        ////// Setup
        Table->setSortingEnabled(true);
        Table->sortByColumn(-1, Qt::AscendingOrder);
        Table->verticalHeader()->hide();
        Table->setShowGrid(false);
        Table->setFrozenColumns(3);

        ////// Model
        Table->setModel(ProjectsRepresentationModel);
        Table->hideColumn(TreeModelDynamicProjects::Column::ColumnCodeWord);
        Table->resizeColumnsToContents();
        if (auto hh = Table->horizontalHeader())
        {
          hh->setSectionResizeMode(QHeaderView::ResizeToContents);
          hh->setStretchLastSection(true);
        }
        Table->setSelectionMode(QAbstractItemView::SingleSelection);
        Table->setSelectionBehavior(QAbstractItemView::SelectRows);
        Table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        Table->setColumnWidth(0, 10);

        //// Handle
        connect(Table, &QAbstractItemView::doubleClicked, [this](const QModelIndex& indexPresentation)
          {
            QModelIndex index = ProjectsRepresentationModel->mapToSource(indexPresentation);
            openProject(Projects->GetItem(index));
          });
        connect(Projects, &TreeModel::ActionTriggered, [this](const QString& actionId, TreeModelItem* item)
          {
            if (actionId == "open")
            {
              openProject(item);
            }
            else if (actionId == "rename")
            {
              renameProject(item);
            }
          });

        //// Table status bar
        {
          TableStatusBar* statusBar = new TableStatusBar(Content, Projects, ProjectsRepresentationModel);
          ContentLayout->addWidget(statusBar);
        }
      }
    }
  }

  void ViewPageProjects::setActiveGroup(int ind)
  {
    using namespace Names::UserContextTags;
    auto userContext = Core::GenesisContextRoot::Get()->GetContextUser();

    if (ind == -1)
      userContext->SetData(kCurrentGroup, QString());
    else
      userContext->SetData(kCurrentGroup, GroupsCombo->currentText());
  }

  void ViewPageProjects::openProject(TreeModelItem *projectItem)
  {
    if(!projectItem || projectItem->GetData("id").toInt() == 0)
      return;
    auto pid = projectItem->GetData("id").toInt();
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
    auto openProjectPage = [this, projectItem]
    {
      if (auto contextRoot = GenesisContextRoot::Get())
      {
        if (auto contextProject = contextRoot->GetContextProject())
        {
          auto data = projectItem->GetData();
          auto icon = Projects->data(projectItem->GetIndex(TreeModelDynamicProjects::ColumnStatus), Qt::DecorationRole);
          contextProject->SetData(data);
          contextProject->SetData(ContextTagNames::ProjectStatusIcon, icon);
          GenesisWindow::Get()->ShowPage(ViewProjectInfoPageName);
        }
      }
    };

    API::REST::Tables::GetProjectInfo
      (
        pid,
        [overlayId, projectItem, openProjectPage, this](QNetworkReply*, QVariantMap result)
        {
          QString codeWord;

          if (result.contains("children"))
          {
            QVariantList children = result["children"].toList();
            for (auto& c : children)
            {
              auto row = c.toMap();

              if (const auto it = row.find("code_word"); it != row.cend())
              {
                codeWord = it->toString();
                break;
              }
            }
          }

          if (codeWord.isEmpty())
          {
            openProjectPage();
          }
          else
          {
            using namespace Dialogs::Templates;
            auto dial = new WebDialogCheckCodeWord(codeWord, this);
            connect(dial, &WebDialogCheckCodeWord::Accepted, this, openProjectPage);
            dial->Open();
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

  void ViewPageProjects::renameProject(TreeModelItem *projectItem)
  {
    if(!projectItem || projectItem->GetData("id").toInt() == 0)
      return;
    auto pid = projectItem->GetData("id").toInt();
    QString title = projectItem->GetData("title").toString();
    auto statusArr = projectItem->GetData("status").toJsonArray();
    QString status;
    if (statusArr.size() > 2)
      status = statusArr[1].toString();
    auto dial = new Dialogs::ProjectTableRenameDialog(this, pid, title);
    connect(dial, &WebDialog::Accepted, this, [dial, pid, title, status, this]()
            {
              QString newTitle = dial->getTitle(), newStatus = dial->getStatus();
              auto callback = [newTitle, newStatus, pid, title, status, this](QNetworkReply*, QJsonDocument doc)
              {

                auto root = doc.object();
                if (root["error"].toBool())
                {
                  Notification::NotifyError(root["msg"].toString(), tr("Server error"));
                  return;
                }
                Notification::NotifyHeaderless(tr("Project(%1) name and status is updated from [%2; \"%3\"] to [%4; \"%5\"]")
                                                 .arg(pid)
                                                 .arg(title)
                                                 .arg(status)
                                                 .arg(newTitle)
                                                 .arg(newStatus));
                Projects->Reset();
              };
              auto errorCallback = [this](QNetworkReply*, QNetworkReply::NetworkError e)
              {
                Notification::NotifyError("", e);
                Projects->Reset();
              };

              API::REST::RenameAndSetProjectStatus(pid, dial->getTitle(), dial->getStatusCode(), callback, errorCallback);
            });
    dial->Open();
  }

  void ViewPageProjects::ApplyContextUser(const QString& dataId, const QVariant& data)
  {
    bool isReset = isDataReset(dataId, data);
    if (!isReset && dataId == UserContextTags::kCurrentGroup)
    {
      ProjectsRepresentationModel->SetItemDataFilter("group", data);
    }
    else if (isReset || dataId == Names::UserContextTags::kGroups)
    {
      QStringList availableGroups;

      if (isReset)
        availableGroups = GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kGroups).toStringList();
      else if (dataId == Names::UserContextTags::kGroups)
        availableGroups = data.toStringList();

      if (GroupsCombo)
      {
        GroupsCombo->blockSignals(true);
        GroupsCombo->clear();
        if (!availableGroups.isEmpty())
        {
          GroupsCombo->addItems(availableGroups);
          GroupsCombo->setCurrentIndex(0);
        }
        GroupsCombo->blockSignals(false);
      }

      setActiveGroup(GroupsCombo->currentIndex());
    }
  }
  void ViewPageProjects::ApplyContextModules(const QString& dataId, const QVariant& data)
  {
    bool isReset = isDataReset(dataId, data);
    using namespace ModulesContextTags;
    if(!isReset && dataId != kModule)
      return;
    Module module = MNoModule;
    if(isReset)
    {
      auto context = GenesisContextRoot::Get()->GetContextModules();
      module = Module(context->GetData(kModule).toInt());
    }
    else if(dataId == kModule)
    {
      module = Module(data.toInt());
    }
    if(module == MNoModule || module == MLast)
    {
      ProjectsRepresentationModel->RemoveItemDataFilter("licence");
      return;
    }

    QString currentModulePseudonim;
    auto& m = kLicenseToModule;
    for(auto it = m.keyValueBegin(); it != m.constKeyValueEnd(); it++)
    {
      if(it->second == module)
      {
        currentModulePseudonim = it->first;
        break;
      }
    }
    if(currentModulePseudonim.isEmpty())
      ProjectsRepresentationModel->RemoveItemDataFilter("licence");
    else
      ProjectsRepresentationModel->SetItemDataFilter("licence", currentModulePseudonim);

    Table->resizeColumnsToContents();
  }
}//namespace Views


