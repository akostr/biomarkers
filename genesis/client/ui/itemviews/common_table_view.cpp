#include "common_table_view.h"
#include "peaks_column_item_delegate.h"
#include "ui/itemviews/tableview_headerview.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

CommonTableView::CommonTableView(QWidget* parent) : CommonTableView(parent, 0)
{
}

CommonTableView::CommonTableView(QWidget* parent, int frozenColumns)
  : QTableView(parent)
  , FrozenColumnsLeft(frozenColumns)
{
  //incorrect header displaying without this
  VerticalHeader = new TableViewHeaderView(Qt::Vertical, this);
  HorizontalHeader = new TableViewHeaderView(Qt::Horizontal, this);

  setVerticalHeader(VerticalHeader);
  setHorizontalHeader(HorizontalHeader);
  HorizontalHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  SetupFrozen();
  VerticalHeader->hide();
  HorizontalHeader->show();

  VerticalFrozenLeft->horizontalHeader()->show();
  VerticalFrozenLeft->setVisible(FrozenColumnsLeft);
  VerticalFrozenLeft->setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(VerticalFrozenLeft->horizontalHeader(), &QHeaderView::sectionResized, this, &CommonTableView::onLeftVerticalFrozenHorizontalHeaderSectionResized/*, Qt::QueuedConnection*/);
  connect(HorizontalHeader, &QHeaderView::sectionResized, this, &CommonTableView::UpdateFrozenSectionWidth/*, Qt::QueuedConnection*/);
  connect(VerticalHeader, &QHeaderView::sectionResized, this, &CommonTableView::UpdateFrozenSectionHeight/*, Qt::QueuedConnection*/);

  connect(VerticalFrozenLeft->verticalScrollBar(), &QAbstractSlider::valueChanged, verticalScrollBar(), &QAbstractSlider::setValue);
  connect(verticalScrollBar(), &QAbstractSlider::valueChanged, VerticalFrozenLeft->verticalScrollBar(), &QAbstractSlider::setValue);
  connect(VerticalFrozenLeft, &QTableView::doubleClicked, this, &QTableView::doubleClicked);
  connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &CommonTableView::SortIndicatorChanged);
  connect(VerticalFrozenLeft->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &CommonTableView::FrozenSortIndicatorChanged);
}

void CommonTableView::setModel(QAbstractItemModel* model)
{
  if (Model)
    disconnect(Model.data(), 0, this, 0);

  Model = model;

  QTableView::setModel(model);

  VerticalFrozenLeft->setModel(model);

  UpdateFrozenTableItems();
  UpdateFrozenTableSectionsGeometry();

  if (model)
  {
    connect(Model.data(), &QAbstractItemModel::columnsInserted, this, &CommonTableView::UpdateFrozenTableItems);
    connect(Model.data(), &QAbstractItemModel::columnsRemoved, this, &CommonTableView::UpdateFrozenTableItems);
    connect(Model.data(), &QAbstractItemModel::modelReset, this, &CommonTableView::UpdateFrozenTableItems);
  }

  VerticalFrozenLeft->setVisible(FrozenColumnsLeft);
  UpdateFrozenTableGeometry();

  resizeColumnsToContents();
  VerticalFrozenLeft->resizeColumnsToContents();

  resizeRowsToContents();
  VerticalFrozenLeft->resizeRowsToContents();

  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &CommonTableView::OnSelectionChanged);
  connect(VerticalFrozenLeft->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CommonTableView::OnFrozenSelectionChanged);
}

void CommonTableView::setDataDrivingColoring(bool isColoredByData)
{
  setProperty("dataDrivenColors", isColoredByData);
  VerticalFrozenLeft->setProperty("dataDrivenColors", isColoredByData);
}

void CommonTableView::resizeEvent(QResizeEvent* event)
{
  QTableView::resizeEvent(event);
  UpdateFrozenTableGeometry();
}

QModelIndex CommonTableView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
  QModelIndex current = QTableView::moveCursor(cursorAction, modifiers);

  int frozenColumnWidth = 0;
  for (int fc = 0; fc < FrozenColumnsLeft; fc++)
    frozenColumnWidth += VerticalFrozenLeft->columnWidth(fc);

  if (cursorAction == MoveUp
    && current.column() >= FrozenColumnsLeft
    && visualRect(current).topLeft().x() < frozenColumnWidth)
  {
    const int newValue = verticalScrollBar()->value()
      + visualRect(current).topLeft().y();
    verticalScrollBar()->setValue(newValue);
  }

  if (cursorAction == MoveLeft
    && current.column() >= FrozenColumnsLeft
    && visualRect(current).topLeft().x() < frozenColumnWidth)
  {
    const int newValue = horizontalScrollBar()->value()
      + visualRect(current).topLeft().x()
      - VerticalFrozenLeft->columnWidth(frozenColumnWidth);
    horizontalScrollBar()->setValue(newValue);
  }

  return current;
}

void CommonTableView::scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint)
{
  if (index.column() >= FrozenColumnsLeft)
    QTableView::scrollTo(index, hint);
}

void CommonTableView::SetupFrozen()
{
  VerticalFrozenLeft = new QTableView(this);
  const auto hheader = new TableViewHeaderView(Qt::Horizontal, VerticalFrozenLeft);
  hheader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  VerticalFrozenLeft->setHorizontalHeader(hheader);
  VerticalFrozenLeft->hide();
  VerticalFrozenLeft->setShowGrid(false);
  VerticalFrozenLeft->verticalHeader()->hide();
  VerticalFrozenLeft->horizontalHeader()->show();

  VerticalFrozenLeft->setSortingEnabled(true);
  VerticalFrozenLeft->sortByColumn(-1, Qt::AscendingOrder);
  VerticalFrozenLeft->setSelectionMode(QAbstractItemView::SingleSelection);
  VerticalFrozenLeft->setSelectionBehavior(QAbstractItemView::SelectRows);

  viewport()->stackUnder(VerticalFrozenLeft);

  VerticalFrozenLeft->raise();
  VerticalFrozenLeft->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  VerticalFrozenLeft->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  UpdateFrozenTableGeometry();
  UpdateFrozenTableSectionsGeometry();

  setHorizontalScrollMode(ScrollPerPixel);
  setVerticalScrollMode(ScrollPerPixel);

  VerticalFrozenLeft->setHorizontalScrollMode(ScrollPerPixel);
  VerticalFrozenLeft->setVerticalScrollMode(ScrollPerPixel);
}

void CommonTableView::UpdateFrozenTableGeometry()
{
  if (FrozenColumnsLeft)
  {
    int frozenColumnWidth = 1;
    for (int fc = 0; fc < FrozenColumnsLeft; fc++)
      frozenColumnWidth += VerticalFrozenLeft->columnWidth(fc);

    // +-1 mb table margin or table border
    VerticalFrozenLeft->setGeometry(0, 0, frozenColumnWidth,
      viewport()->height() + HorizontalHeader->geometry().height() + 2);
    VerticalFrozenLeft->horizontalHeader()->setFixedHeight(HorizontalHeader->height());
  }
}

void CommonTableView::UpdateFrozenTableSectionsGeometry()
{
  for (int c = 0; c < FrozenColumnsLeft; c++)
  {
    VerticalFrozenLeft->setColumnWidth(c, columnWidth(c));
  }
}

void CommonTableView::SortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
  VerticalFrozenLeft->sortByColumn(-1, Qt::SortOrder::AscendingOrder);
  sortByColumn(logicalIndex, order);
}

void CommonTableView::FrozenSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
  sortByColumn(-1, Qt::SortOrder::AscendingOrder);
  VerticalFrozenLeft->sortByColumn(logicalIndex, order);
}

void CommonTableView::UpdateFrozenSectionWidth(int logicalIndex, int, int newSize)
{
  if (logicalIndex <= FrozenColumnsLeft)
    VerticalFrozenLeft->setColumnWidth(logicalIndex, newSize);

  UpdateFrozenTableGeometry();
}

void CommonTableView::onLeftVerticalFrozenHorizontalHeaderSectionResized(int logicalIndex, int oldSize, int newSize)
{
  {//update main table column
    QSignalBlocker l(horizontalHeader());
    setColumnWidth(logicalIndex, newSize);
  }

  {//update left frozen geometry (width)
    auto geom = VerticalFrozenLeft->geometry();
    int width = 0;
    for(int i = 0; i < FrozenColumnsLeft; i++)
    {
      width += VerticalFrozenLeft->horizontalHeader()->sectionSize(i);
    }
    int lineWidth = 1;
    int rightBorderWidth = 1;
    int gridLinesCount = FrozenColumnsLeft - 1;
    geom.setWidth(width + (gridLinesCount * lineWidth - rightBorderWidth));
    VerticalFrozenLeft->setGeometry(geom);
    update();
  }
}

void CommonTableView::UpdateFrozenSectionHeight(int logicalIndex, int, int newSize)
{
  VerticalFrozenLeft->setRowHeight(logicalIndex, newSize);
  UpdateFrozenTableGeometry();
}

void CommonTableView::UpdateSectionWidth(int logicalIndex, int, int newSize)
{
  if (logicalIndex < FrozenColumnsLeft)
  {
    QSignalBlocker l(horizontalHeader());
    setColumnWidth(logicalIndex, newSize);
  }
}

void CommonTableView::UpdateFrozenTableItems()
{
  if (Model)
  {
    for (int col = 0; col < FrozenColumnsLeft; col++)
    {
      VerticalFrozenLeft->setColumnHidden(col, false);
      VerticalFrozenLeft->setColumnWidth(col, columnWidth(col));
    }

    for (int col = FrozenColumnsLeft; col < Model->columnCount(); ++col)
    {
      VerticalFrozenLeft->setColumnHidden(col, true);
    }

    UpdateFrozenTableGeometry();
  }
}

void CommonTableView::OnSelectionChanged(const QItemSelection& selected, const QItemSelection&)
{
  if (SelectionChagned)
    return;

  SelectionChagned = true;
  // Correct way deselect items
  if (auto smodel = VerticalFrozenLeft->selectionModel())
    smodel->select(smodel->selection(), QItemSelectionModel::Deselect);
  VerticalFrozenLeft->selectionModel()->select(selected, QItemSelectionModel::Select);
  SelectionChagned = false;
}

void CommonTableView::OnFrozenSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  if (SelectionChagned)
    return;

  SelectionChagned = true;
  // Correct way deselect items
  if (auto smodel = selectionModel())
    smodel->select(smodel->selection(), QItemSelectionModel::Deselect);
  selectionModel()->select(selected, QItemSelectionModel::Select);
  SelectionChagned = false;
}

QHeaderView* CommonTableView::getHorizontalHeader()
{
  return HorizontalHeader;
}


QHeaderView* CommonTableView::getVerticalHeader()
{
  return VerticalHeader;
}

int CommonTableView::getFrozenColumns() const
{
  return FrozenColumnsLeft;
}

void CommonTableView::setFrozenColumns(int value)
{
  FrozenColumnsLeft = value;
  VerticalFrozenLeft->setVisible(value);
}


void CommonTableView::keyPressEvent(QKeyEvent* event)
{
  switch (event->key())
  {
  case Qt::Key_Delete:
  case Qt::Key_Backspace:
  {
    for (auto& index : selectedIndexes())
    {
      if (index.data(ConcentrationColumnRole).toBool())
        Model->setData(index, QVariant());
    }
    break;
  }
  case Qt::Key_V:
  {
    if (event->modifiers().testFlag(Qt::ControlModifier))
    {
      auto text = qApp->clipboard()->text();
      text.replace(',', '.');
      QStringList rows = text.split('\n');

      rows.removeIf([](const QString& str) {return str.isEmpty(); });

      if (rows.size() == 1)
        rows = text.split('\t');

      QList<QStringList> inputTable;

      for (auto& row : rows)
        inputTable.append(row.split('\t'));

      const auto indexes = selectedIndexes();

      const auto rowMin = std::min(indexes.begin(), indexes.end(),
        [](auto lt, auto rt)
        { return lt->row() < rt->row(); });

      const auto colMin = std::min(indexes.begin(), indexes.end(),
        [](auto lt, auto rt)
        { return lt->column() < rt->column(); });

      for (auto& index : selectedIndexes())
      {
        if (index.data(ConcentrationColumnRole).toBool())
        {
          auto rowInd = index.row() - rowMin->row();
          auto colInd = index.column() - colMin->column();
          if ((inputTable.size() <= rowInd)
            || (inputTable[rowInd].size() <= colInd))
            Model->setData(index, QVariant());
          else
          {
            bool ok = false;
            double val = inputTable[rowInd][colInd].toDouble(&ok);
            Model->setData(index, ok ? val : QVariant());
          }
        }
      }
    }
  }
  default:
    QTableView::keyPressEvent(event);
    break;
  }
}
