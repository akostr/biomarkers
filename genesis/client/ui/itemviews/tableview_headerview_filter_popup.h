#pragma once

#include "tableview_headerview.h"

class FilterPopupEditor;

//// Filter popup
class FilterPopup : public QWidget
{
  Q_OBJECT
public:
  FilterPopup(QMenu* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model);
  ~FilterPopup();
  
private:
  void SetupUi();
  
private:
  //// Logic
  int                             Column;
  QPointer<TreeModelPresentation> Presentation;
  QPointer<TreeModel>             Model;
  
  //// Ui
  QPointer<QMenu>                 Menu;
  QPointer<QVBoxLayout>           Layout;
  QPointer<FilterPopupEditor>     Editor;
  QPointer<QDialogButtonBox>      Buttons;
};
