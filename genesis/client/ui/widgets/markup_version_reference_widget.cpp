#include "markup_version_reference_widget.h"

#include <QVBoxLayout>

#include "ui/widgets/table_view_widget.h"
#include "ui/itemviews/table_identity_model_actions_delegate.h"
#include "ui/contexts/edit_delete_menu_context.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_context_names.h"
#include "api/api_rest.h"
#include "logic/notification.h"
#include "ui/dialogs/templates/save_edit.h"
#include "ui/dialogs/templates/tree_confirmation.h"
#include "ui/dialogs/web_dialog_question.h"
#include "logic/tree_model_dynamic_project_markup_etalon_list.h"
#include "genesis_style/style.h"
#include "logic/tree_model_presentation.h"
#include "ui/itemviews/tableview_headerview.h"
#include "ui/itemviews/tableview_model_actions_column_delegate.h"
#include "logic/tree_model_item.h"
#include "ui/itemviews/common_table_view.h"
#include "ui/dialogs/web_overlay.h"

using namespace Core;
using namespace Names;
using namespace Dialogs;

namespace Widgets
{
  MarkupVersionReferenceWidget::MarkupVersionReferenceWidget(QWidget* parent)
    : QWidget(parent)
  {
    SetupUi();
  }

  void MarkupVersionReferenceWidget::SetupUi()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());
    const auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    ReferenceTableView = new CommonTableView(this);
    ModelEtalonList = new TreeModelDynamicProjectMarkupEtalonList(this);
    ReferenceTableView->setItemDelegateForColumn(TreeModelDynamicProjectMarkupEtalonList::ColumnLast, new TableViewModelActionsColumnDelegate(ReferenceTableView));
    ReferenceTableView->setModel(ModelEtalonList->GetPresentationModel());
    ReferenceTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ReferenceTableView->verticalHeader()->hide();
    //// Setup
    ReferenceTableView->setSortingEnabled(true);
    ReferenceTableView->sortByColumn(TreeModelDynamicProjectMarkupEtalonList::ColumnTitle, Qt::DescendingOrder);
    ReferenceTableView->horizontalHeader()->setStretchLastSection(true);
    ReferenceTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ReferenceTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ReferenceTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ReferenceTableView->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(ReferenceTableView);
    ReferencesCountLabel = new QLabel();
    mainLayout->addWidget(ReferencesCountLabel);
    connect(ReferencesCountLabel, &QLabel::linkActivated, this, [] {TreeModel::ResetInstances("TreeModelDynamicProjectMarkupEtalonList"); });
    connect(ModelEtalonList, &QAbstractItemModel::modelReset, this, &MarkupVersionReferenceWidget::UpdateReferencesCountLabel);
    connect(ModelEtalonList, &TreeModel::ActionTriggered,
      [&](const QString& actionId, TreeModelItem* item)
      {
        if (actionId == "remove")
        {
          DeleteItemFromTable(item);
        }
        else if (actionId == "edit")
        {
          EditItemFromTable(item);
        }
      });
  }

  void MarkupVersionReferenceWidget::DeleteItemFromTable(TreeModelItem* item)
  {
    using namespace Dialogs::Templates;
    auto dialPtr = new TreeConfirmation(this);
    dialPtr->Open();
    //// after dialog accepted
    connect(dialPtr, &WebDialog::Accepted, [this, dialPtr]()
            {
              auto idList = dialPtr->getConfirmedUserDataList();
              if(idList.isEmpty())
                return;
              auto overlay = new WebOverlay(tr("Removing files"));
              for(auto& v : idList)
              {
                int id = v.toInt();
                API::REST::DeleteReference(id,
                  [overlay](QNetworkReply*, QJsonDocument)
                  {
                    TreeModel::ResetInstances("TreeModelDynamicProjectMarkupEtalonList");
                    delete overlay;
                  },
                  [id, overlay](QNetworkReply*, QNetworkReply::NetworkError err)
                  {
                    Notification::NotifyError(tr("Failed when trying to remove file %1").arg(id), err);
                    delete overlay;
                  });
              }
            });

    //// data loading before
    auto rootItem = new TreeItem();
    auto overlay = new WebOverlay(tr("Loading dependencies"), this);
    auto title = item->GetData("_title").toString();
    auto id = item->GetData("_id").toInt();
    auto topLevelItem = new TreeItem();
    topLevelItem->displayRoleData = title;
    topLevelItem->userRoleData = id;
    rootItem->children.append(topLevelItem);

    API::REST::GetReferenceDependencies(id,
      [overlay, topLevelItem, dialPtr, rootItem](QNetworkReply*, QJsonDocument doc)
      {
        auto root = doc.object();
        if(root.contains("error") && root["error"].toBool())
        {
          Notification::NotifyError(root["msg"].toString(), tr("Server error"));
          delete overlay;
          return;
        }
        auto columns = root["columns"].toArray();
        auto data = root["data"].toArray();
        topLevelItem->displayRoleData = topLevelItem->displayRoleData.toString() + tr(" (%n connected element(s))", "", data.size());
        if(data.isEmpty())
        {
          auto treeItem = new TreeItem();
          treeItem->displayRoleData = tr("No dependencies");
          topLevelItem->children.append(treeItem);
        }
        else
        {
          for(int i = 0; i < data.size(); i++)
          {
            auto jitem = data[i].toArray();
            auto treeItem = new TreeItem();
            if(jitem.first().toString().toLower() == "markup")
              treeItem->displayRoleData = tr("Markup") + " \"" + jitem.last().toString() + "\"";
            else if(jitem.first().toString().toLower() == "analysis")
              treeItem->displayRoleData = tr("Analysis") + " \"" + jitem.last().toString() + "\"";
            else if(jitem.first().toString().toLower() == "table")
              treeItem->displayRoleData = tr("Table") + " \"" + jitem.last().toString()+ "\"";
            else if(jitem.first().toString().toLower() == "reference")
              treeItem->displayRoleData = tr("Reference") + " \"" + jitem.last().toString()+ "\"";
            else if(jitem.first().toString().toLower() == "heightratiomatrix")
              treeItem->displayRoleData = tr("Height ratio matrix") + " \"" + jitem.last().toString()+ "\"";
            else
              treeItem->displayRoleData = jitem.first().toString() + " \"" + jitem.last().toString() + "\"";
            topLevelItem->children.append(treeItem);
          }
        }
        auto s = dialPtr->settings();
        s.rootTreeItem = rootItem;
        s.updateCounterLabelFunc = [](int count) -> QString
        {
          return tr("Picked %n reference(s) for removing", "", count);
        };
        s.dialogSettings.buttonsNames = {{QDialogButtonBox::Ok, tr("Remove")}};
        s.dialogSettings.buttonsProperties = {{QDialogButtonBox::Ok, {{"red", true}}}};
        s.dialogSettings.dialogHeader = tr("Reference removing");
        s.phrase = tr("Are you shure that you want to remove reference(s) and it's dependencies?");
        dialPtr->applySettings(s);
        delete overlay;
      },
      [overlay, topLevelItem](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        auto treeItem = new TreeItem();
        treeItem->displayRoleData = tr("Failed to load dependencies");
        topLevelItem->children.append(treeItem);

        Notification::NotifyError(tr("Error while scan file %1 dependencies").arg(topLevelItem->displayRoleData.toString()), e);
        delete overlay;
      });
    dialPtr->Open();
  }

  void MarkupVersionReferenceWidget::EditItemFromTable(TreeModelItem* item)
  {
    auto id = item->GetData("_id").toInt();
    auto title = item->GetData("_title").toString();
    auto comment = item->GetData("_comment").toString();

    Dialogs::Templates::SaveEdit::Settings s;
    s.header = tr("Editing reference");
    s.titleHeader = tr("Reference title");
    s.commentHeader = tr("Comment");
    s.titlePlaceholder = tr("Enter title");
    s.commentPlaceholder = tr("Enter comment");
    s.defaultTitle = title;
    s.defaultComment = comment;
    s.titleTooltipsSet.empty = tr("Title should be not empty");
    s.titleTooltipsSet.forbidden = tr("Title is alredy in use");
    s.titleTooltipsSet.notChecked = tr("Title is not checked yet");
    s.titleTooltipsSet.valid = tr("Title is valid");

    auto dial = new Dialogs::Templates::SaveEdit(this, s);
    connect(dial, &WebDialog::Accepted, this, [dial, id, title, comment, this]()
            {
              auto newTitle = dial->getTitle();
              auto newComment = dial->getComment();
              if(newTitle == title && newComment == comment)
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

  void MarkupVersionReferenceWidget::UpdateReferencesCountLabel()
  {
    QString text = tr("Results total: ") + QString::number(ModelEtalonList->rowCount());
    QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
    ReferencesCountLabel->setText(tmpl);
  }

  void MarkupVersionReferenceWidget::UpdateTitleAndCommentOnServer(int id, const QString& title, const QString& comment)
  {
    API::REST::UpdateReferenceData(id, title, comment,
      [&](QNetworkReply*, QJsonDocument)
      {
        ModelEtalonList->Reset();
        Notification::NotifySuccess(tr("Reference data saved."));
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Error update reference data."), err);
      });
  }

  void MarkupVersionReferenceWidget::DeleteReference(int id)
  {
    API::REST::DeleteReference(id,
      [&](QNetworkReply*, QJsonDocument)
      {
        ModelEtalonList->Reset();
        Notification::NotifySuccess(tr("Reference deleted."));
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Error delete reference."), err);
      });
  }
}
