#include "view_page_project_children.h"

#include "controls/table_status_bar.h"

#include "itemviews/tableview_headerview.h"
#include "itemviews/tableview_model_actions_column_delegate.h"

#include "../logic/tree_model_item.h"
#include "../logic/tree_model_presentation.h"
#include "../logic/context_root.h"
#include <logic/known_context_tag_names.h>
#include <genesis_style/style.h>
#include "ui/known_view_names.h"
#include <ui/genesis_window.h>

#include <QLabel>
#include <QHeaderView>
#include <QAction>
#include <QTimer>

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

////////////////////////////////////////////////////
//// Projects
namespace Views
{
ViewPageProjectChildren::ViewPageProjectChildren(QWidget* parent)
  : View(parent)
{
  SetupModels();
  SetupUi();
}

ViewPageProjectChildren::~ViewPageProjectChildren()
{
}

void ViewPageProjectChildren::SetupModels()
{
  Projects = new TreeModelDynamicProjectChildren(this);
}

void ViewPageProjectChildren::SetupUi()
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
      QLabel* catpion = new QLabel(tr("Subprojects"), this);
      headerLayout->addWidget(catpion);

      catpion->setStyleSheet(Style::Genesis::GetH1());
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
      Table = new QTableView(Content);
      ContentLayout->addWidget(Table);

      //// Reimpls
      Table->setHorizontalHeader(new TableViewHeaderView(Qt::Horizontal, Table));
      Table->setItemDelegateForColumn(TreeModelDynamicProjectChildren::ColumnLast, new TableViewModelActionsColumnDelegate(Table));

      //// Setup
      Table->setSortingEnabled(true);
      Table->sortByColumn(-1, Qt::AscendingOrder);
      Table->horizontalHeader()->setStretchLastSection(true);
      Table->verticalHeader()->hide();
      Table->setShowGrid(false);
      Table->setFrameShape(QFrame::NoFrame);
      Table->setSelectionMode(QAbstractItemView::SingleSelection);
      Table->setSelectionBehavior(QAbstractItemView::SelectRows);

      //// Model
      Table->setModel(Projects->GetPresentationModel());
      Table->resizeColumnsToContents();

      //// Handle
      auto openProject = [](const QModelIndex& index)
      {
        QTimer::singleShot(0, [index]()
        {
          if (index.isValid())
          {
            if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
            {
              if (int pid = item->GetData("id").toInt())
              {
                if (auto contextRoot = GenesisContextRoot::Get())
                {
                  if (auto contextProject = contextRoot->GetContextProject())
                  {
                    auto data = item->GetData();
                    contextProject->SetData(data);
                    GenesisWindow::Get()->ShowPage(ViewProjectInfoPageName);
                  }
                }
              }
            }
          }
        });
      };


      //// Handle
      connect(Table, &QAbstractItemView::doubleClicked, [this, openProject](const QModelIndex& indexPresentation)
        {
          QModelIndex index = Projects->GetPresentationModel()->mapToSource(indexPresentation);
          openProject(index);
        });
      connect(Projects, &TreeModel::ActionTriggered, [openProject](const QString& actionId, TreeModelItem* item)
        {
          QModelIndex index = item->GetIndex();
          if (actionId == "open")
          {
            openProject(index);
          }
        });

      //// Table status bar
      {
        TableStatusBar* statusBar = new TableStatusBar(Content, Projects);
        ContentLayout->addWidget(statusBar);
      }
    }
  }
}
}//namespace Views
