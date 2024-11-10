#ifndef FIELDCOMBOMODEL_H
#define FIELDCOMBOMODEL_H

//#include <QStandardItemModel>
#include <QObject>
#include <QMap>
#include <QSet>
#include <QPointer>

class QStandardItemModel;
class PassportModelsManager : public QObject
{
  Q_OBJECT
public:
  struct ComboEntry
  {
    int id = -1;
    QString name;
    bool isValid(){return id >= 0;}
  };
  struct ComboWellCluster : public ComboEntry  {QList<ComboEntry> wells;};
  struct ComboField       : public ComboEntry  {QList<ComboWellCluster> wellClusters;};
  struct CombosData                            {QList<ComboField> fields; QList<ComboEntry> layers;};
  enum ComboItemDataRole
  {
    idRole = Qt::UserRole + 1,
    childModelRole
  };
  enum ComboModelDependencyType
  {
    NoDependencies,
    WellClusters,
    Wells
  };
  enum ComboModelFileType
  {
    FChromaType,
    FFragmentType
  };

  PassportModelsManager(QObject* parent = nullptr);
  ~PassportModelsManager();

  QPointer<QStandardItemModel> getFieldsModel();
  QPointer<QStandardItemModel> getLayersModel();
  QPointer<QStandardItemModel> getSampleTypesModel();
  QPointer<QStandardItemModel> getFluidTypesModel();
  QPointer<QStandardItemModel> getAltitudesModel();
  QPointer<QStandardItemModel> getClusterPumpingStationsModel();
  QPointer<QStandardItemModel> getBoosterPumpingStationsModel();
  QPointer<QStandardItemModel> getOilAndGasProductionStationsModel();

  QPointer<QStandardItemModel> getDetectorsModel();
  QPointer<QStandardItemModel> getCarrierGasesModel();
  QPointer<QStandardItemModel> getSamplePreparingsModel();

  bool addField(QString name, int id);
  bool addWellCluster(QString name, int id, int fieldId);
  bool addWell(QString name, int id, int wellClusterId);

  void resetModels();
  void loadModels(ComboModelFileType type);

signals:
  void modelsLoaded();

private:
  enum UploadResult
  {
    notLoaded,
    loaded,
    error
  };
  struct UploadResultReport
  {
    UploadResult sampleTypes = notLoaded;
    UploadResult fluidTypes = notLoaded;
    UploadResult samplePreparators = notLoaded;
    UploadResult gasCarriers = notLoaded;
    UploadResult detectors = notLoaded;
    UploadResult fieldsClustersWellsLayersHierarchy = notLoaded;
  };
  UploadResultReport uploadReport;

  QMap<int, QPointer<QStandardItemModel>> mWellClustersModels;
  QMap<int, QPointer<QStandardItemModel>> mWellsModels;

  QPointer<QStandardItemModel> mFieldsModel;
  QPointer<QStandardItemModel> mLayersModel;
  QPointer<QStandardItemModel> mSampleTypesModel;
  QPointer<QStandardItemModel> mFluidTypesModel;
  QPointer<QStandardItemModel> mAltitudesModel;
  QPointer<QStandardItemModel> mClusterPumpingStationsModel;
  QPointer<QStandardItemModel> mBoosterPumpingStationsModel;
  QPointer<QStandardItemModel> mOilAndGasProductionStationsModel;
  QPointer<QStandardItemModel> mDetectorsModel;
  QPointer<QStandardItemModel> mCarrierGasesModel;
  QPointer<QStandardItemModel> mSamplePreparingsModel;
  CombosData mCachedData;

  int mFieldUniqueId = -2;
  int mWellClusterUniqueId = -2;
  int mWellUniqueId = -2;
  int mLayerUniqueId = -2;
  const int kInvalidId = -1;

  QSet<int> mExistedFieldsIds;
  QSet<int> mExistedWellClustersIds;
  QSet<int> mExistedWellsIds;
  QMap<int, QString> mSampleTypesMap;
  QMap<int, QString> mFluidTypesMap;
  QMap<int, QString> mDetectorsMap;
  QMap<int, QString> mCarrierGasesMap;
  QMap<int, QString> mSamplePreparatorsMap;

private:
  int generateUniqueFieldId();
  int generateUniqueWellId();
  int generateUniqueWellClusterId();
  int generateUniqueLayerId();
  void setData(const CombosData &data);
  QPointer<QStandardItemModel> addWellClusterModel(int fieldId);
  QPointer<QStandardItemModel> addWellModel(int wellClusterId);
};
#endif // FIELDCOMBOMODEL_H
