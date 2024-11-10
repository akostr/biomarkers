#ifndef VIEW_PAGE_LIBRARY_H
#define VIEW_PAGE_LIBRARY_H

#include "view.h"
#include "logic/enums.h"
#include <QMutex>

namespace Ui {
class ViewPageLibrary;
}
class TreeModelDynamicLibraryCompoundsAdditional;
class TreeModelPresentation;
namespace Views
{
class ViewPageLibrary : public View
{
  Q_OBJECT

public:
  enum class Tab
  {
    Compounds,
    Coefficients,
    PlotTemplates
  };

  enum CompoundsDetailsPages
  {
    CDPDetails,
    CDPAdd,
    CDPModify,
    CDPEmpty
  };

  enum CompoundsAddingDetailsPages
  {
    CADPAdd,
    CADPDetails
  };

  enum CompoundsTablesPages
  {
    CTPView,
    CTPAdd
  };

  explicit ViewPageLibrary(QWidget *parent = nullptr);
  ~ViewPageLibrary();

  bool loadAttachments(int libraryGroupId, QUuid overlayId, QString pdfPath, QString spectrumPath);
  void removeAttachments(int libraryGroupId, int pdfFileId, QUuid overlayId, bool pdfToRemove, bool spectrumToRemove);

signals:
  void attachmentsLoadingError(int libraryGroupId);
  void compoundUpdatingComplete();
  void coefficientUpdatingComplete();

private:
  Ui::ViewPageLibrary *ui;
  QAction* NameSearchAction = nullptr;
  QAction* NameSearchClearAction = nullptr;
  QAction* MakeSystemCompoundAction = nullptr;
  QAction* ShareAccessCompoundAction = nullptr;
  QAction* MakeSystemCoefficientAction = nullptr;
  QAction* ShareAccessCoefficientAction = nullptr;
  bool isTabUpdateInProgress = false;
  TreeModelDynamicLibraryCompoundsAdditional* mCompundAddingModel = nullptr;
  TreeModelDynamicLibraryCompoundsAdditional* mCoefficientAddingModel = nullptr;
  QPointer<TreeModelPresentation> mCompundAddingProxyModel;
  QPointer<TreeModelPresentation> mCoefficientAddingProxyModel;
  QMap<QUuid, QStringList> mAwaitingRequests;
  int attachmentsLoadingErrorForId = -1;
  QMutex awaitingRequestsMutex;

private:
  void setupUi();
  void setupCompoundTab();
  void setupCoefficientTab();
  void ConnectSignals();
  void connectCompoundSignals();
  void connectCoefficientSignals();
  void connectTemplateSignals();
  void applyComboFilter();
  void tabWidgetIndexChanged(int index);
  void beginCompoundAdding();
  void beginCoefficientAdding();
  void disableNotCurrentTab(bool isDisable);
  void leaveCompoundAdding();
  void leaveCoefficientAdding();
  void disableUiCombobox(bool isDisable);
  void disableUiForOpenShare(bool disable);
  void actionModeTriggered();
  void cancelActionModeTriggered();
  bool checkInputValidity(const QVariantMap &data, QString *msg = nullptr);
  void beginCompoundModifying();
  void beginCoefficientModifying();
  void leaveCompoundModifying();
  void leaveCoefficientModifying();
  void onCompoundPicked(const QVariantMap& item);
  void onCoefficientPicked(const QVariantMap& item);
  void handleEmptyData(const QVariantMap& item);

  void beginSelectedCompoundSharing();
  void beginSelectedCoefficientSharing();
  void beginSelectedCompoundRemoving();
  void beginSelectedCoefficientRemoving();
  void makeCompoundSystemic();
  void makeCoefficientSystemic();

  void onCompoundModificationConfirmed();
  void onCoefficientModificationConfirmed();
  void onCompoundAdditionConfirmed();
  void onCoefficientAdditionConfirmed();

  void updateCompoundSharingMenu();
  void updateCoefficientSharingMenu();
  void share(int libraryGroupId, const QString& group);
  void removeCompound(int libraryGroupId, QString name);
  void removeCoefficient(int libraryGroupId, QString name);
  // void updatePlotTemplatesMenu();

  void addRequestAwait(const QString& requestKey, QUuid overlayId);
  void closeRequest(const QString& requestKey);

  void updateTemplateAxisList();

  // View interface
public:
  void ApplyContextLibrary(const QString &dataId, const QVariant &data) override;
  void ApplyContextUser(const QString &dataId, const QVariant &data) override;

  // QObject interface
public:
  bool eventFilter(QObject *watched, QEvent *event) override;
};
}//namespace Views

#endif // VIEW_PAGE_LIBRARY_H
