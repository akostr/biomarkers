#include "web_dialog_version_list.h"

#include "../../logic/tree_model_dynamic_markup_versions.h"
#include "../../logic/tree_model_item.h"
#include "../../logic/tree_model_presentation.h"
#include "../../logic/tree_model_dynamic_project_markups_list.h"

#include <genesis_style/style.h>

#include "../itemviews/tableview_headerview.h"
#include "../itemviews/tableview_model_actions_column_delegate.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QHeaderView>

/////////////////////////////////////////////////////
//// Web Dialog / open markup version
WebDialogMarkupVersionList::WebDialogMarkupVersionList(QWidget* parent, QSizeF size)
  : WebDialog(parent, nullptr, size, QDialogButtonBox::Open | QDialogButtonBox::Cancel)
{
  SetupModels();
  SetupUi();
}

WebDialogMarkupVersionList::~WebDialogMarkupVersionList()
{
}

QString WebDialogMarkupVersionList::GetSelectedVersionTitle()
{
  return SelectedVersionTitle;
}

void WebDialogMarkupVersionList::Accept()
{
  if (auto sm = View->selectionModel())
  {
    auto index = sm->selectedIndexes().value(0);
    index = Versions->GetPresentationModel()->mapToSource(index);
    if (index.isValid())
    {
      if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
      {
        if (int pid = item->GetData("id").toInt())
        {
          SelectedVersionTitle = item->GetData("version_title").toString();
          emit UploadVersion(pid);
        }
      }
    }
  }
  WebDialog::Accept();
}

void WebDialogMarkupVersionList::SetupModels()
{
  Versions = new TreeModelDynamicMarkupVersions(this);
}

void WebDialogMarkupVersionList::SetupUi()
{
  //// Body
  Body = new QWidget(Content);
  Content->layout()->addWidget(Body);

  BodyLayout = new QVBoxLayout(Body);
  BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
  BodyLayout->setSpacing(0);

  //// Content
  {
    //// Header
    {
      QLabel* caption = new QLabel(tr("Markup versions"), Body);
      BodyLayout->addWidget(caption);

      caption->setAlignment(Qt::AlignCenter);
      caption->setStyleSheet(Style::Genesis::GetH2());
    }

    //// Space
    BodyLayout->addSpacing(Style::Scale(26));

    //// View
    {
      //// Name
      {
        View = new QTableView(this);
        BodyLayout->addWidget(View, 1);

        View->setHorizontalHeader(new TableViewHeaderView(Qt::Horizontal, View));
        View->setItemDelegateForColumn(TreeModelDynamicMarkupVersions::ColumnLast, new TableViewModelActionsColumnDelegate(View));

        View->setSortingEnabled(false);
        View->sortByColumn(-1, Qt::AscendingOrder);
        View->horizontalHeader()->setStretchLastSection(true);
        View->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        View->verticalHeader()->hide();
        View->setShowGrid(false);
        View->setFrameShape(QFrame::NoFrame);
        View->setSelectionMode(QAbstractItemView::SingleSelection);
        View->setSelectionBehavior(QAbstractItemView::SelectRows);
        View->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

        View->setModel(Versions->GetPresentationModel());
        View->resizeColumnsToContents();

        //incorrect header displaying without this
//        View->setStyleSheet("QHeaderView::up-arrow {"
//                            "width:  0px;"
//                            "height: 0px; }"
//                            "QHeaderView::down-arrow {"
//                            "width:  0px;"
//                            "height: 0px; }");


        auto removeMarkup = [](const QModelIndex& index)
        {
          if (index.isValid())
          {
            if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
            {
              if (int pid = item->GetData("id").toInt())
              {

              }
            }
          }
        };

        connect(Versions, &TreeModel::ActionTriggered, [removeMarkup](const QString& actionId, TreeModelItem* item) {
          QModelIndex index = item->GetIndex();
          if (actionId == "remove")
          {
            removeMarkup(index);
          }
        });
      }
    }
  }

  //// Buttons
  if (auto open = ButtonBox->button(QDialogButtonBox::Open))
  {
    open->setText(tr("Open version"));
  }
}
