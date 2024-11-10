#include "fragments_table_widget.h"

#include <genesis_style/style.h>

#include <ui/itemviews/checkable_headerview.h>
#include <ui/itemviews/datatable_item_delegate.h>
#include <ui/itemviews/tableview_model_actions_column_delegate.h>
#include <ui/itemviews/files_table_actions_column_delegate.h>
#include <ui/itemviews/tableview_headerview.h>
#include <ui/genesis_window.h>

#include <ui/dialogs/web_dialog_chromatogram_information.h>
#include <ui/dialogs/web_dialog_markup_creation.h>
#include <ui/dialogs/web_dialog_file_removing.h>
#include <ui/dialogs/web_dialog_input_dialog.h>
#include <ui/dialogs/templates/tree_confirmation.h>
#include <ui/dialogs/chromatogram_passport_dialog.h>

#include <logic/tree_model_dynamic_project_fragments.h>
#include <logic/tree_model_item.h>
#include <logic/notification.h>
#include <logic/context_root.h>
#include <logic/enums.h>
#include <logic/known_context_tag_names.h>
#include "ui/known_view_names.h"
#include "ui/dialogs/templates/dialog.h"
#include "ui/controls/dialog_line_edit_ex.h"
#include "logic/known_json_tag_names.h"

#include <api/api_rest.h>

#include <QVBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QLabel>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QCheckBox>
#include <QScrollbar>

using ModelClass = TreeModelDynamicProjectFragments;

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

namespace
{
  const QString filename = "filename";
}

FragmentsTableWidget::FragmentsTableWidget(QWidget* parent)
  : QWidget(parent)
{
  SetupUi();
  connectModelAndPickAllCheckBox();
}

void FragmentsTableWidget::setScrollPosition(int pos)
{
  m_scrollbarPosition = pos;
}

void FragmentsTableWidget::SetupUi()
{
  auto layout = new QVBoxLayout();
  setLayout(layout);

  auto toolBarLayout = new QHBoxLayout();
  layout->addLayout(toolBarLayout);

  toolBarLayout->setDirection(QHBoxLayout::Direction::RightToLeft);

  m_actionsMenuButton = new QPushButton(tr("Actions with fragments"));
  m_actionsMenuButton->setProperty("menu_secondary", true);
  toolBarLayout->addWidget(m_actionsMenuButton);

  {//// Chromatogramm group actions setup
    auto actionsMenu = new QMenu(m_actionsMenuButton);
    auto collapseExpandAction = actionsMenu->addAction(tr("Collapse/Expand all"));
    collapseExpandAction->setShortcut(QKeySequence(QKeyCombination(Qt::ControlModifier, Qt::Key_W)));
    auto markupAction = actionsMenu->addAction(tr("Markup"));
    auto passportAction = actionsMenu->addAction(tr("Passport"));
    // auto showOnPlotAction = actionsMenu->addAction(tr("Show on plot"));
    // auto copyAction = actionsMenu->addAction(tr("Copy to another project"));
    auto exportAction = actionsMenu->addAction(tr("Export"));
    auto removeAction = actionsMenu->addAction(tr("Remove"));

    m_actionsMenuButton->setMenu(actionsMenu);

    connect(collapseExpandAction, &QAction::triggered, this, &FragmentsTableWidget::toggleCollapsing);
    connect(markupAction, &QAction::triggered, this, &FragmentsTableWidget::PickedMarkupGroupAction);
    // connect(showOnPlotAction, &QAction::triggered, this, &FragmentsTableWidget::PickedShowOnPlotGroupAction);
    // connect(copyAction, &QAction::triggered, this, &FragmentsTableWidget::PickedCopyToOtherProjectGroupAction);
    connect(exportAction, &QAction::triggered, this, &FragmentsTableWidget::PickedExportGroupAction);
    connect(removeAction, &QAction::triggered, this, &FragmentsTableWidget::PickedRemoveGroupAction);
    connect(passportAction, &QAction::triggered, this, &FragmentsTableWidget::PickedPassportGroupAction);
  }

  {//Actions buttons:
    m_actionButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto button = m_actionButtons->button(QDialogButtonBox::Ok);
    button->setFocusPolicy(Qt::NoFocus);
    button->setText("Default ok button");
    connect(m_actionButtons, &QDialogButtonBox::rejected, this, &FragmentsTableWidget::ExitAction);
    m_actionButtons->setVisible(false);
    toolBarLayout->addWidget(m_actionButtons);
  }

  toolBarLayout->addStretch();
  m_pickedFragmentsCountLabel = new QLabel();
  toolBarLayout->addWidget(m_pickedFragmentsCountLabel);
  m_pickedFragmentsCountLabel->setVisible(false);

  m_treeView = new QTreeView();
  m_treeView->setStyleSheet(Style::Genesis::GetTreeViewStyle());
  m_treeView->setProperty("dataDrivenColors", true);
  m_treeView->setHeader(new TableViewHeaderView(Qt::Horizontal, m_treeView));
  m_treeView->header()->setSectionResizeMode(QHeaderView::Fixed);
  m_treeView->header()->setStretchLastSection(true);
  layout->addWidget(m_treeView);

  mModel = new ModelClass(m_treeView);

  {//pick all checkbox
    m_pickAllCheckbox = new QCheckBox("");
    m_pickAllCheckbox->setTristate(true);
    m_pickAllCheckbox->setCheckState(Qt::Unchecked);
    toolBarLayout->addWidget(m_pickAllCheckbox);
    m_pickAllCheckbox->setVisible(false);
  }

  m_sortProxyModel = new QSortFilterProxyModel(mModel);
  m_sortProxyModel->setSourceModel(mModel);
  m_treeView->setModel(m_sortProxyModel);
  //  TreeView->setModel(m_model);
  m_treeView->setSortingEnabled(true);
  auto d = m_treeView->itemDelegate();
  m_treeView->setItemDelegate(new DataTableItemDelegate(m_treeView));
  d->deleteLater();
  m_treeView->setItemDelegateForColumn(ModelClass::ColumnLast,
    new FilesTableActionsColumnDelegate(m_treeView.get()));
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
      ItemViewSamplePassport(mModel->GetItem(m_sortProxyModel->mapToSource(index)));
    });
  addMemoryScrollbarPosition();

  m_fragmentsCountLabel = new QLabel();
  layout->addWidget(m_fragmentsCountLabel);
  connect(m_fragmentsCountLabel, &QLabel::linkActivated, this, [this]()
    {
      updateScrollPosition();
      mModel->Reset();
    });


  connect(mModel, &ModelClass::rowsInserted, this, &FragmentsTableWidget::UpdateFragmentsCountLabel);
  connect(mModel, &ModelClass::rowsRemoved, this, &FragmentsTableWidget::UpdateFragmentsCountLabel);

  connect(mModel, &ModelClass::dataChanged, this, [this](const QModelIndex& topLeft,
    const QModelIndex& bottomRight, const QVector<int>& roles)
    {
      if (m_pickedFragmentsCountLabel->isVisible() && roles.contains(Qt::CheckStateRole))
        UpdatePickedFragmentsCountLabel();
    });

  connect(mModel, &ModelClass::modelReset, this, [this]()
    {
      UpdateFragmentsCountLabel();
      UpdatePickedFragmentsCountLabel();
      m_treeView->expandAll();
      mCollapsed = false;
    });

  connect(mModel, &ModelClass::ActionTriggered, this, [this](const QString& actionId, TreeModelItem* item)
    {
      if (actionId == "edit_mz")
        ItemEditMZ(item);
      else if (actionId == "view_sample_passport")
        ItemViewSamplePassport(item);
      else if (actionId == "export")
        ItemExport(item);
      else if (actionId == "delete")
        ItemDelete(item);
    });
}

void FragmentsTableWidget::connectModelAndPickAllCheckBox()
{
  connect(mModel, &ModelClass::dataChanged, this, &FragmentsTableWidget::onModelDataChanged);
  connect(mModel, &ModelClass::modelReset, this, &FragmentsTableWidget::invalidate);
  connect(m_pickAllCheckbox, &QCheckBox::stateChanged, this, &FragmentsTableWidget::onPickAllCheckboxStateChecked);
}

void FragmentsTableWidget::disconnectModelAndPickAllCheckBox()
{
  disconnect(mModel, &ModelClass::dataChanged, this, &FragmentsTableWidget::onModelDataChanged);
  disconnect(mModel, &ModelClass::modelReset, this, &FragmentsTableWidget::invalidate);
  disconnect(m_pickAllCheckbox, &QCheckBox::stateChanged, this, &FragmentsTableWidget::onPickAllCheckboxStateChecked);
}

void FragmentsTableWidget::UpdateFragmentsCountLabel()
{
  QString text = tr("Results total: ") + QString::number(mModel->nonGroupItemsCount());
  QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\">"
    "<img src=\":/resource/icons/icon_action_refresh_small.png\"/></a>"
    "</td></tr></table>").arg(text);
  m_fragmentsCountLabel->setText(tmpl);
}

void FragmentsTableWidget::UpdatePickedFragmentsCountLabel()
{
  if (mModel->isInMarkupPickMode())
  {
    m_pickedFragmentsCountLabel->setText(tr("Pick all fragments with same m/z for markup (picked %n)" , "",
      mModel->getCheckedItems().size()));
  }
  else
  {
    m_pickedFragmentsCountLabel->setText(tr("Picked %n chromatogramms ", "",
      mModel->getCheckedItems().size()) + m_forWhatActionText);
  }
}

void FragmentsTableWidget::toggleCollapsing()
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

void FragmentsTableWidget::PickedMarkupGroupAction()
{
  ActionAssignAndEnter(tr("Markup"), tr("for markup"), [this]()
    {
      QList<int> checkedIds;
      {
        auto checkedItems = mModel->getCheckedItems(mModel->GetRoot());
        for (auto& item : checkedItems)
          checkedIds << item->GetData("project_file_id").toInt();
      }
      if (checkedIds.isEmpty())
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
                if (auto contextRoot = GenesisContextRoot::Get())
                {
                  if (auto contextProject = contextRoot->GetContextProject())
                  {
                    if (auto markupContext = contextRoot->GetContextMarkup())
                    {
                      QVariantMap data;
                      data[Names::MarkupContextTags::kMarkupId] = markupId;
                      data[Names::MarkupContextTags::kMarkupTitle] = markupName;
                      markupContext->SetData(data);
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
        });
    },
    [this]()
    {
      mModel->setMarkupPickModeEnabled(true);
    },
    [this]()
    {
      mModel->setMarkupPickModeEnabled(false);
    });
}

void FragmentsTableWidget::PickedExportGroupAction()
{
  ActionAssignAndEnter(tr("Export"), tr("for exporting"), [this]()
    {
      auto request = QJsonObject();
      auto files = QJsonArray();
      QString fileName;

      {
        auto checkedItems = mModel->getCheckedItems(mModel->GetRoot());
        if (checkedItems.isEmpty())
        {
          Notification::NotifyError(tr("Ions export error, no files picked"));
          return;
        }
        fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
          "ions.csv",
          tr("*.csv"));
        if (fileName.isEmpty())
          return;

        for (auto& item : checkedItems)
        {

          auto file = QJsonObject();

          if (item->GetData("file_type_id").toInt() == ProjectFiles::sim)
          {

            file[filename] = item->GetData(filename).toString();
            file["project_file_id"] = item->GetData("project_file_id").toInt();

            auto parentIdVariant = item->GetData("parent_id");

            if (parentIdVariant.isValid() && !parentIdVariant.isNull())
            {
              file["parent_id"] = parentIdVariant.toInt();
            }
            else
            {
              file["parent_id"] = 0;
            }
            files << file;
          }
        }
      }

      request["files"] = files;
      request.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
      if (request.isEmpty())
      {
        Notification::NotifyError(tr("For group actions pick one or more chromatogramms in table"));
        return;
      }

      API::REST::ExportIons(request,
        [this, fileName](QNetworkReply* reply, QByteArray data)
        {
          auto reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
          reason.replace('\'', '"').replace("False", "\"False\"").replace("True", "\"True\"");

          if (const auto doc = QJsonDocument::fromJson(reason.toUtf8()); !(doc.isNull() || doc.isEmpty()))
          {
            const auto obj = doc.object();
            const auto error = doc["error"].toBool();

            if (error)
            {
              const auto msg = obj["msg"].toString();
              Notification::NotifyError(tr("Ions export error: ") + msg);
            }
            else
            {
              QFile file(fileName);
              if (file.open(QIODevice::WriteOnly))
              {
                file.write(data);
                file.close();
                Notification::NotifySuccess(tr("Ions export success"));
              }
              else
              {
                Notification::NotifyError(tr("Ions export error"));
              }
            }
          }

        },
        [](QNetworkReply*, QNetworkReply::NetworkError error)
        {
          Notification::NotifyError(tr("Ions export error"), error);
        });
      ExitAction();
    }, [&]() { m_pickAllCheckbox->setEnabled(true); });
}

void FragmentsTableWidget::PickedPassportGroupAction()
{
  ActionAssignAndEnter(tr("Change passport"), tr("for passport editing"), [this]()
    {
      QList<int> checkedFilesIds;
      {
        auto checkedItems = mModel->getCheckedItems(mModel->GetRoot());
        for (auto& item : checkedItems)
          checkedFilesIds << item->GetData("id").toInt();
      }
      if (checkedFilesIds.isEmpty())
      {
        Notification::NotifyError(tr("For grouping changing pick one or more chromatogramms in table"));
        return;
      }
      activatePassportsDialogAction(checkedFilesIds, true);
    },
    [this]()//prepare
    {
      mModel->enterPassportMode();
    },
    [this]()//done
    {
      mModel->exitPassportMode();
    });
}

void FragmentsTableWidget::PickedRemoveGroupAction()
{
  ActionAssignAndEnter(tr("Remove"), tr("for removing"), [this]()
    {
      QList<QPair<int, QString>> checkedFilesInfos;
      {
        auto checkedItems = mModel->getCheckedItems(mModel->GetRoot());
        for (auto& item : checkedItems)
        {
          if (item->GetData("file_type").toString() != "sim")
            continue;
          QString fileName = item->GetData(filename).toString();
          checkedFilesInfos << QPair<int, QString>(item->GetData("project_file_id").toInt(),
            fileName
          );
        }
      }
      if (checkedFilesInfos.isEmpty())
      {
        Notification::NotifyError(tr("For group actions pick one or more chromatogramms in table"));
        return;
      }

      auto dial = Dialogs::Templates::TreeConfirmation::DeleteMarkupFilesWithDependencies(checkedFilesInfos, this);
      connect(dial, &WebDialog::Accepted, this, [this]()
        {
          ExitAction();
        });
    });
}

void FragmentsTableWidget::ActionAssignAndEnter(QString buttonText, QString forWhatAction,
  std::function<void()> func,
  std::function<void()> prepare,
  std::function<void()> done)
{
  mModel->SetCheckable(true);

  prepare();
  m_doneAction = done;
  m_fragmentsCountLabel->blockSignals(true);
  auto okBtn = m_actionButtons->button(QDialogButtonBox::Ok);
  okBtn->setText(buttonText);
  if (m_actionConnection)
    disconnect(m_actionConnection);
  m_actionConnection = connect(m_actionButtons, &QDialogButtonBox::accepted, this, func);
  m_actionButtons->setVisible(true);
  m_pickAllCheckbox->setVisible(true);
  m_actionsMenuButton->setVisible(false);
  m_pickedFragmentsCountLabel->setVisible(true);
  m_forWhatActionText = forWhatAction;
  UpdatePickedFragmentsCountLabel();

  m_treeView->update();
  m_treeView->resizeColumnToContents(0);
}

void FragmentsTableWidget::ExitAction()
{
  if (m_actionConnection)
    disconnect(m_actionConnection);
  m_actionButtons->setVisible(false);
  m_pickAllCheckbox->setVisible(false);
  m_actionsMenuButton->setVisible(true);
  m_pickedFragmentsCountLabel->setVisible(false);
  m_forWhatActionText.clear();

  mModel->setChildrenCheckState(Qt::Unchecked, mModel->GetRoot());
  mModel->SetCheckable(false);
  if(m_doneAction)
  {
    m_doneAction();
    m_doneAction = []() {};
  }
  m_fragmentsCountLabel->blockSignals(false);
}

void FragmentsTableWidget::ItemViewSamplePassport(TreeModelItem* item)
{
  if(!item->IsRoot() && !item->GetChildren().isEmpty())
  {
    if (int fileId = item->GetData("id").toInt())
    {
      activatePassportsDialogAction({ fileId });
    }
  }
  else
  {
    int fileId=-1;
    if(item->GetParent() == mModel->GetRoot())
    {
      fileId = item->GetData("id").toInt();
    }
    else
    {
      fileId = item->GetParent()->GetData("id").toInt();
    }
    if (fileId && fileId != -1)
    {
      activatePassportsDialogAction({ fileId });
    }
  }
}

void FragmentsTableWidget::ItemEditMZ(TreeModelItem* item)
{
  if (item->GetData("file_type_id") != ProjectFiles::sim ||
    item->parent() != mModel->GetRoot())
    return;
  auto projectFileId = item->GetData("project_file_id").toInt();
  auto fileName = item->GetData(filename).toString();

  using Btns = QDialogButtonBox;
  auto s = Dialogs::Templates::Dialog::Settings();
  s.buttonsNames[Btns::Ok] = tr("Apply");
  s.dialogHeader = tr("Editing of m/z");
  auto w = new QWidget();
  auto l = new QVBoxLayout(w);
  auto ls = DialogLineEditEx::Settings();
  ls.textHeader = tr("m/z of ion") + Style::GetInputAlert();
  ls.textMaxLen = -1;
  ls.textPlaceholder = tr("Type ion m/z here");
  ls.textRequired = true;
  ls.textTooltipsSet.empty = tr("m/z should be non-empty");
  ls.textTooltipsSet.notChecked = tr("not checked");
  ls.textTooltipsSet.valid = tr("m/z is valid");
  auto lineEdit = new DialogLineEditEx(w, ls);
  l->addWidget(lineEdit);
  auto dial = new Dialogs::Templates::Dialog(this, s, Btns::Ok | Btns::Cancel, w);
  connect(lineEdit, &DialogLineEditEx::validityChanged, dial, [dial](bool valid)
    {
      dial->SetButtonEnabled(Btns::Ok, valid);
    });
  dial->SetButtonEnabled(Btns::Ok, false);

  connect(dial, &WebDialogInputDialog::Accepted, this, [dial, lineEdit, projectFileId, fileName]()
    {
      auto newMz = lineEdit->text();
      API::REST::EditMZ(projectFileId, newMz,
        [fileName](QNetworkReply* reply, QJsonDocument data)
        {
          TreeModel::ResetInstances("TreeModelDynamicProjectChromatogramms");
          TreeModel::ResetInstances("TreeModelDynamicProjectFragments");
          Notification::NotifySuccess(tr("m/z changed for") + QString(" %1").arg(fileName));
        },
        [fileName](QNetworkReply*, QNetworkReply::NetworkError error)
        {
          Notification::NotifyError(tr("failed to change m/z for") + QString(" %1").arg(fileName));
        });
    });
  dial->Open();
}

void FragmentsTableWidget::ItemExport(TreeModelItem* item)
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
    "ions.csv",
    tr("*.csv"));
  if (fileName.isEmpty())
    return;

  auto request = QJsonObject();
  auto files = QJsonArray();

  auto appendFile = [&files](TreeModelItem* item)
    {
      auto file = QJsonObject();
      file[filename] = item->GetData(filename).toString();
      file["project_file_id"] = item->GetData("project_file_id").toInt();

      auto parentIdVariant = item->GetData("parent_id");

      if (parentIdVariant.isValid() && !parentIdVariant.isNull())
      {
        file["parent_id"] = parentIdVariant.toInt();
      }
      else
      {
        file["parent_id"] = 0;
      }
      files << file;
    };

  if (item->GetData("file_type_id").toInt() == ProjectFiles::sim)
  {
    appendFile(item);
  }
  else if (item->GetData("file_type_id").toInt() == ProjectFiles::tic)
  {
    for (auto child : item->GetChildren())
    {
      appendFile(child);
    }
  }
  request["files"] = files;
  request.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
  if (request.isEmpty())
  {
    Notification::NotifyError(tr("For group actions pick one or more chromatogramms in table"));
    return;
  }
  //      qDebug().noquote() << "import file json:\n" << QJsonDocument(request).toJson();

  API::REST::ExportIons(request,
    [this, fileName](QNetworkReply* reply, QByteArray data)
    {
      auto reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
      reason.replace('\'', '"').replace("False", "\"False\"").replace("True", "\"True\"");

      if (const auto doc = QJsonDocument::fromJson(reason.toUtf8()); !(doc.isNull() || doc.isEmpty()))
      {
        const auto obj = doc.object();
        const auto error = doc["error"].toBool();

        if (error)
        {
          const auto msg = obj["msg"].toString();
          Notification::NotifyError(tr("Ions export error: ") + msg);
        }
        else
        {
          QFile file(fileName);
          if (file.open(QIODevice::WriteOnly))
          {
            file.write(data);
            file.close();
            Notification::NotifySuccess(tr("Ions export success"));
          }
          else
          {
            Notification::NotifyError(tr("Ions export error"));
          }
        }
      }

    },
    [](QNetworkReply*, QNetworkReply::NetworkError error)
    {
      Notification::NotifyError(tr("Ions export error"), error);
    });
}

void FragmentsTableWidget::ItemDelete(TreeModelItem* item)
{
  QString fileName = item->GetData(filename).toString();

  QPair<int, QString> fileInfo = {
    item->GetData("project_file_id").toInt(),
    fileName
  };

  auto dial = Dialogs::Templates::TreeConfirmation::DeleteMarkupFilesWithDependencies({ fileInfo }, this);
}

void FragmentsTableWidget::activatePassportsDialogAction(QList<int> filesIds, bool isInGroupAction)
{
  auto loadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  API::REST::GetPassports(filesIds,
    [this, loadingOverlayId, isInGroupAction](QNetworkReply*, QJsonDocument doc)
    {
      if (doc.object()["error"].toBool())
      {
        Notification::NotifyError(doc.object()["msg"].toString(), tr("Failed to load passport"));
        GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        return;
      }

      auto dial = new Dialogs::ChromatogramPassportDialog(true, doc.object(), this);
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
              if (doc.object()["error"].toBool())
              {
                Notification::NotifyError(doc.object()["msg"].toString(), tr("Failed to upload passport"));
                GenesisWindow::Get()->RemoveOverlay(uploadingOverlayId);
                return;
              }
              Notification::NotifySuccess(tr("Passport uploaded"));
              GenesisWindow::Get()->RemoveOverlay(uploadingOverlayId);
              updateScrollPosition();
              TreeModel::ResetInstances("TreeModelDynamicProjectFragments");
            },
            [uploadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
            {
              Notification::NotifyError("Server error", e);
              GenesisWindow::Get()->RemoveOverlay(uploadingOverlayId);

            });
        });
      dial->Open();
      GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
      if (isInGroupAction)
        ExitAction();
    },
    [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      Notification::NotifyError("Server error", e);
      GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
    }
  );
}

void FragmentsTableWidget::addMemoryScrollbarPosition()
{
  auto test = [this](int min, int max)
    {
      if (min == max)
        return;
      if (m_scrollbarPosition)
        m_treeView->verticalScrollBar()->setValue(m_scrollbarPosition);
    };
  connect(m_treeView->verticalScrollBar(), &QScrollBar::rangeChanged, this, test);
}

void FragmentsTableWidget::updateScrollPosition()
{
  m_scrollbarPosition = m_treeView->verticalScrollBar()->value();
}

void FragmentsTableWidget::invalidate()
{
  const auto checkedSize = mModel->getCheckedItems().size();
  const auto checkableCount = mModel->getCheckableCount();
  m_pickAllCheckbox->setDisabled(!mModel->GetCheckable() || (mModel->isInMarkupPickMode() && checkedSize == 0));
  if (checkableCount == 0)
    m_pickAllCheckbox->setCheckState(Qt::Unchecked);
  else if (checkedSize < checkableCount)
  {
    if (checkedSize == 0)
      m_pickAllCheckbox->setCheckState(Qt::Unchecked);
    else
    {
      m_pickAllCheckbox->setCheckState(Qt::PartiallyChecked);
      if (mModel->isInMarkupPickMode())
      {
        m_pickAllCheckbox->setEnabled(true);
      }
    }
  }
  else
    m_pickAllCheckbox->setCheckState(Qt::Checked);
}

void FragmentsTableWidget::onPickAllCheckboxStateChecked(int state)
{
  disconnectModelAndPickAllCheckBox();
  QScopeGuard connectGuard(std::bind(&FragmentsTableWidget::connectModelAndPickAllCheckBox, this));

  if (state == Qt::PartiallyChecked)
  {
    m_pickAllCheckbox->setCheckState(Qt::Checked);
  }

  if (mModel->isInMarkupPickMode())
  {
    const auto items = mModel->getCheckedItems();
    if (state == Qt::Unchecked)
    {
      mModel->setChildrenCheckState(Qt::Unchecked, mModel->GetRoot());
      mModel->resetIonNameCheckedFilter();
      m_pickAllCheckbox->setDisabled(true);
    }
    else if (!items.isEmpty())
    {
      const auto mzValue = items.first()->GetData(filename);
      mModel->setCheckedByMZValue(mzValue.toString(), state);
    }
  }
  else
    mModel->setChildrenCheckState(m_pickAllCheckbox->checkState(), mModel->GetRoot());
}

void FragmentsTableWidget::onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
  if (!roles.contains(Qt::CheckStateRole))
    return;

  disconnectModelAndPickAllCheckBox();
  QScopeGuard connectGuard(std::bind(&FragmentsTableWidget::connectModelAndPickAllCheckBox, this));

  invalidate();
}