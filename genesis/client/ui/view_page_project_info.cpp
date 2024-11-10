#include "view_page_project_info.h"

#include "dialogs/web_dialog_add_user_to_project.h"
#include <ui/dialogs/project_table_rename_dialog_refactor.h>
#include <ui/dialogs/templates/tree_confirmation.h>
#include <ui/dialogs/naming_pattern_edit_dialog.h>
#include <api/api_rest.h>
#include <logic/notification.h>

#include <genesis_style/style.h>
#include <logic/context_root.h>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QPainter>
#include <QMenu>

using namespace Core;
////////////////////////////////////////////////////
//// Project Info
namespace Views
{
ViewPageProjectInfo::ViewPageProjectInfo(QWidget* parent)
  : View(parent)
{
  SetupModels();
  SetupUi();
}

ViewPageProjectInfo::~ViewPageProjectInfo()
{
}

void ViewPageProjectInfo::SetupModels()
{
  ProjectInfo   = new TreeModelDynamicProjectInfo(this);
//  ProjectUsers  = new TreeModelDynamicProjectUsers(this);
}

void ViewPageProjectInfo::SetupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
  Layout->setSpacing(0);

  //// Caption
  {
    QHBoxLayout* captionLayout = new QHBoxLayout;
    Layout->addLayout(captionLayout);
    captionLayout->setContentsMargins(0, 0, 0, 0);
    captionLayout->setSpacing(Style::Scale(32));

    //// Caption
    {
      mCaption = new QLabel(tr("General information"), this);
      captionLayout->addWidget(mCaption);

      captionLayout->addStretch();

      const auto menuButton = new QPushButton(tr("Manage the project"));
      menuButton->setStyleSheet(
        "QPushButton::menu-indicator{"
        "image: url(:/resource/controls/collpse_dn.png);"
        "width: 10px;right: 3px;}"
        "QMenu{border: 1px solid transparent;}");

      menuButton->setProperty("menu_secondary", true);

      auto menu = new QMenu(menuButton);
      menu->setToolTipsVisible(true);
      auto edit = menu->addAction(tr("Edit project information"));
      connect(edit, &QAction::triggered, this, &ViewPageProjectInfo::editProject);
      auto customize = menu->addAction(tr("Set basic settings"));
      customize->setToolTip(tr("Set the template for generating names of chromatograms and fragments in the project"));
      connect(customize, &QAction::triggered, this, &ViewPageProjectInfo::manageProjectBaseSettings);
      auto deleteProject = menu->addAction(tr("Delete project"));
      connect(deleteProject, &QAction::triggered, this, [this]
      {
        auto id = ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnId)).toInt();
        auto name = ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnName)).toString();
        Dialogs::Templates::TreeConfirmation::DeleteProjectWithDependencies({{id, name}}, this);
      });

      menuButton->setMenu(menu);
      captionLayout->addWidget(menuButton);

      mCaption->setStyleSheet(Style::Genesis::GetH1());
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

    //// Project info
    {
      ProjectInfoArea = new QFrame(Content);
      ContentLayout->addWidget(ProjectInfoArea);

      ProjectInfoAreaLayout = new QVBoxLayout(ProjectInfoArea);
      ProjectInfoAreaLayout->setContentsMargins(0, 0, 0, 0);
      ProjectInfoAreaLayout->setSpacing(0);

      //// Caption
      {
        ProjectInfoAreaCaption = new QLabel(tr("Project information"), ProjectInfoArea);
        auto layout = new QHBoxLayout(ProjectInfoArea);
        layout->addWidget(ProjectInfoAreaCaption);
        layout->addSpacerItem(new QSpacerItem(20, 0));

        layout->addStretch();
        ProjectInfoAreaLayout->addLayout(layout);
        ProjectInfoAreaCaption->setStyleSheet(Style::Genesis::GetH2());

        //// Content
        ProjectInfoAreaContentLayout = new QGridLayout;
        ProjectInfoAreaLayout->addLayout(ProjectInfoAreaContentLayout);

        ProjectInfoAreaContentLayout->setContentsMargins(0, Style::Scale(16), 0, Style::Scale(16));
        ProjectInfoAreaContentLayout->setSpacing(Style::Scale(14));

        enum row
        {
          rowProjectName,
          rowGroup,
          rowStatus,
          rowProjectId,
          rowChromatogramCount,
          rowDateCreated,
          rowComment,
          rowCodeWord
        };

        enum
        {
          columnCaption,
          columnValue
        };

        {
          QLabel* c = new QLabel(tr("Project name"), ProjectInfoArea);
          c->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          c->setStyleSheet(Style::Genesis::Fonts::RegularLight());
          ProjectInfoAreaContentLayout->addWidget(c, rowProjectName, columnCaption);
          
          ProjectName = new QLabel(ProjectInfoArea);
          ProjectInfoAreaContentLayout->addWidget(ProjectName, rowProjectName, columnValue);
          ProjectName->setStyleSheet(Style::Genesis::Fonts::RegularText());
        }
        {
          QLabel* c = new QLabel(tr("Group"), ProjectInfoArea);
          c->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          c->setStyleSheet(Style::Genesis::Fonts::RegularLight());
          ProjectInfoAreaContentLayout->addWidget(c, rowGroup, columnCaption);

          mGroup = new QLabel(ProjectInfoArea);
          ProjectInfoAreaContentLayout->addWidget(mGroup, rowGroup, columnValue);
          mGroup->setStyleSheet(Style::Genesis::Fonts::RegularText());
        }
        {
          QLabel* c = new QLabel(tr("Status"), ProjectInfoArea);
          c->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          c->setStyleSheet(Style::Genesis::Fonts::RegularLight());
          ProjectInfoAreaContentLayout->addWidget(c, rowStatus, columnCaption);

          mStatusPix = new QLabel(ProjectInfoArea);
          mStatusPix->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          mStatus = new QLabel(ProjectInfoArea);
          mStatus->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          mStatus->setStyleSheet(Style::Genesis::Fonts::RegularText());

          auto layout = new QHBoxLayout;
          layout->setSpacing(5);
          layout->addWidget(mStatusPix);
          layout->addWidget(mStatus);
          layout->addStretch();

          ProjectInfoAreaContentLayout->addLayout(layout, rowStatus, columnValue);
        }
        {
          QLabel* c = new QLabel(tr("Number (ID)"), ProjectInfoArea);
          c->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          c->setStyleSheet(Style::Genesis::Fonts::RegularLight());
          ProjectInfoAreaContentLayout->addWidget(c, rowProjectId, columnCaption);

          ProjectId = new QLabel(ProjectInfoArea);
          ProjectId->setStyleSheet(Style::Genesis::Fonts::RegularText());
          ProjectInfoAreaContentLayout->addWidget(ProjectId, rowProjectId, columnValue);
        }
        {
          QLabel* c = new QLabel(tr("Chromatogram count"), ProjectInfoArea);
          c->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          c->setStyleSheet(Style::Genesis::Fonts::RegularLight());
          ProjectInfoAreaContentLayout->addWidget(c, rowChromatogramCount, columnCaption);

          mChromatogramCount = new QLabel(ProjectInfoArea);
          mChromatogramCount->setStyleSheet(Style::Genesis::Fonts::RegularText());
          ProjectInfoAreaContentLayout->addWidget(mChromatogramCount, rowChromatogramCount, columnValue);
        }
        {
          QLabel* c = new QLabel(tr("Date created"), ProjectInfoArea);
          c->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          c->setStyleSheet(Style::Genesis::Fonts::RegularLight());
          ProjectInfoAreaContentLayout->addWidget(c, rowDateCreated, columnCaption);

          ProjectDateCreated = new QLabel(ProjectInfoArea);
          ProjectDateCreated->setStyleSheet(Style::Genesis::Fonts::RegularText());
          ProjectInfoAreaContentLayout->addWidget(ProjectDateCreated, rowDateCreated, columnValue);
        }
        {
          QLabel* c = new QLabel(tr("Comment"), ProjectInfoArea);
          c->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          c->setStyleSheet(Style::Genesis::Fonts::RegularLight());
          ProjectInfoAreaContentLayout->addWidget(c, rowComment, columnCaption);

          mComment = new QLabel(ProjectInfoArea);
          mComment->setStyleSheet(Style::Genesis::Fonts::RegularText());
          ProjectInfoAreaContentLayout->addWidget(mComment, rowComment, columnValue);
        }
        {
          QLabel* c = new QLabel(tr("Code word"), ProjectInfoArea);
          c->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
          c->setStyleSheet(Style::Genesis::Fonts::RegularLight());
          ProjectInfoAreaContentLayout->addWidget(c, rowCodeWord, columnCaption);

          mCodeWord = new QLabel(ProjectInfoArea);
          mCodeWord->setStyleSheet(Style::Genesis::Fonts::RegularText());
          ProjectInfoAreaContentLayout->addWidget(mCodeWord, rowCodeWord, columnValue);
        }
      }
    }

    //// Space
    {
      ContentLayout->addSpacing(Style::Scale(20));

      QFrame* hline = new QFrame(Content);
      hline->setFrameShape(QFrame::HLine);
      hline->setObjectName("hline");
      ContentLayout->addWidget(hline);

      ContentLayout->addSpacing(Style::Scale(20));
    }
    ContentLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Fixed,QSizePolicy::Expanding));
  }
  //// Connect
  auto updateProjectInfo = [this]()
  {
    mCaption->setText(ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnName)).toString());
    ProjectName->setText(ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnName)).toString());
    ProjectId->setText(ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnId)).toString());
    mChromatogramCount->setText(ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnChromatogramCount)).toString());
    mGroup->setText(ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnGroup)).toString());
    mComment->setText(ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnComment)).toString());
    mCodeWord->setText(ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnCodeWord)).toString());

    auto statusList = ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnStatus)).toStringList();
    if (statusList.size()>2)
    {
      mStatus->setText(statusList[1]);
      mStatusPix->setPixmap(createCircleIcon({ 10, 10 }, QColor(statusList[2])));
    }

    auto data = ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnDateCreated));
    auto date = data.toDateTime();
    ProjectDateCreated->setText(QLocale::system().toString(date, QLocale::FormatType::ShortFormat));
  };

  connect(ProjectInfo, &QAbstractItemModel::modelReset, updateProjectInfo);
  updateProjectInfo();
}

QPixmap ViewPageProjectInfo::createCircleIcon(QSize size, QColor color)
{
  QPixmap pmap(size);
  pmap.fill(QColor(0, 0, 0, 0));
  QPainter p(&pmap);
  p.setRenderHint(QPainter::RenderHint::Antialiasing);
  p.setPen(Qt::NoPen);
  p.setBrush(color);
  p.drawEllipse(pmap.rect());
  return pmap;
}

void ViewPageProjectInfo::editProject()
{
  auto pid = ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnId)).toInt();
  auto title = ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnName)).toString();
  auto comment = ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnComment)).toString();
  auto codeWord = ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnCodeWord)).toString();

  QString englishStatus;
  QString status;
  auto statusList = ProjectInfo->data(ProjectInfo->index(0, TreeModelDynamicProjectInfo::ColumnStatus)).toStringList();
  if (statusList.size() > 2)
  {
    englishStatus = statusList[0];
    status = statusList[1];
  }
  
  auto dial = new Dialogs::ProjectEditDialog(pid, title, englishStatus, comment, codeWord, this);
  connect(dial, &WebDialog::Accepted, this, [dial, pid, title, status, this]()
    {
      auto newTitle = dial->getTitle();
      auto newStatus = dial->getStatus();
      auto newCodeWord = dial->getCodeWord();
      auto newComment = dial->getComment();
      auto newStatusIcon = dial->getStatusIcon();

      auto callback = [newTitle, newStatus, newCodeWord, newComment, newStatusIcon, pid, title, status, this](QNetworkReply*, QJsonDocument doc)
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
        TreeModelDynamic::ResetInstances("TreeModelDynamicProjectInfo");
        TreeModelDynamic::ResetInstances("TreeModelDynamicProjects");
      };
      auto errorCallback = [this](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError("", e);
      };

      API::REST::SetProjectData(pid, newTitle, dial->getStatusCode(), newComment,
        newCodeWord, callback, errorCallback);
    });
  dial->Open();
}

void ViewPageProjectInfo::manageProjectBaseSettings()
{
  auto dial = new NamingPatternEditDialog(this);
  dial->Open();
}
}//namespace Views
