#pragma once

#include <QTableView>
#include <QPointer>

class CommonTableView : public QTableView
{
  Q_OBJECT

public:
  explicit CommonTableView(QWidget* parent);
  explicit CommonTableView(QWidget* parent, int frozenColumns);
  ~CommonTableView() override = default;

  void setModel(QAbstractItemModel* model) override;
  void setDataDrivingColoring(bool isColoredByData);

  void setFrozenColumns(int value);
  int getFrozenColumns() const;

  QHeaderView* getVerticalHeader();
  QHeaderView* getHorizontalHeader();

  enum TableItemDataRole
  {
    AreaRole = Qt::UserRole + 1,
    IndexRole,
    IdRole,
    TypeRole,
    RetTimeRole,
    HeightRole,
    ConcentrationColumnRole
  };
  enum HeaderDataRole
  {
    CheckableRole = HeightRole + 1
  };
  enum RowHeaderItemDataRole
  {
    TitleRole = CheckableRole + 1,
    MarkerFlagRole,
    ColorRole,
  };
  enum ColHeaderItemDataRole
  {
    FieldRole = ColorRole + 1,
    WellRole,
    LayerRole,
    DateRole,
    NameRole,
  };
protected:
  void        resizeEvent(QResizeEvent* event) override;
  QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
  void        scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible) override;

  void SetupFrozen();
  void UpdateFrozenTableGeometry();
  void UpdateFrozenTableSectionsGeometry();
  void SortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
  void FrozenSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);

protected slots:
  void UpdateFrozenSectionWidth(int logicalIndex, int oldSize, int newSize);
  void onLeftVerticalFrozenHorizontalHeaderSectionResized(int logicalIndex, int oldSize, int newSize);
  void UpdateFrozenSectionHeight(int logicalIndex, int oldSize, int newSize);
  void UpdateSectionWidth(int logicalIndex, int oldSize, int newSize);
  void UpdateFrozenTableItems();
  void OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
  void OnFrozenSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
  void keyPressEvent(QKeyEvent *event) override;

protected:
  int FrozenColumnsLeft;
  bool SelectionChagned = false;
  QTableView* VerticalFrozenLeft;
  QHeaderView* VerticalHeader;
  QHeaderView* HorizontalHeader;
  QPointer<QAbstractItemModel> Model;
};
