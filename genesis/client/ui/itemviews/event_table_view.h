#pragma once
#ifndef EVENT_TABLE_VIEW_H
#define EVENT_TABLE_VIEW_H

#include <QTableView>

class QSortFilterProxyModel;

namespace Control
{
  class EventTableView final : public QTableView
  {
    Q_OBJECT
  public:
    explicit EventTableView(QWidget* parent = nullptr, int columns = 1);

    void setModel(QAbstractItemModel* newModel) override;
    void OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void OnFrozenSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void SetDefaultAlignment(Qt::Alignment alignment);

  signals:
    void FrozenClicked(const QModelIndex& index);

  protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible) override;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;

    void SetupUi();
    void SetupFrozen();
    void ConnectSignals();
    void UpdateFrozenTableGeometry();
    void UpdateFrozenTableSectionsGeometry();
    void UpdateFrozenTableItems();
    void UpdateFrozenSectionWidth(int logicalIndex, int oldSize, int newSize);
    void UpdateTableSectionWidth(int logicalIndex, int oldSize, int newSize);
    void UpdateFrozenSectionHeight(int logicalIndex, int oldSize, int newSize);
    void FrozenSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void SortIndicatorChanged(int logicalIndex, Qt::SortOrder order);

    int FrozenColumnsLeft = 0;
    bool SelectionChanged = false;
    QTableView* VerticalFrozenLeft = nullptr;
  };
}
#endif