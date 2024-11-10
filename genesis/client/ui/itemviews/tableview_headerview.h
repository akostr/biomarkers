#pragma once

#include "../../logic/tree_model_presentation.h"

#include <QHeaderView>
#include <QMenu>
#include <QPointer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QStringListModel>
#include <QTableView>
#include <QLineEdit>
#include <QLabel>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QStackedWidget>

class TableViewHeaderViewFilterList;

////////////////////////////////////////////////////
//// Table view header
class TableViewHeaderView final : public QHeaderView
{
  Q_OBJECT

public:
  TableViewHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
  ~TableViewHeaderView() = default;

public:
  //// Impls
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
  virtual QSize sectionSizeFromContents(int logicalIndex) const override;
  virtual void setModel(QAbstractItemModel *model) override;
  virtual void updateGeometries() override;
  virtual void resizeEvent(QResizeEvent* event) override;

public slots:
  void Layout();

private:
  //// Impls
  int GetExtrasWidth(int logicalIndex) const;
  int GetExtrasWidthSortIndicator(int logicalIndex) const;
  int GetExtrasWidthFilter(int logicalIndex) const;

  void PaintExtras(QPainter* painter, const QRect& rect, int logicalIndex) const;

  int GetFilterListHeight() const;

private:
  QIcon IconFilter;
  QIcon IconSortAsc;
  QIcon IconSortDes;
  QPointer<QMenu> MenuFilter;

  QPointer<TableViewHeaderViewFilterList> FilterList;
};

