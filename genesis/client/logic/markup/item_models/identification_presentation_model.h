#ifndef IDENTIFICATION_PRESENTATION_MODEL_H
#define IDENTIFICATION_PRESENTATION_MODEL_H

#include <logic/tree_model_presentation.h>
#include <logic/enums.h>

class IdentificationPresentationModel final : public TreeModelPresentation
{
  Q_OBJECT
public:
  IdentificationPresentationModel(TreeModel* parent = nullptr);

public slots:
  void setWhiteListIds(const QList<int> ids);

  // QSortFilterProxyModel interface
protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
  QList<int> mIdWhiteList;
};



#endif // IDENTIFICATION_PRESENTATION_MODEL_H
