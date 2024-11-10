#include "tree_confirmation.h"
#include "ui_tree_confirmation.h"
#include <ui/genesis_window.h>
#include <api/api_rest.h>
#include <genesis_style/style.h>
#include <logic/tree_model.h>
#include "ui/known_view_names.h"

#include <QMenu>
#include <QClipboard>
#include <QPushButton>

using namespace Core;

namespace Dialogs
{
namespace Templates
{

TreeConfirmation::TreeConfirmation(QWidget* parent)
  : Confirm(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  , ui(new Ui::TreeConfirmation)
  , mTreeModel(this)
{
  Size = QSizeF(0.25, 0.33);
  setupUi();
  applySettings(Settings());
  connect(&mTreeModel, &TreeConfirmationModel::checkedCountChanged, this, [this](int count)
  {
    updateCounterLabel(count);
    if(count == 0)
      ButtonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
    else
      ButtonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
  });
}

TreeConfirmation::~TreeConfirmation()
{
  delete ui;
  clearTreeItems();
}

void TreeConfirmation::applySettings(Settings s)
{
  clearTreeItems();
  mSettings = s;
  Confirm::applySettings(mSettings.dialogSettings);
  ui->phraseLabel->setText(mSettings.phrase);
  mTreeModel.fillModel(mSettings.rootTreeItem);
  ui->treeView->resizeColumnToContents(0);
}

TreeConfirmation::Settings TreeConfirmation::settings()
{
  return mSettings;
}

void TreeConfirmation::setupUi()
{
  auto content = getContent();
  auto body = new QWidget(content);
  ui->setupUi(body);
  body->setContentsMargins(0,0,0,0);
  body->layout()->setContentsMargins(0,0,0,0);
  content->layout()->addWidget(body);
  ui->treeView->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_compact_qtreeview.qss"));
  ui->phraseLabel->setStyleSheet(Style::Genesis::Fonts::RegularText());
  ui->chromatogrammCountLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  ui->treeView->setModel(&mTreeModel);
  connect(ui->treeView, &QTreeView::customContextMenuRequested, this, [this](const QPoint& pt)
  {
    auto index = ui->treeView->indexAt(pt);
    if(index.isValid())
    {
      QString displayData = mTreeModel.data(index, Qt::DisplayRole).toString();
      auto menu = new QMenu(this);
      auto copyTextAction = new QAction(tr("Copy text"), menu);
      connect(copyTextAction, &QAction::triggered, this, [displayData]()
      {
        auto clipboard = QGuiApplication::clipboard();
        clipboard->setText(displayData);
      });
      menu->addAction(copyTextAction);
      menu->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qmenu.qss"));
      menu->popup(ui->treeView->viewport()->mapToGlobal(pt));
    }
  });
}

QList<QVariant> TreeConfirmation::getConfirmedUserDataList()
{
  QList<QVariant> ret;
  for(int row = 0; row < mTreeModel.rowCount(); row++)
  {
    auto index = mTreeModel.index(row, 0);
    if(mTreeModel.data(index, Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked)
      ret.append(mTreeModel.data(index, Qt::UserRole));
  }
  return ret;
}

TreeConfirmation *TreeConfirmation::DeleteMarkupFilesWithDependencies(const QList<QPair<int, QString>> &projectFilesInfos, QWidget* parent)
{
  auto dialPtr = new TreeConfirmation(parent);
  dialPtr->Open();
  if(projectFilesInfos.isEmpty())
    return dialPtr;
//// after dialog accepted
  connect(dialPtr, &WebDialog::Accepted, [dialPtr]()
  {
    auto idList = dialPtr->getConfirmedUserDataList();
    if(idList.isEmpty())
      return;

    int* queryCounter = new int(idList.count());
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Removing files"));
    auto queryComplete = [queryCounter, overlayId]()
    {
      if(--*queryCounter <= 0)
      {
        delete queryCounter;
        TreeModel::ResetInstances("TreeModelDynamicProjectChromatogramms");
        TreeModel::ResetInstances("TreeModelDynamicProjectFragments");
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      }
    };
    for(auto& v : idList)
    {
      int id = v.toInt();
      API::REST::RemoveFileAndDependenciesFromProject(id,
      [queryComplete](QNetworkReply*, QJsonDocument)
      {
        queryComplete();
      },
      [id, queryComplete](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Failed when trying to remove file %1").arg(id), err);
        queryComplete();
      });
    }
  });

//// data loading before
  auto counterPtr = new int(projectFilesInfos.count());
  auto rootItem = new TreeItem();
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  auto queryComplete = [counterPtr, overlayId, dialPtr, rootItem]()
  {
    if(--*counterPtr <= 0)
    {
      delete counterPtr;
      auto s = dialPtr->mSettings;
      s.rootTreeItem = rootItem;
      s.updateCounterLabelFunc = [](int count) -> QString
      {
        return tr("Picked %n file(s) for removing", "", count);
      };
      s.dialogSettings.buttonsNames = {{QDialogButtonBox::Ok, tr("Remove")}};
      s.dialogSettings.buttonsProperties = {{QDialogButtonBox::Ok, {{"red", true}}}};
      dialPtr->applySettings(s);
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    }
  };
  for(auto& fileInfo : projectFilesInfos)
  {
    auto topLevelItem = new TreeItem();
    topLevelItem->displayRoleData = fileInfo.second;
    topLevelItem->userRoleData = fileInfo.first;
    rootItem->children.append(topLevelItem);

    API::REST::GetDependentFiles(fileInfo.first,
    [queryComplete, topLevelItem](QNetworkReply*, QJsonDocument doc)
    {
      auto root = doc.object();
      if(root.contains("error"))
      {
        qDebug() << "error" << root["msg"].toString();
        queryComplete();
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
          else
            treeItem->displayRoleData = jitem.first().toString() + " \"" + jitem.last().toString() + "\"";
          topLevelItem->children.append(treeItem);
        }
      }
      queryComplete();
    },
    [queryComplete, topLevelItem](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      auto treeItem = new TreeItem();
      treeItem->displayRoleData = tr("Failed to load dependencies");
      topLevelItem->children.append(treeItem);

      Notification::NotifyError(tr("Error while scan file %1 dependencies").arg(topLevelItem->displayRoleData.toString()), e);
      queryComplete();
    });
  }
  //possible mem leak with counterPtr, when some of network request will not call any of his lambda callbacks
  //if all of requests will reply proper, than counter will <= 0 and pointer will be cleared in queryComplete()
  //won't test shared pointers with lambda now. If anybody want, you can refactor it on shared pointers
  return dialPtr;
}

TreeConfirmation *TreeConfirmation::DeleteProjectWithDependencies(const QList<QPair<int, QString>>& projectInfos, QWidget* parent)
{
  auto dialPtr = new TreeConfirmation(parent);
  auto s = dialPtr->mSettings;
  s.phrase = tr("Are you shure that you want to remove project and it's dependencies?");;
  dialPtr->applySettings(s);
  dialPtr->Open();

//// after dialog accepted
  connect(dialPtr, &WebDialog::Accepted, [dialPtr]()
  {
    auto idList = dialPtr->getConfirmedUserDataList();
    if(idList.isEmpty())
      return;

    int* queryCounter = new int(idList.count());
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Removing projects"));
    auto queryComplete = [queryCounter, overlayId]()
    {
      if(--*queryCounter <= 0)
      {
        delete queryCounter;
        TreeModel::ResetInstances("TreeModelDynamicProjects");
        GenesisWindow::Get()->RemoveOverlay(overlayId);
        GenesisWindow::Get()->ShowPage(ViewPageNames::ViewProjectsPageName);
      }
    };
    for(auto& v : idList)
    {
      int id = v.toInt();
      API::REST::RemoveProjectAndDependencies(id,
      [queryComplete](QNetworkReply*, QJsonDocument)
      {
        queryComplete();
      },
      [id, queryComplete](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Failed when trying to remove project %1").arg(id), err);
        queryComplete();
      });
    }
  });

//// data loading before
  auto rootItem = new TreeItem();
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  auto queryComplete = [counterPtr = std::make_shared<qsizetype>(projectInfos.count()), overlayId, dialPtr, rootItem] () mutable
  {
    if(--*counterPtr <= 0)
    {
      auto s = dialPtr->mSettings;
      s.rootTreeItem = rootItem;
      s.updateCounterLabelFunc = [](int count) -> QString
      {
        return tr("Picked %n project(s) for removing", "", count);
      };
      s.dialogSettings.buttonsNames = {{QDialogButtonBox::Ok, tr("Remove")}};
      s.dialogSettings.buttonsProperties = {{QDialogButtonBox::Ok, {{"red", true}}}};
      dialPtr->applySettings(s);
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    }
  };

  for(auto& projectInfo : projectInfos)
  {
    auto topLevelItem = new TreeItem();
    topLevelItem->displayRoleData = projectInfo.second;
    topLevelItem->userRoleData = projectInfo.first;
    rootItem->children.append(topLevelItem);

    API::REST::GetDependentProjectFiles(projectInfo.first,
    [queryComplete, topLevelItem](QNetworkReply*, QJsonDocument doc) mutable
    {
      auto root = doc.object();
      if(root.contains("error"))
      {
        qDebug() << "error" << root["msg"].toString();
        queryComplete();
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
          else
            treeItem->displayRoleData = jitem.first().toString() + " \"" + jitem.last().toString() + "\"";
          topLevelItem->children.append(treeItem);
        }
      }
      queryComplete();
    },
    [queryComplete, topLevelItem](QNetworkReply*, QNetworkReply::NetworkError e) mutable
    {
      auto treeItem = new TreeItem();
      treeItem->displayRoleData = tr("Failed to load dependencies");
      topLevelItem->children.append(treeItem);

      Notification::NotifyError(tr("Error while scan file %1 dependencies").arg(topLevelItem->displayRoleData.toString()), e);
      queryComplete();
    });
  }

  return dialPtr;
}

TreeConfirmation *TreeConfirmation::DeleteTablesWithDependencies(const QList<QPair<int, QString> > &tablesInfos,
                                                                 QWidget *parent,
                                                                 QString tableKey)
{
  auto dialPtr = new TreeConfirmation(parent);
  dialPtr->Open();
  if(tablesInfos.isEmpty())
    return dialPtr;
//// after dialog accepted
  auto queryComplete = [](int ind, QObject* context, std::function<void()> onFinished)
  {
    if(!context)
      return;
    auto vqr = context->property("queries_results");
    if(!vqr.isValid() || vqr.isNull())
      return;
    auto qr = vqr.value<QList<bool>>();
    Q_ASSERT(qr.size() > ind);
    qr[ind] = true;
    bool finished = true;
    for(auto& v : qr)
    {
      if(!v)
      {
        finished = false;
        break;
      }
    }
    context->setProperty("queries_results", QVariant::fromValue(qr));
    if(finished)
    {
      context->deleteLater();
      onFinished();
    }
  };
  auto prepareForMultipleQueries = [parent](qsizetype queriesCount)->QObject*
  {
    QList<bool> queriesResults(queriesCount);
    queriesResults.fill(false);//CHECK size of queries result
    auto context = new QObject(parent);
    context->setProperty("queries_results", QVariant::fromValue(queriesResults));
    return context;
  };


  connect(dialPtr, &WebDialog::Accepted, [dialPtr, queryComplete,  prepareForMultipleQueries, tableKey]()
  {
    auto idList = dialPtr->getConfirmedUserDataList();
    if(idList.isEmpty())
      return;

    auto queryContext = prepareForMultipleQueries(idList.count());
    auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Removing tables"));
    auto onFinished = [overlayId, tableKey]()
    {
      if(!tableKey.isEmpty())
        TreeModel::ResetInstances(tableKey);
      GenesisWindow::Get()->RemoveOverlay(overlayId);
    };

    for(int i = 0; i < idList.size(); i++)
    {
      auto& v = idList[i];
      int id = v.toInt();
      API::REST::Tables::RemoveTable(id,
      [queryContext, queryComplete, i, onFinished](QNetworkReply*, QJsonDocument)
      {
        queryComplete(i, queryContext, onFinished);
      },
      [id, queryContext, queryComplete, i, onFinished](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        Notification::NotifyError(tr("Failed when trying to remove table %1").arg(id), err);
        queryComplete(i, queryContext, onFinished);
      });
    }
  });

//// data loading before
  auto queryContext = prepareForMultipleQueries(tablesInfos.count());
  auto rootItem = new TreeItem();
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  auto onFinished = [overlayId, dialPtr, rootItem]()
  {
    auto s = dialPtr->mSettings;
    s.rootTreeItem = rootItem;
    s.updateCounterLabelFunc = [](int count) -> QString
    {
      return tr("Picked %n table(s) for removing", "", count);
    };
    s.dialogSettings.buttonsNames = {{QDialogButtonBox::Ok, tr("Remove")}};
    s.dialogSettings.buttonsProperties = {{QDialogButtonBox::Ok, {{"red", true}}}};
    dialPtr->applySettings(s);
    GenesisWindow::Get()->RemoveOverlay(overlayId);
  };

  for(int i = 0; i < tablesInfos.size(); i++)
  {
    auto& fileInfo = tablesInfos[i];
    auto topLevelItem = new TreeItem();
    topLevelItem->displayRoleData = fileInfo.second;
    topLevelItem->userRoleData = fileInfo.first;
    rootItem->children.append(topLevelItem);

    API::REST::GetDependentTables(fileInfo.first,
    [i, queryContext, onFinished, queryComplete, topLevelItem](QNetworkReply*, QJsonDocument doc)
    {
      auto root = doc.object();
      if(root.contains("error"))
      {
        qDebug() << "error" << root["msg"].toString();
        queryComplete(i, queryContext, onFinished);
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
            treeItem->displayRoleData = jitem.first().toString() + " \"" + jitem.last().toString() + "\"";
          topLevelItem->children.append(treeItem);
        }
      }
      queryComplete(i, queryContext, onFinished);
    },
    [i, queryContext, onFinished, queryComplete, topLevelItem](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      auto treeItem = new TreeItem();
      treeItem->displayRoleData = tr("Failed to load dependencies");
      topLevelItem->children.append(treeItem);

      Notification::NotifyError(tr("Error while scan file %1 dependencies").arg(topLevelItem->displayRoleData.toString()), e);
      queryComplete(i, queryContext, onFinished);
    });
  }
  return dialPtr;
}

TreeConfirmation* TreeConfirmation::DeleteCompoundsWithDependencies(const QList<QPair<int, QString>>& compoundInfo, QWidget* parent)
{
  auto dialPtr = new TreeConfirmation(parent);
  dialPtr->Open();
  if (compoundInfo.isEmpty())
    return dialPtr;
  //// after dialog accepted
  connect(dialPtr, &WebDialog::Accepted, [dialPtr]()
    {
      auto idList = dialPtr->getConfirmedUserDataList();
      if (idList.isEmpty())
        return;

      int* queryCounter = new int(idList.count());
      auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Removing files"));
      auto queryComplete = [queryCounter, overlayId]()
        {
          if (--*queryCounter <= 0)
          {
            delete queryCounter;
            TreeModel::ResetInstances("TreeModelDynamicLibraryCompounds");
            GenesisWindow::Get()->RemoveOverlay(overlayId);
          }
        };
      for (auto& v : idList)
      {
        int id = v.toInt();
        API::REST::deleteLibraryGroupCompound(id,
          [queryComplete](QNetworkReply*, QJsonDocument)
          {
            queryComplete();
          },
          [id, queryComplete](QNetworkReply*, QNetworkReply::NetworkError err)
          {
            Notification::NotifyError(tr("Failed when trying to remove file %1").arg(id), err);
            queryComplete();
          });
      }
    });

  //// data loading before
  auto counterPtr = new int(compoundInfo.count());
  auto rootItem = new TreeItem();
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  auto queryComplete = [counterPtr, overlayId, dialPtr, rootItem]()
    {
      if (--*counterPtr <= 0)
      {
        delete counterPtr;
        auto s = dialPtr->mSettings;
        s.rootTreeItem = rootItem;
        s.updateCounterLabelFunc = [](int count) -> QString
          {
            return tr("Picked %n file(s) for removing", "", count);
          };
        s.dialogSettings.buttonsNames = { {QDialogButtonBox::Ok, tr("Remove")} };
        s.dialogSettings.buttonsProperties = { {QDialogButtonBox::Ok, {{"red", true}}} };
        dialPtr->applySettings(s);
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      }
    };
  for (auto& fileInfo : compoundInfo)
  {
    auto topLevelItem = new TreeItem();
    topLevelItem->displayRoleData = fileInfo.second;
    topLevelItem->userRoleData = fileInfo.first;
    rootItem->children.append(topLevelItem);

    API::REST::compounudDependencies(fileInfo.first,
      [queryComplete, topLevelItem](QNetworkReply*, QJsonDocument doc)
      {
        // qDebug() << doc;
        auto root = doc.object();
        if (root.contains("error"))
        {
          qDebug() << "error" << root["msg"].toString();
          queryComplete();
          return;
        }
        auto columns = root["columns"].toArray();
        auto data = root["data"].toArray();
        topLevelItem->displayRoleData = topLevelItem->displayRoleData.toString() + tr(" (%n connected element(s))", "", data.size());
        if (data.isEmpty())
        {
          auto treeItem = new TreeItem();
          treeItem->displayRoleData = tr("No dependencies");
          topLevelItem->children.append(treeItem);
        }
        else
        {
          for (int i = 0; i < data.size(); i++)
          {
            auto jitem = data[i].toArray();
            auto treeItem = new TreeItem();
            QString str;
            QString typeStr;
            for(int c = 0; c < columns.size(); c++)
            {
              auto column = columns[c].toObject()["name"].toString();
              if (column.toLower() == "short_title")
              {
                if(!str.isEmpty())
                  str.append('\n');
                str += tr("Short title: ") + " \"" + jitem[c].toString() + "\"";
              }
              else if (column.toLower() == "full_title")
              {
                if(!str.isEmpty())
                  str.append('\n');
                str += tr("Full title: ") + " \"" + jitem[c].toString() + "\"";
              }
              else if (column.toLower() == "type")
              {
                typeStr = jitem[c].toString();
              }
            }
            str.prepend(QString("%1\n").arg(typeStr));
            treeItem->displayRoleData = str;
            topLevelItem->children.append(treeItem);
          }
        }
        queryComplete();
      },
      [queryComplete, topLevelItem](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        auto treeItem = new TreeItem();
        treeItem->displayRoleData = tr("Failed to load dependencies");
        topLevelItem->children.append(treeItem);

        Notification::NotifyError(tr("Error while scan file %1 dependencies").arg(topLevelItem->displayRoleData.toString()), e);
        queryComplete();
      });
  }
  return dialPtr;
}

TreeConfirmation* TreeConfirmation::DeleteCoefficientWithDependencies(const QList<QPair<int, QString>>& compoundInfo, QWidget* parent)
{
  auto dialPtr = new TreeConfirmation(parent);
  dialPtr->Open();
  if (compoundInfo.isEmpty())
    return dialPtr;
  //// after dialog accepted
  connect(dialPtr, &WebDialog::Accepted, [dialPtr]()
    {
      auto idList = dialPtr->getConfirmedUserDataList();
      if (idList.isEmpty())
        return;

      int* queryCounter = new int(idList.count());
      auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Removing files"));
      auto queryComplete = [queryCounter, overlayId]()
        {
          if (--*queryCounter <= 0)
          {
            delete queryCounter;
            TreeModel::ResetInstances("TreeModelDynamicLibraryCompounds");
            GenesisWindow::Get()->RemoveOverlay(overlayId);
          }
        };
      for (auto& v : idList)
      {
        int id = v.toInt();
        API::REST::deleteLibraryGroupCoefficient(id,
          [queryComplete](QNetworkReply*, QJsonDocument)
          {
            queryComplete();
          },
          [id, queryComplete](QNetworkReply*, QNetworkReply::NetworkError err)
          {
            Notification::NotifyError(tr("Failed when trying to remove file %1").arg(id), err);
            queryComplete();
          });
      }
    });

  //// data loading before
  auto counterPtr = new int(compoundInfo.count());
  auto rootItem = new TreeItem();
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  auto queryComplete = [counterPtr, overlayId, dialPtr, rootItem]()
    {
      if (--*counterPtr <= 0)
      {
        delete counterPtr;
        auto s = dialPtr->mSettings;
        s.rootTreeItem = rootItem;
        s.updateCounterLabelFunc = [](int count) -> QString
          {
            return tr("Picked %n file(s) for removing", "", count);
          };
        s.dialogSettings.buttonsNames = { {QDialogButtonBox::Ok, tr("Remove")} };
        s.dialogSettings.buttonsProperties = { {QDialogButtonBox::Ok, {{"red", true}}} };
        dialPtr->applySettings(s);
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      }
    };
  for (auto& fileInfo : compoundInfo)
  {
    auto topLevelItem = new TreeItem();
    topLevelItem->displayRoleData = fileInfo.second;
    topLevelItem->userRoleData = fileInfo.first;
    rootItem->children.append(topLevelItem);

    API::REST::compounudDependencies(fileInfo.first,
      [queryComplete, topLevelItem](QNetworkReply*, QJsonDocument doc)
      {
        // qDebug() << doc;
        auto root = doc.object();
        if (root.contains("error"))
        {
          qDebug() << "error" << root["msg"].toString();
          queryComplete();
          return;
        }
        auto columns = root["columns"].toArray();
        auto data = root["data"].toArray();
        topLevelItem->displayRoleData = topLevelItem->displayRoleData.toString() + tr(" (%n connected element(s))", "", data.size());
        if (data.isEmpty())
        {
          auto treeItem = new TreeItem();
          treeItem->displayRoleData = tr("No dependencies");
          topLevelItem->children.append(treeItem);
        }
        else
        {
          for (int i = 0; i < data.size(); i++)
          {
            auto jitem = data[i].toArray();
            auto treeItem = new TreeItem();
            if (jitem.first().toString().toLower() == "short_title")
              treeItem->displayRoleData = tr("Short title") + " \"" + jitem.last().toString() + "\"";
            else if (jitem.first().toString().toLower() == "full_title")
              treeItem->displayRoleData = tr("Full title") + " \"" + jitem.last().toString() + "\"";
            topLevelItem->children.append(treeItem);
          }
        }
        queryComplete();
      },
      [queryComplete, topLevelItem](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        auto treeItem = new TreeItem();
        treeItem->displayRoleData = tr("Failed to load dependencies");
        topLevelItem->children.append(treeItem);

        Notification::NotifyError(tr("Error while scan file %1 dependencies").arg(topLevelItem->displayRoleData.toString()), e);
        queryComplete();
      });
  }
  return dialPtr;
}


TreeConfirmation* TreeConfirmation::DeletePlotTemplateWithDependencies(const QList<QPair<int, QString>>& compoundInfo, QWidget* parent)
{
  auto dialPtr = new TreeConfirmation(parent);
  dialPtr->Open();
  if (compoundInfo.isEmpty())
    return dialPtr;
  //// after dialog accepted
  connect(dialPtr, &WebDialog::Accepted, [dialPtr]()
    {
      auto idList = dialPtr->getConfirmedUserDataList();
      if (idList.isEmpty())
        return;

      int* queryCounter = new int(idList.count());
      auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Removing files"));
      auto queryComplete = [queryCounter, overlayId]()
        {
          if (--*queryCounter <= 0)
          {
            delete queryCounter;
            TreeModel::ResetInstances("TreeModelDynamicLibraryCompounds");
            GenesisWindow::Get()->RemoveOverlay(overlayId);
          }
        };
      for (auto& v : idList)
      {
        int id = v.toInt();
        API::REST::deleteLibraryGroupPlotTemplate(id,
          [queryComplete](QNetworkReply*, QJsonDocument)
          {
            queryComplete();
          },
          [id, queryComplete](QNetworkReply*, QNetworkReply::NetworkError err)
          {
            Notification::NotifyError(tr("Failed when trying to remove file %1").arg(id), err);
            queryComplete();
          });
      }
    });

  //// data loading before
  auto counterPtr = new int(compoundInfo.count());
  auto rootItem = new TreeItem();
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  auto queryComplete = [counterPtr, overlayId, dialPtr, rootItem]()
    {
      if (--*counterPtr <= 0)
      {
        delete counterPtr;
        auto s = dialPtr->mSettings;
        s.rootTreeItem = rootItem;
        s.updateCounterLabelFunc = [](int count) -> QString
          {
            return tr("Picked %n file(s) for removing", "", count);
          };
        s.dialogSettings.buttonsNames = { {QDialogButtonBox::Ok, tr("Remove")} };
        s.dialogSettings.buttonsProperties = { {QDialogButtonBox::Ok, {{"red", true}}} };
        dialPtr->applySettings(s);
        GenesisWindow::Get()->RemoveOverlay(overlayId);
      }
    };
  for (auto& fileInfo : compoundInfo)
  {
    auto topLevelItem = new TreeItem();
    topLevelItem->displayRoleData = fileInfo.second;
    topLevelItem->userRoleData = fileInfo.first;
    rootItem->children.append(topLevelItem);

    API::REST::compounudDependencies(fileInfo.first,
      [queryComplete, topLevelItem](QNetworkReply*, QJsonDocument doc)
      {
        // qDebug() << doc;
        auto root = doc.object();
        if (root.contains("error"))
        {
          qDebug() << "error" << root["msg"].toString();
          queryComplete();
          return;
        }
        auto columns = root["columns"].toArray();
        auto data = root["data"].toArray();
        topLevelItem->displayRoleData = topLevelItem->displayRoleData.toString() + tr(" (%n connected element(s))", "", data.size());
        if (data.isEmpty())
        {
          auto treeItem = new TreeItem();
          treeItem->displayRoleData = tr("No dependencies");
          topLevelItem->children.append(treeItem);
        }
        else
        {
          for (int i = 0; i < data.size(); i++)
          {
            auto jitem = data[i].toArray();
            auto treeItem = new TreeItem();
            if (jitem.first().toString().toLower() == "short_title")
              treeItem->displayRoleData = tr("Short title") + " \"" + jitem.last().toString() + "\"";
            else if (jitem.first().toString().toLower() == "full_title")
              treeItem->displayRoleData = tr("Full title") + " \"" + jitem.last().toString() + "\"";
            topLevelItem->children.append(treeItem);
          }
        }
        queryComplete();
      },
      [queryComplete, topLevelItem](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        auto treeItem = new TreeItem();
        treeItem->displayRoleData = tr("Failed to load dependencies");
        topLevelItem->children.append(treeItem);

        Notification::NotifyError(tr("Error while scan file %1 dependencies").arg(topLevelItem->displayRoleData.toString()), e);
        queryComplete();
      });
  }
  return dialPtr;
}

void TreeConfirmation::clearTreeItems()
{
  if(mSettings.rootTreeItem)
  {
    std::function<void(TreeItem*)> clear = [&clear](TreeItem* item)
    {
      if(!item)
        return;
      if(!item->children.isEmpty())
      {
        for(int row = 0; row < item->children.size(); row++)
        {
          clear(item->children[row]);
          delete item->children[row];
        }
        item->children.clear();
      }
    };
    clear(mSettings.rootTreeItem);
    delete mSettings.rootTreeItem;
    mSettings.rootTreeItem = nullptr;
  }
}

void TreeConfirmation::updateCounterLabel(int newCount)
{
  ui->chromatogrammCountLabel->setText(mSettings.updateCounterLabelFunc(newCount));
}

}
}
