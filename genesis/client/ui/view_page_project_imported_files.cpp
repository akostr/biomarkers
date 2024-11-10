#include "view_page_project_imported_files.h"

#include "controls/table_status_bar.h"

#include "itemviews/tableview_headerview.h"
#include "itemviews/tableview_model_actions_column_delegate.h"

#include "../logic/tree_model_item.h"
#include "../logic/tree_model_presentation.h"
#include "../logic/context_root.h"

#include "dialogs/web_dialog_upload_files.h"

#include "../ui/dialogs/web_dialog_chromatogram_information.h"

#include <genesis_style/style.h>
#include <ui/genesis_window.h>

#include <QLabel>
#include <QHeaderView>
#include <QAction>
#include <QPushButton>

using namespace Core;

////////////////////////////////////////////////////
//// Projects
namespace Views
{
ViewPageProjectImportedFiles::ViewPageProjectImportedFiles(QWidget* parent)
  : View(parent)
{
  SetupModels();
  SetupUi();
}

ViewPageProjectImportedFiles::~ViewPageProjectImportedFiles()
{
}

void ViewPageProjectImportedFiles::SetupModels()
{
  Files = new TreeModelDynamicProjectImportedFiles(this);
}

void ViewPageProjectImportedFiles::SetupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
  Layout->setSpacing(0);

  auto headerLayout = new QHBoxLayout;
  Layout->addLayout(headerLayout);
  //// Caption
  {

    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(Style::Scale(8));

    //// Caption
    {
      QLabel* catpion = new QLabel(tr("Imported files"), this);
      headerLayout->addWidget(catpion);

      catpion->setStyleSheet(Style::Genesis::GetH1());
    }
  }

  headerLayout->addStretch();
  //// Buttons
  {
    QPushButton* importFiles = new QPushButton(QIcon(":/resource/icons/icon_button_upload.png"), QString(" ") + tr("Import files"), this);
    headerLayout->addWidget(importFiles);

    importFiles->setFlat(true);

    connect(importFiles, &QPushButton::clicked, [this]
    {
      auto dialog = new WebDialogUploadFiles(this);
      dialog->Open();

      connect(dialog, &WebDialogUploadFiles::Accepted, []()
      {
      });
    });
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
      Table->setItemDelegateForColumn(TreeModelDynamicProjectImportedFiles::ColumnLast, new TableViewModelActionsColumnDelegate(Table));

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
      Table->setModel(Files->GetPresentationModel());
      Table->resizeColumnsToContents();

      //// Handle
      connect(Table, &QAbstractItemView::doubleClicked, [this](const QModelIndex& indexPresentation)
        {
          QModelIndex index = Files->GetPresentationModel()->mapToSource(indexPresentation);
          if (index.isValid())
          {
            if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
            {
              if (int pid = item->GetData("id").toInt())
              {
                if (auto contextRoot = GenesisContextRoot::Get())
                {
                  if (auto contextCdf = contextRoot->GetContextFileCDF())
                  {
                    auto data = item->GetData();
                    contextCdf->SetData(data);
                    //// switch to file view
                    //// GenesisWindow::Get()->ShowPage("view_page_???");
                  }
                }
              }
            }
          }
        });

      //// Table status bar
      {
        TableStatusBar* statusBar = new TableStatusBar(Content, Files);
        ContentLayout->addWidget(statusBar);
      }
    }
  }

  connect(Files, &TreeModel::ActionTriggered, [this](const QString& actionId, TreeModelItem* item)
  {
    if (actionId == "information")
    {
      if (int fileId = item->GetData("id").toInt())
      {
        WebDialogChromatogramInformation* dialog = new WebDialogChromatogramInformation(this, fileId);
        dialog->Open();
        connect(dialog, &WebDialog::Accepted, []
          {
          });
      }
    }
  });
}
}//namespace Views
