#include "dialog_add_chromatogramms.h"
#include "qcombobox.h"
#include <genesis_style/style.h>
#include "logic/notification.h"
#include <QJsonArray>
#include <QLabel>
#include "qpushbutton.h"
#include "qsortfilterproxymodel.h"
#include "ui/dialogs/web_dialog_box_with_radio.h"
#include "ui/widgets/markup/readonly_simple_tree_model.h"
#include "ui_dialog_add_slave_chromatogramm.h"
//#include "ui_dialog_add_slave_chromatogramm.h"
#include <QStandardItemModel>
#include <logic/context_root.h>
#include <api/api_rest.h>
#include "logic/models/markup_reference_model.h"
#include "logic/service/service_locator.h"
#include "logic/service/ijson_serializer.h"
#include "logic/structures/common_structures.h"
#include <logic/known_context_tag_names.h>

//#include <ui_dialog_add_slave_chromatogramm.h>

using namespace GenesisMarkup;
using namespace Model;
using namespace Service;

DialogAddChromatogramms::DialogAddChromatogramms(QList<int> existedIds, int referenceId, QWidget *parent)
  : WebDialogStacked(parent)
  , mExistedIds(existedIds)
{

  mWebDialogBoxWithRadio = new WebDialogBoxWithRadio(nullptr,  tr("Adding chromatograms to the markup"));

  mDialogAddReferenceWithMarkup = new DialogAddReferenceWithMarkup(this, referenceId);

  mDialogAddSlaveChromatogramm = new DialogAddSlaveChromatogramm(this, mExistedIds);


  mWebDialogBoxWithRadio->makeRadioButtonsWithHandle({{tr("AddReferenceWithMarkup"), [this](){
                                                         mWebDialogBoxWithRadio->buttonBox()->button(Btns::Ok)->setDisabled(false); }},
                                                      {tr("AddSlaveChromatogramm"), [this](){
                                                         mWebDialogBoxWithRadio->buttonBox()->button(Btns::Ok)->setDisabled(false);
                                                       }} },
                                                     tr("Choose the type of files to be added"),

                                                     {[this](){switchToAddReferenceDialog();},
                                                      [this](){switchToAddSlaveDialog();} },
                                                     [this](){qDebug() << "REJECTED";
    cancelDialog();});
  mWebDialogBoxWithRadio->setAttribute(Qt::WA_DeleteOnClose, false);


  mStackedLayout->addWidget(mWebDialogBoxWithRadio);
  mStackedLayout->addWidget(mDialogAddReferenceWithMarkup);
  mStackedLayout->addWidget(mDialogAddSlaveChromatogramm);

  setLayout(mStackedLayout);
  mWebDialogBoxWithRadio->buttonBox()->button(Btns::Ok)->setDisabled(true);
}

int DialogAddChromatogramms::getReferenceId()
{
  return mDialogAddReferenceWithMarkup->getNewReferenceId();
}

int DialogAddChromatogramms::getReferenceFileId()
{
  return mDialogAddReferenceWithMarkup->getNewReferenceFileId();
}

QList<int> DialogAddChromatogramms::getIdList()
{
  return mDialogAddSlaveChromatogramm->getIdList();
}

GenesisMarkup::DialogAddChromatogramms::SelectionMode DialogAddChromatogramms::currentMode() const
{
  return mCurrentMode;
}


void DialogAddChromatogramms::acceptDialog()
{
  accept();
}

void DialogAddChromatogramms::cancelDialog()
{
  reject();
}

void DialogAddChromatogramms::switchToAddReferenceDialog()
{
  mCurrentMode = addReference;
  mStackedLayout->setCurrentWidget(mDialogAddReferenceWithMarkup);
}

void DialogAddChromatogramms::switchToAddSlaveDialog()
{
  mCurrentMode = addSlave;
  mStackedLayout->setCurrentWidget(mDialogAddSlaveChromatogramm);
}


GenesisMarkup::DialogAddReferenceWithMarkup::DialogAddReferenceWithMarkup(IDialogAddChromatogramms *interface, int referenceId, QWidget *parent)
  : WebDialog(parent, nullptr, QSizeF(), Btns::Ok | Btns::Cancel)
  , mIntrf(interface)
  , mReferenceId(referenceId)
{

  mBody = new QWidget(Content);
  Content->layout()->addWidget(mBody);

  mBodyLayout = new QVBoxLayout(mBody);
  mBodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
  mBodyLayout->setSpacing(0);

  auto captionLabel = new QLabel;
  captionLabel->setText("Adding reference to makrup");
  captionLabel->setStyleSheet(Style::Genesis::GetH1());
  mBodyLayout->addWidget(captionLabel);

  auto comboboxLable = new QLabel;
  comboboxLable->setText("Choose a reference to add for the current markup");
  mBodyLayout->addWidget(comboboxLable);

  mCombobox = new QComboBox;
  mCombobox->setPlaceholderText("Choose a reference");

  mBodyLayout->addWidget(mCombobox);

  mBodyLayout->addStretch();
  loadFiles();

}

void DialogAddReferenceWithMarkup::Accept()
{
  mIntrf->acceptDialog();
}

void DialogAddReferenceWithMarkup::Reject()
{
  mIntrf->cancelDialog();
}

void DialogAddReferenceWithMarkup::loadFiles()
{
  int projectId = Core::GenesisContextRoot::Get()->ProjectId();
  API::REST::GetProjectReferences(projectId,
                                  [this](QNetworkReply*, QJsonDocument doc)
  {
    //MarkupReferenceModelJsonSerializer
    const auto serializer = ServiceLocator::Instance().Resolve<MarkupReferenceModel, IJsonSerializer>();
    const auto model = serializer->ToModel(doc.object());
    auto references = model->Get<Structures::ReferenceList>(MarkupReferenceModel::ReferenceList);
    mCombobox->clear();
    mLoadedReferenceIds.clear();
    for(auto& ref : references)    {

      mLoadedReferenceIds.insert(ref.Id, QString::fromStdString(ref.Title));
      if (mCombobox){
        mCombobox->addItem(QString("(%1)%2").arg(ref.Id).arg(QString::fromStdString(ref.Title)));
        mCombobox->setItemData(mCombobox->count()-1, (int)ref.Id, ReferenceIdRole);
        mCombobox->setItemData(mCombobox->count()-1, (int)ref.ProjectFileId, ReferenceFileIdRole);
      }
    }
  },
  [](QNetworkReply*, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Error receive reference data."), err);
  });
}

int DialogAddReferenceWithMarkup::getNewReferenceId() const
{
  if(mCombobox->currentIndex() == -1)
    return -1;
  return mCombobox->currentData(ComboRoles::ReferenceIdRole).toInt();
}

int DialogAddReferenceWithMarkup::getNewReferenceFileId() const
{

  if(mCombobox->currentIndex() == -1)
    return -1;
  return mCombobox->currentData(ComboRoles::ReferenceFileIdRole).toInt();
}

DialogAddSlaveChromatogramm::DialogAddSlaveChromatogramm(IDialogAddChromatogramms *interface, QList<int> existedIds, QWidget *parent)
  : WebDialog(parent, nullptr, QSizeF(), Btns::Ok | Btns::Cancel)
  , mIntrf(interface)
  , ui(new Ui::DialogAddSlaveChromatogramm),
    mExistedIds(existedIds)
{
  //  ui->setupUi(this);
  setupUI();
  initTreeView();
}

DialogAddSlaveChromatogramm::~DialogAddSlaveChromatogramm()
{
  delete ui;
}

void DialogAddSlaveChromatogramm::initTreeView()
{
  mModel = new QStandardItemModel(this);
  loadTreeModelData(mModel);
  connect(mModel, &QStandardItemModel::itemChanged, this, &DialogAddSlaveChromatogramm::checkboxingChildren);
  mProxy = new QSortFilterProxyModel(mModel); //just QObject parent
  mProxy->setRecursiveFilteringEnabled(true);
  mProxy->setSourceModel(mModel);
  ui->treeView->setModel(mProxy);
  ui->treeView->setRootIsDecorated(false);
  ui->treeView->setHeaderHidden(true);

  connect(ui->checkBox, &QCheckBox::stateChanged,
          [this](int state)
  {
    onModelInvalidation = true;
    auto root = mModel->invisibleRootItem();
    for(int row = 0; row < root->rowCount(); row++)
    {
      if(state == Qt::Checked)
        root->child(row)->setData(Qt::Checked, Qt::CheckStateRole);
      else if(state == Qt::Unchecked)
        root->child(row)->setData(Qt::Unchecked, Qt::CheckStateRole);
    }
    onModelInvalidation = false;
  });

  connect(mModel, &QStandardItemModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>())
  {
    if(onModelInvalidation)
      return;
    auto root = mModel->invisibleRootItem();
    Qt::CheckState newState = Qt::Unchecked;
    for(int row = 0; row < root->rowCount(); row++)
    {
      auto item = root->child(row);
      switch(newState)
      {
      case Qt::Unchecked:
      {
        auto itemState = item->data(Qt::CheckStateRole).toInt();
        switch((Qt::CheckState)itemState)
        {
        case Qt::Unchecked:
          break;//do nothing
        case Qt::PartiallyChecked:
          newState = Qt::PartiallyChecked;
          break;//call and finish
        case Qt::Checked:
          newState = Qt::Checked;
          break;
        }
        break;
      }
      case Qt::PartiallyChecked:
        newState = Qt::PartiallyChecked;
        break;//call and finish
      case Qt::Checked:
        auto itemState = item->data(Qt::CheckStateRole).toInt();
        switch((Qt::CheckState)itemState)
        {
        case Qt::Unchecked:
          newState = Qt::PartiallyChecked;
          break;//call and finish
        case Qt::PartiallyChecked:
          newState = Qt::PartiallyChecked;
          break;//call and finish
        case Qt::Checked:
          break;//doNothing
        }
        break;
      }
      if(newState == Qt::PartiallyChecked)
        break;
    }
    ui->checkBox->blockSignals(true);
    ui->checkBox->setCheckState(newState);
    ui->checkBox->blockSignals(false);
  });

  ui->treeView->adjustSize();
}

QList<int> DialogAddSlaveChromatogramm::getIdList()
{
  auto modelRoot = mModel->invisibleRootItem();
  QList<int> idList;
  getChildIdList(modelRoot, idList);
  return idList;
}

void DialogAddSlaveChromatogramm::loadTreeModelData(QStandardItemModel* model)
{
  model->clear();
  int projectId = Core::GenesisContextRoot::Get()->ProjectId();
  //  int projectId = 230;
  API::REST::Tables::GetProjectFiles(projectId,
                                     [this, model](QNetworkReply*, QVariantMap data)
  {
    auto modelRoot = model->invisibleRootItem();
    QMap<QString, QStandardItem*> groupRootsMap;
    auto jchildren = data["children"].toJsonArray();
    for(int i = 0; i < jchildren.size(); i++)
    {
      QString name;
      QString groupName;
      int id;
      auto jitem = jchildren[i].toObject();

      id = jitem["project_file_id"].toInt();
      if(mExistedIds.contains(id))
        continue;

      if(!jitem["title"].isNull())
        name = jitem["title"].toString();
      else
        name = jitem["filename"].toString().split('/').last();
      if(!jitem["group_title"].isNull())
        groupName = jitem["group_title"].toString();

      auto item = new ReadOnlySimpleTreeItem(name, id);
      //      item->setData(id, Qt::UserRole);

      if(!groupName.isEmpty())
      {
        if(groupRootsMap.contains(groupName))
        {
          auto groupItem = groupRootsMap[groupName];
          groupItem->setChild(groupItem->rowCount(), item);
        }
        else
        {
          auto groupItem = new ReadOnlySimpleTreeItem(groupName + ":", -1);
          groupRootsMap[groupName] = groupItem;
          modelRoot->setChild(modelRoot->rowCount(), groupItem);
          groupItem->setChild(groupItem->rowCount(), item);
        }
      }
      else
      {
        modelRoot->setChild(modelRoot->rowCount(), item);
      }
    }
    ui->treeView->expandAll();
  },
  [](QNetworkReply*, QNetworkReply::NetworkError e)
  {
    Notification::NotifyError(tr("Failed to load project files: "), e);
  });
}


void DialogAddSlaveChromatogramm::checkboxingChildren(QStandardItem* item)
{
  auto rootCheckState = item->data(Qt::CheckStateRole);
  if(rootCheckState == Qt::Checked)
  {
    for(int row = 0; row < item->rowCount(); row++)
    {
      item->child(row)->setData(Qt::Checked, Qt::CheckStateRole);
    }
  }
  else if(rootCheckState == Qt::Unchecked)
  {
    for(int row = 0; row < item->rowCount(); row++)
    {
      item->child(row)->setData(Qt::Unchecked, Qt::CheckStateRole);
    }
  }
  //  auto rootItemClicked = mModel->itemFromIndex(index);

  //  if (rootItemClicked->checkState() == Qt::CheckState::Unchecked){
  //    rootItemClicked->setCheckState(Qt::CheckState::Checked);
  //    checkboxingAllChildren(rootItemClicked, Qt::CheckState::Checked);
  //  } else{

  //    rootItemClicked->setCheckState(Qt::CheckState::Unchecked);
  //    checkboxingAllChildren(rootItemClicked, Qt::CheckState::Unchecked);
  //  }
}

void DialogAddSlaveChromatogramm::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  //  auto list = ui->treeView->selectionModel()->selectedIndexes();
  //  for (auto &modelIndex: list){
  //    auto checkBoxState = ui->checkBox->checkState();
  //    auto item = mModel->itemFromIndex(modelIndex);
  //    if (checkBoxState == Qt::CheckState::Checked){
  //      selectAllChildren(item, Qt::CheckState::Checked);
  //    } else {
  //      selectAllChildren(item, Qt::CheckState::Unchecked);
  //    }
  //  }
}

void DialogAddSlaveChromatogramm::checkboxingAllChildren(QStandardItem *child, Qt::CheckState state)
{
  for (int i=0; i< child->rowCount(); ++i){
    child->child(i)->setCheckState(state);
    if (!child->child(i)->hasChildren()){
      continue;
    }
    checkboxingAllChildren(child->child(i), state);
  }
}

void DialogAddSlaveChromatogramm::selectAllChildrenMIndex(const QModelIndex &index, Qt::CheckState state)
{

  ui->treeView->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::Select);

}

void DialogAddSlaveChromatogramm::getChildIdList(QStandardItem *child, QList<int> &ids)
{
  int id = child->data(Qt::UserRole).toInt();
  if(id != -1/*group item*/ && id != 0/*invalid id data*/
     && child->checkState() == Qt::Checked)
    ids << id;

  if(child->hasChildren())
    for(int row = 0; row < child->rowCount(); row++)
      getChildIdList(child->child(row), ids);
}

void DialogAddSlaveChromatogramm::setupUI()
{
  auto body = new QWidget(this);
  ui->setupUi(body);
  Content->layout()->addWidget(body);

  if(ButtonLayout->count() > 2)
  {
    auto firstStretch = ButtonLayout->itemAt(2)->spacerItem();
    if (firstStretch){
      ButtonLayout->takeAt(2);
    }
  }

  auto chosenCountLable = new QLabel(tr("Chosen "));
  auto chosenCountLableData = new QLabel("0/0");

  ButtonLayout->insertWidget(0, chosenCountLable);
  ButtonLayout->insertWidget(1, chosenCountLableData);

  body->setStyleSheet(Style::Genesis::GetUiStyle());
  ui->captionLabel->setStyleSheet(Style::Genesis::GetH1());
  //  ui->treeView->setStyleSheet(Style::Genesis::GetTreeViewStyle());
  ui->treeView->setStyleSheet("QTreeView::item {border: none;} QTreeView::branch {border: none; image: none; border-image:none; background: none;}");

  connect(ui->lineEdit, &QLineEdit::textEdited, this, [this](const QString& s)
  {
    mProxy->setFilterFixedString(s);
    ui->treeView->expandAll();
  });
}

void DialogAddSlaveChromatogramm::Accept()
{
  mIntrf->acceptDialog();
}

void DialogAddSlaveChromatogramm::Reject()
{
  mIntrf->cancelDialog();
}


