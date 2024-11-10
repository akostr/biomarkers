#include "dialog_add_slave_chromatogramm.h"
#include "ui_dialog_add_slave_chromatogramm.h"
#include <genesis_style/style.h>
#include "logic/notification.h"
#include "ui/dialogs/web_dialog_box_with_radio.h"
#include <QStackedLayout>
#include <QRadioButton>
#include <QLayout>
#include <QComboBox>
#include <QStandardItemModel>
#include <QPushButton>
#include <ui/itemviews/datatable_item_delegate.h>
#include <ui/itemviews/files_table_actions_column_delegate.h>
#include <logic/tree_model_dynamic_project_chromatogramms.h>
#include "readonly_simple_tree_model.h"
#include <api/api_rest.h>
#include <QTreeView>
#include <logic/context_root.h>
#include <logic/known_context_tag_names.h>

class DialogAddReferenceWithMarkupOld: public WebDialog
{
  Q_OBJECT
public:
  explicit DialogAddReferenceWithMarkupOld(QWidget *parent = nullptr);

//  void switch

protected:

  QPointer<QWidget> mBody;
  QPointer<QVBoxLayout> mBodyLayout;
};

namespace Ui {
class DialogAddSlaveChromatogramm;
}

class DialogAddSlaveChromatogrammOld : public WebDialog
{
  Q_OBJECT

public:
  explicit DialogAddSlaveChromatogrammOld(QList<int> existedIds, QWidget *parent = nullptr);
  ~DialogAddSlaveChromatogrammOld();

  void initTreeView();
  QList<int> getIdList();
  void loadTreeModelData(QStandardItemModel *model);

  void checkboxingChildren(QStandardItem *item);

  void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  void checkboxingAllChildren(QStandardItem * child, Qt::CheckState state);
  void selectAllChildrenMIndex(const QModelIndex &index, Qt::CheckState state);

private:
  void getChildIdList(QStandardItem *child, QList<int>& ids);
  void setupUI();

private:
  QPointer<QStandardItemModel> mModel;
  QPointer<QSortFilterProxyModel> mProxy;
  bool onModelInvalidation = false;
  Ui::DialogAddSlaveChromatogramm *ui;

  QList<int> mExistedIds;
};
using Btns = QDialogButtonBox::StandardButton;

DialogAddSlaveChromatogrammOld::DialogAddSlaveChromatogrammOld(QList<int> existedIds, QWidget *parent)
  : WebDialog(parent, nullptr, QSizeF(), Btns::Ok | Btns::Cancel),
    ui(new Ui::DialogAddSlaveChromatogramm),
    mExistedIds(existedIds)
{
//  ui->setupUi(this);
  setupUI();
  initTreeView();
}

DialogAddSlaveChromatogrammOld::~DialogAddSlaveChromatogrammOld()
{
  delete ui;
}

void DialogAddSlaveChromatogrammOld::initTreeView()
{
  mModel = new QStandardItemModel(this);
  loadTreeModelData(mModel);
  connect(mModel, &QStandardItemModel::itemChanged, this, &DialogAddSlaveChromatogrammOld::checkboxingChildren);
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

QList<int> DialogAddSlaveChromatogrammOld::getIdList()
{
  auto modelRoot = mModel->invisibleRootItem();
  QList<int> idList;
  getChildIdList(modelRoot, idList);
  return idList;
}

void DialogAddSlaveChromatogrammOld::loadTreeModelData(QStandardItemModel* model)
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


void DialogAddSlaveChromatogrammOld::checkboxingChildren(QStandardItem* item)
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

void DialogAddSlaveChromatogrammOld::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
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

void DialogAddSlaveChromatogrammOld::checkboxingAllChildren(QStandardItem *child, Qt::CheckState state)
{
  for (int i=0; i< child->rowCount(); ++i){
    child->child(i)->setCheckState(state);
    if (!child->child(i)->hasChildren()){
      continue;
    }
    checkboxingAllChildren(child->child(i), state);
  }
}

void DialogAddSlaveChromatogrammOld::selectAllChildrenMIndex(const QModelIndex &index, Qt::CheckState state)
{

  ui->treeView->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::Select);

}

void DialogAddSlaveChromatogrammOld::getChildIdList(QStandardItem *child, QList<int> &ids)
{
  int id = child->data(Qt::UserRole).toInt();
  if(id != -1/*group item*/ && id != 0/*invalid id data*/
     && child->checkState() == Qt::Checked)
    ids << id;

  if(child->hasChildren())
    for(int row = 0; row < child->rowCount(); row++)
      getChildIdList(child->child(row), ids);
}

void DialogAddSlaveChromatogrammOld::setupUI()
{
  auto body = new QWidget(this);
  ui->setupUi(body);
  Content->layout()->addWidget(body);

  auto firstStretch = ButtonLayout->itemAt(2)->spacerItem();
  if (firstStretch){
    ButtonLayout->takeAt(2);
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




DialogAddChromatogramm::DialogAddChromatogramm(QList<int> existedIds, QWidget *parent)
//  : QWidget(parent)
  : WebDialog(parent, nullptr, QSizeF(), Btns::NoButton)
  , mExistedIds(existedIds)
{
  setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);

//  connect(this, &WebDialog::Accepted,
//          this, &QWidget::close);

  mBody = new QWidget(Content);
  Content->layout()->addWidget(mBody);

  mBodyLayout = new QVBoxLayout(mBody);
  mBodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
  mBodyLayout->setSpacing(0);


  setAttribute(Qt::WA_DeleteOnClose);
  mStackedLayout = new QStackedLayout(mBodyLayout);

  mWebDialogBoxWithRadio = new WebDialogBoxWithRadio(nullptr,  tr("Adding chromatograms to the markup"));

  mDialogAddReferenceWithMarkup = new DialogAddReferenceWithMarkupOld;

  mDialogAddSlaveChromatogramm = new DialogAddSlaveChromatogrammOld(mExistedIds);

  auto radioList = mWebDialogBoxWithRadio->makeRadioButtonsWithHandle({{tr("AddReferenceWithMarkup"), [this](){
//                                                                            qDebug() << "AddReferenceWithMarkup selected";
                                                                            mWebDialogBoxWithRadio->buttonBox()->button(Btns::Ok)->setDisabled(false);
                                                                        }},
                                                                       {tr("AddSlaveChromatogramm"), [this](){
//                                                                            qDebug() << "AddSlaveChromatogramm selected";
                                                                            mWebDialogBoxWithRadio->buttonBox()->button(Btns::Ok)->setDisabled(false);

                                                                        }} },
                                                                      tr("Choose the type of files to be added"));

  radioList.first()->setDisabled(true);

  connect(mWebDialogBoxWithRadio, &WebDialog::Accepted,
          [this, radioList](){
    QPointer<QRadioButton> selectedBtn;

    for (auto &rButton: radioList){
      if (rButton->isChecked()){
        selectedBtn = rButton;
        break;
      }
    }
    if (radioList.indexOf(selectedBtn) == 0){ // first RadioButton Selected
//      switchToPageAddReferenceWithMarkup();
      return;
    }
    if (radioList.indexOf(selectedBtn) == 1){ // first RadioButton Selected
      switchToPageAddSlaveChromatogramm();
      return;
    }


  });

  connect(mWebDialogBoxWithRadio, &WebDialog::Rejected,
          this, &DialogAddChromatogramm::close);
  connect(mDialogAddSlaveChromatogramm, &WebDialog::Rejected,
          this, &DialogAddChromatogramm::close);
  connect(mDialogAddReferenceWithMarkup, &WebDialog::Rejected,
          this, &DialogAddChromatogramm::close);

  connect(mDialogAddSlaveChromatogramm, &WebDialog::Accepted,
          this, &DialogAddChromatogramm::onAcceptedPageAddSlaveChromatogramm);



  mStackedLayout->addWidget(mWebDialogBoxWithRadio);
  mStackedLayout->addWidget(mDialogAddReferenceWithMarkup);
  mStackedLayout->addWidget(mDialogAddSlaveChromatogramm);

//  mDialogAddReferenceWithMarkup->hide();
//  mDialogAddSlaveChromatogramm->hide();





  setLayout(mStackedLayout);
  mWebDialogBoxWithRadio->buttonBox()->button(Btns::Ok)->setDisabled(true);

}

QList<int> DialogAddChromatogramm::getIdList()
{
  return mDialogAddSlaveChromatogramm->getIdList();
}

void DialogAddChromatogramm::switchToPageAddReferenceWithMarkup()
{
  mStackedLayout->setCurrentWidget(mDialogAddReferenceWithMarkup);
}

void DialogAddChromatogramm::switchToPageAddSlaveChromatogramm()
{
  mStackedLayout->setCurrentWidget(mDialogAddSlaveChromatogramm);
}

void DialogAddChromatogramm::onAcceptedPageAddSlaveChromatogramm()
{
  Accept();
}


DialogAddReferenceWithMarkupOld::DialogAddReferenceWithMarkupOld(QWidget *parent)
  : WebDialog(parent, nullptr, QSizeF(), Btns::Ok | Btns::Cancel)
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

  auto cBox = new QComboBox;
  cBox->setPlaceholderText("Choose a reference");

  mBodyLayout->addWidget(cBox);

  mBodyLayout->addStretch();
}



//SimpleCheckBoxItemDelegate::SimpleCheckBoxItemDelegate(QObject *parent)
//  : QStyledItemDelegate(parent)
//{

//}

//QWidget *SimpleCheckBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//  auto checkBox = new QCheckBox(parent);
//  return checkBox;

//}

//void SimpleCheckBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
//{
//  QCheckBox *cb = qobject_cast<QCheckBox *>(editor);
//  cb->setChecked(index.data().toBool());

//}

//void SimpleCheckBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
//{
//  QCheckBox *cb = static_cast<QCheckBox *>(editor);
//  int value = (cb->checkState()==Qt::Checked)? 1 : 0;
//  model->setData(index, value, Qt::EditRole);
//}

//void SimpleCheckBoxItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//  QStyleOptionButton checkboxstyle;
//  QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);

//  //Центрирование
//  checkboxstyle.rect = option.rect;
//  checkboxstyle.rect.setLeft(option.rect.x() +
//                             option.rect.width()/2 - checkbox_rect.width()/2);

//  editor->setGeometry(checkboxstyle.rect);

//}

#ifdef PaintSimpleCheckBoxItemDelegate
void SimpleCheckBoxItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //Получаем данные
    bool data = index.model()->data(index, Qt::DisplayRole).toBool();

    //Создаем стиль CheckBox
    QStyleOptionButton checkboxstyle;
    QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);

    //Центрирование
    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x() +
                               option.rect.width()/2 - checkbox_rect.width()/2);
    //Выбрано или не выбрано
    if(data)
        checkboxstyle.state = QStyle::State_On|QStyle::State_Enabled;
    else
        checkboxstyle.state = QStyle::State_Off|QStyle::State_Enabled;

    //Готово! Отображаем!
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxstyle, painter);
}
#endif
#include "dialog_add_slave_chromatogramm.moc"
