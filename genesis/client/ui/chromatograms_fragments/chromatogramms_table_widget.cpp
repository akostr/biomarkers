#include "chromatogramms_table_widget.h"

#include <genesis_style/style.h>

#include <ui/itemviews/files_table_actions_column_delegate.h>
#include <ui/itemviews/datatable_item_delegate.h>
#include <ui/itemviews/tableview_headerview.h>

#include "ui/dialogs/web_dialog_markup_creation.h"
#include <ui/dialogs/web_dialog_group_editor.h>
#include <ui/dialogs/web_dialog_files_group_rename.h>
#include <ui/dialogs/web_dialog_ions_fragmentation.h>
#include <ui/dialogs/templates/tree_confirmation.h>
#include "ui/dialogs/table_rename_dialog.h"
#include <logic/tree_model_dynamic_project_chromatogramms.h>
#include <logic/tree_model_item.h>
#include <logic/context_root.h>
#include <logic/notification.h>
#include "logic/known_json_tag_names.h"
#include <logic/known_context_tag_names.h>
#include "ui/known_view_names.h"
#include <ui/genesis_window.h>

#include <api/api_rest.h>

#include <QVBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QCheckBox>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QScrollBar>

#include <ui/dialogs/chromatogram_passport_dialog.h>

using ModelClass = TreeModelDynamicProjectChromatogramms;

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

ChromatogrammsTableWidget::ChromatogrammsTableWidget(QWidget *parent)
  : QWidget(parent)
{
  SetupUi();
}

void ChromatogrammsTableWidget::Reload()
{
  m_model->Reset();
}

void ChromatogrammsTableWidget::setScrollPosition(int pos)
{
  m_scrollbarPosition = pos;
}

void ChromatogrammsTableWidget::SetupUi()
{
  auto layout = new QVBoxLayout();
  setLayout(layout);

  auto toolBarLayout = new QHBoxLayout();
  layout->addLayout(toolBarLayout);

  toolBarLayout->setDirection(QHBoxLayout::Direction::RightToLeft);

  m_actionsMenuButton = new QPushButton(tr("Actions with chromatogramms"));
  m_actionsMenuButton->setProperty("menu_secondary", true);
  toolBarLayout->addWidget(m_actionsMenuButton);

  {//// Chromatogramm group actions setup
    auto actionsMenu = new QMenu(m_actionsMenuButton);
    auto collapseExpandAction = actionsMenu->addAction(tr("Collapse/Expand all"));
    collapseExpandAction->setShortcut(QKeySequence(QKeyCombination(Qt::ControlModifier, Qt::Key_W)));
    auto markupAction = actionsMenu->addAction(tr("Markup"));
    auto passportAction = actionsMenu->addAction(tr("Passport"));
    auto extractAction = actionsMenu->addAction(tr("Extract fragments ions"));
    auto groupingAction = actionsMenu->addAction(tr("Change grouping"));
    // auto copyAction = actionsMenu->addAction(tr("Copy to another project"));
    auto exportAction = actionsMenu->addAction(tr("Export"));
    auto removeAction = actionsMenu->addAction(tr("Delete"));

    m_actionsMenuButton->setMenu(actionsMenu);

    connect(collapseExpandAction, &QAction::triggered, this, &ChromatogrammsTableWidget::toggleCollapsing);
    connect(markupAction, &QAction::triggered, this, &ChromatogrammsTableWidget::PickedMarkupGroupAction);
    connect(extractAction, &QAction::triggered, this, &ChromatogrammsTableWidget::PickedIonsExtractGroupAction);
    connect(groupingAction, &QAction::triggered, this, &ChromatogrammsTableWidget::PickedChangeGroupingAction);
    // connect(copyAction, &QAction::triggered, this, &ChromatogrammsTableWidget::PickedCopyToOtherProjectGroupAction);
    connect(exportAction, &QAction::triggered, this, &ChromatogrammsTableWidget::PickedExportGroupAction);
    connect(removeAction, &QAction::triggered, this, &ChromatogrammsTableWidget::PickedRemoveGroupAction);
    connect(passportAction, &QAction::triggered, this, &ChromatogrammsTableWidget::PickedPassportsGroupAction);
  }

  {//Actions buttons:
    m_actionButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto button = m_actionButtons->button(QDialogButtonBox::Ok);
    button->setFocusPolicy(Qt::NoFocus);
    button->setText("Default ok button");
    connect(m_actionButtons, &QDialogButtonBox::rejected, this, &ChromatogrammsTableWidget::ExitAction);
    m_actionButtons->setVisible(false);
    toolBarLayout->addWidget(m_actionButtons);
  }

  toolBarLayout->addStretch();
  m_pickedChromatogrammsCountLabel = new QLabel();
  toolBarLayout->addWidget(m_pickedChromatogrammsCountLabel);
  m_pickedChromatogrammsCountLabel->setVisible(false);

  m_treeView = new QTreeView();
  m_treeView->setStyleSheet(Style::Genesis::GetTreeViewStyle());
  m_treeView->setProperty("dataDrivenColors", true);
  m_treeView->setHeader(new TableViewHeaderView(Qt::Horizontal, m_treeView));
  m_treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_treeView->header()->setStretchLastSection(true);
  layout->addWidget(m_treeView);

  m_model = new ModelClass(m_treeView);

  {//pick all checkbox
    m_pickAllCheckbox = new QCheckBox("");
    m_pickAllCheckbox->setTristate(true);
    m_pickAllCheckbox->setCheckState(Qt::Unchecked);
    m_pickAllCheckbox->setProperty("invalidation_to_model_in_progress", false);
    toolBarLayout->addWidget(m_pickAllCheckbox);
    auto invalidate = [this](bool toModel)
    {
      if(toModel)
      {
        m_pickAllCheckbox->setProperty("invalidation_to_model_in_progress", true);
        m_model->SetChildrenCheckState(m_pickAllCheckbox->checkState(), m_model->GetRoot());
        m_pickAllCheckbox->setProperty("invalidation_to_model_in_progress", false);
      }
      else
      {
        auto sb = QSignalBlocker(m_pickAllCheckbox);
        auto checkedSize = m_model->GetCheckedItems().size();
        auto checkableCount = m_model->GetCheckableCount();
        if(checkableCount == 0)
          m_pickAllCheckbox->setCheckState(Qt::Unchecked);
        else if(checkedSize < checkableCount)
        {
          if(checkedSize == 0)
            m_pickAllCheckbox->setCheckState(Qt::Unchecked);
          else
            m_pickAllCheckbox->setCheckState(Qt::PartiallyChecked);
        }
        else
          m_pickAllCheckbox->setCheckState(Qt::Checked);
      }
    };
    connect(m_model, &ModelClass::dataChanged, this,
    [this, invalidate](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>())
    {
      if(m_pickAllCheckbox->property("invalidation_to_model_in_progress").toBool())
        return;
      if(roles.contains(Qt::CheckStateRole))
        invalidate(false);
    });
    connect(m_model, &ModelClass::modelReset, this,
    [this, invalidate]()
    {
      if(m_pickAllCheckbox->property("invalidation_to_model_in_progress").toBool())
        return;
      invalidate(false);
    });

    connect(m_pickAllCheckbox, &QCheckBox::stateChanged, this,
    [this, invalidate](int state)
    {
      if(state == Qt::PartiallyChecked)
      {
        auto sb = QSignalBlocker(m_pickAllCheckbox);
        m_pickAllCheckbox->setCheckState(Qt::Checked);
      }
      invalidate(true);
    });
    m_pickAllCheckbox->setVisible(false);
  }

  m_sortProxyModel = new QSortFilterProxyModel(m_model);
  m_sortProxyModel->setSourceModel(m_model);
  m_treeView->setModel(m_sortProxyModel);
//  TreeView->setModel(m_model);
  m_treeView->setSortingEnabled(true);
  auto d = m_treeView->itemDelegate();
  m_treeView->setItemDelegate(new DataTableItemDelegate(m_treeView));
  d->deleteLater();
  m_treeView->setItemDelegateForColumn(ModelClass::ColumnLast, new FilesTableActionsColumnDelegate(m_treeView.get()));
  m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_treeView, &QTreeView::customContextMenuRequested, this,
    [&](const QPoint& pos)
    {
      auto model = m_treeView->selectionModel();
      auto index = m_treeView->currentIndex();
      m_treeView->clicked(m_sortProxyModel->index(index.row(), ModelClass::ColumnLast, index.parent()));
    });

  connect(m_treeView, &QTreeView::doubleClicked, this, [&](const QModelIndex& index)
    {
      ItemViewSamplePassport(m_model->GetItem(m_sortProxyModel->mapToSource( index)));
    });

  addMemoryScrollbarPosition();

  m_chromatogrammsCountLabel = new QLabel();
  layout->addWidget(m_chromatogrammsCountLabel);
  connect(m_chromatogrammsCountLabel, &QLabel::linkActivated, this, [this]()
  {
    updateScrollPosition();
    m_model->Reset();
  });

  connect(m_model, &ModelClass::rowsInserted, this, &ChromatogrammsTableWidget::UpdateChromatogrammCountLabel);
  connect(m_model, &ModelClass::rowsRemoved, this, &ChromatogrammsTableWidget::UpdateChromatogrammCountLabel);

  connect(m_model, &ModelClass::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
  {
      if (m_pickedChromatogrammsCountLabel->isVisible() && roles.contains(Qt::CheckStateRole))
        UpdatePickedChromatogramsCountLabel();
  });
  connect(m_model, &ModelClass::modelReset, this, [this]()
  {
    UpdateChromatogrammCountLabel();
    UpdatePickedChromatogramsCountLabel();
    m_treeView->expandAll();
    mCollapsed = false;
  });

  connect(m_model, &ModelClass::ActionTriggered, this, [this](const QString& actionId, TreeModelItem* item)
  {
    if(!ModelClass::isItemIsGroup(item))
    {//item is not group-item actions
      if(actionId == "extract_ions")
        ItemExtractIons(item);
      else if(actionId == "change_grouping")
        ItemChangeGrouping(item);
      else if(actionId == "copy_to_other_project")
        ItemCopyToOtherProject(item);
      else if(actionId == "view_sample_passport")
        ItemViewSamplePassport(item);
      else if(actionId == "export")
        ItemExport(item);
      else if(actionId == "delete")
        ItemDelete(item);
    }
    else
    {//item is group-item actions
      if (actionId == "delete_group")
        ItemGroupDelete(item);
      else if (actionId == "rename_group")
        ItemGroupRename(item);
      else if (actionId == "export")
        ItemGroupExport(item);
      else if (actionId == "edit_group")
        ItemGroupEdit(item);
    }
  });
}

void ChromatogrammsTableWidget::toggleCollapsing()
{
  if(mCollapsed)
  {
    m_treeView->expandAll();
    mCollapsed = false;
  }
  else
  {
    m_treeView->collapseAll();
    mCollapsed = true;
  }
}

void ChromatogrammsTableWidget::UpdateChromatogrammCountLabel()
{
  QString text = tr("Results total: ") + QString::number(m_model->NonGroupItemsCount());
  QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
  m_chromatogrammsCountLabel->setText(tmpl);
}

void ChromatogrammsTableWidget::UpdatePickedChromatogramsCountLabel()
{
  m_pickedChromatogrammsCountLabel->setText(tr("Picked %n chromatogramms ", "", m_model->GetCheckedItems().size()) + m_forWhatActionText);
}

void ChromatogrammsTableWidget::PickedMarkupGroupAction()
{
  ActionAssignAndEnter(tr("Markup"), tr("for markup"), [this]()
  {
    QList<int> checkedIds;
    {
      auto checkedItems = m_model->GetCheckedItems(m_model->GetRoot());
      for(auto& item : checkedItems)
        checkedIds << item->GetData("project_file_id").toInt();
    }
    if(checkedIds.isEmpty())
    {
      Notification::NotifyError(tr("To create markup, pick one or more chromatogramms in table"));
      return;
    }
    auto dial = new Dialogs::WebDialogMarkupCreation(checkedIds, this);
    dial->Open();
    connect(dial, &WebDialog::Accepted, this, [this, dial]()
    {
      auto markupName = dial->GetMarkupName();
      auto comment = dial->GetComment();
      auto filesIds = dial->GetFilesIds();
      int projectId = GenesisContextRoot::Get()->ProjectId();
      API::REST::AddMarkupWithFiles(projectId, markupName, comment, filesIds,
      [this, markupName](QNetworkReply* r, QJsonDocument doc)
      {
        bool error = doc.object()["error"].toBool();
        if (error)
        {
          QString msg = doc.object()["msg"].toString();
          Notification::NotifyError(tr("Error create markup: ") + msg);
        }
        else
        {
          ExitAction();
          Notification::NotifySuccess(tr("Markup created"));
          auto root = doc.object();
          auto markupId = doc["indexing_id"].toInt();
          auto versionId = doc["version_id"].toInt();
          TreeModelDynamic::ResetInstances("TreeModelDynamicProjectMarkupsList");
          if (auto contextRoot = GenesisContextRoot::Get())
          {
            if (auto contextProject = contextRoot->GetContextProject())
            {
              if (auto markupContext = contextRoot->GetContextMarkup())
              {
                markupContext->SetData(Names::MarkupContextTags::kMarkupId, markupId);
                markupContext->SetData(Names::MarkupContextTags::kMarkupTitle, markupName);
              }
              if (auto markupVersionContext = contextRoot->GetContextMarkupVersion())
              {
                markupVersionContext->SetData(Names::MarkupVersionContextTags::kMarkupVersionId, versionId);
              }
              GenesisWindow::Get()->ShowPage(ViewProjectMarkupPageName);
            }
          }
        }
      },
      [](QNetworkReply* r, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Error create markup: "), err);
      });
//      API::REST:: AddFilesToGroup(GenesisContextRoot::Get()->ProjectId(),
//                                  checkedIds,
//                                  groupInfo.first,
//                                  groupInfo.second,
//                                  [this, checkedIds](QNetworkReply* r, QJsonDocument doc)
//      {
//        Model->Reset();
//        ExitAction();
//        Notification::Notify("chromatogramms/files_groups", Notification::StatusSuccess, tr("%n Chromatogramms successfully changed their grouping", "", checkedIds.size()));
//      },
//      [](QNetworkReply* r, QNetworkReply::NetworkError err)
//      {
//        Notification::Notify("chromatogramms/files_groups", Notification::StatusError, tr("Failed to add file to group"), err);
//      });
    });
  });
}

void ChromatogrammsTableWidget::PickedIonsExtractGroupAction()
{
  ActionAssignAndEnter(tr("Extract ions"), tr("for ions extraction"), [this]()
  {
    QList<int> checkedFilesIds;
    {
      auto checkedItems = m_model->GetCheckedItems(m_model->GetRoot());
      for(auto& item : checkedItems)
        checkedFilesIds << item->GetData("project_file_id").toInt();
    }
    if(checkedFilesIds.isEmpty())
    {
      Notification::NotifyError(tr("For extraction pick one or more GS-MS chromatogramms in table"));
      return;
    }
    auto projectId = GenesisContextRoot::Get()->ProjectId();
    API::REST::GetIons(projectId, checkedFilesIds,
    [this, checkedFilesIds](QNetworkReply*, QJsonDocument doc)
    {
      auto jAllIons = doc.object()["ions"].toObject()["all"].toArray();
      QList<int> availableIons;

      for(auto ion : jAllIons)
        availableIons << ion.toInt();

      auto ionsFragmentationDial = new Dialogs::WebDialogIonsFragmentation(this);
      ionsFragmentationDial->setIons(availableIons);
      connect(ionsFragmentationDial, &Dialogs::WebDialogIonsFragmentation::Accepted, this, [this, checkedFilesIds, ionsFragmentationDial]()
      {
        auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Ions extraction"));
        auto projectId = GenesisContextRoot::Get()->ProjectId();
        API::REST::ExtractIons(projectId, checkedFilesIds, ionsFragmentationDial->getSum(), ionsFragmentationDial->getIons(),
        [this, overlayId](QNetworkReply*, QJsonDocument doc)
        {
          if(doc.object()["error"].toBool())
          {
            Notification::NotifyError(doc.object()["msg"].toString(), tr("Failed to extract ions"));
          }

          ExitAction();
          TreeModel::ResetInstances("TreeModelDynamicProjectChromatogramms");
          TreeModel::ResetInstances("TreeModelDynamicProjectFragments");
          GenesisWindow::Get()->RemoveOverlay(overlayId);
        },
        [overlayId](QNetworkReply*, QNetworkReply::NetworkError e)
        {
          Notification::NotifyError("Failed to extract ions", e);
          GenesisWindow::Get()->RemoveOverlay(overlayId);
        });
      });
      ionsFragmentationDial->Open();
    },
    [](QNetworkReply*, QNetworkReply::NetworkError)
    {

    });

  },
  [this]()
  {
    m_model->setCheckableOnlyWithFragments(true);
  },
  [this]()
  {
    m_model->setCheckableOnlyWithFragments(false);
  });
}

void ChromatogrammsTableWidget::PickedChangeGroupingAction()
{
  ActionAssignAndEnter(tr("Change grouping"), tr("for grouping change"), [this]()
    {
      const auto projectId = GenesisContextRoot::Get()->ProjectId();
      API::REST::GetFileGroups(projectId, [this](QNetworkReply* r, QJsonDocument doc)
        {
          QList<int> checkedIds;
          {
            auto checkedItems = m_model->GetCheckedItems(m_model->GetRoot());
            for (auto& item : checkedItems)
              checkedIds << item->GetData("project_file_id").toInt();
          }
          if (checkedIds.isEmpty())
          {
            Notification::NotifyError(tr("For grouping changing pick one or more chromatogramms in table"));
            return;
          }
          std::map<QString, int> groups;
          auto jdata = doc.object()["data"].toArray();
          for (int i = 0; i < jdata.size(); i++)
          {
            auto jitem = jdata[i].toArray();
            groups.emplace(jitem[1].toString(), jitem[0].toInt());
          }
          ShowDialogForGrouping(checkedIds, groups);
          ExitAction();
        },
        [&](QNetworkReply* r, QNetworkReply::NetworkError err)
        {
          ExitAction();
          Notification::NotifyError(tr("Failed to load file groups information"), err);
        });
    });
}

void ChromatogrammsTableWidget::PickedCopyToOtherProjectGroupAction()
{
  ActionAssignAndEnter(tr("Copy to other project"), tr("for copying to other project"), []()
  {
    qInfo() << "copy to other project";
  });
}

void ChromatogrammsTableWidget::PickedExportGroupAction()
{
  ActionAssignAndEnter(tr("Export"), tr("for exporting"), []()
  {
    qInfo() << "export";
  });
}

void ChromatogrammsTableWidget::PickedRemoveGroupAction()
{
  ActionAssignAndEnter(tr("Remove"), tr("for removing"), [this]()
  {
    QList<QPair<int, QString>> checkedFilesInfos;
    {
      auto checkedItems = m_model->GetCheckedItems(m_model->GetRoot());
      for(auto& item : checkedItems)
        checkedFilesInfos << QPair<int, QString>(item->GetData("project_file_id").toInt(),
                       item->GetData("filename").toString()
                      );
    }
    if(checkedFilesInfos.isEmpty())
    {
      Notification::NotifyError(tr("For grouping changing pick one or more chromatogramms in table"));
      return;
    }

    auto dial = Dialogs::Templates::TreeConfirmation::DeleteMarkupFilesWithDependencies(checkedFilesInfos, this);
    connect(dial, &WebDialog::Accepted, this, [this]()
    {
      ExitAction();
    });
  });
}

void ChromatogrammsTableWidget::PickedPassportsGroupAction()
{
  ActionAssignAndEnter(tr("Change passport"), tr("for passport editing"), [this]()
  {
    QList<int> checkedFilesIds;
    {
      auto checkedItems = m_model->GetCheckedItems(m_model->GetRoot());
      for(auto& item : checkedItems)
        checkedFilesIds << item->GetData("id").toInt();
    }
    if(checkedFilesIds.isEmpty())
    {
      Notification::NotifyError(tr("For grouping changing pick one or more chromatogramms in table"));
      return;
    }
    activatePassportsDialogAction(checkedFilesIds, true);
  });
}

void ChromatogrammsTableWidget::ActionAssignAndEnter(QString buttonText, QString forWhatAction,
                                                     std::function<void ()> func,
                                                     std::function<void ()> prepare,
                                                     std::function<void ()> done)
{
  prepare();
  m_doneAction = done;
  auto okBtn = m_actionButtons->button(QDialogButtonBox::Ok);
  okBtn->setText(buttonText);
  if(m_actionConnection)
    disconnect(m_actionConnection);
  m_actionConnection = connect(m_actionButtons, &QDialogButtonBox::accepted, this, func);
  m_actionButtons->setVisible(true);
  m_pickAllCheckbox->setVisible(true);
  m_actionsMenuButton->setVisible(false);
  m_pickedChromatogrammsCountLabel->setVisible(true);
  m_forWhatActionText = forWhatAction;
  UpdatePickedChromatogramsCountLabel();

  m_model->SetCheckable(true);
  m_treeView->update();
  m_treeView->resizeColumnToContents(0);
}

void ChromatogrammsTableWidget::ExitAction()
{
  if(m_actionConnection)
    disconnect(m_actionConnection);
  m_actionButtons->setVisible(false);
  m_actionsMenuButton->setVisible(true);
  m_pickAllCheckbox->setVisible(false);
  m_pickedChromatogrammsCountLabel->setVisible(false);
  m_forWhatActionText.clear();

  m_model->SetChildrenCheckState(Qt::Unchecked, m_model->GetRoot());
  m_model->SetCheckable(false);
  if(m_doneAction)
  {
    m_doneAction();
    m_doneAction = [](){};
  }
//  TreeView->header()->resizeSections(QHeaderView::ResizeToContents);
//  TreeView->update();
}

void ChromatogrammsTableWidget::ItemExtractIons(TreeModelItem *item)
{
  auto projectFileId = item->GetData("project_file_id").toInt();
  auto projectId = GenesisContextRoot::Get()->ProjectId();
  API::REST::GetIons(projectId, {projectFileId},
  [this, projectFileId](QNetworkReply*, QJsonDocument doc)
  {
    auto jAllIons = doc.object()["ions"].toObject()["all"].toArray();
    QList<int> availableIons;

    for(auto ion : jAllIons)
      availableIons << ion.toInt();

    auto ionsFragmentationDial = new Dialogs::WebDialogIonsFragmentation(this);
    ionsFragmentationDial->setIons(availableIons);
    connect(ionsFragmentationDial, &Dialogs::WebDialogIonsFragmentation::Accepted, this, [projectFileId, ionsFragmentationDial]()
    {
      auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Ions extraction"));
      auto projectId = GenesisContextRoot::Get()->ProjectId();
      API::REST::ExtractIons(projectId, {projectFileId}, ionsFragmentationDial->getSum(), ionsFragmentationDial->getIons(),
                             [overlayId](QNetworkReply*, QJsonDocument doc)
      {
        if(doc.object()["error"].toBool())
        {
          Notification::NotifyError(doc.object()["msg"].toString(), tr("Failed to extract ions"));
        }
        TreeModel::ResetInstances("TreeModelDynamicProjectChromatogramms");
        TreeModel::ResetInstances("TreeModelDynamicProjectFragments");
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      },
      [overlayId](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError("Failed to extract ions", e);
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      });
    });
    ionsFragmentationDial->Open();
  },
  [](QNetworkReply* r, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Failed to ungroup file"), err);
  });
}

void ChromatogrammsTableWidget::ItemChangeGrouping(TreeModelItem* item)
{
  const auto projectId = GenesisContextRoot::Get()->ProjectId();
  API::REST::GetFileGroups(projectId, [this, item](QNetworkReply* r, QJsonDocument doc)
    {
      std::map<QString, int> groups;
      auto jdata = doc.object()["data"].toArray();
      for (int i = 0; i < jdata.size(); i++)
      {
        auto jitem = jdata[i].toArray();
        groups.emplace(jitem[1].toString(), jitem[0].toInt());
      }
      ShowDialogForGrouping({ item->GetData("project_file_id").toInt() }, groups);
    },
    [](QNetworkReply* r, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to load file groups information"), err);
    });
}

void ChromatogrammsTableWidget::ItemCopyToOtherProject(TreeModelItem *item)
{
  ///@todo: nothing here yet
}

void ChromatogrammsTableWidget::ItemViewSamplePassport(TreeModelItem *item)
{
  if(int fileId = item->GetData("id").toInt())
  {
    activatePassportsDialogAction({fileId});
  }
}

void ChromatogrammsTableWidget::ItemExport(TreeModelItem *item)
{
  ///@todo: nothing here yet
}

void ChromatogrammsTableWidget::ItemDelete(TreeModelItem *item)
{
  QPair<int, QString> fileInfo = {
    item->GetData("project_file_id").toInt(),
    item->GetData("filename").toString()
  };
  auto dial = Dialogs::Templates::TreeConfirmation::DeleteMarkupFilesWithDependencies({fileInfo}, this);
//  connect(dial, &WebDialog::Finished, this, []()
//  {

//  });

//  auto dial = new WebDialogFileRemovingConfirmation({fileInfo}, this);
//  connect(dial, &WebDialog::Accepted, this, [fileInfo]()
//  {
//    API::REST::RemoveFileAndDependenciesFromProject(fileInfo.first,
//    [](QNetworkReply* r, QJsonDocument doc)
//    {
//      TreeModel::ResetInstances("TreeModelDynamicProjectChromatogramms");
//      TreeModel::ResetInstances("TreeModelDynamicProjectFragments");
//    },
//    [](QNetworkReply* r, QNetworkReply::NetworkError err)
//    {

//    });
//  });
//  dial->Open();
}

void ChromatogrammsTableWidget::ItemGroupDelete(TreeModelItem *item)
{
  if(!ModelClass::isItemIsGroup(item))
  {
    Q_ASSERT(false); //can't apply this to non-group item
    return;
  }
  auto dial = WebDialog::Question(tr("Are you shure to delete group ") + item->GetData("project_file_id").toString() + " ?", this);
  connect(dial, &WebDialog::Accepted, this, [item, this]()
  {
    API::REST::DeleteGroup(GenesisContextRoot::Get()->ProjectId(),
                            item->GetData("group_id").toInt(),
                            [this](QNetworkReply* r, QJsonDocument doc)
    {
      m_model->Reset();
    },
    [](QNetworkReply* r, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to delete group"), err);
    });
  });
  dial->Open();
}

void ChromatogrammsTableWidget::ItemGroupRename(TreeModelItem *item)
{
  if(!ModelClass::isItemIsGroup(item))
    return;
  auto dial = new TableRenameDialog(this, item->GetData("project_file_id").toString());
  dial->SetLineEditCaption(tr("Group name"));
  connect(dial, &WebDialog::Accepted, this, [item, dial, this]()
  {
    API::REST::RenameGroup(GenesisContextRoot::Get()->ProjectId(),
                           item->GetData("group_id").toInt(),
                           dial->getTitle(),
                           [this](QNetworkReply* r, QJsonDocument doc)
    {
      m_model->Reset();
    },
    [](QNetworkReply* r, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to delete group"), err);
    });
  });
  dial->Open();
}

void ChromatogrammsTableWidget::ItemGroupExport(TreeModelItem *item)
{
  ///@todo: nothing here yet
}

void ChromatogrammsTableWidget::ItemGroupEdit(TreeModelItem* item)
{
  const auto projectId = GenesisContextRoot::Get()->ProjectId();
  API::REST::GetFileGroups(projectId, [this, item](QNetworkReply* r, QJsonDocument doc)
    {
      std::map<QString, int> groups;
      auto jdata = doc.object()["data"].toArray();
      for (int i = 0; i < jdata.size(); i++)
      {
        auto jitem = jdata[i].toArray();
        groups.emplace(jitem[1].toString(), jitem[0].toInt());
      }
      const auto childs = item->GetChildren();
      QList<int> ids;
      for (const auto& child : childs)
        ids.append(child->GetData("project_file_id").toInt());
      ShowDialogForGrouping(ids, groups);
    },
    [](QNetworkReply* r, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to load file groups information"), err);
    });
}

void ChromatogrammsTableWidget::activatePassportsDialogAction(QList<int> filesIds, bool isInGroupAction)
{
  auto loadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  API::REST::GetPassports(filesIds,
    [this, loadingOverlayId, isInGroupAction](QNetworkReply*, QJsonDocument doc)
    {
      if(doc.object()["error"].toBool())
      {
        Notification::NotifyError(doc.object()["msg"].toString(), tr("Failed to load passport"));
        GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        return;
      }
      // qDebug().noquote() << doc.toJson();
      auto dial = new Dialogs::ChromatogramPassportDialog(false, doc.object(), this);
      connect(dial, &WebDialog::Accepted, this, [&, dial]()
      {
        auto uploadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Uploading passport"));
        //qDebug() << dial->toJson();
        QJsonObject obj;
        obj.insert(JsonTagNames::ChromatogrammData, dial->toJson());
        obj.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
        API::REST::SavePassportV2(obj,
          [&, uploadingOverlayId](QNetworkReply*, QJsonDocument doc)
          {
            if(doc.object()["error"].toBool())
            {
              Notification::NotifyError(doc.object()["msg"].toString(), tr("Failed to upload passport"));
              GenesisWindow::Get()->RemoveOverlay(uploadingOverlayId);
              return;
            }
            Notification::NotifySuccess(tr("Passport uploaded"));
            GenesisWindow::Get()->RemoveOverlay(uploadingOverlayId);
            updateScrollPosition();
            TreeModel::ResetInstances("TreeModelDynamicProjectChromatogramms");
          },
          [uploadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
          {
            Notification::NotifyError("Server error", e);
            GenesisWindow::Get()->RemoveOverlay(uploadingOverlayId);

          });
      });
      dial->Open();
      GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
      if(isInGroupAction)
        ExitAction();
    },
    [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      Notification::NotifyError("Server error", e);
      GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
    }
  );
}

void ChromatogrammsTableWidget::addMemoryScrollbarPosition()
{
  auto test = [this](int min, int max)
    {
      if (min == max)
        return;
      if(m_scrollbarPosition)
        m_treeView->verticalScrollBar()->setValue(m_scrollbarPosition);
    };
  connect(m_treeView->verticalScrollBar(), &QScrollBar::rangeChanged, this, test);
}

void ChromatogrammsTableWidget::updateScrollPosition()
{
  m_scrollbarPosition = m_treeView->verticalScrollBar()->value();
}

void ChromatogrammsTableWidget::ShowDialogForGrouping(const QList<int>& ids, const std::map<QString, int>& groupInfo)
{
  auto dial = new Dialogs::WebDialogGroupEditor(this);
  dial->SetCaptionLabel(tr("Move chosen chromatogramms"));
  dial->SetUngroupRadioButtonText(tr("To chromatogramms list"));
  dial->SetGroupRadioButtonText(tr("To chromatogramms group"));
  dial->SetGroupInfo(groupInfo);

  connect(dial, &WebDialog::Accepted, this, [this, dial, ids]()
    {
      if (dial->IsAddToGroup())
      {
        auto groupInfo = dial->GetGroupInfo();
        API::REST::AddFilesToGroup(GenesisContextRoot::Get()->ProjectId(),
          ids,
          groupInfo.first,
          groupInfo.second,
          [this, ids](QNetworkReply* r, QJsonDocument doc)
          {
            m_model->Reset();
            Notification::NotifySuccess(tr("%n Chromatogramms successfully changed their grouping", "", ids.size()));
          },
          [](QNetworkReply* r, QNetworkReply::NetworkError err)
          {
            Notification::NotifyError(tr("Failed to add file to group"), err);
          });
      }
      else
      {
        API::REST::UngroupFiles(GenesisContextRoot::Get()->ProjectId(),
          ids,
          [this, ids](QNetworkReply* r, QJsonDocument doc)
          {
            m_model->Reset();
            Notification::NotifySuccess(tr("%n Chromatogramms successfully changed their grouping", "", ids.size()));
          },
          [](QNetworkReply* r, QNetworkReply::NetworkError err)
          {
            Notification::NotifyError(tr("Failed to ungroup file"), err);
          });
      }
    });

  dial->Open();
}
