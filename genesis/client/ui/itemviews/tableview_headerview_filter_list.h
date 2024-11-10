#pragma once

#include "logic/tree_model_presentation.h"

#include <QWidget>
#include <QLabel>

////////////////////////////////////////////////////
//// Table view header filter list
class TableViewHeaderViewFilterListItem : public QWidget
{
  Q_OBJECT

public:
  explicit TableViewHeaderViewFilterListItem(QWidget* parent);

  void SetFilterData(TreeModelPresentation* presentation,
                     int column,
                     ColumnInfo::FilterType filterType,
                     const QVariant& data);

private:
  //// Model
  QPointer<TreeModelPresentation> Presentation;
  int                             Column;

  //// Ui
  QPointer<QLabel>                CloseLabel;
  QPointer<QLabel>                Label;
};

////////////////////////////////////////////////////
//// Table view header filter list
class TableViewHeaderViewFilterList : public QWidget
{
  Q_OBJECT

public:
  explicit TableViewHeaderViewFilterList(QWidget* parent = nullptr);
  void SetModel(QAbstractItemModel *model);

public slots:
  void Update();
  int GetHeight();
  void OnSectionCountChanged(int oldCount, int newCount);
  void OnSectionSizeChanged(int logicalIndex, int oldSize, int newSize);
  void OnHeaderResize();
  void SetHeaderSectionPositionAccessFunction(std::function<int(int)> accessFunc){SectionPositionAccessFunction = accessFunc;};
  void SetHeaderSectionWidthAccessFunction(std::function<int(int)> accessFunc){SectionWidthAccessFunction = accessFunc;};

private:
  QPointer<TreeModelPresentation> Presentation;
  QList<QPointer<TableViewHeaderViewFilterListItem>> Items;
  std::function<int(int)> SectionPositionAccessFunction;
  std::function<int(int)> SectionWidthAccessFunction;

  void moveItem(int index);
};

