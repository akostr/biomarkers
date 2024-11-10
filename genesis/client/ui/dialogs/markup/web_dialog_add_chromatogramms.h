#ifndef WEB_DIALOG_ADD_CHROMATOGRAMMS_H
#define WEB_DIALOG_ADD_CHROMATOGRAMMS_H

#include <ui/dialogs/templates/dialog.h>

namespace Ui {
class WebDialogAddChromatogramms;
}
class QStandardItemModel;
class QSortFilterProxyModel;
class QStandardItem;
namespace Dialogs
{
class WebDialogAddChromatogramms : public Templates::Dialog
{
  Q_OBJECT
private:
  enum Page
  {//taken from designer
    choise,//0
    reference,//1
    chromas//2
  };
  enum UploadStatus
  {
    uploadError,
    uploadSuccess,
    uploadInProgress,
    notUploaded
  };
  enum ReferenceComboDataRole
  {
    RIdRole = Qt::UserRole + 1,
    RFileIdRole,
    RLast
  };
  enum ChromasDataRole
  {
    CIdRole = RLast + 1,
    CProjectFileIdRole,
    CAdditionalTitleRole,
    CGroupTitleRole,
    CGroupIdRole,
    CIsGroupRole,
    CFileTypeRole
  };

public:
  explicit WebDialogAddChromatogramms(QList<int> existedIds, int referenceId, QWidget *parent = nullptr);
    void loadModels(QList<int> acceptableFilesTypes, QString m_z=QString());
  ~WebDialogAddChromatogramms();
  int getReferenceId();
  int getReferenceFileId();
  QList<int> getChromasIds();
  bool isReferenceMode();
  bool isChromasMode();
  void setRegExSearch(bool regExSearch);
  void applyFilter(const QString& filter);

signals:
  void modelsLoaded();

public slots:
  void Accept() override;

private:
  Ui::WebDialogAddChromatogramms *ui;
  QWidget* mContent;
  QList<int> mExistedChromasIds;
  int mReferenceId;
  QStandardItemModel* mModel;
  QSortFilterProxyModel* mProxy;
  bool mRegExSearch = false;

  UploadStatus referencesLoaded = notUploaded;
  UploadStatus chromasLoaded = notUploaded;
  QString MZ;

private:
  void setupUi();
  void loadReferences(QList<int> acceptableFilesTypes);
  void loadChomas(QList<int> acceptableFilesTypes);
  void handleUploadStatus();
  void setAllChromasCheckState(Qt::CheckState state);
  Qt::CheckState getChromasTableCheckState();
  void onCheckAllStateChanged(int newState);
  QList<int> getSelectedProjectFilesIds();
  void onItemCheckStateChanged(QStandardItem* item, Qt::CheckState state);
  void validateItemCheckState(QStandardItem* item);

  void validateButtons();
};
}//namespace Dialogs
#endif // WEB_DIALOG_ADD_CHROMATOGRAMMS_H
