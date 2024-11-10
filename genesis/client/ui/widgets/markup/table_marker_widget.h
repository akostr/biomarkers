#pragma once
#ifndef TABLE_MARKER_WIDGET_H
#define TABLE_MARKER_WIDGET_H

#include <QWidget>

//#include "logic/markup/abstract_data_model.h"
#include "logic/markup/i_markup_tab.h"
#include "logic/markup/chromatogram_data_model.h"

class QSortFilterProxyModel;
class QUndoCommand;
namespace Ui
{
  class TableMarkerWidget;
}

namespace Models
{
  class CovatsIndexModelTable;
}
namespace Widgets
{
  class TableMarkerWidget : public IMarkupTab
  {
    Q_OBJECT

  public:
    TableMarkerWidget(QWidget* parent = nullptr);

  private:
    Models::CovatsIndexModelTable* mModel;
    Ui::TableMarkerWidget* ui = nullptr;
    QSortFilterProxyModel* mSortModel = nullptr;

    // IMarkupTab interface
  public:
    void setMarkupModel(GenesisMarkup::MarkupModelPtr model) override;
    bool isVisibleOnStep(GenesisMarkup::StepInfo step) override;
    void setModule(Names::ModulesContextTags::Module module) override;
  };
}
#endif
