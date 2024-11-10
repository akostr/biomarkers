#include "event_table_view.h"

#include <QApplication>
#include <QKeyEvent>
#include <QAbstractItemModel>
#include <QClipboard>
#include <QHeaderView>
#include <QScrollBar>
#include <QSortFilterProxyModel>

#include "ui/item_models/analysis_data_table_model.h"
#include "ui/itemviews/data_checkable_headerview.h"

using namespace Models;

namespace Control
{
  EventTableView::EventTableView(QWidget* parent, int columns)
    : QTableView(parent), FrozenColumnsLeft(columns)
  {
    SetupUi();
    SetupFrozen();
    ConnectSignals();
  }

  void EventTableView::setModel(QAbstractItemModel* newModel)
  {
    if (auto currentModel = model())
    {
      disconnect(currentModel, &QAbstractItemModel::columnsInserted, this, &EventTableView::UpdateFrozenTableItems);
      disconnect(currentModel, &QAbstractItemModel::columnsRemoved, this, &EventTableView::UpdateFrozenTableItems);
      disconnect(currentModel, &QAbstractItemModel::modelReset, this, &EventTableView::UpdateFrozenTableItems);
    }

    if (newModel)
    {
      connect(newModel, &QAbstractItemModel::columnsInserted, this, &EventTableView::UpdateFrozenTableItems);
      connect(newModel, &QAbstractItemModel::columnsRemoved, this, &EventTableView::UpdateFrozenTableItems);
      connect(newModel, &QAbstractItemModel::modelReset, this, &EventTableView::UpdateFrozenTableItems);
    }

    QTableView::setModel(newModel);
    VerticalFrozenLeft->setModel(newModel);

    verticalHeader()->resizeSections(QHeaderView::Interactive);
    verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->resizeSections(QHeaderView::Interactive);
    resizeColumnsToContents();
    UpdateFrozenTableItems();
    UpdateFrozenTableSectionsGeometry();

    VerticalFrozenLeft->setVisible(FrozenColumnsLeft);
    UpdateFrozenTableGeometry();
  }

  void EventTableView::OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
  {
    if (SelectionChanged)
      return;

    SelectionChanged = true;
    if (selected.isEmpty())
      return;
    if (auto smodel = VerticalFrozenLeft->selectionModel())
      smodel->select(smodel->selection(), QItemSelectionModel::Deselect);
    VerticalFrozenLeft->selectionModel()->select(selected, QItemSelectionModel::Select);
    SelectionChanged = false;
  }

  void EventTableView::OnFrozenSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
  {
    if (SelectionChanged)
      return;

    SelectionChanged = true;
    if (selected.isEmpty())
      return;

    if (auto smodel = selectionModel())
      smodel->select(smodel->selection(), QItemSelectionModel::Deselect);
    selectionModel()->select(selected, QItemSelectionModel::Select);
    SelectionChanged = false;
  }

  void EventTableView::SetDefaultAlignment(Qt::Alignment alignment)
  {
    horizontalHeader()->setDefaultAlignment(alignment);
    VerticalFrozenLeft->horizontalHeader()->setDefaultAlignment(alignment);
  }

  void EventTableView::keyPressEvent(QKeyEvent* event)
  {
    const auto currentModel = qobject_cast<AnalysisDataTableModel*>(model());
    if (!currentModel)
    {
      QTableView::keyPressEvent(event);
      return;
    }
    const auto yColumn = currentModel->GetYConcentrationColumn();

    switch (event->key())
    {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
    {
      for (auto& index : selectedIndexes())
      {
        if (index.column() == yColumn)
          currentModel->setData(index, QVariant());
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

        auto indexes = selectedIndexes();

        int rowMin = indexes.first().row();
        int colMin = indexes.first().column();

        for (const auto& index : indexes)
        {
          if (index.row() < rowMin) rowMin = index.row();
          if (index.column() < colMin) colMin = index.column();
        };

        for (auto& index : selectedIndexes())
        {
          if (index.column() == yColumn)
          {
            auto rowInd = index.row() - rowMin;
            auto colInd = index.column() - colMin;
            if (inputTable.size() <= rowInd
              || inputTable[rowInd].size() <= colInd)
            {
              currentModel->setData(index, QVariant());
              currentModel->setData(index, QVariant());
            }
            else
            {
              bool ok = false;
              double val = inputTable[rowInd][colInd].toDouble(&ok);
              currentModel->setData(index, ok ? val : QVariant());
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

  void EventTableView::resizeEvent(QResizeEvent* event)
  {
    QTableView::resizeEvent(event);
    UpdateFrozenTableGeometry();
  }

  QModelIndex EventTableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
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

  void EventTableView::scrollTo(const QModelIndex& index, ScrollHint hint)
  {
    if (index.column() >= FrozenColumnsLeft)
      QTableView::scrollTo(index, hint);
  }

  void EventTableView::SetupUi()
  {
    const auto hh = new DataCheckableHeaderView(Qt::Horizontal, this);
    hh->setSectionResizeMode(QHeaderView::ResizeToContents);
    setHorizontalHeader(hh);

    const auto vh = new DataCheckableHeaderView(Qt::Vertical, this);
    vh->resizeSections(QHeaderView::ResizeToContents);
    vh->setSortIndicatorShown(false);
    setVerticalHeader(vh);
  }

  void EventTableView::SetupFrozen()
  {
    VerticalFrozenLeft = new QTableView(this);
    VerticalFrozenLeft->setShowGrid(true);
    VerticalFrozenLeft->setSortingEnabled(true);
    VerticalFrozenLeft->sortByColumn(-1, Qt::AscendingOrder);
    VerticalFrozenLeft->setSelectionMode(QAbstractItemView::SingleSelection);
    VerticalFrozenLeft->setSelectionBehavior(QAbstractItemView::SelectRows);
    VerticalFrozenLeft->setHorizontalHeader(new DataCheckableHeaderView(Qt::Horizontal, this));
    VerticalFrozenLeft->horizontalHeader()->show();
    VerticalFrozenLeft->verticalHeader()->hide();
    VerticalFrozenLeft->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    VerticalFrozenLeft->setVisible(FrozenColumnsLeft);
    VerticalFrozenLeft->setEditTriggers(QAbstractItemView::NoEditTriggers);
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

  void EventTableView::ConnectSignals()
  {
    connect(VerticalFrozenLeft->horizontalHeader(), &QHeaderView::sectionResized, this, &EventTableView::UpdateTableSectionWidth);
    connect(VerticalFrozenLeft->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &EventTableView::FrozenSortIndicatorChanged);

    connect(horizontalHeader(), &QHeaderView::sectionResized, this, &EventTableView::UpdateFrozenSectionWidth);
    connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &EventTableView::SortIndicatorChanged);
    connect(verticalHeader(), &QHeaderView::sectionResized, this, &EventTableView::UpdateFrozenSectionHeight);

    connect(VerticalFrozenLeft->verticalScrollBar(), &QAbstractSlider::valueChanged, verticalScrollBar(), &QAbstractSlider::setValue);
    connect(verticalScrollBar(), &QAbstractSlider::valueChanged, VerticalFrozenLeft->verticalScrollBar(), &QAbstractSlider::setValue);
    connect(VerticalFrozenLeft, &QTableView::doubleClicked, this, &QTableView::doubleClicked);
    connect(VerticalFrozenLeft, &QTableView::clicked, this, &EventTableView::FrozenClicked);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &EventTableView::OnSelectionChanged);
    connect(VerticalFrozenLeft->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EventTableView::OnFrozenSelectionChanged);
  }

  void EventTableView::UpdateFrozenTableGeometry()
  {
    if (FrozenColumnsLeft)
    {
      int frozenColumnWidth = 1;
      for (int fc = 0; fc < FrozenColumnsLeft; fc++)
        frozenColumnWidth += VerticalFrozenLeft->columnWidth(fc);

      // +-1 mb table margin or table border
      const auto height = horizontalHeader()->height();
      VerticalFrozenLeft->setGeometry(verticalHeader()->width(), 0, frozenColumnWidth,
        viewport()->height() + height + 2);

      VerticalFrozenLeft->horizontalHeader()->setFixedHeight(height);
    }
  }

  void EventTableView::UpdateFrozenTableSectionsGeometry()
  {
    for (int c = 0; c < FrozenColumnsLeft; c++)
      VerticalFrozenLeft->setColumnWidth(c, columnWidth(c));
  }

  void EventTableView::UpdateFrozenTableItems()
  {
    for (int col = 0; col < FrozenColumnsLeft; col++)
    {
      VerticalFrozenLeft->showColumn(col);
      VerticalFrozenLeft->setColumnWidth(col, columnWidth(col));
    }

    if (const auto currentModel = model())
    {
      for (int col = FrozenColumnsLeft; col < currentModel->columnCount(); ++col)
      {
        VerticalFrozenLeft->setColumnHidden(col, true);
      }
    }
    UpdateFrozenTableGeometry();
  }

  void EventTableView::UpdateFrozenSectionWidth(int logicalIndex, int oldSize, int newSize)
  {
    if (logicalIndex <= FrozenColumnsLeft)
      VerticalFrozenLeft->setColumnWidth(logicalIndex, newSize);
    UpdateFrozenTableGeometry();
  }

  void EventTableView::UpdateTableSectionWidth(int logicalIndex, int oldSize, int newSize)
  {
    if(logicalIndex < FrozenColumnsLeft)
      setColumnWidth(logicalIndex, newSize);
  }

  void EventTableView::UpdateFrozenSectionHeight(int logicalIndex, int oldSize, int newSize)
  {
    VerticalFrozenLeft->setRowHeight(logicalIndex, newSize);
    UpdateFrozenTableGeometry();
  }

  void EventTableView::FrozenSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
  {
    sortByColumn(-1, Qt::SortOrder::AscendingOrder);
    VerticalFrozenLeft->sortByColumn(logicalIndex, order);
  }

  void EventTableView::SortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
  {
    VerticalFrozenLeft->sortByColumn(-1, Qt::SortOrder::AscendingOrder);
    sortByColumn(logicalIndex, order);
  }
}
