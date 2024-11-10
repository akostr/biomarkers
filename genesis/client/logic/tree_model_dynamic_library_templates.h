#ifndef TREEMODELDYNAMICLIBRARYTEMPLATES_H
#define TREEMODELDYNAMICLIBRARYTEMPLATES_H

#include "tree_model_dynamic_library_compounds.h"

class TreeModelDynamicLibraryTemplates : public TreeModelDynamicLibraryCompounds
{
  Q_OBJECT

public:
  explicit TreeModelDynamicLibraryTemplates(QObject *parent = nullptr);

  void uploadTemplatesForTable(int tableId);
  virtual void FetchMore(QPointer<TreeModelItem> item) override;

private:
  int m_tableId;
};

#endif // TREEMODELDYNAMICLIBRARYTEMPLATES_H
