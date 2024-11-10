#pragma once
#ifndef TREE_MODEL_PRESENTATION_LIBRARY_COMPOUND_H
#define TREE_MODEL_PRESENTATION_LIBRARY_COMPOUND_H

#include "tree_model_presentation.h"
#include "enums.h"

class TreeModelPresentationLibraryCompounds final : public TreeModelPresentation
{
  Q_OBJECT

public:
  explicit TreeModelPresentationLibraryCompounds(TreeModel* parent = nullptr);
  ~TreeModelPresentationLibraryCompounds() = default;

  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
  QVariant GetModelValue(int sourceRow, int sourceColumn, const QString& key, const QModelIndex& sourceParent) const;
  QVariant GetModelValue(const QModelIndex& index, const QString& key) const;

  bool FilterByFileType(LibraryFilter::FileType type, int sourceRow, const QModelIndex& sourceParent) const;
  bool FilterByClassifier(LibraryFilter::Classifiers classifier, int sourceRow, const QModelIndex& sourceParent) const;
  bool FilterBySpecific(LibraryFilter::Specifics classifier, int sourceRow, const QModelIndex& sourceParent) const;

  bool FilterByGroup(const QString& groupName, int sourceRow, const QModelIndex& sourceParent) const;
  bool FilterByName(const QString& name, int sourceRow, const QModelIndex& sourceParent) const;

protected:
  //sorting disabled and enabled items
  bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};
#endif
