#include "identification_presentation_model.h"
#include "logic/tree_model_item.h"

using namespace LibraryFilter;

IdentificationPresentationModel::IdentificationPresentationModel(TreeModel *parent)
  :TreeModelPresentation(parent)
{

}

void IdentificationPresentationModel::setWhiteListIds(const QList<int> ids)
{
  mIdWhiteList = ids;
  invalidate();
}

bool IdentificationPresentationModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  auto result = TreeModelPresentation::filterAcceptsRow(source_row, source_parent);
  if(!result)
    return result;

  auto srcInd = Source->index(source_row, 0, source_parent);
  auto item = Source->GetItem(srcInd);
  if(mIdWhiteList.contains(item->GetData("library_group_id").toInt()))
    return true;
  return false;
}
