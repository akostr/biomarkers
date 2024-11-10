#pragma once

#include "tree_model.h"
#include "context.h"

////////////////////////////////////////////////////
//// Tree model dynamic
class TreeModelDynamic : public TreeModel
{
  Q_OBJECT
public:
  TreeModelDynamic(QObject* parent);
  ~TreeModelDynamic();

  //// Dynamic
  virtual void Reset() override;

  //// Override to handle any context, any data change
  virtual void SetContext(GenesisContextPtr context);

  //// Override to handle specific context, any data change
  virtual void ApplyContextRoot(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextUi(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextUser(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextLibrary(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextProject(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextFileCDF(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkup(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkupVersion(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkupVersionAnalysisPCA(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextMarkupVersionAnalysisMCR(const QString& /*dataId*/, const QVariant& /*data*/) {}
  virtual void ApplyContextCreationPlot(const QString& /*dataId*/, const QVariant& /*data*/) {}

public slots:
  //// Override to handle specific context data change
  virtual void ApplyContextChangesSlot(Core::IStorage* context, const QString& dataId, const QVariant& data);

public:
  //// Dynamic features
  virtual void PeekMore();
  virtual void FetchMore(QPointer<TreeModelItem> item) = 0;

protected:
  //// Contexts stored by class id
  QMap<QString, GenesisContextPtr> Contexts;
  bool isDataReset(const QString& dataId, const QVariant& data);

private:
  void onContextReset(Core::IStorage* context);
};
