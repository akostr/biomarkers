#include "data_table_context.h"

#include "genesis_style/style.h"

namespace Widgets
{
  DataTableContext::DataTableContext(QWidget* parent)
    : QMenu(parent)
  {
    Setup();
  }

  void DataTableContext::Setup()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());

    OpenRatioMatrixAction = addAction(/*QIcon(":/resource/icons/icon_repeat.png"), */tr("Open ratio matrix"));
    CalculationCoefficientTableAction = addAction(tr("Calculation coefficient table"));
    BuildPlotAction = addAction(tr("Build plot"));
    OpenMarkupAction = addAction(/*QIcon(":/resource/icons/icon_union.png"), */tr("Open markup"));
    EditAction = addAction(/*QIcon(":/resource/icons/icon_union.png"), */tr("Edit title and comment"));
    ExportAction = addAction(/*QIcon(":/resource/icons/icon_action_export.png"), */tr("Export"));
    RemoveAction = addAction(/*QIcon(":/resource/icons/icon_action_export.png"), */tr("Remove"));
  }
}
