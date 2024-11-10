#include "view_page_project_markups_list.h"

#include "api/api_rest.h"
#include "controls/table_status_bar.h"
#include "dialogs/web_dialog_create_markup.h"
#include "genesis_style/style.h"
#include "itemviews/tableview_headerview.h"
#include "itemviews/tableview_model_actions_column_delegate.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/notification.h"
#include "logic/tree_model_dynamic_project_markups_list.h"
#include "logic/tree_model_item.h"
#include "logic/tree_model_presentation.h"
#include "ui/dialogs/templates/confirm.h"
#include "ui/dialogs/templates/save_edit.h"
#include "ui/genesis_window.h"
#include "ui/known_view_names.h"
#include "ui/itemviews/event_table_view.h"

#include <QDebug>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

namespace Views
{
  ViewPageProjectMarkupsList::ViewPageProjectMarkupsList(QWidget* parent)
    : View(parent)
  {
    Markups = new TreeModelDynamicProjectMarkupsList(this);
    SetupUi();
  }

  void ViewPageProjectMarkupsList::SetupUi()
  {
    //// Self
    setStyleSheet(Style::Genesis::GetUiStyle());

    const auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    //// Markups list / table view
    {
      Table = new Control::EventTableView(this);
      Table->setStyleSheet("QTableView::item{ padding : 2px; }");
      mainLayout->addWidget(Table);
      MarkupsCountLabel = new QLabel();
      mainLayout->addWidget(MarkupsCountLabel);
      connect(MarkupsCountLabel, &QLabel::linkActivated, this, [] {
        TreeModel::ResetInstances("TreeModelDynamicProjectMarkupsList");
        });
      connect(Markups, &QAbstractItemModel::modelReset, this, &ViewPageProjectMarkupsList::UpdateMarkupsCountLabel);

      //// Reimpls
      Table->setItemDelegateForColumn(TreeModelDynamicProjectMarkupsList::ColumnLast, new TableViewModelActionsColumnDelegate(Table));
      Table->verticalHeader()->hide();
      //// Setup
      Table->setSortingEnabled(true);
      Table->sortByColumn(TreeModelDynamicProjectMarkupsList::ColumnChanged, Qt::DescendingOrder);
      Table->horizontalHeader()->setStretchLastSection(true);
      Table->setSelectionMode(QAbstractItemView::SingleSelection);
      Table->setSelectionBehavior(QAbstractItemView::SelectRows);

      Table->setModel(Markups->GetPresentationModel());

      //// Handle
      auto openMarkup = [](const QModelIndex& index)
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
                    if (auto contextUi = contextRoot->GetContextMarkup())
                    {
                      auto data = item->GetData();
                      contextUi->SetData(data);
                    }
                    GenesisWindow::Get()->ShowPage(ViewProjectMarkupPageName);
                  }
                }
              }
            }
          }
        };

      auto removeMarkup = [this](int id)
        {
          API::REST::Tables::RemoveMarkup(id, [](QNetworkReply*, QJsonDocument)
            {
              TreeModel::ResetInstances("TreeModelDynamicProjectMarkupsList");
            },
            [](QNetworkReply*, QNetworkReply::NetworkError err)
            {
              Notification::NotifyError(tr("Failed to remove markup from database"), err);
            });
        };

      auto editMarkup = [this](int id, QString title, QString comment)
        {
          auto s = Dialogs::Templates::SaveEdit::Settings();
          s.header = tr("Edit");
          s.titleHeader = tr("Markup title");
          s.commentHeader = tr("Comment");
          s.titleRequired = true;
          s.commentRequired = false;
          s.titlePlaceholder = tr("Enter title here");
          s.commentPlaceholder = tr("Enter comment here");
          s.defaultTitle = title;
          s.defaultComment = comment;
          s.titleTooltipsSet = {
            tr("Markup title empty"),
            tr("Error"),
            tr("Not checked"),
            tr("Markup title is valid")
          };
          auto dial = new Dialogs::Templates::SaveEdit(this, s);
          connect(dial, &WebDialog::Accepted, this, [id, dial]()
            {
              API::REST::UpdateMarkupData(id, dial->getTitle(), dial->getComment(),
              [](QNetworkReply*, QJsonDocument doc)
                {
                  auto obj = doc.object();
                  if (obj["error"].toBool())
                  {
                    Notification::NotifyError(tr("Server error"), obj["msg"].toString());
                  }
                  TreeModel::ResetInstances("TreeModelDynamicProjectMarkupsList");
                  Notification::NotifySuccess(tr("Markup renamed"), tr("Success"));
                },
                [](QNetworkReply*, QNetworkReply::NetworkError e)
                {
                  Notification::NotifyError(tr("Server error"), e);
                });
            });
          dial->Open();
        };

      //// Handle
      connect(Table, &Control::EventTableView::FrozenClicked, [this, openMarkup](const QModelIndex& indexPresentation)
        {
          QModelIndex index = Markups->GetPresentationModel()->mapToSource(indexPresentation);
          if (auto tableView = qobject_cast<Control::EventTableView*>(Table))
          {
            if (index.column() == 0)
            {
              openMarkup(index);
            }
          }
        });
      connect(Markups, &TreeModel::ActionTriggered,
        [this, removeMarkup, editMarkup](const QString& actionId, TreeModelItem* item)
        {
          int id = item->GetData("id").toInt();
          QString title = item->GetData("title").toString();
          QString comment = item->GetData("comment").toString();
          if (actionId == "remove")
          {
            using Btns = QDialogButtonBox;
            auto s = Dialogs::Templates::Confirm::Settings();
            s.buttonsNames[Btns::Ok] = tr("Remove");
            s.buttonsProperties[Btns::Ok]["red"] = true;
            s.dialogHeader = tr("Markup removing");
            auto l = new QLabel(tr("Are you shure, that you want to remove markup \"%1\" from project? This action can't be cancelled.").arg(title));
            l->setWordWrap(true);
            auto dial = new Dialogs::Templates::Confirm(this, s, Btns::Ok | Btns::Cancel, l);
            connect(dial, &WebDialog::Accepted, this, [removeMarkup, id]()
              {
                removeMarkup(id);
              });
            dial->Open();
          }
          else if (actionId == "edit")
          {
            editMarkup(id, title, comment);
          }
        });

    }
  }

  void ViewPageProjectMarkupsList::UpdateMarkupsCountLabel()
  {
    if (Markups)
    {
      QString text = tr("Results total: ") + QString::number(Markups->rowCount());
      QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
      MarkupsCountLabel->setText(tmpl);
    }
  }

  void ViewPageProjectMarkupsList::ApplyContextModules(const QString &dataId, const QVariant &data)
  {
    bool isReset = isDataReset(dataId, data);

    using namespace Names::ModulesContextTags;

    if(!isReset && dataId != kModule)
      return;

    Module module = MNoModule;

    if(isReset)
    {
      auto context = Core::GenesisContextRoot::Get()->GetContextModules();
      module = Module(context->GetData(kModule).toInt());
    }
    else if(dataId == kModule)
    {
      if(module == data.toInt())
        return;
      module = Module(data.toInt());
    }

    auto presentationModel = Markups->GetPresentationModel();
    switch(module)
    {
    case Names::ModulesContextTags::MBiomarkers:
    {
      presentationModel->AddRejectedColumn(TreeModelDynamicProjectMarkupsList::ColumnReference);
      auto actionsDelegate = Table->itemDelegateForColumn(TreeModelDynamicProjectMarkupsList::ColumnLast);
      Table->setItemDelegateForColumn(TreeModelDynamicProjectMarkupsList::ColumnLast - 1, actionsDelegate);
      break;
    }
    case Names::ModulesContextTags::MReservoir:
    case Names::ModulesContextTags::MPlots:
    case Names::ModulesContextTags::MNoModule:
    case Names::ModulesContextTags::MLast:
    default:
    {
      presentationModel->RemoveRejectedColumn(TreeModelDynamicProjectMarkupsList::ColumnReference);
      auto defaultDelegate = Table->itemDelegate();
      Table->setItemDelegateForColumn(TreeModelDynamicProjectMarkupsList::ColumnLast - 1, defaultDelegate);
      break;
    }
    }
  }

}//namespace Views
