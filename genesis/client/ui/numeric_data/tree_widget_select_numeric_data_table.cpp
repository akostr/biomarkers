#include "tree_widget_select_numeric_data_table.h"
#include "logic/tree_model_presentation.h"
#include "logic/tree_model_dynamic_numeric_tables.h"
#include <genesis_style/style.h>
#include <ui/genesis_window.h>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <ui/numeric_data/join_numeric_dialog.h>
#include <QDialogButtonBox>
#include <logic/notification.h>
#include <QHeaderView>
#include <api/api_rest.h>
#include <QFileDialog>
#include <QTimer>
#include <QRegExp>
#include <logic/known_context_tag_names.h>
#include <logic/tree_model_item.h>
#include <ui/known_view_names.h>
#include <logic/context_root.h>
#include <ui/itemviews/tableview_headerview.h>
#include <ui/itemviews/tableview_model_actions_column_delegate.h>
#include <ui/itemviews/datatable_item_delegate.h>
#include <ui/dialogs/templates/save_edit.h>
#include <ui/dialogs/templates/tree_confirmation.h>
#include "ui/dialogs/web_dialog_calculate_coefficient_table.h"

using ModelClass = TreeModelDynamicNumericTables;
using namespace Core;
using namespace Names;
using namespace ViewPageNames;

TreeWidgetSelectNumericDataTable::TreeWidgetSelectNumericDataTable(QWidget* parent)
  :QWidget(parent)
{
  SetupUi();
  ConnectActions();
}

void TreeWidgetSelectNumericDataTable::SetupUi()
{
  setStyleSheet(Style::Genesis::GetUiStyle());
  mLayout = new QVBoxLayout(this);

  m_pickedCountLabel = new QLabel();
  m_pickedCountLabel->setVisible(false);

  //  "_id"
  //  "_title"
  //  "_username"
  //  "_table_type"
  //  "_uploaded"
  //  "_sample_id"
  //  "_sample_title"
  //  "_chromatograms_count"
  //  "_is_identified"

  // add sync
  mModel = new TreeModelDynamicNumericTables(Names::Group::DataTable, this);
  mTreeView = new QTreeView(this);

  mTreeView->setStyleSheet(Style::Genesis::GetTreeViewStyle());
  mTreeView->setProperty("dataDrivenColors", true);
  mTreeView->setHeader(new TableViewHeaderView(Qt::Horizontal, mTreeView));
  mTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  mTreeView->header()->setStretchLastSection(true);
  mPresentationModel = mModel->GetPresentationModel();
  mTreeView->setModel(mPresentationModel);
  mTreeView->setSortingEnabled(true);

  m_ActionMenuButton = new QPushButton(tr("Actions with tables"), this);
  m_ActionMenuButton->setProperty("menu_secondary", true);
  auto actionsMenu = new QMenu(m_ActionMenuButton);
  m_JoinAction = actionsMenu->addAction(tr("Join included tables"));
  m_ExportAction = actionsMenu->addAction(tr("Export"));
  m_RemoveAction = actionsMenu->addAction(tr("Remove"));

  auto d = mTreeView->itemDelegate();
  mTreeView->setItemDelegate(new DataTableItemDelegate(mTreeView));
  d->deleteLater();
  mTreeView->setItemDelegateForColumn(ModelClass::ColumnLast, new TableViewModelActionsColumnDelegate(mTreeView));
  mTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

  auto internalMenuLayout = new QHBoxLayout();

  mActionButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  auto button = mActionButtons->button(QDialogButtonBox::Ok);
  button->setFocusPolicy(Qt::NoFocus);

  mActionButtons->setVisible(false);
  internalMenuLayout->setDirection(QHBoxLayout::Direction::RightToLeft);
  internalMenuLayout->addWidget(m_ActionMenuButton);
  internalMenuLayout->addWidget(mActionButtons);

  internalMenuLayout->addStretch();
  internalMenuLayout->addWidget(m_pickedCountLabel);

  {//pick all checkbox
    m_pickAllCheckbox = new QCheckBox("");
    m_pickAllCheckbox->setTristate(true);
    m_pickAllCheckbox->setCheckState(Qt::Unchecked);
    m_pickAllCheckbox->setProperty("invalidation_to_model_in_progress", false);
    internalMenuLayout->addWidget(m_pickAllCheckbox);
    m_pickAllCheckbox->setVisible(false);
  }

  m_ActionMenuButton->setVisible(true);
  m_ActionMenuButton->setMenu(actionsMenu);

  mLayout->addLayout(internalMenuLayout);
  mLayout->addWidget(mTreeView);

  mCountLabel = new QLabel(this);


  mLayout->addWidget(mCountLabel);
  mTreeView->expandAll();
}

void TreeWidgetSelectNumericDataTable::ConnectActions()
{
  connect(mTreeView, &QTreeView::customContextMenuRequested, this, [&]()
    {
      const auto model = mTreeView->selectionModel();
      const auto index = model->currentIndex();
      const auto modelIndex = mPresentationModel->mapToSource(index);
      mTreeView->clicked(mModel->index(modelIndex.row(), ModelClass::ColumnLast, modelIndex.parent()));
    });

  connect(mCountLabel, &QLabel::linkActivated, this, [this]()
    {
      TreeModel::ResetInstances("TreeModelDynamicNumericTables");
      mModel->enterDefaultMode();
    });

  connect(mActionButtons, &QDialogButtonBox::rejected, [this]()
    {
      mModel->enterDefaultMode();
      HideMenuButtons(true);
    });

  connect(mActionButtons, &QDialogButtonBox::accepted, [this]()
    {
      if (mModel->mode() == ModelClass::joinMode)
      {
        auto list = mModel->getTableIdsWithName();
        if (list.size() < 2)
        {
          Notification::NotifyError(tr("Select two or more tables"));
          return;
        }
        auto joinDialog = new Dialogs::JoinNumericDialog(list, this);
        joinDialog->Open();
        connect(joinDialog, &WebDialog::Accepted, this, [this]()
          {
            HideMenuButtons(true);
            mModel->enterDefaultMode();
            refreshTimer = new QTimer(this);
            refreshTimer->setSingleShot(true);
            refreshTimer->setInterval(500);
            connect(refreshTimer, &QTimer::timeout, this, [this]()
              {
                TreeModel::ResetInstances("TreeModelDynamicNumericTables");
              }, Qt::SingleShotConnection);
            refreshTimer->start();
          });

        connect(joinDialog, &WebDialog::Rejected, this, [this]()
          {
            HideMenuButtons(true);
            mModel->enterDefaultMode();
            TreeModel::ResetInstances("TreeModelDynamicNumericTables");
          });
        return;
      }

      else if (mModel->mode() == ModelClass::exportMode)
      {
        auto list = mModel->getTableIdsWithName();
        if (list.isEmpty())
        {
          return;
        }
        auto idsList = mModel->getTableIds();
        Notification::NotifyInfo(tr("Starting process of tables export"), tr("Preparing export"));
        const auto projectId = GenesisContextRoot::Get()->ProjectId();
        API::REST::ExportDataTables(projectId, idsList,
          [this](QNetworkReply* reply, QByteArray buffer)
          {
            auto fileName = QString::fromUtf8(reply->rawHeader("file-title"));
            QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
            int pos = 0;
            while ((pos = rx.indexIn(fileName, pos)) != -1) {
              fileName.replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
            }
            fileName.replace("\"", "");

            auto fileExtension = QString::fromUtf8(reply->rawHeader("file-extension"));
            fileExtension.replace("\"", "");
            Notification::NotifySuccess(tr("Choose export path"), tr("Tables are ready for export"));
            fileName = QFileDialog::getSaveFileName(nullptr, tr("Choose export path"), fileName, QString("*.%1;;All files (*.*)").arg(fileExtension));

            if (!fileName.isEmpty())
            {
              QFile file(fileName);
              if (file.open(QFile::WriteOnly))
              {
                file.write(buffer);
                file.close();
              }
              else
              {
                Notification::NotifyError(tr("Can't open file: %1").arg(fileName));
              }
            }
          },
          [this](QNetworkReply*, QNetworkReply::NetworkError error)
          {
            Notification::NotifyError(tr("Export data tables error"), error);
          });
        HideMenuButtons(true);
        mModel->enterDefaultMode();
      }
      else if (mModel->mode() == ModelClass::removeMode)
      {
        auto list = mModel->getTableIdsWithName();
        if (list.isEmpty())
          return;
        Dialogs::Templates::TreeConfirmation::DeleteTablesWithDependencies(list, this, "TreeModelNumericData");
        HideMenuButtons(true);
        mModel->enterDefaultMode();
      }
    });
  connect(mModel, &ModelClass::dataChanged, this, &TreeWidgetSelectNumericDataTable::disableCheckableNotInParent);
  connect(mModel, &ModelClass::dataChanged, this,
    [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>())
    {
      if (m_pickAllCheckbox->property("invalidation_to_model_in_progress").toBool())
        return;
      if (roles.contains(Qt::CheckStateRole))
        Invalidate(false);
    });

  connect(mModel, &ModelClass::modelReset, this,
    [this]()
    {
      if (m_pickAllCheckbox->property("invalidation_to_model_in_progress").toBool())
        return;
      Invalidate(false);
    });

  connect(m_pickAllCheckbox, &QCheckBox::stateChanged, this,
    [this](int state)
    {
      if (state == Qt::PartiallyChecked)
      {
        auto sb = QSignalBlocker(m_pickAllCheckbox);
        m_pickAllCheckbox->setCheckState(Qt::Checked);
      }
      if (state == Qt::Unchecked)
      {
        mModel->SetItemsCheckable(true);
      }
      Invalidate(true);
    });

  connect(mCountLabel, &QLabel::linkActivated, this, [this]()
    {
      TreeModel::ResetInstances("TreeModelNumericData");
      mModel->enterDefaultMode();
      HideMenuButtons(true);
    });

  connect(mModel, &ModelClass::rowsInserted, this, &TreeWidgetSelectNumericDataTable::UpdateCountLabel);
  connect(mModel, &ModelClass::rowsRemoved, this, &TreeWidgetSelectNumericDataTable::UpdateCountLabel);

  connect(mModel, &ModelClass::dataChanged, this, &TreeWidgetSelectNumericDataTable::UpdatePickedCountLabel);

  connect(mModel, &ModelClass::modelReset, this, [this]()
    {
      UpdateCountLabel();
      UpdatePickedCountLabel();
      mTreeView->expandAll();
    });

  connect(m_JoinAction, &QAction::triggered, [this]()
    {
      mActionButtons->button(QDialogButtonBox::Ok)->setText(tr("Join tables"));
      HideMenuButtons(false);
      m_pickAllCheckbox->setDisabled(true);
      mModel->enterJoinMode();
    });

  connect(m_ExportAction, &QAction::triggered, [this]()
    {
      mActionButtons->button(QDialogButtonBox::Ok)->setText(tr("Export tables"));
      HideMenuButtons(false);
      m_pickAllCheckbox->setDisabled(false);
      mModel->enterExportMode();
    });

  connect(m_RemoveAction, &QAction::triggered, [this]()
    {
      mActionButtons->button(QDialogButtonBox::Ok)->setText(tr("Remove tables"));
      HideMenuButtons(false);
      m_pickAllCheckbox->setDisabled(false);
      mModel->enterRemoveMode();
    });

  connect(mTreeView, &QAbstractItemView::doubleClicked, [this](const QModelIndex& indexPresentation)
    {
      auto index = mModel->GetPresentationModel()->mapToSource(indexPresentation);
      TreeModelItem* item = static_cast<TreeModelItem*>(index.internalPointer());
      int id = item->GetData("_id").toInt();
      auto mode = mModel->mode();
      if (indexPresentation.column() == 0 && mode == ModelClass::ModelMode::viewDefault && item->IsEmpty())
      {
        OpenTable(id);
      }
    });

  connect(mModel, &TreeModel::ActionTriggered, [this](const QString& actionId, TreeModelItem* item)
    {
      QModelIndex index = item->GetIndex();
      int id = item->GetData("_id").toInt();
      QString title = item->GetData("_title").toString();
      QString comment = item->GetData("_comment").toString();
      if (actionId == "open")
      {
        //disabled for now
        OpenTable(id);
      }
      else if (actionId == "remove")
      {
        RemoveTable({ id, title });
      }
      else if (actionId == "edit")
      {
        EditTable(id, title, comment);
      }
      else if (actionId == "export")
      {
        Export({ id });
      }
      else if (actionId == "export_group")
      {
        QList<int> idsList;
        for (auto& child : item->GetChildren())
        {
          idsList.append(child->GetData("_id").toInt());
        }
        Export(idsList);
      }
    });
}

void TreeWidgetSelectNumericDataTable::UpdateCountLabel()
{
  QString text = tr("Results total: ") + QString::number(mModel->NonGroupItemsCount());
  QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
  mCountLabel->setText(tmpl);
}

void TreeWidgetSelectNumericDataTable::UpdatePickedCountLabel()
{
  m_pickedCountLabel->setText(tr("Picked %n tables ", "", mModel->GetCheckedItems().size()));
}

void TreeWidgetSelectNumericDataTable::Export(QList<int> idList)
{
  Notification::NotifyInfo(tr("Starting process of table export"), tr("Preparing export"));
  const auto projectId = GenesisContextRoot::Get()->ProjectId();
  API::REST::ExportDataTables(projectId, idList,
    [this](QNetworkReply* reply, QByteArray buffer)
    {
      auto fileName = QString::fromUtf8(reply->rawHeader("file-title"));
      QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
      int pos = 0;
      while ((pos = rx.indexIn(fileName, pos)) != -1) {
        fileName.replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
      }
      fileName.replace("\"", "");

      auto fileExtension = QString::fromUtf8(reply->rawHeader("file-extension"));
      fileExtension.replace("\"", "");

      Notification::NotifySuccess(tr("Choose export path"), tr("Table is ready for export"));
      fileName = QFileDialog::getSaveFileName(nullptr, tr("Choose export path"), fileName, QString("*.%1;;All files (*.*)").arg(fileExtension));

      if (!fileName.isEmpty())
      {
        QFile file(fileName);
        if (file.open(QFile::WriteOnly))
        {
          file.write(buffer);
          file.close();
        }
        else
        {
          Notification::NotifyError(tr("Can't open file: %1").arg(fileName));
        }
      }
    },
    [this](QNetworkReply*, QNetworkReply::NetworkError)
    {
      Notification::NotifyError(tr("Export data tables error"));
    });
}

void TreeWidgetSelectNumericDataTable::Invalidate(bool toModel)
{
  if (toModel)
  {
    m_pickAllCheckbox->setProperty("invalidation_to_model_in_progress", true);
    mModel->SetChildrenCheckState(m_pickAllCheckbox->checkState(), mModel->GetRoot());
    m_pickAllCheckbox->setProperty("invalidation_to_model_in_progress", false);
    return;
  }

  auto sb = QSignalBlocker(m_pickAllCheckbox);
  const auto checkedSize = mModel->GetCheckedItems().size();
  const auto checkableCount = mModel->GetCheckableCount();

  m_pickAllCheckbox->setCheckState(Qt::Checked);

  if (checkableCount == 0 || checkedSize == 0)
  {
    m_pickAllCheckbox->setCheckState(Qt::Unchecked);
  }
  else if (checkedSize < checkableCount)
  {
    m_pickAllCheckbox->setCheckState(Qt::PartiallyChecked);
  }
}

void TreeWidgetSelectNumericDataTable::OpenTable(int id)
{
  if (id < 1)
    return;

  if (auto contextRoot = GenesisContextRoot::Get())
  {
    const auto context = contextRoot->GetChildContextStepBasedMarkup();
    if (!context)
      return;
    // should always set qvariant map for erase selected checkboxes
    // when picked before
    context->SetData({ {ContextTagNames::ReferenceId, id} });
    GenesisWindow::Get()->ShowPage(ViewPageAnalysisDataTablePageName);
  }
}

void TreeWidgetSelectNumericDataTable::RemoveTable(const QPair<int, QString>& data)
{
  Dialogs::Templates::TreeConfirmation::DeleteTablesWithDependencies({ data }, this, "TreeModelHeightRatio");
}

void TreeWidgetSelectNumericDataTable::EditTable(int id, const QString& title, const QString& comment)
{
  if (id < 1)
    return;

  Dialogs::Templates::SaveEdit::Settings s;
  s.header = tr("Editing");
  s.titleHeader = tr("Table title");
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

      auto callback = [newTitle, id, title, this](QNetworkReply*, QJsonDocument doc)
        {

          auto root = doc.object();
          if (root["error"].toBool())
          {
            Notification::NotifyError(root["msg"].toString(), tr("Server error"));
            return;
          }
          if (newTitle != title)
          {
            Notification::NotifyHeaderless(tr("Table(%1) name is renamed from [%2] to [%3]")
              .arg(id)
              .arg(title)
              .arg(newTitle));
          }
          else
          {
            Notification::NotifyHeaderless(tr("Table(%1) is changed")
              .arg(newTitle));
          }
          QTimer::singleShot(0, [this]()
            {
              mModel->Reset();
            });
        };
      auto errorCallback = [this](QNetworkReply*, QNetworkReply::NetworkError e)
        {
          Notification::NotifyError("", e);
        };
      if (newTitle == title && newComment == comment)
      {
        Notification::NotifyInfo(tr("There were no changes"));
      }
      else
      {
        API::REST::EditTable(id, newTitle, newComment, callback, errorCallback);
      }

    });
  dial->Open();
}

void TreeWidgetSelectNumericDataTable::HideMenuButtons(bool hide)
{
  mActionButtons->setVisible(!hide);
  m_ActionMenuButton->setVisible(hide);
  m_pickedCountLabel->setVisible(!hide);
  m_pickAllCheckbox->setVisible(!hide);
}

void TreeWidgetSelectNumericDataTable::disableCheckableNotInParent(const QModelIndex& topLeft, const QModelIndex& bottomRight,
  const QList<int>& roles)
{
  if (mModel->mode() != mModel->joinMode &&
    !roles.contains(Qt::CheckStateRole))
    return;

  QSignalBlocker blocker(mModel);
  auto root = mModel->GetRoot();
  const auto childs = root->GetChildrenRecursive();
  for (const auto& child : childs)
  {
    child->SetData("checkable", true);
  }

  const auto items = mModel->GetCheckedItems();
  if (items.isEmpty())
    return;
  const auto parent = items.first()->GetParent();
  for (const auto& child : childs)
  {
    if (child->GetParent() == parent)
    {
      continue;
    }
    child->SetData("checkable", false);
  }
}
