#ifndef TREEMODELDYNAMICPROJECTMODELSLIST_H
#define TREEMODELDYNAMICPROJECTMODELSLIST_H

#include "tree_model_dynamic.h"
#include "enums.h"

class TreeModelDynamicProjectModelsList : public TreeModelDynamic
{
  Q_OBJECT
public:
  enum Column
  {
    ColumnTitle,
    ColumnTableTitle,
    ColumnComment,
    ColumnSamplesCount,
    ColumnPeakCount,
    ColumnCreated,

    ColumnLast
  };

  TreeModelDynamicProjectModelsList(QObject *parent,
  									Constants::AnalysisType modelType = Constants::AnalysisType::PCA,
  									Constants::AnalysisType filteringType = Constants::AnalysisType::PCA);
  ~TreeModelDynamicProjectModelsList();

public:
  //// Handle
  virtual void ApplyContextProject(const QString &dataId, const QVariant &data) override;

  //// Dynamic
  virtual void Reset() override;

  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;

  virtual void FetchMore(QPointer<TreeModelItem> item) override;
  void SetType(Constants::AnalysisType type);
  void SetFilteringType(Constants::AnalysisType type);

private:
  Constants::AnalysisType ModelType = Constants::AnalysisType::PCA;
  Constants::AnalysisType FilteringType = Constants::AnalysisType::PCA;

private:
  void preprocessFetch(QVariantMap& variantMap);
};

#endif // TREEMODELDYNAMICPROJECTMODELSLIST_H
