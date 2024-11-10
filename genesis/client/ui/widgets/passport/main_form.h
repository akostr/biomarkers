#ifndef PASSPORT_MAIN_FORM_H
#define PASSPORT_MAIN_FORM_H

#include <QWidget>
#include <QPointer>
#include <logic/structures/passport_form_structures.h>
#include <logic/models/passport_combo_models.h>

namespace Ui {
class PassportMainForm;
}

class QTabBar;
class QStandardItemModel;
class QStandardItem;
class QMenu;

class PassportMainForm : public QWidget
{
  Q_OBJECT

public:
  struct ChromaData
  {
    QString name;
    int id;
    Passport::SampleData data;
  };
  enum ColumnNum
  {
    ColumnChroma = 0,
    ColumnRemove,
    ColumnRefresh,
    ColumnLast
  };

public:
  explicit PassportMainForm(QWidget *parent = nullptr);
  ~PassportMainForm();
  void loadComboModels(bool isFragment);

  void SetSampleData(const Passport::SampleData& data);
  Passport::SampleData SampleData();

  void setChromatogrammsList(const QMap<QUuid, QString>& chromasList);
  void setSingleChromatogramm(QString chromaName, int chromaId, Passport::SampleData data);
  void setChromatogramms(const QList<ChromaData>& chromasData);
  void updateChromaLoadingPrecentage(QUuid uid, int loadingPrecentage);
  void updateChromaCompletionPrecentage(QUuid uid, int completionPrecentage);
  void updateChromaFileId(QUuid uid, int fileId);
  bool allChromasHasFileId();
  QJsonArray toJson();
  bool CheckValidity();

signals:
  void modelsLoaded();
  void reloadChroma(QUuid chromaUid);
  void allFilesUploadFinished(bool withErrors);

private slots:
  void on_recommendedToggleButton_toggled(bool checked);
  void on_extendedToggleButton_toggled(bool checked);
  void on_jointRecommendedToggleButton_toggled(bool checked);
  void on_jointExtendedToggleButton_toggled(bool checked);
  void on_clearButton_clicked();
  void recalcSummarySubstance();
  void setSingleUi(bool isSingle);
  void recalcGeometry();
  void remove(QPersistentModelIndex ind);
  void refresh(QPersistentModelIndex ind);

private:

  Ui::PassportMainForm *ui;
  QPointer<QTabBar> mComponentsTabBar;
  QPointer<QStandardItemModel> mChromasModel;
  QStandardItem* mCommonItem;
  QPointer<QMenu> mCommonMenu;
  QPointer<QMenu> mItemMenu;
  QMap<QUuid, QPersistentModelIndex> mRowsMap;
  Passport::SampleData mDataBuffer;
  PassportModelsManager mModelsManager;
  bool isModelsLoaded = false;
  Passport::SampleData mInputDataCache;
  QPointer<QWidget> mLoadingOvelray;

private:
  void setupUi();
  void addComponent(const Passport::MixedSampleGeoComponent& component = Passport::MixedSampleGeoComponent());
  void recalcTabOrder();
  void clearChromas();
  void switchToCommon();
  void setupRemoveButton(int row);
  void setupRefreshButton(int row);
  void removeRefreshButton(int row);
  void removeRemoveButton(int row);
  void handleFilesUploading();
  void showLoadingOverlay();
  void hideLoadingOverlay();

  // QObject interface
protected:
  bool eventFilter(QObject *watched, QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
};



#endif // PASSPORT_MAIN_FORM_H
