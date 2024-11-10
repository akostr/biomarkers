#include "checkable_headerview.h"
#include "common_table_view.h"
#include <QPainter>
#include <QMouseEvent>
#include <QStandardItemModel>

CheckableHeaderView::CheckableHeaderView(Qt::Orientation orientation, QWidget *parent)
  : QHeaderView(orientation, parent)
  , platesMargins(5)
  , CheckBoxHovered(-1)
{
  setSectionsClickable(true);
  QStyleOptionButton option;
  QSize checkBoxSize;
  checkBoxSize = style()->sizeFromContents(QStyle::ContentsType::CT_CheckBox, &option, checkBoxSize);
  if(orientation == Qt::Horizontal)
    CheckBoxLocalRect = QRect(platesMargins, height()/2 - checkBoxSize.height()/2, checkBoxSize.width(), checkBoxSize.height());
  else
    CheckBoxLocalRect = QRect(width()/2 - checkBoxSize.width()/2, 0, checkBoxSize.width(), checkBoxSize.height());
  setSelectionBehavior(QAbstractItemView::SelectColumns);

  connect(this, &CheckableHeaderView::checkStateChanged, this, &CheckableHeaderView::onCheckStateChanged);
}

void CheckableHeaderView::SetNonCheckableSections(const QSet<int> &sections)
{
  if(!model())
    return;
  bool wasChanged = false;
  for(int i = 0; i < count(); i++)
  {
    bool oldCheckable = model()->headerData(i, orientation(), CommonTableView::CheckableRole).toBool();
    bool newCheckable = !sections.contains(i);
    if(oldCheckable != newCheckable)
    {
      model()->setHeaderData(i, orientation(), newCheckable, CommonTableView::CheckableRole);
      wasChanged = true;
    }
  }

  if(wasChanged)
  {
//    emit checkStateChanged(SectionsCheckStates());
    update();
  }
}

void CheckableHeaderView::SetSectionCheckable(int section, bool checkable)
{
  if(!model())
    return;

  if(model()->headerData(section, orientation(), CommonTableView::CheckableRole).toBool()
     != checkable)
  {
    model()->setHeaderData(section, orientation(), checkable, CommonTableView::CheckableRole);
//    emit checkStateChanged(SectionsCheckStates());
    update();
  }
}

QSet<int> CheckableHeaderView::CheckedSections() const
{
  QSet<int> checkedSections;
  if(!model())
    return checkedSections;
  for(auto i = 0; i < count(); i++)
  {
    if(model()->headerData(i, orientation(), CommonTableView::CheckableRole).toBool()
       && model()->headerData(i, orientation(), Qt::CheckStateRole).toBool())
      checkedSections.insert(i);
  }
  return checkedSections;
}

QSet<int> CheckableHeaderView::UnCheckedSections() const
{
  QSet<int> uncheckedSections;
  if (!model())
    return uncheckedSections;
  for (auto i = 0; i < count(); i++)
  {
    if (model()->headerData(i, orientation(), CommonTableView::CheckableRole).toBool()
      && !model()->headerData(i, orientation(), Qt::CheckStateRole).toBool())
      uncheckedSections.insert(i);
  }
  return uncheckedSections;
}

QSet<int> CheckableHeaderView::CheckableSections() const
{
  QSet<int> sections;
  for(int i(0); i < count(); i++)
    if(model()->headerData(i, orientation(), CommonTableView::CheckableRole).toBool())
      sections.insert(i);
  return sections;
}

void CheckableHeaderView::SetSectionChecked(int section, bool checked)
{
  if(!model())
    return;
  if(!model()->headerData(section, orientation(), CommonTableView::CheckableRole).toBool())
    return;

  if(model()->headerData(section, orientation(), Qt::CheckStateRole).toBool()
     != checked)
  {
    model()->setHeaderData(section, orientation(), checked, Qt::CheckStateRole);
//    emit checkStateChanged(SectionsCheckStates());
  }
}

void CheckableHeaderView::SetSectionsChecked(const QList<QPair<int, bool> > &sectionsStates)
{
  if(!model())
    return;
//  bool wasChanged = false;
  for(auto s : sectionsStates)
  {
    if(model()->headerData(s.first, orientation(), CommonTableView::CheckableRole).toBool())
    {
      continue;
    }
    else
    {
      if(model()->headerData(s.first, orientation(), Qt::CheckStateRole).toBool()
         != s.second)
      {
        model()->setHeaderData(s.first, orientation(), s.second, Qt::CheckStateRole);
//        wasChanged = true;
      }
    }
  }
//  if(wasChanged)
//    emit checkStateChanged(SectionsCheckStates());
}

std::set<std::pair<int, bool>> CheckableHeaderView::SectionsCheckStates() const
{
  std::set<std::pair<int, bool>> states;
  if(!model())
    return states;
  for(int i(0); i < count(); i++)
    if(model()->headerData(i, orientation(), CommonTableView::CheckableRole).toBool())
      states.insert({i, model()->headerData(i, orientation(), Qt::CheckStateRole).toBool()});
  return states;
}

void CheckableHeaderView::SetAllCheckState(bool bChecked)
{
  if(!model())
    return;
  bool wasChanged = false;
  for(int i(0); i < count(); i++)
    if(model()->headerData(i, orientation(), CommonTableView::CheckableRole).toBool())
      if(model()->headerData(i, orientation(), Qt::CheckStateRole).toBool()
         != bChecked)
      {
        model()->setHeaderData(i, orientation(), bChecked, Qt::CheckStateRole);
        wasChanged = true;
      }
  if(wasChanged)
  {
    // TODO web assembly crash on emit signal
//    emit checkStateChanged(SectionsCheckStates());
    update();
  }
}

void CheckableHeaderView::SetSectionsChecked(const QList<int> &sections, bool isChecked)
{
  SetSectionsChecked(QSet<int>{sections.begin(), sections.end()}, isChecked);
}

void CheckableHeaderView::SetSectionsChecked(const QSet<int> &sections, bool isChecked)
{
  if(!model())
    return;
  bool wasChanged = false;
  for(int i : sections)
  {
    if(i < 0 || i > count() - 1)
      continue;
    if(model()->headerData(i, orientation(), CommonTableView::CheckableRole).toBool())
      if(model()->headerData(i, orientation(), Qt::CheckStateRole).toBool()
         != isChecked)
      {
        model()->setHeaderData(i, orientation(), isChecked, Qt::CheckStateRole);
        wasChanged = true;
      }
  }
  if(wasChanged)
  {
//    emit checkStateChanged(SectionsCheckStates());
    update();
  }
}

void CheckableHeaderView::SetSectionsCheckedByIdRole(const QList<int>& sections, bool isChecked)
{
  const auto columns = count();
  const auto currentModel = model();
  bool wasChanged = false;
  for (int col = 0; col < columns; col++)
  {
    const auto id = currentModel->headerData(col, Qt::Horizontal, CommonTableView::IdRole).toInt();
    if (sections.contains(id))
    {
      model()->setHeaderData(col, orientation(), isChecked, Qt::CheckStateRole);
      wasChanged = true;
    }
  }
  if (wasChanged)
  {
    update();
  }
}

void CheckableHeaderView::SetCheckboxesHidden(bool isHidden)
{
  isCheckBoxesHidden = isHidden;
  updateItemTextColors();
  update();
}

void CheckableHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
  if(!model() ||
     !model()->headerData(logicalIndex, orientation(), CommonTableView::CheckableRole).toBool() ||
     isCheckBoxesHidden)
  {
    QHeaderView::paintSection(painter, rect, logicalIndex);
    return;
  }

  painter->save();
  QStyleOptionButton option = calcCheckboxOpt(rect, logicalIndex);

  defaultPaintWithLabelOffset(painter, rect, logicalIndex, QPoint(option.rect.right() - rect.left() + platesMargins, 0));

  style()->drawControl(QStyle::CE_CheckBox, &option, painter, this);
  painter->restore();
}

QSize CheckableHeaderView::sectionSizeFromContents(int logicalIndex) const
{
  QSize sz = QHeaderView::sectionSizeFromContents(logicalIndex);

  if(!model()->headerData(logicalIndex, orientation(), CommonTableView::CheckableRole).toBool())
    return sz;

  QSize checkBoxSize;
  QStyleOptionButton option;
  sz.setWidth(sz.width() + style()->sizeFromContents(QStyle::ContentsType::CT_CheckBox, &option, checkBoxSize).width());
  return sz;
}

void CheckableHeaderView::resizeEvent(QResizeEvent *event)
{
  QHeaderView::resizeEvent(event);
  if(orientation() == Qt::Horizontal)
    CheckBoxLocalRect.moveTop(height()/2 - CheckBoxLocalRect.height()/2);
  else
    CheckBoxLocalRect.moveLeft(width()/2 - CheckBoxLocalRect.width()/2);
}

void CheckableHeaderView::mousePressEvent(QMouseEvent *event)
{
  if(handleCheckBoxClick(event->pos()))
  {
    event->accept();
    return;
  }
  QHeaderView::mousePressEvent(event);
}

void CheckableHeaderView::mouseDoubleClickEvent(QMouseEvent *event)
{
  if(handleCheckBoxClick(event->pos()))
  {
    event->accept();
    return;
  }
  QHeaderView::mouseDoubleClickEvent(event);
}

void CheckableHeaderView::mouseMoveEvent(QMouseEvent *event)
{
  if(model())
  {
    auto checkBoxRect = CheckBoxLocalRect;
    for(int i = 0; i < count(); i++)
    {
      if(!model()->headerData(i, orientation(), CommonTableView::CheckableRole).toBool())
        continue;

      if(orientation() == Qt::Horizontal)
        checkBoxRect.moveLeft(sectionViewportPosition(i) + CheckBoxLocalRect.left());
      else
        checkBoxRect.moveTop(sectionViewportPosition(i) + sectionSize(i)/2 - CheckBoxLocalRect.height()/2);

      if(checkBoxRect.contains(event->pos()))
      {
        CheckBoxHovered = i;
        event->accept();
        update();
        QHeaderView::mouseMoveEvent(event);
        return;
      }
    }
    CheckBoxHovered = -1;
    update();
  }
  QHeaderView::mouseMoveEvent(event);
}

void CheckableHeaderView::setModel(QAbstractItemModel *model)
{
  QHeaderView::setModel(model);
  connect(model, &QAbstractItemModel::headerDataChanged, this,
          [this](Qt::Orientation orientation, int first, int last)
  {
    if(orientation == this->orientation())
      emit checkStateChanged(SectionsCheckStates());
  });
  if(!this->model())
    return;
  updateItemTextColors();
}

QStyleOptionButton CheckableHeaderView::calcCheckboxOpt(const QRect& rect, int logicalIndex) const
{
  if(!model() ||
     !model()->headerData(logicalIndex, orientation(), CommonTableView::CheckableRole).toBool())
  {
    return QStyleOptionButton();
  }
  QStyleOptionButton option;
  auto headerData = model()->headerData(logicalIndex, orientation(), Qt::CheckStateRole).toBool();
  if (headerData)
    option.state = QStyle::State_On | QStyle::State_Enabled;
  else
    option.state = QStyle::State_Off | QStyle::State_Enabled;
  if(CheckBoxHovered == logicalIndex)
  {
    option.state |= QStyle::State_MouseOver;
  }
  option.rect = CheckBoxLocalRect;
  if(orientation() == Qt::Horizontal)
    option.rect.moveLeft(option.rect.left() + rect.left());
  else
    option.rect.moveTop(rect.top() + sectionSize(logicalIndex)/2 - option.rect.height()/2);

  if(!isEnabled())
    option.state.setFlag(QStyle::State_Enabled, false);

  return option;
}

void CheckableHeaderView::defaultPaintWithLabelOffset(QPainter *painter, const QRect &rect, int logicalIndex, QPoint labelTranslation) const
{
  painter->save();
  {//default painting
    QStyleOptionHeaderV2 opt;
    QPointF oldBO = painter->brushOrigin();

    initStyleOption(&opt);

    QBrush oBrushButton = opt.palette.brush(QPalette::Button);
    QBrush oBrushWindow = opt.palette.brush(QPalette::Window);

    initStyleOptionForIndex(&opt, logicalIndex);
    // We set rect here. If it needs to be changed it can be changed by overriding this function
    opt.rect = rect;

    QBrush nBrushButton = opt.palette.brush(QPalette::Button);
    QBrush nBrushWindow = opt.palette.brush(QPalette::Window);

    // If relevant brushes are not the same as from the regular widgets we set the brush origin
    if (oBrushButton != nBrushButton || oBrushWindow != nBrushWindow) {
      painter->setBrushOrigin(opt.rect.topLeft());
    }

    // draw the section.
    style()->drawControl(QStyle::CE_HeaderSection, &opt, painter, this);

    //not default part
    painter->translate(labelTranslation);
    //end of non default part
    style()->drawControl(QStyle::CE_HeaderLabel, &opt, painter, this);

    painter->setBrushOrigin(oldBO);
  }
  painter->restore();
}

bool CheckableHeaderView::handleCheckBoxClick(const QPoint &pos)
{
  if(!model())
    return false;

  auto checkBoxRect = CheckBoxLocalRect;
  for(int i = 0; i < count(); i++)
  {
    if(!model()->headerData(i, orientation(), CommonTableView::CheckableRole).toBool())
      continue;

    if(orientation() == Qt::Horizontal)
      checkBoxRect.moveLeft(sectionViewportPosition(i) + CheckBoxLocalRect.left());
    else
      checkBoxRect.moveTop(sectionViewportPosition(i) + sectionSize(i)/2 - checkBoxRect.height()/2);

    if(checkBoxRect.contains(pos))
    {
      if(!model())
        return false;

      QSet<int> selectedLines;
      if(orientation() == Qt::Horizontal)
        for(auto ind : this->selectionModel()->selectedColumns())
          selectedLines.insert(ind.column());
      else
        for(auto ind : this->selectionModel()->selectedRows())
          selectedLines.insert(ind.row());


      if(selectedLines.isEmpty() || !selectedLines.contains(i))
      {
        auto headerData = model()->headerData(i, orientation(), Qt::CheckStateRole).toBool();
        model()->setHeaderData(i, orientation(), !headerData, Qt::CheckStateRole);
      }
      else
      {
        auto headerData = model()->headerData(i, orientation(), Qt::CheckStateRole).toBool();
        for(auto ind : selectedLines)
          model()->setHeaderData(ind, orientation(), !headerData, Qt::CheckStateRole);
      }
//      emit checkStateChanged(SectionsCheckStates());
      return true;
    }
  }
  return false;
}

void CheckableHeaderView::onCheckStateChanged()
{
  if(!model())
    return;

  updateItemTextColors();
}

void CheckableHeaderView::updateItemTextColors()
{
  if(!model())
    return;

  for(int row = 0; row < model()->rowCount(); row++)
  {
    for(int col = 0; col < model()->columnCount(); col++)
    {
      if(!model()->headerData(row, Qt::Vertical, CommonTableView::CheckableRole).toBool()
         || !model()->headerData(col, Qt::Horizontal, CommonTableView::CheckableRole).toBool()
         || isCheckBoxesHidden)
      {
        model()->setData(model()->index(row, col), palette().text(), Qt::ForegroundRole);
      }
      else if(model()->headerData(row, Qt::Vertical, Qt::CheckStateRole).toBool()
         && model()->headerData(col, Qt::Horizontal, Qt::CheckStateRole).toBool())
      {
        model()->setData(model()->index(row, col), palette().text(), Qt::ForegroundRole);
      }
      else
      {
        model()->setData(model()->index(row, col), palette().midlight(), Qt::ForegroundRole);
      }
    }
  }
//  if(this->orientation() == Qt::Horizontal)

//      if(NonCheckableSections.contains(section))
//        model()->setData(model()->index(row, section), palette().text(), Qt::ForegroundRole);
//      else if(checked)
//        model()->setData(model()->index(row, section), palette().text(), Qt::ForegroundRole);
//      else
//        model()->setData(model()->index(row, section), palette().midlight(), Qt::ForegroundRole);
//  else
//    for(int col = 0; col < model()->columnCount(); col++)
//      if(NonCheckableSections.contains(section))
//        model()->setData(model()->index(section, col), palette().text(), Qt::ForegroundRole);
//      else if(checked)
//        model()->setData(model()->index(section, col), palette().text(), Qt::ForegroundRole);
//      else
//        model()->setData(model()->index(section, col), palette().midlight(), Qt::ForegroundRole);
}
