#include "view_page_mcr_model_page.h"

#include "genesis_style/style.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/enums.h"
#include "api/api_rest.h"
#include "logic/known_json_tag_names.h"
#include "logic/notification.h"
#include <ui/dialogs/templates/save_edit.h>
#include "dialogs/web_dialog_question.h"
#include "known_view_names.h"
#include "logic/tree_model_presentation.h"
#include "ui/itemviews/tableview_headerview.h"
#include "ui/itemviews/tableview_model_actions_column_delegate.h"
#include "logic/tree_model_item.h"
#include "logic/tree_model_dynamic_project_models_list.h"
#include "ui/itemviews/common_table_view.h"
#include "genesis_window.h"
#include "dialogs/templates/confirm.h"

#include <QVBoxLayout>
#include <QLabel>


using namespace Core;
using namespace Names;
using namespace ViewPageNames;
using namespace Constants;
using namespace Dialogs;

namespace Views
{
  ViewPageMcrModelPage::ViewPageMcrModelPage(QWidget* parent)
    :View(parent)
  {
    SetupUi();
  }

  void ViewPageMcrModelPage::DeleteItemFromTable(TreeModelItem* item)
  {
    using namespace Dialogs::Templates;
    using Btn = QDialogButtonBox::StandardButton;
    auto analysisName = item->GetData("_analysis_title").toString();
    auto dial = Confirm::confirmationWithRedPrimary(this,
                                                    tr("MCR model removing"),
                                                    tr("Are you shure that you want to remove model\n"
                                                       "\"%1\"?\n"
                                                       "You can't undo this action!").arg(analysisName),
                                                    Btn::Ok | Btn::Cancel,
                                                    tr("Remove"));
    connect(dial, &WebDialog::Accepted, this,
            [&, item]()
            {
              DeleteAnalysisFromSeriver(item->GetData("_analysis_id").toInt());
            });
    dial->Open();
  }

  void ViewPageMcrModelPage::EditItemFromTable(TreeModelItem* item)
  {
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
  }

  void ViewPageMcrModelPage::SetupUi()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());
    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    const auto caption = new QLabel(tr("MCR models"));
    caption->setStyleSheet(Style::Genesis::GetH1());
    mainLayout->addWidget(caption);

    auto tableContainer = new QWidget(this);
    tableContainer->setProperty("style", "white_base");
    tableContainer->setLayout(new QVBoxLayout());
    mainLayout->addWidget(tableContainer);

    StatusBarLabel = new QLabel(this);
    mainLayout->addWidget(StatusBarLabel);

    McrModelTableView = new CommonTableView(this, 1);
    McrModelList = new TreeModelDynamicProjectModelsList(this, AnalysisType::MCR);
    McrModelTableView->setModel(McrModelList->GetPresentationModel());
    connect(StatusBarLabel, &QLabel::linkActivated, this,
      [&]()
      {
        McrModelList->Reset();
        StatusBarLabel->setText(GetStatusBarText());
      });
    connect(McrModelList, &TreeModel::ActionTriggered,
      [&](const QString& actionId, TreeModelItem* item)
      {
        if (actionId == "open")
        {
          OpenMcrModel(item);
        }
        else if (actionId == "remove")
        {
          DeleteItemFromTable(item);
        }
        else if (actionId == "edit")
        {
          EditItemFromTable(item);
        }
      });

    //// Reimpls
    McrModelTableView->setItemDelegateForColumn(TreeModelDynamicProjectModelsList::ColumnLast, new TableViewModelActionsColumnDelegate(McrModelTableView));
    //// Setup
    McrModelTableView->setSortingEnabled(true);
    McrModelTableView->sortByColumn(-1, Qt::AscendingOrder);
    McrModelTableView->setShowGrid(false);
    McrModelTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    McrModelTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    McrModelTableView->horizontalHeader()->setStretchLastSection(true);

    if (auto layout = tableContainer->layout())
    {
      layout->setContentsMargins(26, 26, 26, 26);
      layout->addWidget(McrModelTableView);
    }

    connect(McrModelList, &QAbstractItemModel::modelReset, this,
      [&]()
      {
        McrModelTableView->resizeColumnsToContents();
        StatusBarLabel->setText(GetStatusBarText());
      });
    mainLayout->addWidget(StatusBarLabel);
    connect(McrModelTableView, &QAbstractItemView::doubleClicked, [&](const QModelIndex& indexPresentation)
      {
        QModelIndex index = McrModelList->GetPresentationModel()->mapToSource(indexPresentation);
        if(TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
          OpenMcrModel(item);
      });
  }

  void ViewPageMcrModelPage::OpenMcrModel(TreeModelItem* item)
  {
    if (auto contextRoot = GenesisContextRoot::Get())
    {
      if (auto markupPls = contextRoot->GetContextMarkupVersionAnalisysMCR())
        markupPls->SetData(ContextTagNames::AnalysisId, item->GetData("_analysis_id").toInt());
      GenesisWindow::Get()->ShowPage(ViewPageProjectMcrGraphicPlotPageName);
    }
  }

  void ViewPageMcrModelPage::DeleteAnalysisFromSeriver(int id)
  {
    API::REST::RemoveAnalysis(id,
      [&, id](QNetworkReply*, QJsonDocument doc)
      {
        const auto jsonObject = doc.object();
        if (jsonObject.value(JsonTagNames::Error).toBool())
        {
          Notification::NotifyError(tr("Error receive analysis model list."));
        }
        else
        {
          McrModelList->Reset();
          Notification::NotifySuccess(tr("Analysis model [id = %1] was deleted").arg(id));
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Error receive analysis model list."), err);
      });
  }

  void ViewPageMcrModelPage::UpdateTitleAndCommentOnServer(int id, const QString& title,
    const QString& comment)
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
          McrModelList->Reset();
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
          McrModelList->Reset();
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError)
      {
        Notification::NotifyError(tr("Error update analysis."));
      });
  }

  QString ViewPageMcrModelPage::GetStatusBarText() const
  {
    QString text;
    if (McrModelList)
    {
      text = tr("Items: %1").arg(McrModelList->rowCount());
    }
    return QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
  }
}
