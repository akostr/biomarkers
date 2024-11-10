#pragma once
#ifndef DATA_TABLE_CONTEXT_H
#define DATA_TABLE_CONTEXT_H

#include <QMenu>
#include <QPointer>

namespace Widgets
{
  class DataTableContext : public QMenu
  {
    Q_OBJECT

  public:
    explicit DataTableContext(QWidget* parent = nullptr);
    ~DataTableContext() = default;

    QPointer<QAction> OpenRatioMatrixAction;
    QPointer<QAction> CalculationCoefficientTableAction;
    QPointer<QAction> BuildPlotAction;
    QPointer<QAction> OpenMarkupAction;
    QPointer<QAction> EditAction;
    QPointer<QAction> ExportAction;
    QPointer<QAction> RemoveAction;

  private:
    void Setup();
  };
}
#endif
