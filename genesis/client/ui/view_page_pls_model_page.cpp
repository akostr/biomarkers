#include "view_page_pls_model_page.h"

#include <QLayout>
#include <QLabel>
#include <QFile>
#include <QTableView>

#include "genesis_style/style.h"
#include "widgets/tab_widget.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_context_names.h"
#include "known_view_names.h"
#include "api/api_rest.h"
#include "logic/notification.h"
#include "logic/enums.h"
#include "logic/known_json_tag_names.h"
#include "logic/context_root.h"
#include "itemviews/table_identity_model_actions_delegate.h"
#include <ui/dialogs/templates/save_edit.h>
#include "dialogs/web_dialog_question.h"
#include "dialogs/templates/confirm.h"
#include "logic/tree_model_dynamic_project_models_list.h"
#include "logic/tree_model_presentation.h"
#include "ui/itemviews/tableview_headerview.h"
#include "ui/itemviews/tableview_model_actions_column_delegate.h"
#include "logic/tree_model_item.h"
#include "ui/widgets/tab_widget.h"
#include "ui/itemviews/common_table_view.h"
#include "genesis_window.h"

using namespace Names;
using namespace ViewPageNames;
using namespace Constants;
using namespace Core;
using namespace Dialogs;
using namespace Widgets;

namespace Views
{
  ViewPagePlsModelPage::ViewPagePlsModelPage(QWidget* parent)
    : View(parent)
  {
    SetupUi();
  }

  void ViewPagePlsModelPage::SetupUi()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());
    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    const auto caption = new QLabel(tr("PLS models and predictions"));
    caption->setStyleSheet(Style::Genesis::GetH1());
    mainLayout->addWidget(caption);

    auto tabContainer = new QWidget(this);
    tabContainer->setProperty("style", "white_base");
    tabContainer->setLayout(new QVBoxLayout());
    mainLayout->addWidget(tabContainer);

    const auto tabWidget = new TabWidget(tabContainer);
    connect(tabWidget, &TabWidget::IndexChanged, this, [&]() { StatusBarLabel->setText(GetStatusBarText()); });
    if (const auto& layout = tabContainer->layout())
    {
      layout->setContentsMargins(26, 26, 26, 26);
      layout->addWidget(tabWidget);
    }
    StatusBarLabel = new QLabel(this);

    mainLayout->addWidget(StatusBarLabel);

    PlsModelTableView = new CommonTableView(this, 1);
    PlsModelList = new TreeModelDynamicProjectModelsList(this, AnalysisType::PLS, AnalysisType::PLS);
    PlsModelTableView->setModel(PlsModelList->GetPresentationModel());
    connect(PlsModelList, &QAbstractItemModel::modelReset, this,
      [&]()
      {
        PlsModelTableView->resizeColumnsToContents();
        StatusBarLabel->setText(GetStatusBarText());
      });
    connect(PlsModelList, &TreeModel::ActionTriggered,
      [&](const QString& actionId, TreeModelItem* item)
      {
        if (actionId == "open")
        {
          OpenPlsModel(item);
        }
        else if (actionId == "remove")
        {
          DeleteItemFromTable(item, false);
        }
        else if (actionId == "edit")
        {
          EditItemFromTable(item);
        }
      });
    connect(PlsModelTableView, &QAbstractItemView::doubleClicked, [&](const QModelIndex& indexPresentation)
      {
        QModelIndex index = PlsModelList->GetPresentationModel()->mapToSource(indexPresentation);
        if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
          OpenPlsModel(item);
      });
    //// Reimpls
    PlsModelTableView->setItemDelegateForColumn(TreeModelDynamicProjectModelsList::ColumnLast, new TableViewModelActionsColumnDelegate(PlsModelTableView));
    //// Setup
    PlsModelTableView->setSortingEnabled(true);
    PlsModelTableView->sortByColumn(-1, Qt::AscendingOrder);
    PlsModelTableView->horizontalHeader()->setStretchLastSection(true);
    PlsModelTableView->verticalHeader()->hide();
    PlsModelTableView->setShowGrid(false);
    PlsModelTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    PlsModelTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    PlsModelTableView->horizontalHeader()->setStretchLastSection(true);

    tabWidget->AddTabWidget(PlsModelTableView, tr("PLS models"));

    PlsPredictionTableView = new CommonTableView(this, 1);
    PlsPredictModelList = new TreeModelDynamicProjectModelsList(this, AnalysisType::PLS, AnalysisType::PLSPredict);
    PlsPredictionTableView->setModel(PlsPredictModelList->GetPresentationModel());
    connect(PlsPredictModelList, &QAbstractItemModel::modelReset, this,
      [&]()
      {
        PlsPredictionTableView->resizeColumnsToContents();
        StatusBarLabel->setText(GetStatusBarText());
      });
    connect(PlsPredictModelList, &TreeModel::ActionTriggered,
      [&](const QString& actionId, TreeModelItem* item)
      {
        if (actionId == "open")
        {
          OpenPlsPredictModel(item);
        }
        else if (actionId == "remove")
        {
          DeleteItemFromTable(item, true);
        }
        else if (actionId == "edit")
        {
          EditItemFromTable(item);
        }
      });
    connect(PlsPredictionTableView, &QAbstractItemView::doubleClicked, [&](const QModelIndex& indexPresentation)
      {
        QModelIndex index = PlsPredictModelList->GetPresentationModel()->mapToSource(indexPresentation);
        if (TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer()))
          OpenPlsPredictModel(item);
      });
    //// Reimpls
    PlsPredictionTableView->setItemDelegateForColumn(TreeModelDynamicProjectModelsList::ColumnLast, new TableViewModelActionsColumnDelegate(PlsPredictionTableView));
    //// Setup
    PlsPredictionTableView->setSortingEnabled(true);
    PlsPredictionTableView->sortByColumn(-1, Qt::AscendingOrder);
    PlsPredictionTableView->horizontalHeader()->setStretchLastSection(true);
    PlsPredictionTableView->verticalHeader()->hide();
    PlsPredictionTableView->setShowGrid(false);
    PlsPredictionTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    PlsPredictionTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    PlsPredictionTableView->horizontalHeader()->setStretchLastSection(true);

    tabWidget->AddTabWidget(PlsPredictionTableView, tr("PLS predictions"));
    connect(StatusBarLabel, &QLabel::linkActivated, this,
      [&]()
      {
        PlsModelList->Reset();
        PlsPredictModelList->Reset();
        StatusBarLabel->setText(GetStatusBarText());
      });
  }

  void ViewPagePlsModelPage::DeleteAnalysisFromSeriver(int id)
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
          PlsModelList->Reset();
          Notification::NotifySuccess(tr("Analysis model [id = %1] was deleted").arg(id));
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Error receive analysis model list."), err);
      });
  }

  void ViewPagePlsModelPage::UpdateTitleAndCommentOnServer(int id,
    const QString& title, const QString& comment)
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
          PlsModelList->Reset();
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
          PlsModelList->Reset();
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError)
      {
        Notification::NotifyError(tr("Error update analysis."));
      });
  }

  void ViewPagePlsModelPage::OpenPlsModel(TreeModelItem* item)
  {
    if (auto contextRoot = GenesisContextRoot::Get())
    {
      if (auto markupPls = contextRoot->GetContextMarkupVersionAnalisysPLS())
          markupPls->SetData({ {ContextTagNames::AnalysisId, item->GetData("_analysis_id").toInt()} });
      GenesisWindow::Get()->ShowPage(ViewPageProjectPLSGraphicsPlotPageName);
    }
  }

  void ViewPagePlsModelPage::OpenPlsPredictModel(TreeModelItem* item)
  {
    if (auto contextRoot = GenesisContextRoot::Get())
    {
      if (auto markupPlsPredict = contextRoot->GetContextMarkupVersionAnalisysPLSPredict())
          markupPlsPredict->SetData({ {ContextTagNames::AnalysisId, item->GetData("_analysis_id").toInt()} });
      GenesisWindow::Get()->ShowPage(ViewPageNames::ViewPageProjectPlsPredictName);
    }
  }

  QString ViewPagePlsModelPage::GetStatusBarText() const
  {
    QString text;
    if (auto tabWidget = findChild<TabWidget*>())
    {
      if (auto tableView = qobject_cast<QTableView*>(tabWidget->GetCurrentWidget()))
      {
        if (auto model = tableView->model())
        {
          const auto count = model->rowCount();
          text = tr("Items: %1").arg(model->rowCount());
        }
      }
    }
    return QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
  }

  void ViewPagePlsModelPage::ApplyContextMarkupVersionAnalysisPLS(const QString& dataId, const QVariant& data)
  {
    if (dataId == Names::UserContextTags::kUserState)
    {
      if (data.toInt() == Names::UserContextTags::UserState::notAuthorized)
        PlsModelList->Clear();
      else
        PlsModelList->Reset();
    }
  }

  void ViewPagePlsModelPage::DeleteItemFromTable(TreeModelItem* item, bool isPredict)
  {
    using namespace Dialogs::Templates;
    using Btn = QDialogButtonBox::StandardButton;
    auto analysisName = item->GetData("_analysis_title").toString();
    auto dial = Confirm::confirmationWithRedPrimary(this,
                                                    isPredict ? tr("PLS predict removing") : tr("PLS model removing"),
                                                    tr("Are you shure that you want to remove\n"
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

  void ViewPagePlsModelPage::EditItemFromTable(TreeModelItem* item)
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
}
