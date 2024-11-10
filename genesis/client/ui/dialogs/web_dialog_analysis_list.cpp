#include "web_dialog_analysis_list.h"

#include "../../logic/tree_model_dynamic_analyzes.h"
#include "../../logic/tree_model_item.h"
#include "../../logic/tree_model_presentation.h"

#include <genesis_style/style.h>
#include <genesis/client/ui/genesis_window.h>
#include <api/api_rest.h>

#include "../itemviews/tableview_headerview.h"
#include "../itemviews/tableview_model_actions_column_delegate.h"

#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

WebDialogAnalysisList::WebDialogAnalysisList(QWidget* parent)
  : WebDialog(parent, nullptr, QSizeF(0.4, 0), QDialogButtonBox::Open | QDialogButtonBox::Cancel)
{
  SetupModels();
  SetupUi();
}

void WebDialogAnalysisList::Accept()
{
  if (auto sm = View->selectionModel())
  {
    auto index = sm->selectedIndexes().value(0);
    index = Analyzes->GetPresentationModel()->mapToSource(index);
    if (index.isValid())
    {
      if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
      {
        if (int pid = item->GetData("_analysis_id").toInt())
        {
          emit UploadAnalysis(pid);
        }
      }
    }
  }
  WebDialog::Accept();
}

void WebDialogAnalysisList::SetupModels()
{
  Analyzes = new TreeModelDynamicAnalyzes(this);
}

void WebDialogAnalysisList::SetupUi()
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
      QLabel* caption = new QLabel(tr("Analyzes"), Body);
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
        View->setItemDelegateForColumn(TreeModelDynamicAnalyzes::ColumnLast, new TableViewModelActionsColumnDelegate(View));

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

        View->setModel(Analyzes->GetPresentationModel());
        View->resizeColumnsToContents();

        //incorrect header displaying without this
//        View->setStyleSheet("QHeaderView::up-arrow {"
//                            "width:  0px;"
//                            "height: 0px; }"
//                            "QHeaderView::down-arrow {"
//                            "width:  0px;"
//                            "height: 0px; }");


        auto removeAnalyze = [this](const QModelIndex& index)
        {
          if (index.isValid())
          {
            if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
            {
              if (int pid = item->GetData("_analysis_id").toInt())
              {
                API::REST::RemoveAnalysis(pid,
                [this](QNetworkReply*, QJsonDocument)
                {
                  Analyzes->Reset();
                  Notification::NotifySuccess(tr("Analysis successfully removed"));
                },
                [](QNetworkReply*, QNetworkReply::NetworkError e)
                {
                  Notification::NotifyError(tr("Error removing analysis data"), e);
                });
              }
            }
          }
        };

        connect(View, &QAbstractItemView::doubleClicked, [this](const QModelIndex& indexPresentation) {
          QModelIndex index = Analyzes->GetPresentationModel()->mapToSource(indexPresentation);
          if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
          {
            if (int pid = item->GetData("_analysis_id").toInt())
            {
              emit UploadAnalysis(pid);
              WebDialog::Accept();
            }
          }
        });

        connect(Analyzes, &TreeModel::ActionTriggered, [removeAnalyze](const QString& actionId, TreeModelItem* item) {
          QModelIndex index = item->GetIndex();
          if (actionId == "remove")
          {
            removeAnalyze(index);
          }
        });
      }
    }
  }

  //// Buttons
  if (auto open = ButtonBox->button(QDialogButtonBox::Open))
  {
    open->setText(tr("Open analysis"));
  }
}
