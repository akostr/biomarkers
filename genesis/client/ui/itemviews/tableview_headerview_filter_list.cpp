#include "tableview_headerview_filter_list.h"
#include "tableview_headerview.h"

#include <genesis_style/style.h>

#include <QHBoxLayout>
#include <QDate>
#include <QDateTime>
#include <QVariant>

namespace Details
{
  const int IconSize = 24;
  const int HorizontalPad = 0;
  const int VerticalPad = 15;
  const QString CloseButton = "&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_cross.png\"></a>";
}

////////////////////////////////////////////////////
//// Table view header filter list model
TableViewHeaderViewFilterListItem::TableViewHeaderViewFilterListItem(QWidget* parent)
  : QWidget(parent)
  , Column(-1)
{
  setStyleSheet("TableViewHeaderViewFilterListItem { background-color: #edf2f4; }");

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  Label = new QLabel(this);
  Label->setTextFormat(Qt::RichText);
  Label->setStyleSheet("QLabel{background-color: rgba(0,0,0,0%)}");
  Label->setContentsMargins(Style::Scale(Details::VerticalPad), Style::Scale(Details::HorizontalPad), Style::Scale(Details::VerticalPad), Style::Scale(Details::HorizontalPad));
  layout->addWidget(Label);

  layout->addStretch(1);
  CloseLabel = new QLabel(this);
  CloseLabel->setStyleSheet("QLabel{background-color: rgba(0,0,0,0%)}");
  CloseLabel->setTextFormat(Qt::RichText);
  CloseLabel->setContentsMargins(Style::Scale(Details::VerticalPad), Style::Scale(Details::HorizontalPad), Style::Scale(Details::VerticalPad), Style::Scale(Details::HorizontalPad));
  CloseLabel->setText(Details::CloseButton);
  layout->addWidget(CloseLabel);

  connect(CloseLabel, &QLabel::linkActivated, [&]()
    {
      Presentation->SetColumnFilter(Column, QVariant());
    });
}

void TableViewHeaderViewFilterListItem::SetFilterData(TreeModelPresentation* presentation,
  int column,
  ColumnInfo::FilterType filterType,
  const QVariant& data)
{
  Presentation = presentation;
  Column = column;


  switch (filterType)
  {
  case ColumnInfo::FilterTypeNone:
  {
    hide();
  }
  break;
  case ColumnInfo::FilterTypeTextValuesList:
  {
    QVariantList vl = data.toList();
    setVisible(vl.isEmpty());
    if (vl.isEmpty())
      return;
    QStringList str;
    str.reserve(vl.size());
    std::transform(vl.begin(), vl.end(), std::back_inserter(str),
      [](const QVariant& item)
      {
        QString text = item.toString();
        if (text.isEmpty())
          text = "[empty]";
        return text;
      });
    Label->setText(str.join(", "));
  }
  break;
  case ColumnInfo::FilterTypeTextSubstring:
  {
    Label->setText(data.toString());
    setVisible(!data.toString().isEmpty());
  }
  break;
  case ColumnInfo::FilterTypeDateRange:
  {
    QVariantList vl = data.toList();
    if (vl.size() == 2)
    {
      Label->setText(vl.first().toDate().toString("dd.MM.yyyy") + " - " + vl.last().toDate().toString("dd.MM.yyyy"));
      show();
    }
    else
    {
      hide();
    }
  }
  break;
  case ColumnInfo::FilterTypeIntegerRange:
  {
    QVariantList vl = data.toList();
    if (vl.size() == 2)
    {
      Label->setText(QString::number(vl.first().toInt()) + " - " +
        QString::number(vl.last().toInt()));
      show();
    }
    else
    {
      hide();
    }
  }
  break;
  default:
    break;
  }
}

////////////////////////////////////////////////////
//// Table view header filter list
TableViewHeaderViewFilterList::TableViewHeaderViewFilterList(QWidget* parent)
  : QWidget(parent)
{}

void TableViewHeaderViewFilterList::SetModel(QAbstractItemModel* model)
{
  Presentation = qobject_cast<TreeModelPresentation*>(model);
  if (Presentation)
  {
    connect(Presentation, &TreeModelPresentation::FilterChanged, this, &TableViewHeaderViewFilterList::Update);
    Update();
  }
}

void TableViewHeaderViewFilterList::Update()
{
  if (Presentation)
  {
    //// Create if necessary
    while (Items.size() < Presentation->columnCount())
    {
      TableViewHeaderViewFilterListItem* item = new TableViewHeaderViewFilterListItem(this);
      Items << item;
      OnHeaderResize();
    }

    //// Update
    if (auto m = Presentation->sourceModel())
    {
      if (auto t = qobject_cast<TreeModel*>(m))
      {
        for (auto item = Items.begin(); item != Items.end(); item++)
        {
          auto itemPtr = *item;
          ColumnInfo::FilterType filterType = ColumnInfo::FilterTypeNone;
          QVariant filterData;

          int c = std::distance(Items.begin(), item);
          if (c < t->columnCount())
          {
            filterType = t->GetColumnInfo(c).Filter;
            filterData = Presentation->GetColumnFilter(c);
          }

          QString filterColumnName = t->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString();
          itemPtr->SetFilterData(Presentation, c, filterType, filterData);
        }
      }

      //// Should be visible?
      auto v = std::find_if(Items.begin(), Items.end(), [](QPointer<TableViewHeaderViewFilterListItem> item)
                            { return item->isVisible(); });
      if (v != Items.end())
      {
        show();
      }
      else
      {
        hide();
      }
    }
  }
}

int TableViewHeaderViewFilterList::GetHeight()
{
  if (isVisible() && !Items.isEmpty())
  {
    return Items.first()->height();
  }
  return 0;
}

void TableViewHeaderViewFilterList::OnSectionCountChanged(int oldCount, int newCount)
{
  Update();
}

void TableViewHeaderViewFilterList::OnSectionSizeChanged(int logicalIndex, int oldSize, int newSize)
{
  if (logicalIndex < Items.size())
  {
    for (int i = logicalIndex; i < Items.size(); i++)
      moveItem(i);
  }
}

void TableViewHeaderViewFilterList::OnHeaderResize()
{
  for (int i = 0; i < Items.size(); i++)
    moveItem(i);
}

void TableViewHeaderViewFilterList::moveItem(int index)
{
  QRect itemRect = Items[index]->geometry();
  itemRect.moveLeft(SectionPositionAccessFunction(index));
  itemRect.setWidth(SectionWidthAccessFunction(index));
  Items[index]->setGeometry(itemRect);
}
