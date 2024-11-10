#include "tree_model_presentation_library_compounds.h"
#include "logic/enums.h"
#include "logic/structures/library/library_structures.h"
#include "logic/tree_model_dynamic_library_compounds.h"
#include "logic/tree_model_item.h"

using namespace LibraryFilter;
using namespace Structures;

using ModelColumns = TreeModelDynamicLibraryCompounds::Column;

TreeModelPresentationLibraryCompounds::TreeModelPresentationLibraryCompounds(TreeModel* parent)
  :TreeModelPresentation(parent)
{
}

bool TreeModelPresentationLibraryCompounds::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
  auto result = TreeModelPresentation::filterAcceptsRow(sourceRow, sourceParent);
  if (!result || !RowFilter.canConvert<LibraryFilterState>())
    return result;

  const auto filterState = RowFilter.value<LibraryFilterState>();
  if (!FilterByGroup(filterState.Group, sourceRow, sourceParent))
    return false;

  if (!FilterByFileType(filterState.Type, sourceRow, sourceParent))
    return false;

  if (!FilterByClassifier(filterState.Classifier, sourceRow, sourceParent))
    return false;

  if (!FilterBySpecific(filterState.Specific, sourceRow, sourceParent))
    return false;

  if (!FilterByName(filterState.Name, sourceRow, sourceParent))
    return false;

  return result;
}

QVariant TreeModelPresentationLibraryCompounds::GetModelValue(int sourceRow, int sourceColumn, const QString& key, const QModelIndex& sourceParent) const
{
  return GetModelValue(Source->index(sourceRow, sourceColumn, sourceParent), key);
}

QVariant TreeModelPresentationLibraryCompounds::GetModelValue(const QModelIndex& index, const QString& key) const
{
  const auto groupItem = Source->GetItem(index);
  return groupItem->GetData(key);
}

bool TreeModelPresentationLibraryCompounds::FilterByFileType(LibraryFilter::FileType type, int sourceRow, const QModelIndex& sourceParent) const
{
  if (type == FileType::All)
    return true;

  const auto groupData = GetModelValue(sourceRow, static_cast<int>(ModelColumns::GROUP), "group", sourceParent);

  if (!groupData.isValid())
    return true;

  const auto isSystem = GetModelValue(sourceRow, 0, "system", sourceParent).toBool();
  if (type == FileType::System
    && !isSystem)
    return false;

  if (type == FileType::User
    && isSystem)
    return false;

  const auto favoriteData = GetModelValue(sourceRow, static_cast<int>(ModelColumns::FAVOURITES), "favourites", sourceParent);
  if(type == FileType::Favorite
    && !favoriteData.toBool())
    return false;

  return true;
}

bool TreeModelPresentationLibraryCompounds::FilterByClassifier(Classifiers classifier, int sourceRow, const QModelIndex& sourceParent) const
{
  if (classifier == Classifiers::All)
    return true;

  const auto rowClassifier = GetModelValue(sourceRow, static_cast<int>(ModelColumns::CLASSIFIER_ID), "classifier_id", sourceParent);
  if (!rowClassifier.isValid())
    return true;

  return classifier == static_cast<Classifiers>(rowClassifier.toInt());
}

bool TreeModelPresentationLibraryCompounds::FilterBySpecific(LibraryFilter::Specifics classifier, int sourceRow, const QModelIndex& sourceParent) const
{
  if (classifier == Specifics::All)
    return true;

  const auto rowSpecific = GetModelValue(sourceRow, static_cast<int>(ModelColumns::SPECIFIC_ID), "specific_id", sourceParent);
  if (!rowSpecific.isValid())
    return true;

  return classifier == static_cast<Specifics>(rowSpecific.toInt());
}

bool TreeModelPresentationLibraryCompounds::FilterByGroup(const QString& groupName, int sourceRow, const QModelIndex& sourceParent) const
{
  if (groupName.isEmpty())
    return true;
  const auto groupData = GetModelValue(sourceRow, static_cast<int>(ModelColumns::GROUP), "group", sourceParent);
  if (groupData.isNull())
    return true;
  return groupData.toString() == groupName;
}

bool TreeModelPresentationLibraryCompounds::FilterByName(const QString &name, int sourceRow, const QModelIndex &sourceParent) const
{
  if(name.isEmpty())
    return true;
  const auto shortNameData = GetModelValue(sourceRow, static_cast<int>(ModelColumns::SHORT_TITLE), "short_title", sourceParent).toString();
  const auto fullNameData = GetModelValue(sourceRow, static_cast<int>(ModelColumns::FULL_TITLE), "full_title", sourceParent).toString();
  if(shortNameData.contains(name, Qt::CaseInsensitive) ||
      fullNameData.contains(name, Qt::CaseInsensitive))
    return true;
  return false;
}

bool TreeModelPresentationLibraryCompounds::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
  if (source_left.column() == ModelColumns::FAVOURITES)
  {
    const auto leftValue = GetModelValue(source_left, "favourites");
    const auto rightValue = GetModelValue(source_right, "favourites");
    return leftValue.toBool() && !rightValue.toBool();
  }
  else if (source_left.column() == ModelColumns::GROUP)
  {
    const auto leftValue = GetModelValue(source_left, "system");
    const auto rightValue = GetModelValue(source_right, "system");
    return leftValue.toBool() && !rightValue.toBool();
  }
  return TreeModelPresentation::lessThan(source_left, source_right);
}
