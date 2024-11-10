#include "view_page_project_models_list.h"

#include "itemviews/tableview_headerview.h"
#include "itemviews/tableview_model_actions_column_delegate.h"

#include "../logic/tree_model_dynamic_project_models_list.h"
#include "../logic/tree_model_item.h"
#include <logic/known_context_tag_names.h>
#include "known_view_names.h"
#include "logic/known_json_tag_names.h"
#include "controls/table_status_bar.h"
#include "dialogs/web_dialog.h"
#include "dialogs/templates/confirm.h"
#include <ui/dialogs/templates/save_edit.h>
#include "ui/itemviews/common_table_view.h"

#include <logic/context_root.h>
#include <genesis_style/style.h>
#include <genesis/client/ui/genesis_window.h>
#include <api/api_rest.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QPlainTextEdit>

using namespace Names;
using namespace ViewPageNames;
using namespace Core;

namespace Views
{
  ViewPageProjectModelsList::ViewPageProjectModelsList(QWidget* parent)
    : View(parent)
  {
    Models = new TreeModelDynamicProjectModelsList(this);
    SetupUi();
  }

  ViewPageProjectModelsList::~ViewPageProjectModelsList()
  {
  }

  void ViewPageProjectModelsList::SetupUi()
  {

    //// Self
    setStyleSheet(Style::Genesis::GetUiStyle());

    //// Layout
    Layout = new QVBoxLayout(this);
    Layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
    Layout->setSpacing(0);

    //// Content
    {

      //// Markups list table title / caption
      {
        QHBoxLayout* captionLayout = new QHBoxLayout;
        Layout->addLayout(captionLayout);

        captionLayout->setContentsMargins(0, 0, 0, 0);
        captionLayout->setSpacing(Style::Scale(16));

        //// Caption
        {
          auto projectMarkupsListCaptions = new QLabel(tr("PCA Models list"), this);
          captionLayout->addWidget(projectMarkupsListCaptions);

          projectMarkupsListCaptions->setStyleSheet(Style::Genesis::GetH1());
        }

        captionLayout->addStretch(1);

        //      //// Add
        //      {
        //        QPushButton* add = new QPushButton(QIcon(":/resource/icons/icon_action_add.png"), tr("Add markup"), Content);
        //        captionLayout->addWidget(add);
        //        add->setDefault(true);
        //        connect(add, &QPushButton::clicked, this, &ViewPageProjectModelsList::AddModel);
        //      }
      }

      //// Markups list / table view
      {
        auto tableContainer = new QWidget(this);
        tableContainer->setProperty("style", "white_base");
        tableContainer->setLayout(new QHBoxLayout());
        tableContainer->layout()->setContentsMargins(26, 26, 26, 26);
        Layout->addWidget(tableContainer);

        Table = new CommonTableView(tableContainer, 1);
        Table->setModel(Models->GetPresentationModel());
        if (const auto tablelayout = qobject_cast<QHBoxLayout*>(tableContainer->layout()))
        {
          tablelayout->addWidget(Table, 1);
        }

        ////// Reimpls
        Table->setItemDelegateForColumn(TreeModelDynamicProjectModelsList::ColumnLast, new TableViewModelActionsColumnDelegate(Table));
        ////// Setup
        Table->setSortingEnabled(true);
        Table->setShowGrid(false);
        Table->sortByColumn(-1, Qt::AscendingOrder);
        Table->setSelectionMode(QAbstractItemView::SingleSelection);
        Table->setSelectionBehavior(QAbstractItemView::SelectRows);
        Table->horizontalHeader()->setStretchLastSection(true);
        Table->resizeColumnsToContents();
        //// Handle
        auto openModel = [](const QModelIndex& index)
          {
            if (index.isValid())
            {
              if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
              {
                if (int id = item->GetData("_analysis_id").toInt())
                {
                  if (auto contextRoot = GenesisContextRoot::Get())
                  {
                    if (auto contextProject = contextRoot->GetContextProject())
                    {
                      if (auto context = contextRoot->GetContextMarkup())
                      {
                        context->SetData(MarkupContextTags::kMarkupId, item->GetData("_markup_id").toInt());
                      }
                      if (auto context = contextRoot->GetContextMarkupVersion())
                      {
                        context->SetData("version_id", item->GetData("_version_id").toInt());
                      }
                      if (auto context = contextRoot->GetContextMarkupVersionAnalisysPCA())
                      {
                        context->SetData({ {JsonTagNames::analysis_id, id} });
                      }
                      GenesisWindow::Get()->ShowPage(ViewPageProjectPCAGraphicsPlotPageName);
                    }
                  }
                }
              }
            }
          };

        auto removeModel = [this](const QModelIndex& index)
          {
            if (index.isValid())
            {
              if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
              {
                if (int pid = item->GetData("_analysis_id").toInt())
                {
                  using namespace Dialogs::Templates;
                  using Btn = QDialogButtonBox::StandardButton;
                  auto analysisName = item->GetData("_analysis_title").toString();
                  auto dial = Confirm::confirmationWithRedPrimary(this,
                                                                  tr("PCA model removing"),
                                                                  tr("Are you shure that you want to remove model\n"
                                                                     "\"%1\"?\n"
                                                                     "You can't undo this action!").arg(analysisName),
                                                                  Btn::Ok | Btn::Cancel,
                                                                  tr("Remove"));
                  connect(dial, &WebDialog::Accepted, this,
                          [this, pid]()
                          {
                            API::REST::RemoveAnalysis(pid,
                              [this](QNetworkReply*, QJsonDocument)
                              {
                                Models->Reset();
                                Notification::NotifySuccess(tr("Analysis successfully removed"));
                              },
                              [](QNetworkReply*, QNetworkReply::NetworkError e)
                              {
                                Notification::NotifyError(tr("Error removing analysis data"), e);
                              });
                          });
                  dial->Open();
                }
              }
            }
          };

        auto editItem = [this](const QModelIndex& index)
          {
            if (!index.isValid())
              return;
            TreeModelItem* item = Models->GetItem(index);
            if(!item)
              return;
            auto modelTitle = item->GetData("_analysis_title").toString();
            auto modelComment = item->GetData("_analysis_comment").toString();
            auto id = item->GetData("_analysis_id").toInt();

            Dialogs::Templates::SaveEdit::Settings s;
            s.header = tr("Editing");
            s.titleHeader = tr("Model title");
            s.commentHeader = tr("Comment");
            s.titlePlaceholder = tr("Enter title");
            s.commentPlaceholder = tr("Enter comment");
            s.defaultTitle = modelTitle;
            s.defaultComment = modelComment;
            s.titleTooltipsSet.empty = tr("Title should be not empty");
            s.titleTooltipsSet.forbidden = tr("Title is alredy in use");
            s.titleTooltipsSet.notChecked = tr("Title is not checked yet");
            s.titleTooltipsSet.valid = tr("Title is valid");

            auto dial = new Dialogs::Templates::SaveEdit(this, s);
            connect(dial, &WebDialog::Accepted, this, [dial, id, modelTitle, modelComment, this]()
                    {
                      auto newTitle = dial->getTitle();
                      auto newComment = dial->getComment();

                      if(newTitle == modelTitle && newComment == modelComment)
                      {
                        Notification::NotifyInfo(tr("There were no changes"));
                      }
                      else
                      {
                        UpdateTitleAndCommentOnServer(id, newTitle, newComment);
                      }

                    });
            dial->Open();
          };

        //// Handle
        connect(Table, &QAbstractItemView::doubleClicked, [this, openModel](const QModelIndex& indexPresentation) {
          QModelIndex index = Models->GetPresentationModel()->mapToSource(indexPresentation);
          openModel(index);
          });
        connect(Models, &TreeModel::ActionTriggered, [openModel, removeModel, editItem](const QString& actionId, TreeModelItem* item) {
          QModelIndex index = item->GetIndex();
          if (actionId == "open")
          {
            openModel(index);
          }
          else if (actionId == "edit")
          {
            editItem(index);
          }
          else if (actionId == "remove")
          {
            removeModel(index);
          }
          });
      }
    }

    //// Table status bar
    {
      TableStatusBar* statusBar = new TableStatusBar(this, Models);
      Layout->addWidget(statusBar);
    }
    }

    void ViewPageProjectModelsList::UpdateTitleAndCommentOnServer(int id, const QString &title, const QString &comment)
    {
      API::REST::UpdateTitleForAnalysis(id, title,
        [&](QNetworkReply*, QJsonDocument doc)
        {
          const auto jsonObject = doc.object();
          if (jsonObject.value(JsonTagNames::Error).toBool())
          {
            Notification::NotifyError(tr("Error update analysis."));
          }
          else
          {
            Notification::NotifySuccess(tr("Update analysis succesfull."));
            Models->Reset();
          }
        },
        [](QNetworkReply*, QNetworkReply::NetworkError)
        {
          Notification::NotifyError(tr("Error update analysis."));
        });

      API::REST::UpdateCommentForAnalysis(id, comment,
        [&](QNetworkReply*, QJsonDocument doc)
        {
          const auto jsonObject = doc.object();
          if (jsonObject.value(JsonTagNames::Error).toBool())
          {
            Notification::NotifyError(tr("Error update analysis."));
          }
          else
          {
            Notification::NotifySuccess(tr("Update analysis succesfull."));
            Models->Reset();
          }
        },
        [](QNetworkReply*, QNetworkReply::NetworkError)
        {
          Notification::NotifyError(tr("Error update analysis."));
        });
    }
}//namespace Views
