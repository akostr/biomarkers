#include "web_dialog_add_chromatogramms.h"
#include "ui_web_dialog_add_chromatogramms.h"
#include <api/api_rest.h>
#include <logic/context_root.h>
#include <logic/notification.h>
#include <logic/enums.h>
#include <genesis_style/style.h>

#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QAction>
#include <QMenu>
#include <logic/known_context_tag_names.h>

namespace Dialogs
{
using Btns = QDialogButtonBox;
WebDialogAddChromatogramms::WebDialogAddChromatogramms(QList<int> existedIds, int referenceId, QWidget *parent) :
  Templates::Dialog(parent, Btns::Ok | Btns::Cancel),
  ui(new Ui::WebDialogAddChromatogramms),
  mContent(new QWidget()),
  mExistedChromasIds(existedIds),
  mReferenceId(referenceId),
  mModel(new QStandardItemModel(this))
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Adding chromatogramms to markup");
  s.buttonsNames[Btns::Ok] = tr("Continue");
  applySettings(s);
  MZ = 0;
  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);
  setupUi();
}

void WebDialogAddChromatogramms::loadModels(QList<int> acceptableFilesTypes, QString m_z)
{
  MZ=m_z;
  loadReferences(acceptableFilesTypes);
  loadChomas(acceptableFilesTypes);
}

WebDialogAddChromatogramms::~WebDialogAddChromatogramms()
{
  delete ui;
}

int WebDialogAddChromatogramms::getReferenceId()
{
  return ui->referenceCombo->currentData(RIdRole).toInt();
}

int WebDialogAddChromatogramms::getReferenceFileId()
{
  return ui->referenceCombo->currentData(RFileIdRole).toInt();
}

QList<int> WebDialogAddChromatogramms::getChromasIds()
{
  return getSelectedProjectFilesIds();
}

bool WebDialogAddChromatogramms::isReferenceMode()
{
  return ui->stackedWidget->currentIndex() == Page::reference;
}

bool WebDialogAddChromatogramms::isChromasMode()
{
  return ui->stackedWidget->currentIndex() == Page::chromas;
}

void WebDialogAddChromatogramms::setRegExSearch(bool regExSearch)
{
  if(mRegExSearch == regExSearch)
    return;
  mRegExSearch = regExSearch;
  if(mRegExSearch)
    ui->searchLine->actions().first()->setIcon(QIcon(":/resource/icons/search_regEx.png"));
  else
    ui->searchLine->actions().first()->setIcon(QIcon(":/resource/icons/search.png"));
  applyFilter(ui->searchLine->text());
}

void WebDialogAddChromatogramms::applyFilter(const QString &filter)
{
  if(mRegExSearch)
    mProxy->setFilterRegularExpression(filter);
  else
    mProxy->setFilterFixedString(filter);
}

void WebDialogAddChromatogramms::Accept()
{
  if(ui->stackedWidget->currentIndex() == Page::choise)
  {
    if(ui->radioButtonReference->isChecked())
      ui->stackedWidget->setCurrentIndex(Page::reference);
    else if(ui->radioButtonChromas->isChecked())
      ui->stackedWidget->setCurrentIndex(Page::chromas);
    ButtonBox->button(Btns::Ok)->setText(tr("Add to markup"));
    validateButtons();
  }
  else
  {
    WebDialog::Accept();
  }
}

void WebDialogAddChromatogramms::setupUi()
{
  ui->stackedWidget->setCurrentIndex(Page::choise);
  ui->choiseSubHeaderLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  ui->chromasSubHeaderLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  ui->referenceSubHeaderLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  auto searchAction = ui->searchLine->addAction(QIcon(":/resource/icons/search.png"), QLineEdit::LeadingPosition);
  auto searchMenu = new QMenu(this);
  searchMenu->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qmenu.qss"));
  auto regExAction = searchMenu->addAction(tr("regEx"));
  regExAction->setCheckable(true);
  regExAction->setChecked(false);
  connect(regExAction, &QAction::toggled, this, [this](bool on)
          {
            setRegExSearch(on);
          });

  searchAction->setProperty("menu", QVariant::fromValue(searchMenu));
  connect(searchAction, &QAction::triggered, this, [searchAction, this]()
          {
            auto menu = searchAction->property("menu").value<QMenu*>();
    menu->popup(ui->searchLine->mapToGlobal(ui->searchLine->rect().topLeft()));
          });

  QString listTreeStyle =
    "QTreeView::item {border: none; padding: 1px; margin: 1px;} QTreeView::branch {border: none;}";
  ui->chromasView->setStyleSheet(listTreeStyle);
  mProxy = new QSortFilterProxyModel(mModel);
  mProxy->setSourceModel(mModel);
  connect(ui->searchLine, &QLineEdit::textChanged, this, &WebDialogAddChromatogramms::applyFilter);
  ui->chromasView->setModel(mProxy);
  ui->chromasView->setSortingEnabled(true);
  connect(mModel, &QStandardItemModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>())
          {
            if(roles.contains(Qt::CheckStateRole))
            {
              for(int r = topLeft.row(); r <= bottomRight.row(); r++)
              {
                auto item = mModel->itemFromIndex(mModel->index(r, 0, topLeft.parent()));
                onItemCheckStateChanged(item, item->checkState());
              }
            }
          });

  connect(ui->checkAllChromasBox, &QCheckBox::stateChanged, this, &WebDialogAddChromatogramms::onCheckAllStateChanged);
  connect(ui->radioButtonChromas, &QRadioButton::toggled, this, &WebDialogAddChromatogramms::validateButtons);
  connect(ui->radioButtonReference, &QRadioButton::toggled, this, &WebDialogAddChromatogramms::validateButtons);
  connect(ui->referenceCombo, &QComboBox::currentIndexChanged, this, &WebDialogAddChromatogramms::validateButtons);
  connect(mModel, &QStandardItemModel::itemChanged, this, [this]()
          {
            ui->checkAllChromasBox->blockSignals(true);
            ui->checkAllChromasBox->setCheckState(getChromasTableCheckState());
            ui->checkAllChromasBox->blockSignals(false);
          });
  validateButtons();
}

void WebDialogAddChromatogramms::loadReferences(QList<int> acceptableFilesTypes)
{
  API::REST::GetProjectReferences(Core::GenesisContextRoot::Get()->ProjectId(),
    [this, acceptableFilesTypes](QNetworkReply*, QJsonDocument doc)
    {
      ui->referenceCombo->clear();
      auto jroot = doc.object();
      if(jroot["error"].toBool())
      {
        qCritical() << "Error while reference list loading: "<< jroot["msg"].toString();
        Notification::NotifyError(tr("Error while reference list loading"), jroot["msg"].toString());
      }
      else
      {
        auto jcolumns = jroot["columns"].toArray();
        QMap<QString, int> indexMap;
        for(int i = 0; i < jcolumns.size(); i++)
          indexMap[jcolumns[i].toObject()["name"].toString()] = i;
        auto jdata = jroot["data"].toArray();
        for(int i = 0; i < jdata.size(); i++)
        {
          auto jrow = jdata[i].toArray();
          if(jrow[indexMap["_id"]].toInt() == mReferenceId)
          {
            continue;
          }
          if(jrow[indexMap["_m_z"]].toString()!= MZ)
          {
              continue;
          }
          if(!acceptableFilesTypes.contains(jrow[indexMap["_file_type_id"]].toInt()))
          {
              continue;
          }

          auto ind = ui->referenceCombo->count();
          ui->referenceCombo->addItem(jrow[indexMap["_title"]].toString());
          ui->referenceCombo->setItemData(ind, jrow[indexMap["_id"]].toInt(), RIdRole);
          ui->referenceCombo->setItemData(ind, jrow[indexMap["_project_file_id"]].toInt(), RFileIdRole);
        }
        referencesLoaded = uploadSuccess;
        handleUploadStatus();
      }
    },
    [this](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      Notification::NotifyError(tr("Network error while reference list loading"), e);
      referencesLoaded = uploadError;
      handleUploadStatus();
    });

}

void WebDialogAddChromatogramms::loadChomas(QList<int> acceptableFilesTypes)
{
  mModel->clear();
  API::REST::Tables::GetProjectFiles(Core::GenesisContextRoot::Get()->ProjectId(),
    [this, acceptableFilesTypes](QNetworkReply*, QVariantMap data)
    {
      QMap<int, QStandardItem*> itemsMap;
      auto readItem = [&itemsMap](const QVariantMap& vmap)->QStandardItem*
      {
        auto item = new QStandardItem();
        auto nameV = vmap["m_z"];
        auto typeV = vmap["file_type_id"];
        if(nameV.isValid() && !nameV.isNull() && !nameV.toString().isEmpty())
        {
          item->setData("ion " + nameV.toString(), Qt::DisplayRole);
        }
        else
        {
          if(typeV.isValid() && !typeV.isNull() && typeV.toInt() == ProjectFiles::sim)
            item->setData("ion " + vmap["filename"].toString(), Qt::DisplayRole);
          else
            item->setData(vmap["filename"].toString(), Qt::DisplayRole);
        }
        item->setData(vmap["id"].toInt(), CIdRole);
        item->setData(vmap["project_file_id"].toInt(), CProjectFileIdRole);
        item->setData(vmap["title"].toString(), CAdditionalTitleRole);
        item->setData(vmap["group_title"].toString(), CGroupTitleRole);
        item->setData(vmap["id_file_group"].toInt(), CGroupIdRole);
        item->setData(typeV, CFileTypeRole);
        item->setData(false, CIsGroupRole);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        itemsMap[item->data(CIdRole).toInt()] = item;
        return item;
      };

      QList<QStandardItem*> topItems;
      QMap<int, QStandardItem*> groupsMap;
      QMap<int, QStandardItem*> fragmentsGroups;
      auto getGroupItem = [&groupsMap, &topItems](const QVariantMap& vmap)->QStandardItem*
      {
        auto groupId = vmap["id_file_group"].toInt();
        if(groupsMap.contains(groupId))
        {
          return groupsMap[groupId];
        }

        auto item = new QStandardItem();
        item->setData(vmap["group_title"].toString(), Qt::DisplayRole);
        item->setData(vmap["group_title"].toString(), CGroupTitleRole);
        item->setData(groupId, CGroupIdRole);
        item->setData(true, CIsGroupRole);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setFlags(item->flags() | Qt::ItemIsAutoTristate | Qt::ItemIsUserCheckable);
        groupsMap[groupId] = item;
        topItems.append(item);
        return item;
      };
      auto getGroupFragmentsItem = [&fragmentsGroups, &groupsMap, &topItems](const QString& name, int id)->QStandardItem*
      {
        if(groupsMap.contains(id))
        {
          return groupsMap[id];
        }

        auto item = new QStandardItem();
        item->setData(name, Qt::DisplayRole);
        item->setData(name, CGroupTitleRole);
        item->setData(id, CGroupIdRole);
        item->setData(true, CIsGroupRole);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setFlags(item->flags() | Qt::ItemIsAutoTristate | Qt::ItemIsUserCheckable);
        groupsMap[id] = item;
        topItems.append(item);
        fragmentsGroups[id] = item;
        return item;
      };

      auto vitems = data["children"].toList();
      for(auto vitem : vitems)
      {
        auto vmap = vitem.toMap();
        auto vgroupId = vmap["id_file_group"];
        auto parentId = vmap["parent_id"];

        auto item = readItem(vmap);

        // qDebug()<<QString("MZ: %1, m_z: %2").arg(MZ).arg(vmap["m_z"].toString());

        if(vmap["m_z"].toString()!= MZ)
        {
            // qDebug()<<"skip";
            continue;
        }

        if(vgroupId.isValid() && !vgroupId.isNull())
        {
          auto groupItem = getGroupItem(vmap);
          groupItem->appendRow(item);
        }
        else if(vmap["file_type_id"].toInt() == ProjectFiles::sim && parentId.isValid() && !parentId.isNull())
        {
          //at this moment parent item can not be in our list, so we skip group naming step
          auto groupItem = getGroupFragmentsItem("", -parentId.toInt());//SUPER IMPORTANT sign inversion to avoid intersection with groups ids
          groupItem->appendRow(item);
        }
        else
        {
          topItems.append(item);
        }
      }

      //because we skiped naming step earlier, we do it now:
      ///Fragments groups naming step:
      for(auto it = fragmentsGroups.constKeyValueBegin(); it !=fragmentsGroups.constKeyValueEnd(); it++)
      {
        auto parentId = -it->first;//revert sign inversion
        auto fragmentGroupItem = it->second;
        if(!itemsMap.contains(parentId))
        {
          fragmentGroupItem->setData(QString("PARENT NOT FOUND"), Qt::DisplayRole);
          fragmentGroupItem->setData(QString("PARENT NOT FOUND"), CGroupTitleRole);
        }
        else
        {
          auto item = itemsMap[parentId];
          item->setData(ProjectFiles::tic, CFileTypeRole);
          item->setData("TIC(" + item->data(Qt::DisplayRole).toString() + ")", Qt::DisplayRole);
          fragmentGroupItem->setData(item->data(Qt::DisplayRole), Qt::DisplayRole);
          fragmentGroupItem->setData(item->data(Qt::DisplayRole), CGroupTitleRole);
        }
      }
      ///

      itemsMap.clear();//will be invalid after clearing cycle
      fragmentsGroups.clear();//will be invalid after clearing cycle
      groupsMap.clear();//will be invalid after clearing cycle
      for(auto it = topItems.begin(); it != topItems.end(); it++)
      {
        auto tlitem = *it;
        auto isGroupV = tlitem->data(CIsGroupRole);
        if(isGroupV.isValid() && !isGroupV.isNull() && isGroupV.toBool())
        {
          for(int r = 0; r < tlitem->rowCount(); r++)
          {
            auto item = tlitem->child(r);
            if(!acceptableFilesTypes.contains(item->data(CFileTypeRole).toInt()) ||
                mExistedChromasIds.contains(item->data(CProjectFileIdRole).toInt()))
              tlitem->removeRow(r--);//delete containing item, so our pointers invalid now
          }
          if(tlitem->rowCount() == 0)
          {
            delete tlitem;
            it = topItems.erase(it);
            it--;
          }
        }
        else
        {
          if(!acceptableFilesTypes.contains(tlitem->data(CFileTypeRole).toInt()) ||
              mExistedChromasIds.contains(tlitem->data(CProjectFileIdRole).toInt()))
          {
            delete tlitem;
            it = topItems.erase(it);
            it--;
          }
        }
      }

      for(auto& item : topItems)
      {
        mModel->appendRow(item);
      }

      chromasLoaded = uploadSuccess;
      handleUploadStatus();
    },
    [this](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      Notification::NotifyError(tr("Failed to load project files: "), e);
      chromasLoaded = uploadError;
      handleUploadStatus();
    });
}

void WebDialogAddChromatogramms::handleUploadStatus()
{
  auto isUploadFinished = [](UploadStatus s)->bool{return s == uploadError || s == uploadSuccess;};
  if(isUploadFinished(referencesLoaded) && isUploadFinished(chromasLoaded))
  {
    referencesLoaded = notUploaded;
    chromasLoaded = notUploaded;
    emit modelsLoaded();
  }
}

void WebDialogAddChromatogramms::setAllChromasCheckState(Qt::CheckState state)
{
  std::function<void(QStandardItem*, const Qt::CheckState&)> setCheckStateRecursively;
  setCheckStateRecursively = [&setCheckStateRecursively](QStandardItem* item, const Qt::CheckState& state)
  {
    for(int c = 0; c < item->rowCount(); c++)
    {
      setCheckStateRecursively(item->child(c), state);
    }
    item->setCheckState(state);
  };
  for(auto r = 0; r < mModel->rowCount(); r++)
  {
    auto item = mModel->item(r);
    setCheckStateRecursively(item, state);
  }
}

Qt::CheckState WebDialogAddChromatogramms::getChromasTableCheckState()
{
  auto concatenateCheckState = [](Qt::CheckState s1, Qt::CheckState s2)->Qt::CheckState
  {
    if(s1 == s2)
      return s1;
    if(s1 == Qt::Unchecked && s2 == Qt::Unchecked)
      return Qt::Unchecked;
    if(s1 == Qt::Unchecked && (s2 == Qt::PartiallyChecked || s2 == Qt::Checked))
      return Qt::PartiallyChecked;
    if(s1 == Qt::PartiallyChecked)
      return Qt::PartiallyChecked;
    if(s1 == Qt::Checked && (s2 == Qt::PartiallyChecked || s2 == Qt::Unchecked))
      return Qt::PartiallyChecked;
    return s1;
  };
  std::function<Qt::CheckState(QStandardItem*)> itemCheckState;
  itemCheckState = [&itemCheckState, concatenateCheckState](QStandardItem* item)->Qt::CheckState
  {
    Qt::CheckState state;
    if(item->rowCount() == 0)
      state = item->checkState();
    else
    {
      for(int c = 0; c < item->rowCount(); c++)
      {
        if(c == 0)
          state = itemCheckState(item->child(c));
        else
        {
          auto s = itemCheckState(item->child(c));
          state = concatenateCheckState(state, s);
        }
      }
    }
    return state;
  };
  Qt::CheckState state = Qt::Unchecked;
  for(int r = 0; r < mModel->rowCount(); r++)
  {
    if(r == 0)
      state = itemCheckState(mModel->item(r));
    else
      state = concatenateCheckState(state, itemCheckState(mModel->item(r)));
  }
  return state;
}

void WebDialogAddChromatogramms::validateButtons()
{
  switch((Page)ui->stackedWidget->currentIndex())
  {
  case choise:
    ButtonBox->button(Btns::Ok)->setEnabled(ui->radioButtonReference->isChecked() || ui->radioButtonChromas->isChecked());
    break;
  case reference:
  {
    auto d = ui->referenceCombo->currentData(RIdRole);
    ButtonBox->button(Btns::Ok)->setEnabled(d.isValid() && !d.isNull() && d.toInt() > 0);
    break;
  }
  case chromas:
  default:
    break;
  }
}


void WebDialogAddChromatogramms::onCheckAllStateChanged(int newState)
{
  if(newState == Qt::Unchecked)
  {
    setAllChromasCheckState(Qt::Unchecked);
  }
  else if(newState == Qt::Checked)
  {
    setAllChromasCheckState(Qt::Checked);
  }
  else if(newState == Qt::PartiallyChecked)
  {
    ui->checkAllChromasBox->setCheckState(Qt::Checked);
  }
}

QList<int> WebDialogAddChromatogramms::getSelectedProjectFilesIds()
{
  std::function<void(QStandardItem*, QList<int>&)> scanRecursive = [&scanRecursive](QStandardItem* item, QList<int>& list)
  {
    for(int c = 0; c < item->rowCount(); c++)
    {
      auto child = item->child(c);
      scanRecursive(child, list);
    }
    auto vpid = item->data(CProjectFileIdRole);
    if(vpid.isValid() && !vpid.isNull() && item->data(Qt::CheckStateRole).toInt() == Qt::Checked)
      list.append(vpid.toInt());
  };
  QList<int> ret;
  for(int r = 0; r < mModel->rowCount(); r++)
  {
    scanRecursive(mModel->item(r), ret);
  }
  return ret;
}

void WebDialogAddChromatogramms::onItemCheckStateChanged(QStandardItem *item, Qt::CheckState state)
{
  if(item->flags().testFlags(Qt::ItemIsAutoTristate))
  {
    if(state != Qt::PartiallyChecked)
      for(int r = 0; r < item->rowCount(); r++)
      {
        auto child = item->child(r);
        if(child->checkState() != item->checkState())
          child->setCheckState(state);
      }
  }
  validateItemCheckState(item->parent());
}

void WebDialogAddChromatogramms::validateItemCheckState(QStandardItem *item)
{
  if(!item)
    return;
  if(item->flags().testFlags(Qt::ItemIsAutoTristate))
  {
    Qt::CheckState state = item->checkState();
    for(int r = 0; r < item->rowCount(); r++)
    {
      auto childState = item->child(r)->checkState();
      if(r == 0)
      {
        state = childState;
        continue;
      }
      if(childState == state)
        continue;
      if(state == Qt::Unchecked && childState != Qt::Unchecked)
      {
        state = Qt::PartiallyChecked;
        break;
      }
      if(state == Qt::Checked && childState == Qt::Unchecked)
      {
        state = Qt::PartiallyChecked;
        break;
      }
    }
    if(state != item->checkState())
      item->setCheckState(state);
  }
  validateItemCheckState(item->parent());
}

}//namespace Dialogs
