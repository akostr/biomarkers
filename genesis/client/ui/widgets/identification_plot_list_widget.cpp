#include "identification_plot_list_widget.h"
#include "qjsondocument.h"
#include "ui_identification_plot_list_widget.h"
#include "genesis_style/style.h"

#include "identification_plot_widget.h"
#include "ui/genesis_window.h"
#include "ui/known_view_names.h"
#include "edit_group_widget.h"
#include "logic/tree_model_dynamic_library_templates.h"
#include "logic/tree_model_item.h"
#include "logic/context_root.h"
#include "logic/enums.h"
#include "logic/known_context_tag_names.h"
#include "api/api_rest.h"
#include "ui/controls/dialog_line_edit_ex.h"
#include "ui/controls/dialog_text_edit_ex.h"
#include "ui/dialogs/templates/dialog.h"
#include "ui/dialogs/templates/confirm.h"
#include "logic/models/reference_analysis_data_table_model.h"
#include "ui/item_models/analysis_data_table_model.h"

#include <QMenu>
#include <QAction>
#include <QStyle>
#include <QStackedWidget>
#include <QFileDialog>

#include <QJsonObject>
namespace  {
const int minimalSpacerWidgetHeight = 100;
}

IdentificationPlotListWidget::IdentificationPlotListWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::IdentificationPlotListWidget)
{
  setupUi();
  setupConnections();
}

IdentificationPlotListWidget::~IdentificationPlotListWidget()
{
  delete ui;
}

void IdentificationPlotListWidget::setupUi()
{
  setStyleSheet(Style::Genesis::GetUiStyle());
  ui->setupUi(this);
  ui->allPlotsCheckBox->setVisible(false);
  ui->exportButton->setVisible(false);
  ui->saveButton->setVisible(false);
  ui->cancelButton->setVisible(false);

  mMenu = new QMenu(this);
  mCreateAction = mMenu->addAction(tr("Save"));
  mSaveAction = mMenu->addAction(tr("Save changes"));
  mSaveAsNewAction = mMenu->addAction(tr("Save as new plot"));
  mEditTitleAction = mMenu->addAction(tr("Edit title and comment"));
  mExportAction = mMenu->addAction(tr("Export"));
  mRemoveAction = mMenu->addAction(tr("Remove"));

  ui->managePlotButton->setMenu(mMenu);
  ui->managePlotButton->style()->polish(ui->managePlotButton);

  ui->backButton->setFlat(true);

  ui->captionLabel->setStyleSheet(Style::Genesis::Fonts::H2());
  ui->mainWidget->setAttribute(Qt::WA_StyledBackground, true);
  ui->mainWidget->setStyleSheet("background: white;");
  setMode(OpenMode);
}

void IdentificationPlotListWidget::setupConnections()
{
  connect(mCreateAction, &QAction::triggered, this, &IdentificationPlotListWidget::beginSaveMode);
  connect(mExportAction, &QAction::triggered, this, &IdentificationPlotListWidget::beginExportMode);

  connect(mSaveAction, &QAction::triggered, this, &IdentificationPlotListWidget::saveChanges);
  connect(mSaveAsNewAction, &QAction::triggered, this, &IdentificationPlotListWidget::beginSaveMode);
  connect(mEditTitleAction, &QAction::triggered, this, &IdentificationPlotListWidget::openEditTitleDialog);
  connect(mRemoveAction, &QAction::triggered, this, &IdentificationPlotListWidget::remove);

  connect(ui->saveButton, &QPushButton::clicked, this, &IdentificationPlotListWidget::savePlots);
  connect(ui->exportButton, &QPushButton::clicked, this, &IdentificationPlotListWidget::exportPlots);
  connect(ui->cancelButton, &QPushButton::clicked, this, &IdentificationPlotListWidget::leaveMode);
  connect(ui->backButton, &QPushButton::clicked, this, &IdentificationPlotListWidget::showPreviousPage);
  connect(ui->allPlotsCheckBox, &QCheckBox::clicked, this, &IdentificationPlotListWidget::selectAll);
  connect(ui->plotListSplitter, &QSplitter::splitterMoved, this, &IdentificationPlotListWidget::updateSplitter);
}

void IdentificationPlotListWidget::createPlots(QList<int> plotTemplateIds,
                                          QPointer<TreeModelDynamicLibraryTemplates> templateListModel,
                                          int tableId,
                                          QString dataType)
{
  setMode(CreateMode);
  mTableId = tableId;
  QList<IdentificationPlotWidget*> needToRemove;
  for (IdentificationPlotWidget *plot: mPlotList)
  {
    int id = plot->getTemplateId();
    if (plotTemplateIds.contains(id))
    {
      plotTemplateIds.removeAll(id);
    }
    else
    {
      needToRemove.append(plot);
    }
  }

  for (auto plotWidget: needToRemove)
  {
    plotWidget->deleteLater();
    int index = mPlotList.indexOf(plotWidget);
    mPlotList.remove(index);
    mSplitterSized.remove(index);
  }

  for (int templateId: plotTemplateIds)
  {
    IdentificationPlotWidget *plot = new IdentificationPlotWidget(ui->plotListSplitter);
    connect (plot, &IdentificationPlotWidget::selectChanged, this, &IdentificationPlotListWidget::selectedItemChanged);
    connect (plot, &IdentificationPlotWidget::needToUpdateHeight, this, &IdentificationPlotListWidget::updateHeight);
    mPlotList.append(plot);
    plot->setData(templateId,
                  templateListModel,
                  tableId,
                  dataType);
    ui->plotListSplitter->insertWidget(plotTemplateIds.indexOf(templateId), plot);
  }
  checkSizes();
}

void IdentificationPlotListWidget::openPlot(int plotId)
{
  API::REST::LoadPlot(plotId,
    [plotId, this](QNetworkReply*, QJsonDocument result)
    {
      auto obj = result.object();
      openPlot(obj["plot_info"].toObject());
      emit loadedPlot(plotId, obj);
      Notification::NotifySuccess(tr("Success"), tr("Selected plot loaded"));
    },
    [this](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to load plot"), err);
      for (IdentificationPlotWidget* plotWidget: mPlotList)
      {
        plotWidget->deleteLater();
      }
      mPlotList.clear();
      mSplitterSized.clear();
      emit showPreviousPage();
    });
}

void IdentificationPlotListWidget::openPlot(const QJsonObject &obj)
{
  setMode(OpenMode);
  int projectId = obj["project_id"].toInt();
  mTableId = obj["table_id"].toInt();
  QJsonObject legendSettings = obj["legend_settings"].toObject();
  QJsonArray items = obj["plots"].toArray();
  mGroupId= obj["plot_group_id"].toInt();
  mGroupTitle = QString();

  if (obj.contains("plot_group_id"))
    mGroupId = obj["plot_group_id"].toInt();
  else
    mGroupId = -1;

  for (IdentificationPlotWidget* plotWidget: mPlotList)
  {
    plotWidget->deleteLater();
  }
  mPlotList.clear();
  QJsonObject item = items.first().toObject();
  IdentificationPlotWidget *plot = new IdentificationPlotWidget(ui->plotListSplitter);
  plot->setData(item);
  plot->setSelected(true);
  ui->plotListSplitter->insertWidget(0, plot);
  mSplitterSized.clear();
  checkSizes();
  ui->captionLabel->setText(plot->title());
  connect(plot, &IdentificationPlotWidget::titleChanged, ui->captionLabel, &QLabel::setText);
  connect (plot, &IdentificationPlotWidget::selectChanged, this, &IdentificationPlotListWidget::selectedItemChanged);
  connect (plot, &IdentificationPlotWidget::needToUpdateHeight, this, &IdentificationPlotListWidget::updateHeight);
  mPlotList.append(plot);
}

void IdentificationPlotListWidget::clear()
{
  for (auto plotWidget: mPlotList)
  {
    plotWidget->deleteLater();
  }
  mPlotList.clear();
  mSplitterSized.clear();
  leaveMode();
  ui->captionLabel->setText(tr("Create plots"));
}

int IdentificationPlotListWidget::plotCount()
{
  return mPlotList.size();
}

QJsonObject IdentificationPlotListWidget::generateJson(bool ignorePlotId)
{
  int projectId = Core::GenesisContextRoot::Get()->ProjectId();
  int tableId = mTableId;

  QString groubTitle = mGroupTitle;
  int groupId = mGroupId;
  bool needToCreateGroup = !groubTitle.isEmpty() && groupId <= 0;
  QJsonObject legendSettings;

  QJsonObject obj;
  obj.insert("table_id", tableId);
  obj.insert("project_id", projectId);
  obj.insert("legend_settings", legendSettings);
  obj.insert("create_group", needToCreateGroup);
  if (needToCreateGroup)
  {
    obj.insert("group_title", groubTitle);
  }
  else
  {
    if (groupId > 0)
      obj.insert("plot_group_id", groupId);
  }

  QJsonArray items;
  for (IdentificationPlotWidget *plot: mPlotList)
  {
    if (plot->isSelected())
      items.append(plot->saveToJson(ignorePlotId));
  }
  obj.insert("plots", items);
  return obj;
}

void IdentificationPlotListWidget::saveChanges()
{
  if (mPlotList.size() != 1)
    return;
  mPlotList.first()->setSelected(true);
  mSaveAsNew = false;
  selectAll(true);

  if (mPlotList.size() != 1)
    return;

  int plotId = mPlotList.first()->getPlotId();
  if (mPlotList.first()->getPlotId() <= 0)
    return;

  QString plotTitle = mPlotList.first()->title();
  using B = QDialogButtonBox::StandardButton;
  Dialogs::Templates::Confirm::Settings s;
  s.dialogHeader = tr("Saving");
  s.buttonsNames[B::Ok] = tr("Save");
  s.buttonsProperties[B::Ok] = {{"blue", true}};
  s.buttonsNames[B::No] = tr("Don't save");
  s.buttonsProperties[B::No] = {{"red", true}};
  auto content = new QLabel(tr("Do you want to save changes in plot “%1”?").arg(plotTitle));
  content->setWordWrap(true);
  auto dial = new Dialogs::Templates::Confirm(this, s, B::Ok | B::Cancel, content);
  connect(dial, &WebDialog::Accepted, this, [this, plotTitle]() {
    QJsonObject obj = generateJson(false);
    API::REST::SavePlot(obj,
      [this, plotTitle](QNetworkReply*, QJsonDocument doc)
      {
        Notification::NotifySuccess(tr("Changes were saved successfully"), tr("Changes in plot “%1” have been successfully saved").arg(plotTitle));
        emit needToReloadTable();
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Failed to save plot"), err);
      });
  });
  dial->Open();
}

void IdentificationPlotListWidget::beginSaveMode()
{
  mSaveAsNew = true;
  selectAll(true);
  if (mPlotList.size() == 1)
  {
    savePlots();
  }
  else
  {
    showCheckBoxes(true);
    ui->saveButton->setVisible(true);
    ui->cancelButton->setVisible(true);
  }
}

void IdentificationPlotListWidget::savePlots()
{
  if (mSaveAsNew)
  {
    openSaveAsNewDialog();
  }

  leaveMode();
}

void IdentificationPlotListWidget::beginExportMode()
{
  selectAll(true);
  if (mPlotList.size() == 1)
  {
    exportPlots();
  }
  else
  {
    ui->exportButton->setVisible(true);
    ui->cancelButton->setVisible(true);
    showCheckBoxes(true);
  }
}

void IdentificationPlotListWidget::exportPlots()
{
  auto path = QFileDialog::getExistingDirectory(this, tr("Open folder"), QDir::currentPath(),
                                                QFileDialog::ReadOnly | QFileDialog::ShowDirsOnly);
  path = QDir::toNativeSeparators(path);
  if (path.isEmpty())
  QString path;
  bool isOk = true;
  for (IdentificationPlotWidget *plot: mPlotList)
  {
    if (plot->isSelected())
    {
      isOk &= plot->saveToFileSystem(path);
    }
  }
  leaveMode();
  if (isOk)
  {
    Notification::NotifySuccess(tr("Export success"),
                                tr("Selected plots have been exported"));
  }
  else
  {
    Notification::NotifyError(tr("Export error"), tr("Export error"));
  }
}

void IdentificationPlotListWidget::leaveMode()
{
  showCheckBoxes(false);
  ui->exportButton->setVisible(false);
  ui->saveButton->setVisible(false);
  ui->cancelButton->setVisible(false);
}

void IdentificationPlotListWidget::showCheckBoxes(bool isShow)
{
  ui->allPlotsCheckBox->setVisible(isShow);

  for (IdentificationPlotWidget *plot: mPlotList)
  {
    plot->setSelectable(isShow);
  }
}

void IdentificationPlotListWidget::setMode(Mode mode)
{
  mMode = mode;
  switch (mMode)
  {
  case OpenMode:
    mCreateAction->setVisible(false);
    mSaveAction->setVisible(true);
    mSaveAsNewAction->setVisible(true);
    mEditTitleAction->setVisible(true);
    mRemoveAction->setVisible(true);
    break;
  case CreateMode:
    mCreateAction->setVisible(true);
    mSaveAction->setVisible(false);
    mSaveAsNewAction->setVisible(false);
    mEditTitleAction->setVisible(false);
    mRemoveAction->setVisible(false);
    break;
  }
}

void IdentificationPlotListWidget::loadEntity(int tableId, QList<int> libraryGroupIds)
{

}

void IdentificationPlotListWidget::remove()
{
  if (mPlotList.size() != 1)
    return;

  int plotId = mPlotList.first()->getPlotId();
  if (mPlotList.first()->getPlotId() <= 0)
    return;

  QString plotTitle = mPlotList.first()->title();
  using B = QDialogButtonBox::StandardButton;
  Dialogs::Templates::Confirm::Settings s;
  s.dialogHeader = tr("Deleting");
  s.buttonsNames[B::Ok] = tr("Delete");
  s.buttonsProperties[B::Ok] = {{"red", true}};
  auto content = new QLabel(tr("Are you sure you want to delete the plot “%1”? It will be impossible to cancel this action.").arg(plotTitle));
  content->setWordWrap(true);
  auto dial = new Dialogs::Templates::Confirm(this, s, B::Ok | B::Cancel, content);
  connect(dial, &WebDialog::Accepted, this, [this, plotId, plotTitle]() {
    API::REST::deleteIdentificationPlot(plotId,
      [this, plotTitle](QNetworkReply*, QJsonDocument doc)
      {
        Notification::NotifySuccess(tr("Plot have been deleted"),
                                    tr("Plot “%1” have been deleted from project").arg(plotTitle));
        emit needToReloadTable();
      },
      [this](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Failed to delete plot"), err);
      });
  });
  dial->Open();
}

void IdentificationPlotListWidget::openEditTitleDialog()
{
  selectAll(true);
  using Btns = QDialogButtonBox;
  auto s = Dialogs::Templates::Dialog::Settings();
  s.dialogHeader = tr("Editing");
  QList<std::tuple <IdentificationPlotWidget*, DialogLineEditEx*, DialogTextEditEx*>> controls;
  QWidget *titleWidget = createTitleWidgetForSaveDialog(controls);
  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setWidget(titleWidget);
  auto dial = new Dialogs::Templates::Dialog(this, s, Btns::Cancel | Btns::Save, titleWidget);

  for (auto control: controls)
  {
    connect(std::get<DialogLineEditEx*>(control), &DialogLineEditEx::validityChanged, this, [controls, dial](bool) {
      bool valid = true;
      for (auto control: controls)
      {
        if (!std::get<DialogLineEditEx*>(control)->checkValidity())
        {
          valid = false;
          break;
        }
      }
      dial->SetButtonEnabled(QDialogButtonBox::Ok, valid);
    });
  }

  connect(dial, &WebDialog::Accepted, this, [this, controls]() {
    for (auto control: controls)
    {
      IdentificationPlotWidget* plotWidget = std::get <IdentificationPlotWidget*>(control);
      DialogLineEditEx* title = std::get <DialogLineEditEx*>(control);
      plotWidget->setTitle(title->text());
      DialogTextEditEx* comment = std::get <DialogTextEditEx*>(control);
      plotWidget->setComment(comment->text());
    }
  });
  dial->Open();
}

void IdentificationPlotListWidget::openSaveAsNewDialog()
{
  using Btns = QDialogButtonBox;
  auto s = Dialogs::Templates::Dialog::Settings();
  s.dialogHeader = tr("Saving");
  QStackedWidget *stackedWidget = new QStackedWidget();
  auto dial = new Dialogs::Templates::Dialog(this, s, Btns::Cancel, stackedWidget);

  QList<std::tuple <IdentificationPlotWidget*, DialogLineEditEx*, DialogTextEditEx*>> controls;
  QWidget *titleWidget = createTitleWidgetForSaveDialog(controls);
  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setWidget(titleWidget);
  QWidget *groupWidget = new QWidget(dial);
  groupWidget->setStyleSheet(Style::Genesis::GetUiStyle());
  groupWidget->setStyleSheet("background: white;");
  QVBoxLayout *groupLayout = new QVBoxLayout();
  groupWidget->setLayout(groupLayout);
  Widgets::EditGroupWidget *editGroupWidget = createGroupWidgetForSaveDialog();
  groupLayout->addWidget(editGroupWidget);
  groupLayout->addStretch(1);

  stackedWidget->addWidget(titleWidget);
  stackedWidget->addWidget(groupWidget);

  stackedWidget->setCurrentWidget(titleWidget);
  auto* buttonBoxLayout = dial->GetButtonBoxLayout();
  auto* box = dial->GetButtonBox();

  QPushButton *backButton = new QPushButton(QIcon(":/resource/icons/icon_button_lt.png"), tr("Previous step"), dial);
  buttonBoxLayout->insertWidget(0, backButton);
  buttonBoxLayout->insertStretch(1,10);
  backButton->setStyleSheet(Style::Genesis::GetUiStyle());


  QPushButton *nextButton = new QPushButton(tr("Save"), dial);
  QPushButton *saveButton = new QPushButton(tr("Save"), dial);

  box->addButton(nextButton, QDialogButtonBox::ActionRole);
  box->addButton(saveButton, QDialogButtonBox::AcceptRole);

  backButton->setVisible(false);
  saveButton->setVisible(false);
  nextButton->setVisible(true);
  stackedWidget->setCurrentWidget(titleWidget);

  connect (backButton, &QPushButton::clicked, this, [stackedWidget, titleWidget, backButton, saveButton, nextButton]{
    stackedWidget->setCurrentWidget(titleWidget);
    backButton->setVisible(false);
    saveButton->setVisible(false);
    nextButton->setVisible(true);
  });
  connect (nextButton, &QPushButton::clicked, this, [stackedWidget, groupWidget, backButton, saveButton, nextButton]{
    stackedWidget->setCurrentWidget(groupWidget);
    backButton->setVisible(true);
    saveButton->setVisible(true);
    nextButton->setVisible(false);
  });

  auto disablOkButton = [dial, editGroupWidget]()
  {
    dial->SetButtonEnabled(QDialogButtonBox::Ok, !(editGroupWidget->IsGroupChecked() && editGroupWidget->GroupTitle().isEmpty()));
  };
  connect(editGroupWidget, &Widgets::EditGroupWidget::groupComboTextChanged, this, disablOkButton);
  connect(editGroupWidget, &Widgets::EditGroupWidget::ungroupRadioClicked, this, disablOkButton);

  for (auto control: controls)
  {
    connect(std::get<DialogLineEditEx*>(control), &DialogLineEditEx::validityChanged, this, [controls, dial](bool) {
      bool valid = true;
      for (auto control: controls)
      {
        if (!std::get<DialogLineEditEx*>(control)->checkValidity())
        {
          valid = false;
          break;
        }
      }
      dial->SetButtonEnabled(QDialogButtonBox::Ok, valid);
    });
  }

  connect(dial, &WebDialog::Accepted, this, [this, controls, editGroupWidget]() {
    if (editGroupWidget->IsCommonChecked())
    {
      mGroupId = -1;
      mGroupTitle = QString();
    }
    else
    {
      mGroupTitle = editGroupWidget->GroupTitle();
      mGroupId = editGroupWidget->IsCreateNewGroup() ? -1: editGroupWidget->SelectedGroupId();
    }
    for (auto control: controls)
    {
      IdentificationPlotWidget* plotWidget = std::get <IdentificationPlotWidget*>(control);
      DialogLineEditEx* title = std::get <DialogLineEditEx*>(control);
      plotWidget->setTitle(title->text());
      DialogTextEditEx* comment = std::get <DialogTextEditEx*>(control);
      plotWidget->setComment(comment->text());
    }

    QJsonObject obj = generateJson(true);
    API::REST::SavePlot(obj,
      [this](QNetworkReply*, QJsonDocument doc)
      {
        if (mMode == OpenMode)
        {
          Notification::NotifySuccess(tr("The changes have been saved successfully"),
                                      tr("The changes in the plot “%1” have been successfully saved").arg(mPlotList.first()->title()));
        }
        else
        {
          if (mPlotList.size() == 1)
          {
            Notification::NotifySuccess(tr("The plot have been saved"),
                                        tr("The plot “%1” has been successfully saved in the “Plots” section").arg(mPlotList.first()->title()));
          }
          else
          {
            Notification::NotifySuccess(tr("The plots have been saved"),
                                        tr("The selected plots have been successfully saved in the “Plots” section"));
          }
        }
        emit needToReloadTable();
      },
      [this](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Failed to save plots", nullptr, mPlotList.size()), err);
      });
  });
  dial->Open();
}

QWidget* IdentificationPlotListWidget::createTitleWidgetForSaveDialog(
  QList<std::tuple <IdentificationPlotWidget*, DialogLineEditEx*, DialogTextEditEx*>> &controls)
{
  auto widget = new QWidget();
  widget->setStyleSheet(Style::Genesis::GetUiStyle());
  widget->setStyleSheet("background: white;");
  auto layout = new QVBoxLayout(widget);
  for (IdentificationPlotWidget* plotWidget: mPlotList)
  {
    if (!plotWidget->isSelected())
      continue;

    auto w = new QWidget();
    auto l = new QVBoxLayout(w);

    auto line = new QFrame(w);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    auto ls = DialogLineEditEx::Settings();
    ls.textHeader = tr("Plot title") + Style::GetInputAlert();
    ls.textMaxLen = 50;
    ls.textPlaceholder = tr("Type new plot title");
    ls.defaultText = plotWidget->title();
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
    ts.defaultText = plotWidget->comment();
    auto commentTextEdit = new DialogTextEditEx(w);
    commentTextEdit->applySettings(ts);
    l->addWidget(commentTextEdit);
    layout->addWidget(w);

    controls.append({plotWidget, titleLineEdit, commentTextEdit});
  }
  return widget;
}

Widgets::EditGroupWidget* IdentificationPlotListWidget::createGroupWidgetForSaveDialog()
{
  Widgets::EditGroupWidget *groupWidget = new Widgets::EditGroupWidget();

  groupWidget->SetCaptionText(tr("Select a place to save the chart", nullptr, mPlotList.size()));
  groupWidget->SetUngroupRadioText("To the general list of plots");
  groupWidget->SetGroupRadioText("To the plot group");
  groupWidget->layout()->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum));

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
      if (mGroupId > 0)
        groupWidget->SetSelectedGroupId(mGroupId);
    },
    [](QNetworkReply* r, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to load file groups information"), err);
    });

  return groupWidget;
}


void IdentificationPlotListWidget::selectAll(bool isChecked)
{
  for (IdentificationPlotWidget *plot: mPlotList)
  {
    plot->setSelected(isChecked);
  }
  ui->allPlotsCheckBox->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
}

void IdentificationPlotListWidget::selectedItemChanged()
{
  int uncheckedCount = 0;
  int checkedCount = 0;
  for (const auto &item: mPlotList)
  {
    if (item->isSelected())
      checkedCount++;
    else
      uncheckedCount++;
  }
  if (checkedCount > 0 && uncheckedCount > 0)
    ui->allPlotsCheckBox->setCheckState(Qt::PartiallyChecked);
  else if (checkedCount > 0)
    ui->allPlotsCheckBox->setCheckState(Qt::Checked);
  else
    ui->allPlotsCheckBox->setCheckState(Qt::Unchecked);
}

void IdentificationPlotListWidget::updateHeight(int newHeight, bool applyForAll)
{
  checkSizes();
  if (applyForAll)
  {
    for (int index = 0; index < mPlotList.count(); index++)
    {
      mSplitterSized[index] = newHeight;
    }
    mSplitterSized.last() = (minimalSpacerWidgetHeight);
  }
  else
  {
    IdentificationPlotWidget *widget = qobject_cast<IdentificationPlotWidget *>(sender());
    if (!widget)
      return;
    int index = mPlotList.indexOf(widget);
    mSplitterSized[index] = newHeight;
    int totalSize = 0;
    for (int size: mSplitterSized)
    {
      totalSize += size;
    }
  }
  updateSizes();
}

void IdentificationPlotListWidget::resizeEvent(QResizeEvent *event)
{
  mSplitterSized = ui->plotListSplitter->sizes();
  QWidget::resizeEvent(event);
}

void IdentificationPlotListWidget::updateSplitter(int pos, int index)
{
  checkSizes();
  if (index > 0)
    index--;
  QList<int> newSizes = ui->plotListSplitter->sizes();
  mSplitterSized[index] = newSizes[index];
  mSplitterSized.last() = minimalSpacerWidgetHeight;
  updateSizes();
}

void IdentificationPlotListWidget::updateSizes()
{
  int totalSize = 0;
  for (int size: mSplitterSized)
  {
    totalSize += size;
  }
  ui->scrollAreaWidgetContents->setFixedHeight(totalSize +
                                               (mSplitterSized.count() - 1) * ui->plotListSplitter->handleWidth());
  ui->plotListSplitter->setSizes(mSplitterSized);
}

void IdentificationPlotListWidget::checkSizes()
{
  bool needToChange = false;
  if (mSplitterSized.count() != ui->plotListSplitter->count())
  {
    needToChange = true;
  }
  else
  {
    for (auto size: mSplitterSized)
    {
      if (size <= 0)
      {
        needToChange = true;
        break;
      }
    }
  }
  if (needToChange)
    mSplitterSized = ui->plotListSplitter->sizes();
}
