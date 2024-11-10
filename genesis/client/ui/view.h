#pragma once

#include "../logic/context.h"

#include <QWidget>
#include <QPointer>

/////////////////////////////////////////////////////
//// Fwds
class GenesisContext;
class TreeModel;

/////////////////////////////////////////////////////
//// View base class
namespace Views
{
class View : public QWidget
{
  Q_OBJECT
public:
  View(QWidget* parent = 0);
  ~View();

  //// Create
  static View* Create(const QString& factoryId, QWidget* parent);
  static View* Create(const std::string& factoryId, QWidget* parent);
  static bool isDataReset(const QString& dataId, const QVariant& data);

  //// Override to handle any context, any data change
//  virtual void SetContext(GenesisContextPtr context);
  virtual void SetContext(GenesisContextPtr context);

  //// Override to handle specific context, any data change
  virtual void ApplyContextRoot(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextUi(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextUser(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextLibrary(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextModules(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextProject(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextFileCDF(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkup(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextStepBasedMarkup(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkupVersion(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkupVersionAnalysisPCA(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkupVersionAnalysisPLS(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkupVersionAnalysisPLSPredict(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkupVersionAnalysisMCR(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextPCAPlots(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextCreationPlot(const QString& /*dataId*/, const QVariant& /*data*/) {}

  virtual void SwitchToPage(const QString& pageId);
  virtual void SwitchToModule(int module);
  virtual void RejectPageSwitching();
  virtual void RejectModuleSwitching();
  virtual void LogOut();
  virtual void RejectLoggingOut();

  //// Helpers
  QList<QPointer<TreeModel>>  GetModels();
  void                        ResetModels();

  static void ForseResetModule();
  static void ForseSetModule(int module);
  static QString CurrentPageId();
  static int CurrentModule();

public slots:
  //// Override to handle specific context data change
  virtual void ApplyContextChangesSlot(Core::IStorage* context, const QString& dataId, const QVariant& data);
//  virtual void ApplyContextChangesSlot(Core::IStorage *storage);

signals:
  void moduleSwitchProcessed(int module, bool switchAccepted);
  void pageSwitchProcessed(QString pageId, bool switchAccepted);
  void logOutAcceptStatus(bool rejected = false);

protected:
  //// Contexts stored by class id
  QMap<QString, GenesisContextPtr> Contexts;

private:
  void onContextReset(Core::IStorage* context);
};
}//namespace Views;
