#include "passport_combo_models.h"
#include <logic/structures/passport_form_structures.h>
#include <QCoreApplication>
#include <QStandardItemModel>
#include <api/api_rest.h>
#include <logic/notification.h>

PassportModelsManager::PassportModelsManager(QObject *parent)
  : QObject(parent)
{
  mFieldsModel = new QStandardItemModel(0, 1, this);
  mLayersModel = new QStandardItemModel(0, 1, this);
  mSampleTypesModel = new QStandardItemModel(0, 1, this);
  mFluidTypesModel = new QStandardItemModel(0, 1, this);
  mAltitudesModel = new QStandardItemModel(0, 1, this);
  mClusterPumpingStationsModel = new QStandardItemModel(0, 1, this);
  mBoosterPumpingStationsModel = new QStandardItemModel(0, 1, this);
  mOilAndGasProductionStationsModel = new QStandardItemModel(0, 1, this);
  mDetectorsModel = new QStandardItemModel(0, 1, this);
  mCarrierGasesModel = new QStandardItemModel(0, 1, this);
  mSamplePreparingsModel = new QStandardItemModel(0, 1, this);

  connect(mFieldsModel, &QStandardItemModel::rowsInserted, this,
          [this](const QModelIndex& parent, int first, int last)
  {
    for(int r = first; r <= last; r++)
    {
      auto model = mFieldsModel;
      auto ind = model->index(r, 0);
      auto vId = model->data(ind, idRole);
      auto vChildModel = model->data(ind, idRole);
      if(!vId.isValid() || vId.isNull() || !vChildModel.isValid() || vChildModel.isNull())
      {//if new item hasn't id and|or child model pointer, we should create it:
        int newFieldId = generateUniqueFieldId();
        mExistedFieldsIds.insert(newFieldId);
        auto newWellClusterModel = addWellClusterModel(newFieldId);
        model->setData(ind, newFieldId, idRole);
        model->setData(ind, QVariant::fromValue(newWellClusterModel.data()), childModelRole);
      }
      else
      {
        mExistedFieldsIds.insert(vId.toInt());
      }
    }
  });
  connect(mLayersModel, &QStandardItemModel::rowsInserted, this,
          [this](const QModelIndex& parent, int first, int last)
  {
    for(int r = first; r <= last; r++)
    {
      auto model = mLayersModel;
      auto ind = model->index(r, 0);
      auto vId = model->data(ind, idRole);
      if(!vId.isValid() || vId.isNull())
      {//if new item hasn't id, we should create it:
        int newLayerId = generateUniqueLayerId();
        model->setData(ind, newLayerId, idRole);
      }
    }
  });
}

PassportModelsManager::~PassportModelsManager()
{
  resetModels();
}

QPointer<QStandardItemModel>PassportModelsManager::getFieldsModel()
{
  return mFieldsModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getLayersModel()
{
  return mLayersModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getSampleTypesModel()
{
  return mSampleTypesModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getFluidTypesModel()
{
  return mFluidTypesModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getAltitudesModel()
{
  return mAltitudesModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getClusterPumpingStationsModel()
{
  return mClusterPumpingStationsModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getBoosterPumpingStationsModel()
{
  return mBoosterPumpingStationsModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getOilAndGasProductionStationsModel()
{
  return mOilAndGasProductionStationsModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getDetectorsModel()
{
  return mDetectorsModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getCarrierGasesModel()
{
  return mCarrierGasesModel;
}

QPointer<QStandardItemModel>PassportModelsManager::getSamplePreparingsModel()
{
  return mSamplePreparingsModel;
}

bool PassportModelsManager::addField(QString name, int id)
{
  if(mExistedFieldsIds.contains(id))
    return false;
  auto newFieldItem = new QStandardItem(name);
  newFieldItem->setData(id, ComboItemDataRole::idRole);
  auto newWellClusterModel = addWellClusterModel(id);
  mExistedFieldsIds.insert(id);
  newFieldItem->setData(QVariant::fromValue(newWellClusterModel.data()), childModelRole);
  mFieldsModel->appendRow(newFieldItem);
  return true;
}

bool PassportModelsManager::addWellCluster(QString name, int id, int fieldId)
{
  if(mExistedWellClustersIds.contains(id))
    return false;
  auto newWellClusterItem = new QStandardItem(name);
  newWellClusterItem->setData(id, ComboItemDataRole::idRole);
  auto newWellModel = addWellModel(id);
  mExistedWellClustersIds.insert(id);
  newWellClusterItem->setData(QVariant::fromValue(newWellModel.data()), childModelRole);
  mWellClustersModels[fieldId]->appendRow(newWellClusterItem);
  return true;
}

bool PassportModelsManager::addWell(QString name, int id, int wellClusterId)
{
  if(mExistedWellsIds.contains(id))
    return false;
  auto newWellItem = new QStandardItem(name);
  newWellItem->setData(id, ComboItemDataRole::idRole);
  mExistedWellsIds.insert(id);
  mWellsModels[wellClusterId]->appendRow(newWellItem);
  return true;
}

void PassportModelsManager::resetModels()
{
  mLayersModel->clear();
  mAltitudesModel->clear();
  mClusterPumpingStationsModel->clear();
  mBoosterPumpingStationsModel->clear();
  mOilAndGasProductionStationsModel->clear();

  setData(mCachedData);
}

void PassportModelsManager::loadModels(ComboModelFileType type)
{
  uploadReport = UploadResultReport();

//#define DEBUG_FILL
#ifndef DEBUG_FILL
  auto fillModel = [type](QStandardItemModel* model, const QJsonDocument& doc)
  {
    if(!model)
      return;
    auto obj = doc.object();
    if(!obj.contains("data"))
      return;
    auto data = obj["data"].toArray();
    for(int i = 0; i < data.size(); i++)
    {
      auto elem = data[i].toArray();
      if(elem.size() < 2)
        return;
      if(elem.size() == 3 &&
          type == FFragmentType &&
          !elem[2].toBool())
        continue;
      QStandardItem* item;
      item = new QStandardItem(elem[1].toString());
      item->setData(elem[0].toInt(), Qt::UserRole);
      model->appendRow(item);
    }
  };

  auto handleReport = [this]()
  {
    bool loadingFinished = uploadReport.sampleTypes &&
                           uploadReport.fluidTypes &&
                           uploadReport.samplePreparators &&
                           uploadReport.gasCarriers &&
                           uploadReport.detectors &&
                           uploadReport.fieldsClustersWellsLayersHierarchy;
    if(loadingFinished)
      emit modelsLoaded();
  };

  API::REST::GetSampleTypes(
        [this, fillModel, handleReport](QNetworkReply*, QJsonDocument doc)
  {
    fillModel(mSampleTypesModel, doc);
    uploadReport.sampleTypes = loaded;
    handleReport();
  },
  [this, handleReport](QNetworkReply*, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Can't load sample types from server"), tr("Network error"), err);
    uploadReport.sampleTypes = error;
    handleReport();
  });

  API::REST::GetFluidTypes(
        [this, fillModel, handleReport](QNetworkReply*, QJsonDocument doc)
  {
    fillModel(mFluidTypesModel, doc);
    uploadReport.fluidTypes = loaded;
    handleReport();
  },
  [this, handleReport](QNetworkReply*, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Can't load fluid types from server"), tr("Network error"), err);
    uploadReport.fluidTypes = error;
    handleReport();
  });

  API::REST::GetSamplePreparators(
        [this, fillModel, handleReport](QNetworkReply*, QJsonDocument doc)
  {
    fillModel(mSamplePreparingsModel, doc);
    uploadReport.samplePreparators = loaded;
    handleReport();
  },
  [this, handleReport](QNetworkReply*, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Can't load sample preparators types from server"), tr("Network error"), err);
    uploadReport.samplePreparators = error;
    handleReport();
  });

  API::REST::GetGasCarriers(
        [this, fillModel, handleReport](QNetworkReply*, QJsonDocument doc)
  {
    fillModel(mCarrierGasesModel, doc);
    uploadReport.gasCarriers = loaded;
    handleReport();
  },
  [this, handleReport](QNetworkReply*, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Can't load gas carriers types from server"), tr("Network error"), err);
    uploadReport.gasCarriers = error;
    handleReport();
  });

  API::REST::GetDetectors(
        [this, fillModel, handleReport](QNetworkReply*, QJsonDocument doc)
  {
    fillModel(mDetectorsModel, doc);
    uploadReport.detectors = loaded;
    handleReport();
  },
  [this, handleReport](QNetworkReply*, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Can't load detectors types from server"), tr("Network error"), err);
    uploadReport.detectors = error;
    handleReport();
  });

  API::REST::GetFieldsClustersWellsLayersHierarchy(
        [this, handleReport](QNetworkReply*, QJsonDocument doc)
  {
    CombosData data;
    auto obj = doc.object();
    //hierarchy:
    auto jfields = obj["fields"].toArray();
    for(int jfieldInd = 0; jfieldInd < jfields.size(); jfieldInd++)
    {
      ComboField field;
      auto jfield = jfields[jfieldInd].toObject();
      field.id = jfield["field_id"].toInt();
      field.name = jfield["field_title"].toString();

      auto jwellClusters = jfield["well_clusters"].toArray();
      for(int jwcInd = 0; jwcInd < jwellClusters.size(); jwcInd++)
      {
        auto jwcluster = jwellClusters[jwcInd].toObject();
        ComboWellCluster wellCluster;
        wellCluster.id = jwcluster["well_cluster_id"].toInt();
        wellCluster.name = jwcluster["well_cluster_title"].toString();

        auto jwells = jwcluster["wells"].toArray();
        for(int jwellInd = 0; jwellInd < jwells.size(); jwellInd++)
        {
          auto jwell = jwells[jwellInd].toObject();
          ComboEntry well;
          well.id = jwell["well_id"].toInt();
          well.name = jwell["well_title"].toString();

          wellCluster.wells << well;
        }
        field.wellClusters << wellCluster;
      }
      data.fields << field;
    }
    //layers:
    auto jlayers = obj["layers"].toArray();
    for(int i = 0; i < jlayers.size(); i++)
    {
      auto jlayer = jlayers[i].toObject();
      ComboEntry layer;
      layer.id = jlayer["layer_id"].toInt();
      layer.name = jlayer["layer_title"].toString();
      data.layers << layer;
    }
    setData(data);
    uploadReport.fieldsClustersWellsLayersHierarchy = loaded;
    handleReport();
  },
  [this, handleReport](QNetworkReply*, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Can't load fields hierarchy types from server"), tr("Network error"), err);
    uploadReport.fieldsClustersWellsLayersHierarchy = error;
    handleReport();
  });

#else//leave it here for debug purposes
  auto addItem = [](QStandardItemModel* model, QString itemText, QVariant itemUserData)
  {
    QStandardItem* item;
    item = new QStandardItem(itemText);
    item->setData(itemUserData, Qt::UserRole);
    model->appendRow(item);
  };

  addItem(mSampleTypesModel, QCoreApplication::translate("Passport Namespace","Pure"), 1);
  addItem(mSampleTypesModel, QCoreApplication::translate("Passport Namespace","Mixed"), 2);
  addItem(mSampleTypesModel, QCoreApplication::translate("Passport Namespace","Joint"), 3);

  addItem(mFluidTypesModel, QCoreApplication::translate("Passport Namespace","Water"), 1);
  addItem(mFluidTypesModel, QCoreApplication::translate("Passport Namespace","Oil"), 2);
  addItem(mFluidTypesModel, QCoreApplication::translate("Passport Namespace","Gas"), 3);
  addItem(mFluidTypesModel, QCoreApplication::translate("Passport Namespace","Condensate"), 4);
  addItem(mFluidTypesModel, QCoreApplication::translate("Passport Namespace","Mixed"), 5);

  addItem(mSamplePreparingsModel, QCoreApplication::translate("Passport Namespace","Dry fluid"), 1);
  addItem(mSamplePreparingsModel, QCoreApplication::translate("Passport Namespace","Extracted fluid"), 2);
  addItem(mSamplePreparingsModel, QCoreApplication::translate("Passport Namespace","No preparation"), 3);
  addItem(mSamplePreparingsModel, QCoreApplication::translate("Passport Namespace","Saturated fraction"), 4);
  addItem(mSamplePreparingsModel, QCoreApplication::translate("Passport Namespace","Aromatic fraction"), 5);
  addItem(mSamplePreparingsModel, QCoreApplication::translate("Passport Namespace","Maltene"), 6);

  addItem(mDetectorsModel, QCoreApplication::translate("Passport Namespace","FID (flame ionization detector)"), 1);
  addItem(mDetectorsModel, QCoreApplication::translate("Passport Namespace","FPD (flame photometric detector)"), 2);
  addItem(mDetectorsModel, QCoreApplication::translate("Passport Namespace","TCD (thermal conductivity detector)"), 3);
  addItem(mDetectorsModel, QCoreApplication::translate("Passport Namespace","MD (mass-detector)"), 4);

  addItem(mCarrierGasesModel, QCoreApplication::translate("Passport Namespace","Helium"), 1);
  addItem(mCarrierGasesModel, QCoreApplication::translate("Passport Namespace","Nitrogen"), 2);
  addItem(mCarrierGasesModel, QCoreApplication::translate("Passport Namespace","Hydrogen"), 3);
  addItem(mCarrierGasesModel, QCoreApplication::translate("Passport Namespace","Argon"), 4);

  CombosData data;
  int wcid = 0;
  int wid = 0;
  QList<ComboField> fields;
  for(int f = 0; f < 3; f++)
  {
    ComboField field;
    field.id = f;
    field.name = QString("f%1").arg(f);
    for(int wc = 0; wc < 3; wc++)
    {
      ComboWellCluster wellCluster;
      wellCluster.id = wcid++;
      wellCluster.name = QString("%1 wc%2").arg(field.name).arg(wellCluster.id);
      for(int w = 0; w < 3; w++)
      {
        ComboEntry well;
        well.id = wid++;
        well.name = QString("%1 w%2").arg(wellCluster.name).arg(well.id);
        wellCluster.wells << well;
      }
      field.wellClusters << wellCluster;
    }
    fields << field;
  }
  QList<ComboEntry> layers = {{0, "l1"},{1, "l2"},{2, "l3"}};
  data.fields = fields;
  data.layers = layers;
  setData(data);
  QTimer::singleShot(5000, this, [this](){emit modelsLoaded();});
#endif
}

int PassportModelsManager::generateUniqueFieldId()
{
  return mFieldUniqueId--;
}

int PassportModelsManager::generateUniqueWellId()
{
  return mWellUniqueId--;
}

int PassportModelsManager::generateUniqueWellClusterId()
{
  return mWellClusterUniqueId--;
}

int PassportModelsManager::generateUniqueLayerId()
{
  return mLayerUniqueId--;
}

void PassportModelsManager::setData(const CombosData &data)
{
  mFieldsModel->clear();
  for(auto& m : mWellClustersModels)
    m->deleteLater();
  mWellClustersModels.clear();
  for(auto& m : mWellsModels)
    m->deleteLater();
  mWellsModels.clear();
  mExistedFieldsIds.clear();
  mExistedWellsIds.clear();
  mExistedWellClustersIds.clear();
  bool added = false;
  for(auto& f : data.fields)
  {
    added = addField(f.name, f.id);
    Q_ASSERT(added);
    for(auto& c : f.wellClusters)
    {
      added = addWellCluster(c.name, c.id, f.id);
      Q_ASSERT(added);
      for(auto& w : c.wells)
      {
        added = addWell(w.name, w.id, c.id);
        Q_ASSERT(added);
      }
    }
  }
  for(auto& l : data.layers)
  {
    auto layerItem = new QStandardItem(l.name);
    layerItem->setData(l.id, idRole);
    mLayersModel->appendRow(layerItem);
  }
}

QPointer<QStandardItemModel> PassportModelsManager::addWellClusterModel(int fieldId)
{
  if(!mWellClustersModels.contains(fieldId))
  {
    mWellClustersModels[fieldId] = new QStandardItemModel(0, 1, this);
    connect(mWellClustersModels[fieldId], &QStandardItemModel::rowsInserted, this,
            [fieldId, this](const QModelIndex& parent, int first, int last)
    {
      for(int r = first; r <= last; r++)
      {
        auto model = mWellClustersModels[fieldId];
        auto ind = model->index(r, 0);
        auto vId = model->data(ind, idRole);
        auto vChildModel = model->data(ind, idRole);
        if(!vId.isValid() || vId.isNull() || !vChildModel.isValid() || vChildModel.isNull())
        {//if new item hasn't id and|or child model pointer, we should create it:
          int newWellClusterId = generateUniqueWellClusterId();
          mExistedWellClustersIds.insert(newWellClusterId);
          auto newWellModel = addWellModel(newWellClusterId);
          model->setData(ind, newWellClusterId, idRole);
          model->setData(ind, QVariant::fromValue(newWellModel.data()), childModelRole);
        }
        else
        {
          mExistedWellClustersIds.insert(vId.toInt());
        }
      }
    });
  }
  return mWellClustersModels[fieldId];
}

QPointer<QStandardItemModel> PassportModelsManager::addWellModel(int wellClusterId)
{
  if(!mWellsModels.contains(wellClusterId))
  {
    mWellsModels[wellClusterId] = new QStandardItemModel(0, 1, this);
    connect(mWellsModels[wellClusterId], &QStandardItemModel::rowsInserted, this,
            [wellClusterId, this](const QModelIndex& parent, int first, int last)
    {
      for(int r = first; r <= last; r++)
      {
        auto model = mWellsModels[wellClusterId];
        auto ind = model->index(r, 0);
        auto vId = model->data(ind, idRole);
        if(!vId.isValid() || vId.isNull())
        {//if new item hasn't id, we should create it:
          int newWellId = generateUniqueWellId();
          mExistedWellsIds.insert(newWellId);
          model->setData(ind, newWellId, idRole);
        }
        else
        {
          mExistedWellsIds.insert(vId.toInt());
        }
      }
    });
  }
  return mWellsModels[wellClusterId];
}
