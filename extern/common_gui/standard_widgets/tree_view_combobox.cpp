#include "tree_view_combobox.h"

#include <QTreeView>
#include <QMouseEvent>
#include <QHeaderView>
#include <QLineEdit>
#include <QStylePainter>
#include <QApplication>
#include <QStyledItemDelegate>
#include <QTextLayout>
#include <QFontMetrics>

TreeViewComboBox::TreeViewComboBox(QWidget* parent)
  : QComboBox(parent)
{
  auto view = new QTreeView(this);
  view->setFrameShape(QFrame::NoFrame);
  view->setEditTriggers(QTreeView::NoEditTriggers);
  view->setAlternatingRowColors(false);
  view->setSelectionBehavior(QTreeView::SelectRows);
  view->setSelectionMode(QAbstractItemView::SingleSelection);
  view->setRootIsDecorated(false);
  view->setAllColumnsShowFocus(true);
  view->setItemsExpandable(false);
  view->header()->setStretchLastSection(true);
  view->header()->setVisible(false);
  view->setStyleSheet("QTreeView::branch {  border-image: url(none.png); }");
  view->setWordWrap(true);
  view->setUniformRowHeights(false);
  view->setItemDelegate(new TreeViewComboBoxItemDlegate(view));
  setView(view);

  setEditable(true);

  lineEdit()->setReadOnly(true);
  disconnect(lineEdit(), 0, this, 0);
  OpenTimer.setSingleShot(true);

  setCurrentIndex(-1);
  connect(this, SIGNAL(activated(int)), this, SLOT(OnCurrentIndexChanged()));

  findChild<QFrame*>()->installEventFilter(this);
  lineEdit()->installEventFilter(this);
  view->installEventFilter(this);
  view->viewport()->installEventFilter(this);
}

TreeViewComboBox::~TreeViewComboBox()
{
}

void TreeViewComboBox::SetCanAcceptIndexFunction(TreeViewComboBox::CanAcceptIndexFunc func)
{
  CanAcceptIndex = func;
}

void TreeViewComboBox::SetCurrentIndex(const QModelIndex& index)
{
  if (index.isValid())
    CurrentModelIndex = index;
  else
    CurrentModelIndex = QPersistentModelIndex();
  setCurrentIndex(-1);
  UpdateCurrentText();
}

QModelIndex TreeViewComboBox::GetCurrentIndex() const
{
  return CurrentModelIndex;
}

void TreeViewComboBox::setModel(QAbstractItemModel* m)
{
  if (auto m = model())
    disconnect(m, 0, this, 0);

  QComboBox::setModel(m);

  if (m)
  {
    connect(m, &QAbstractItemModel::dataChanged, this, &TreeViewComboBox::OnModelDataChanged);
    connect(m, &QAbstractItemModel::rowsInserted, this, &TreeViewComboBox::OnModelDataChanged);
    connect(m, &QAbstractItemModel::rowsRemoved, this, &TreeViewComboBox::OnModelDataChanged);
    connect(m, &QAbstractItemModel::columnsInserted, this, &TreeViewComboBox::OnModelDataChanged);
    connect(m, &QAbstractItemModel::columnsRemoved, this, &TreeViewComboBox::OnModelDataChanged);
    connect(m, &QAbstractItemModel::modelReset, this, &TreeViewComboBox::OnModelDataChanged);
  }
  OnModelDataChanged();
}

void TreeViewComboBox::showPopup()
{
  QComboBox::showPopup();
  if (auto sm = view()->selectionModel())
  {
    if (CurrentModelIndex.isValid())
    {
      sm->setCurrentIndex(CurrentModelIndex, QItemSelectionModel::ClearAndSelect);
    }
    else
    {
      sm->clearCurrentIndex();
      sm->clearSelection();
    }
  }
}

void TreeViewComboBox::OnCurrentIndexChanged()
{
  QModelIndex newIndex = CurrentModelIndex;
  if (auto treeView = qobject_cast<QTreeView*>(view()))
  {
    if (auto sm = treeView->selectionModel())
    {
      auto index = sm->currentIndex();
      if (index.isValid() && (!CanAcceptIndex || CanAcceptIndex(index)))
      {
        newIndex = index;
      }
    }
  }

  if (newIndex.isValid() && newIndex != CurrentModelIndex)
  {
    CurrentModelIndex = newIndex;
    UpdateCurrentText();
    emit CurrentIndexChanged(CurrentModelIndex);
  }
  else
  {
    UpdateCurrentText();
  }
}

void TreeViewComboBox::OnModelDataChanged()
{
  if (auto treeView = qobject_cast<QTreeView*>(view()))
  {
    treeView->expandAll();
    treeView->setItemsExpandable(false);
  }
  if (!CurrentModelIndex.isValid())
  {
    setCurrentIndex(-1);
    OnCurrentIndexChanged();
  }
}

void TreeViewComboBox::UpdateCurrentText()
{
  QString text;
  if (CurrentModelIndex.isValid())
  {
    text = CurrentModelIndex.data(Qt::DisplayRole).toString();
  }
  setCurrentText(text);
}

void TreeViewComboBox::paintEvent(QPaintEvent* event)
{
  if (auto le = lineEdit())
  {
    // remove indent for icon (becouse icon paint does not work with tree view)
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    QRect editRect = style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                                QStyle::SC_ComboBoxEditField, this);
    QRect comboRect(editRect);
    editRect.setWidth(editRect.width());
    editRect = QStyle::alignedRect(layoutDirection(), Qt::AlignRight,
                                   editRect.size(), comboRect);
    le->setGeometry(editRect);
  }
  QComboBox::paintEvent(event);
}

bool TreeViewComboBox::eventFilter(QObject* object, QEvent* event)
{
  if (object)
  {
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)
    {
      auto ev = static_cast<QMouseEvent*>(event);
      auto v = view();
      if (v && v->isVisible() && CanAcceptIndex)
      {
        auto pos = v->mapFromGlobal(ev->globalPos());
        if (v->rect().contains(pos))
        {
          auto index = v->indexAt(pos);
          if (index.isValid() && CanAcceptIndex && !CanAcceptIndex(index))
          {
            view()->show();
            event->accept();
            return true;
          }
          if (index == CurrentModelIndex)
          {
            hidePopup();
            event->accept();
            return true;
          }
        }
        else // outside
        {
          if (event->type() == QEvent::MouseButtonRelease)
          {
            return true;
          }
        }
      }
    }

    if (event->type() == QEvent::MouseButtonPress)
    {
      auto ev = static_cast<QMouseEvent*>(event);
      if (object == lineEdit() || object == this)
      {
        if (rect().contains(mapFromGlobal(ev->globalPos())))
        {
          if (view()->visibleRegion().boundingRect().height() == 0  && !OpenTimer.isActive())
          {
            showPopup();
            event->accept();
            return true;
          }
        }
      }
    }
  }
  if (object == findChild<QFrame*>())
  {
    if (view()->visibleRegion().boundingRect().height() == 0)
      OpenTimer.start(QApplication::doubleClickInterval() / 2);
  }
  return false;
}

TreeViewComboBoxItemDlegate::TreeViewComboBoxItemDlegate(QObject* parent)
  : QStyledItemDelegate(parent)
  , CachedComboWidth(-1)
{
}

TreeViewComboBoxItemDlegate::~TreeViewComboBoxItemDlegate()
{
}

void TreeViewComboBoxItemDlegate::NotifySizeHintChanged(QAbstractItemModel* model, const QModelIndex& index)
{
  if (!model)
    return;

  for (int r = 0; r < model->rowCount(); r++)
  {
    QModelIndex child = model->index(r, 0, index);
    if (child.isValid())
    {
      NotifySizeHintChanged(model, child);
    }
  }
  emit sizeHintChanged(index);
}

void TreeViewComboBoxItemDlegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter, option, index);

  const QWidget* widget = option.widget;
  if (const QTreeView* tree = qobject_cast<const QTreeView*>(widget))
  {
    if (QWidget* treeParent = tree->parentWidget())
    {
      if (QWidget* treeParentCombo = treeParent->parentWidget())
      {
        if (QComboBox* combo = qobject_cast<QComboBox*>(treeParentCombo))
        {
          if (combo->width() != CachedComboWidth)
          {
            CachedComboWidth = combo->width();
            const_cast<TreeViewComboBoxItemDlegate*>(this)->NotifySizeHintChanged(tree->model());
          }
          else
          {
            QSize hint = sizeHint(option, index);
            if (hint.height() > option.rect.height())
            {
              const_cast<TreeViewComboBoxItemDlegate*>(this)->NotifySizeHintChanged(tree->model());
            }
          }
        }
      }
    }
  }
}

QSize TreeViewComboBoxItemDlegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QSize result = QStyledItemDelegate::sizeHint(option, index);

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  const QWidget* widget = option.widget;
  if (widget && index.isValid())
  {
    if (const QTreeView* tree = qobject_cast<const QTreeView*>(widget))
    {
      if (QWidget* treeParent = tree->parentWidget())
      {
        if (QWidget* treeParentCombo = treeParent->parentWidget())
        {
          if (tree->isVisibleTo(tree->window()) && !tree->visibleRegion().isEmpty())
          {
            if (QComboBox* combo = qobject_cast<QComboBox*>(treeParentCombo))
            {
              // Margin
              int textMargin = 3;

              // data
              QString text    = index.data(Qt::DisplayRole).toString();
              QVariant icon   = index.data(Qt::DecorationRole);
              QFontMetrics fm = widget->fontMetrics();
              int w           = tree->header()->sectionSize(index.column());
              
              // Full rect
              QRect rowRect;
              rowRect.setWidth(w);
              rowRect.setHeight(std::numeric_limits<int>::max());

              // Calculate indentation
              int nesting = 0;
              QModelIndex nestedIndex = index;
              while (nestedIndex.parent().isValid())
              {
                ++nesting;
                nestedIndex = nestedIndex.parent();
              }
              int indentation = nesting * tree->indentation();

              // Text rect
              QRect textRect = rowRect
                // adjusted by text margin
                .adjusted(textMargin, 0, -textMargin, 0)
                // adjusted by decoration
                .adjusted(icon.isNull() ? 0 : option.decorationSize.width() + textMargin, 0, 0, 0)
                // adjusted by indentation
                .adjusted(indentation, 0, 0, 0);

              // Measure
              QRect measured = fm.boundingRect(textRect, (Qt::AlignLeft | Qt::AlignTop | (option.features & QStyleOptionViewItem::WrapText)) ? Qt::TextWordWrap : 0,
                                               text);
              // Apply height
              result.setHeight(measured.height() + textMargin * 2);
            }
          }
        }
      }
    }
  }
  return result;
}
