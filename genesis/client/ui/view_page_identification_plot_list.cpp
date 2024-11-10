#include "view_page_identification_plot_list.h"
#include "ui_view_page_identification_plot_list.h"

#include "api/api_rest.h"
#include "genesis_style/style.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/notification.h"
#include "logic/tree_model_dynamic_identification_plots.h"
#include "logic/tree_model_presentation.h"
#include "ui/itemviews/tableview_model_actions_column_delegate.h"
#include "ui/itemviews/tableview_headerview.h"

#include "ui/widgets/edit_group_widget.h"
#include "ui/controls/dialog_line_edit_ex.h"
#include "ui/controls/dialog_text_edit_ex.h"
#include "ui/dialogs/templates/dialog.h"
#include "ui/dialogs/templates/confirm.h"

#include <QAction>
#include <QMenu>
#include <QScrollArea>


namespace Views
{

ViewPageIdentificationPlotList::ViewPageIdentificationPlotList(QWidget *parent)
  : View(parent)
  , ui(new Ui::ViewPageIdentificationPlotList)
{
  mModel = new TreeModelDynamicIdentificationPlots(this);
  mPresentationModel = mModel->GetPresentationModel();
  setupUi();
  setupConnections();
  ui->treeView->setModel(mPresentationModel);
  ui->treeView->setHeader(new TableViewHeaderView(Qt::Horizontal, ui->treeView));
}

ViewPageIdentificationPlotList::~ViewPageIdentificationPlotList()
{
  delete ui;
}

void ViewPageIdentificationPlotList::ApplyContextProject(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if(!isReset && dataId != Names::ContextTagNames::ProjectId)
    return;
  int projectId;
  if(isReset)
    projectId = Core::GenesisContextRoot::Get()->ProjectId();
  else
    projectId = data.toInt();

  if (projectId != mProjectId)
  {
    mProjectId = projectId;
    ui->plotListWidget->clear();
    ui->stackedWidget->setCurrentWidget(ui->tablePage);
  }
}


void ViewPageIdentificationPlotList::setupUi()
{
  ui->setupUi(this);
  setStyleSheet(Style::Genesis::GetUiStyle());

  mChangeGroup = new QAction(tr("Change group"));
  mRemove = new QAction(tr("Remove"));
  mButtonMenu = new QMenu(ui->actionsButton);
  mButtonMenu->addAction(mChangeGroup);
  mButtonMenu->addAction(mRemove);

  ui->actionsButton->setMenu(mButtonMenu);
  ui->actionsButton->style()->polish(ui->actionsButton);
  ui->titleLabel->setStyleSheet(Style::Genesis::GetH1());

  ui->mainWidget->setAttribute(Qt::WA_StyledBackground, true);
  ui->mainWidget->setStyleSheet("background: white;");
  ui->mainWidget->setAttribute(Qt::WA_StyledBackground, true);
  ui->mainWidget->setStyleSheet("background: white;");

  ui->treeView->setItemDelegateForColumn(TreeModelDynamicIdentificationPlots::ColumnLast,
                                         new TableViewModelActionsColumnDelegate(ui->treeView));
  ui->treeView->setSortingEnabled(true);
  ui->treeView->sortByColumn(TreeModelDynamicIdentificationPlots::ColumnTableTitle, Qt::AscendingOrder);

  ui->treeView->setStyleSheet(Style::Genesis::GetTreeViewStyle());
  ui->treeView->setProperty("dataDrivenColors", true);
  ui->treeView->setHeader(new TableViewHeaderView(Qt::Horizontal, ui->treeView));
  ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->treeView->header()->setStretchLastSection(false);

  ui->stackedWidget->setCurrentWidget(ui->tablePage);

  ui->actionsButton->setVisible(true);
  ui->cancelButton->setVisible(false);
  ui->removeButton->setVisible(false);
  ui->changeGroupButton->setVisible(false);

  ui->reloadLabel->setText("<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a>");
}

void ViewPageIdentificationPlotList::setupConnections()
{
  connect(ui->treeView, &QTreeView::doubleClicked, this, &ViewPageIdentificationPlotList::showPlot);
  connect(ui->plotListWidget, &IdentificationPlotListWidget::showPreviousPage, this, [this] () {
    ui->stackedWidget->setCurrentWidget(ui->tablePage);
  });

  connect(ui->plotListWidget, &IdentificationPlotListWidget::needToReloadTable, this, &ViewPageIdentificationPlotList::reloadModel);

  connect(ui->plotListWidget, &IdentificationPlotListWidget::loadedPlot, this, [this] () {
    ui->stackedWidget->setCurrentWidget(ui->plotListPage);
  });

  connect(mModel, &TreeModel::ActionTriggered, this, [this](const QString& actionId, TreeModelItem* item) {
    actionTrigger(actionId, item);
  });

  connect(mChangeGroup, &QAction::triggered, this, [this]() {
    beginSelectMode();
    ui->changeGroupButton->setVisible(true);
  });

  connect(mRemove, &QAction::triggered, this, [this]() {
    beginSelectMode();
    ui->removeButton->setVisible(true);
  });

  connect(ui->cancelButton, &QPushButton::clicked, this, [this]() {
    endSelectMode();
  });
  connect(ui->removeButton, &QPushButton::clicked, this, [this]() {
    removeItem();
  });
  connect(ui->changeGroupButton, &QPushButton::clicked, this, [this]() {
    changeGroup();
  });

  connect(ui->reloadLabel, &QLabel::linkActivated, this, [this]() {mModel->Reset(); });
  connect(mModel, &TreeModelDynamicIdentificationPlots::modelReset, this, [this]() {
    for (int i = 0; i < mModel->columnCount(); i++)
      ui->treeView->resizeColumnToContents(i);
  });

  connect(mModel, &TreeModelDynamicIdentificationPlots::dataChanged, this, &ViewPageIdentificationPlotList::updateTableCheckboxStates);
}

void ViewPageIdentificationPlotList::updateTableCheckboxStates(const QModelIndex&, const QModelIndex&,
  const QList<int>& roles)
{
  if (roles.contains(Qt::CheckStateRole))
    ui->treeView->update();
}

void ViewPageIdentificationPlotList::showPlot(const QModelIndex &index)
{
  if (!index.isValid())
    return;
  auto item = mPresentationModel->GetItem(index);;
  if (item && !mModel->isItemIsGroup(item) && !mModel->GetCheckable())
  {
    ui->plotListWidget->openPlot(item->GetData("plot_id").toInt());
  }
}

void ViewPageIdentificationPlotList::reloadModel()
{
  ui->stackedWidget->setCurrentWidget(ui->tablePage);
  endSelectMode();
  mModel->Reset();
}

void ViewPageIdentificationPlotList::actionTrigger(const QString& actionId, TreeModelItem* item)
{
  auto row = mModel->rowCount();
  auto presRow = mPresentationModel->rowCount();
  if (row == presRow)
  {
    if (actionId == "edit_group_title")
    {
      editGroupTitle(item);
    }
    else if (actionId == "change_group")
    {
      changeGroup(item);
    }
    else if (actionId == "edit_item_title")
    {
      editItemTitle(item);
    }
    else if (actionId == "remove_item")
    {
      removeItem(item);
    }
  }
}

void ViewPageIdentificationPlotList::editItemTitle(TreeModelItem* item)
{
  auto widget = new QWidget();
  widget->setStyleSheet(Style::Genesis::GetUiStyle());
  widget->setStyleSheet("background: white;");
  auto layout = new QVBoxLayout(widget);

  auto w = new QWidget();
  auto l = new QVBoxLayout(w);
  auto ls = DialogLineEditEx::Settings();
  ls.textHeader = tr("Plot title") + Style::GetInputAlert();
  ls.textMaxLen = 50;
  ls.textPlaceholder = tr("Type new plot title");
  ls.defaultText = item->GetData("plot_title").toString();
  ls.textRequired = true;
  ls.textTooltipsSet.empty = tr("Title should be non-empty");
  ls.textTooltipsSet.notChecked = tr("Not checked");
  ls.textTooltipsSet.valid = tr("Title is valid");
  auto titleLineEdit = new DialogLineEditEx(w, ls);
  l->addWidget(titleLineEdit);

  auto ts = DialogTextEditEx::Settings();
  ts.textHeader = tr("Comment");
  ts.textMaxLen = 70;
  ts.textPlaceholder = tr("Type comment");
  ts.textRequired = false;
  ts.defaultText = item->GetData("comment").toString();
  auto commentTextEdit = new DialogTextEditEx(w);
  commentTextEdit->applySettings(ts);
  l->addWidget(commentTextEdit);
  layout->addWidget(w);

  auto line = new QFrame(w);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  layout->addWidget(line);

  using Btns = QDialogButtonBox;
  auto s = Dialogs::Templates::Dialog::Settings();
  s.dialogHeader = tr("Editing");
  s.buttonsNames[Btns::Ok] = tr("Save");
  auto dial = new Dialogs::Templates::Dialog(this, s, Btns::Ok | Btns::Cancel, widget);

  connect(titleLineEdit, &DialogLineEditEx::validityChanged, this, [titleLineEdit, dial](bool) {
    dial->SetButtonEnabled(QDialogButtonBox::Ok, titleLineEdit->checkValidity());
  });

  int plotId = item->GetData("plot_id").toInt();
  connect(dial, &WebDialog::Accepted, this, [this, titleLineEdit, commentTextEdit, plotId]() {
    API::REST::changePlotTitle(plotId,titleLineEdit->text(), commentTextEdit->text(), [this](QNetworkReply* r, QJsonDocument doc)
      {
        Notification::NotifySuccess(tr("Plot was changed"),
                                    tr("Selected plot have successfully changed"));
        reloadModel();
      },
      [](QNetworkReply* r, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Failed to update plot"), err);
      });
  });
  dial->Open();
}

void ViewPageIdentificationPlotList::editGroupTitle(TreeModelItem* item)
{
  if (!mModel->isItemIsGroup(item))
  {
    return;
  }

  auto w = new QWidget();
  w->setStyleSheet(Style::Genesis::GetUiStyle());
  w->setStyleSheet("background: white;");
  auto l = new QVBoxLayout(w);
  auto ls = DialogLineEditEx::Settings();
  ls.textHeader = tr("Group title") + Style::GetInputAlert();
  ls.textMaxLen = 50;
  ls.textPlaceholder = tr("Type new group title");
  ls.defaultText = item->GetData("plot_title").toString();
  ls.textRequired = true;
  ls.textTooltipsSet.empty = tr("Title should be non-empty");
  ls.textTooltipsSet.notChecked = tr("Not checked");
  ls.textTooltipsSet.valid = tr("Title is valid");
  auto titleLineEdit = new DialogLineEditEx(w, ls);
  l->addWidget(titleLineEdit);
  using Btns = QDialogButtonBox;
  auto s = Dialogs::Templates::Dialog::Settings();
  s.dialogHeader = tr("Editing");
  s.buttonsNames[Btns::Ok] = tr("Save");
  auto dial = new Dialogs::Templates::Dialog(this, s, Btns::Ok | Btns::Cancel, w);

  connect(titleLineEdit, &DialogLineEditEx::validityChanged, this, [titleLineEdit, dial](bool) {
    dial->SetButtonEnabled(QDialogButtonBox::Ok, titleLineEdit->checkValidity());
  });

  int groupId = item->GetData("plot_group_id").toInt();
  connect(dial, &WebDialog::Accepted, this, [this, titleLineEdit, groupId]() {

    const auto projectId = Core::GenesisContextRoot::Get()->ProjectId();
    API::REST::changeGroupTitle(projectId, groupId, titleLineEdit->text(), [this](QNetworkReply* r, QJsonDocument doc)
      {
        Notification::NotifySuccess(tr("Plot was changed"),
                                    tr("Selected plot have successfully changed"));

        reloadModel();
      },
      [](QNetworkReply* r, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Failed to update plot"), err);
      });
  });
  dial->Open();
}

void ViewPageIdentificationPlotList::beginSelectMode()
{
  mModel->SetCheckable(true);
  mModel->GetRoot()->SetCheckState(Qt::Unchecked);
  ui->actionsButton->setVisible(false);
  ui->cancelButton->setVisible(true);
  ui->removeButton->setVisible(false);
  ui->changeGroupButton->setVisible(false);
}

void ViewPageIdentificationPlotList::endSelectMode()
{
  mModel->SetCheckable(false);
  mModel->GetRoot()->SetCheckState(Qt::Unchecked);
  ui->actionsButton->setVisible(true);
  ui->cancelButton->setVisible(false);
  ui->removeButton->setVisible(false);
  ui->changeGroupButton->setVisible(false);
}

void ViewPageIdentificationPlotList::changeGroup(QPointer<TreeModelItem> selectedItem)
{
  QList <int> plotIds;
  QList <QString> plotTitles;
  if (selectedItem)
  {
    if (!mModel->isItemIsGroup(selectedItem))
    {
      plotIds.append(selectedItem->GetData("plot_id").toInt());
      plotTitles.append(selectedItem->GetData("plot_title").toString());
    }
    else
    {
      selectedItem->GetChildrenRecursive();
      for (auto item: selectedItem->GetChildrenRecursive())
      {
        if (!mModel->isItemIsGroup(item))
        {
          plotIds.append(item->GetData("plot_id").toInt());
          plotTitles.append(item->GetData("plot_title").toString());
        }
      }
    }
  }
  else
  {
    for (auto item: mModel->GetRoot()->GetChildrenRecursive())
    {
      if (!mModel->isItemIsGroup(item) && item->GetCheckState() == Qt::Checked)
      {
        plotIds.append(item->GetData("plot_id").toInt());
        plotTitles.append(item->GetData("plot_title").toString());
      }
    }
  }
  if (plotIds.empty())
    return;

  using Btns = QDialogButtonBox;
  auto s = Dialogs::Templates::Dialog::Settings();
  s.dialogHeader = tr("Change group");
  s.buttonsNames[Btns::Ok] = tr("Move");

  QWidget *widget = new QWidget();
  widget->setStyleSheet(Style::Genesis::GetUiStyle());
  auto layout = new QVBoxLayout(widget);
  Widgets::EditGroupWidget *groupWidget = new Widgets::EditGroupWidget();

  groupWidget->SetCaptionText(tr("Move selected plots", nullptr, plotIds.size()));
  groupWidget->SetUngroupRadioText("To the general list of plots");
  groupWidget->SetGroupRadioText("To the plot group");
  groupWidget->layout()->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum));

  layout->addWidget(groupWidget);
  layout->addStretch(1);


  const auto projectId = Core::GenesisContextRoot::Get()->ProjectId();
  API::REST::getIdentificationPlotGroups(projectId, [this, groupWidget](QNetworkReply* r, QJsonDocument doc)
    {
      std::map<QString, int> groups;
      auto jdata = doc.object()["data"].toArray();
      for (int i = 0; i < jdata.size(); i++)
      {
        auto jitem = jdata[i].toArray();
        groups.emplace(jitem[1].toString(), jitem[0].toInt());
      }
      groupWidget->SetGroupInfo(groups);
    },
    [](QNetworkReply* r, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to load file groups information"), err);
    });


  auto dial = new Dialogs::Templates::Dialog(this, s, Btns::Ok | Btns::Cancel, widget);

  connect(dial, &WebDialog::Accepted, this, [this, plotIds, plotTitles, groupWidget]() {
    int groupId = -1;
    QString groupTitle;
    if (groupWidget->IsCommonChecked())
    {
      groupId = -1;
      groupTitle = QString();
    }
    else
    {
      groupTitle = groupWidget->GroupTitle();
      groupId = groupWidget->IsCreateNewGroup() ? -1: groupWidget->SelectedGroupId();
    }
    bool needToCreate = groupWidget->IsCreateNewGroup();
    const auto projectId = Core::GenesisContextRoot::Get()->ProjectId();
    API::REST::changePlotsGroup(projectId, plotIds, needToCreate, groupId, groupTitle,
      [this](QNetworkReply*, QJsonDocument doc)
      {
        Notification::NotifySuccess(tr("Group was changed"),
                                    tr("Selected plots have successfully changed group"));

        reloadModel();
      },
      [this](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Failed to changed group"), err);
      });
  });

  dial->Open();
}

void ViewPageIdentificationPlotList::removeItem(QPointer<TreeModelItem> selectedItem)
{
  QList <int> plotIds;
  QList <QString> plotTitles;
  if (selectedItem)
  {
    plotIds.append(selectedItem->GetData("plot_id").toInt());
    plotTitles.append(selectedItem->GetData("plot_title").toString());
  }
  else
  {
    for (auto item: mModel->GetRoot()->GetChildrenRecursive())
    {
      if (!mModel->isItemIsGroup(item) && item->GetCheckState() == Qt::Checked)
      {
        plotIds.append(item->GetData("plot_id").toInt());
        plotTitles.append(item->GetData("plot_title").toString());
      }
    }
  }
  if (plotIds.empty())
    return;

  using Btns = QDialogButtonBox;

  Dialogs::Templates::Confirm::Settings s;
  s.dialogHeader = tr("Deleting");
  s.buttonsNames[Btns::Ok] = tr("Delete");
  s.buttonsProperties[Btns::Ok] = {{"red", true}};

  QWidget *widget = new QWidget();
  widget->setStyleSheet(Style::Genesis::GetUiStyle());
  widget->setAttribute(Qt::WA_StyledBackground, true);
  widget->setStyleSheet("background: white;");
  auto layout = new QVBoxLayout(widget);
  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setWidget(widget);

  if (plotIds.count() > 1)
  {
    QLabel *question = new QLabel(widget);
    question->setText(tr("Are you sure you want to remove selected plots? It will be impossible to cancel this action."));
    question->setWordWrap(true);
    layout->addWidget(question);
    QLabel *count = new QLabel(widget);
    count->setText(tr("Selected %n plots", nullptr, plotIds.count()));
    count->setWordWrap(true);
    count->setStyleSheet("font-weight: bold");
    layout->addWidget(count);
    for (const auto &title: plotTitles)
    {
      QLabel *titleLabel = new QLabel(widget);
      titleLabel->setText(title);
      layout->addWidget(titleLabel);
    }
  }
  else
  {
    QLabel *question = new QLabel(widget);
    question->setText(tr("Are you sure you want to remove the plot “%1”? It will be impossible to cancel this action.").arg(plotTitles.first()));;
    question->setWordWrap(true);
    layout->addWidget(question);
  }
  auto dial = new Dialogs::Templates::Confirm(this, s, Btns::Ok | Btns::Cancel, widget);
  connect(dial, &WebDialog::Accepted, this, [this, plotIds, plotTitles]() {
    for (int i = 0; i < plotIds.size(); i++)
    {
      int plotId = plotIds[i];
      QString plotTitle = plotTitles[i];
      API::REST::deleteIdentificationPlot(plotId,
        [this, plotTitle](QNetworkReply*, QJsonDocument doc)
        {
          Notification::NotifySuccess(tr("Plot have been removed"),
                                      tr("Plot “%1” have been removed from project").arg(plotTitle));
          reloadModel();
        },
        [this](QNetworkReply*, QNetworkReply::NetworkError err)
        {
          Notification::NotifyError(tr("Failed to remove plot"), err);
        });

    }
  });
  dial->Open();
}

}
