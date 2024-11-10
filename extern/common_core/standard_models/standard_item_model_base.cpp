#include "standard_item_model_base.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qbitarray.h>
#include <QtCore/qmimedata.h>

#include <QIODevice>

#include "standard_item_model_base_p.h"

#include <qdebug.h>
#include <algorithm>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define VARIANT_TYPE_ID typeId
#else
#define VARIANT_TYPE_ID type
#endif

QT_BEGIN_NAMESPACE

static inline QString StandardItemModelBaseDataListMimeType()
{
    return QStringLiteral("application/x-standarditemmodelbasedatalist");
}

class StandardItemModelBaseLessThan
{
public:
    inline StandardItemModelBaseLessThan()
        { }

    inline bool operator()(const QPair<StandardItem*, int> &l,
                           const QPair<StandardItem*, int> &r) const
    {
        return *(l.first) < *(r.first);
    }
};

class StandardItemModelBaseGreaterThan
{
public:
    inline StandardItemModelBaseGreaterThan()
        { }

    inline bool operator()(const QPair<StandardItem*, int> &l,
                           const QPair<StandardItem*, int> &r) const
    {
        return *(r.first) < *(l.first);
    }
};

/*!
  \internal
*/
QPair<int, int> StandardItemPrivate::position() const
{
    if (StandardItem *par = parent) {
        int idx = par->d_func()->childIndex(q_func());
        if (idx == -1)
            return QPair<int, int>(-1, -1);
        return QPair<int, int>(idx / par->columnCount(), idx % par->columnCount());
    }
    // ### support header items?
    return QPair<int, int>(-1, -1);
}

/*!
  \internal
*/
void StandardItemPrivate::setChild(int row, int column, StandardItem *item,
                                    bool emitChanged)
{
    Q_Q(StandardItem);
    if (item == q) {
        qWarning("StandardItem::setChild: Can't make an item a child of itself %p",
                 item);
        return;
    }
    if ((row < 0) || (column < 0))
        return;
    if (rows <= row)
        q->setRowCount(row + 1);
    if (columns <= column)
        q->setColumnCount(column + 1);
    int index = childIndex(row, column);
    Q_ASSERT(index != -1);
    StandardItem *oldItem = children.at(index);
    if (item == oldItem)
        return;

    if (model && emitChanged) {
        emit model->layoutAboutToBeChanged();
    }

    if (item) {
        if (item->d_func()->parent == 0) {
            item->d_func()->setParentAndModel(q, model);
        } else {
            qWarning("StandardItem::setChild: Ignoring duplicate insertion of item %p",
                     item);
            return;
        }
    }
    if (oldItem)
        oldItem->d_func()->setModel(0);
    delete oldItem;
    children.replace(index, item);
    if (item)
        item->d_func()->lastKnownIndex = index;

    if (model && emitChanged)
        emit model->layoutChanged();

    if (emitChanged && model) {
        if (item) {
            model->d_func()->itemChanged(item);
        } else {
            const QModelIndex idx = model->index(row, column, q->index());
            emit model->dataChanged(idx, idx);
        }
    }
}


/*!
  \internal
*/
void StandardItemPrivate::changeFlags(bool enable, Qt::ItemFlags f)
{
    Q_Q(StandardItem);
    Qt::ItemFlags flags = q->flags();
    if (enable)
        flags |= f;
    else
        flags &= ~f;
    q->setFlags(flags);
}

/*!
  \internal
*/
void StandardItemPrivate::childDeleted(StandardItem *child)
{
    int index = childIndex(child);
    Q_ASSERT(index != -1);
    const auto modelIndex = child->index();
    children.replace(index, 0);
    emit model->dataChanged(modelIndex, modelIndex);
}

namespace {

    struct ByNormalizedRole
    {
        static int normalizedRole(int role)
        {
            return role == Qt::EditRole ? Qt::DisplayRole : role;
        }

       bool operator()(const StandardItemData& standardItemData, const std::pair<const int &, const QVariant&>& roleMapIt) const
       {
           return standardItemData.role < normalizedRole(roleMapIt.first);
       }
       bool operator()(const std::pair<const int&, const QVariant &>& roleMapIt, const StandardItemData& standardItemData) const
       {
           return normalizedRole(roleMapIt.first) < standardItemData.role;
       }

    };

    /*
        Based on std::transform with a twist. The inputs are iterators of <int, QVariant> pair.
        The variant is checked for validity and if not valid, that element is not taken into account
        which means that the resulting output might be shorter than the input.
    */
    template<class Input, class OutputIt>
    OutputIt roleMapStandardItemDataTransform(Input first1, Input last1, OutputIt d_first)
    {
        while (first1 != last1) {
            if ((*first1).second.isValid())
                *d_first++ = StandardItemData(*first1);
            ++first1;
        }
        return d_first;
    }


    /*
        Based on std::set_union with a twist. The idea is to create a union of both inputs
        with an additional constraint: if an input contains an invalid variant, it means
        that this one should not be taken into account for generating the output.
    */
    template<class Input1, class Input2,
             class OutputIt, class Compare>
    OutputIt roleMapStandardItemDataUnion(Input1 first1, Input1 last1,
                                          Input2 first2, Input2 last2,
                                          OutputIt d_first, Compare comp)
    {
        for (; first1 != last1; ++d_first) {
            if (first2 == last2) {
                return roleMapStandardItemDataTransform(first1, last1, d_first);
            }
            if (comp(*first2, *first1)) {
                *d_first = *first2++;
            } else {
                if ((*first1).second.isValid())
                    *d_first = StandardItemData(*first1);
                if (!comp(*first1, *first2))
                    ++first2;
                ++first1;
            }
        }
        return std::copy(first2, last2, d_first);
    }
}

/*!
  \internal
*/
void StandardItemPrivate::setItemData(const QMap<int, QVariant> &roles)
{
    Q_Q(StandardItem);

    auto byRole = [](const StandardItemData& item1, const StandardItemData& item2) {
        return item1.role < item2.role;
    };

    std::sort(values.begin(), values.end(), byRole);

    /*
        Create a vector of StandardItemData that will contain the original values
        if the matching role is not contained in roles, the new value if it is and
        if the new value is an invalid QVariant, it will be removed.
    */
    QVector<StandardItemData> newValues;
    newValues.reserve(values.size());
    roleMapStandardItemDataUnion(roles.keyValueBegin(),
                                 roles.keyValueEnd(),
                                 values.cbegin(), values.cend(),
                                 std::back_inserter(newValues), ByNormalizedRole());

    if (newValues != values) {
        values.swap(newValues);
        if (model) {
            QVector<int> roleKeys;
            roleKeys.reserve(roles.size() + 1);
            bool hasEditRole = false;
            bool hasDisplayRole = false;
            for (auto it = roles.keyBegin(); it != roles.keyEnd(); ++it) {
                roleKeys.push_back(*it);
                if (*it == Qt::EditRole)
                    hasEditRole = true;
                else if (*it == Qt::DisplayRole)
                    hasDisplayRole = true;
            }
            if (hasEditRole && !hasDisplayRole)
                roleKeys.push_back(Qt::DisplayRole);
            else if (!hasEditRole && hasDisplayRole)
                roleKeys.push_back(Qt::EditRole);
            model->d_func()->itemChanged(q, roleKeys);
        }
    }
}

/*!
  \internal
*/
const QMap<int, QVariant> StandardItemPrivate::itemData() const
{
    QMap<int, QVariant> result;
    QVector<StandardItemData>::const_iterator it;
    for (it = values.cbegin(); it != values.cend(); ++it){
        // Qt::UserRole - 1 is used internally to store the flags
        if (it->role != Qt::UserRole - 1)
            result.insert(it->role, it->value);
    }
    return result;
}

/*!
  \internal
*/
void StandardItemPrivate::sortChildren(int column, Qt::SortOrder order)
{
    Q_Q(StandardItem);
    if (column >= columnCount())
        return;

    QVector<QPair<StandardItem*, int>> sortable;
    QVector<int> unsortable;

    sortable.reserve(rowCount());
    unsortable.reserve(rowCount());

    for (int row = 0; row < rowCount(); ++row) {
        StandardItem *itm = q->child(row, column);
        if (itm)
            sortable.append(QPair<StandardItem*,int>(itm, row));
        else
            unsortable.append(row);
    }

    if (order == Qt::AscendingOrder) {
        StandardItemModelBaseLessThan lt;
        std::stable_sort(sortable.begin(), sortable.end(), lt);
    } else {
        StandardItemModelBaseGreaterThan gt;
        std::stable_sort(sortable.begin(), sortable.end(), gt);
    }

    QModelIndexList changedPersistentIndexesFrom, changedPersistentIndexesTo;
    QVector<StandardItem*> sorted_children(children.count());
    for (int i = 0; i < rowCount(); ++i) {
        int r = (i < sortable.count()
                 ? sortable.at(i).second
                 : unsortable.at(i - sortable.count()));
        for (int c = 0; c < columnCount(); ++c) {
            StandardItem *itm = q->child(r, c);
            sorted_children[childIndex(i, c)] = itm;
            if (model) {
                QModelIndex from = model->createIndex(r, c, q);
                if (model->d_func()->persistent.indexes.contains(from)) {
                    QModelIndex to = model->createIndex(i, c, q);
                    changedPersistentIndexesFrom.append(from);
                    changedPersistentIndexesTo.append(to);
                }
            }
        }
    }

    children = sorted_children;

    if (model) {
        model->changePersistentIndexList(changedPersistentIndexesFrom, changedPersistentIndexesTo);
    }

    QVector<StandardItem*>::iterator it;
    for (it = children.begin(); it != children.end(); ++it) {
        if (*it)
            (*it)->d_func()->sortChildren(column, order);
    }
}

/*!
  \internal
  set the model of this item and all its children
  */
void StandardItemPrivate::setModel(StandardItemModelBase *mod)
{
    if (children.isEmpty()) {
        if (model)
            model->d_func()->invalidatePersistentIndex(model->indexFromItem(q_ptr));
        model = mod;
    } else {
        QStack<StandardItem*> stack;
        stack.push(q_ptr);
        while (!stack.isEmpty()) {
            StandardItem *itm = stack.pop();
            if (itm->d_func()->model) {
                itm->d_func()->model->d_func()->invalidatePersistentIndex(itm->d_func()->model->indexFromItem(itm));
            }
            itm->d_func()->model = mod;
            const QVector<StandardItem*> &childList = itm->d_func()->children;
            for (int i = 0; i < childList.count(); ++i) {
                StandardItem *chi = childList.at(i);
                if (chi)
                    stack.push(chi);
            }
        }
    }
}

/*!
  \internal
*/
StandardItemModelBasePrivate::StandardItemModelBasePrivate()
    : root(new StandardItem),
      itemPrototype(0),
      sortRole(Qt::DisplayRole),
      sortCaseSensitivity(Qt::CaseInsensitive)
{
    root->setFlags(Qt::ItemIsDropEnabled);
}

/*!
  \internal
*/
StandardItemModelBasePrivate::~StandardItemModelBasePrivate()
{
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::init()
{
    Q_Q(StandardItemModelBase);
    QObject::connect(q, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                     q, SLOT(_q_emitItemChanged(QModelIndex,QModelIndex)));
}

/*!
    \internal
*/
void StandardItemModelBasePrivate::_q_emitItemChanged(const QModelIndex &topLeft,
                                                   const QModelIndex &bottomRight)
{
    Q_Q(StandardItemModelBase);
    QModelIndex parent = topLeft.parent();
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
        for (int column = topLeft.column(); column <= bottomRight.column(); ++column) {
            QModelIndex index = q->index(row, column, parent);
            if (StandardItem *item = itemFromIndex(index))
                emit q->itemChanged(item);
        }
    }
}

/*!
    \internal
*/
bool StandardItemPrivate::insertRows(int row, const QList<StandardItem*> &items)
{
    Q_Q(StandardItem);
    if ((row < 0) || (row > rowCount()) || items.isEmpty())
        return false;
    int count = items.count();
    if (model)
        model->d_func()->rowsAboutToBeInserted(q, row, row + count - 1);
    if (rowCount() == 0) {
        if (columnCount() == 0)
            q->setColumnCount(1);
        children.resize(columnCount() * count);
        rows = count;
    } else {
        rows += count;
        int index = childIndex(row, 0);
        if (index != -1)
            children.insert(index, columnCount() * count, 0);
    }
    for (int i = 0; i < items.count(); ++i) {
        if (StandardItem *item = items.at(i)) {
          item->d_func()->model = model;
          item->d_func()->parent = q;
          int index = childIndex(i + row, 0);
          children.replace(index, item);
          item->d_func()->lastKnownIndex = index;
        }
    }
    if (model)
        model->d_func()->rowsInserted(q, row, count);
    return true;
}

bool StandardItemPrivate::insertRows(int row, int count, const QList<StandardItem*> &items)
{
    Q_Q(StandardItem);
    if ((count < 1) || (row < 0) || (row > rowCount()) || count == 0)
        return false;
    if (model)
        model->d_func()->rowsAboutToBeInserted(q, row, row + count - 1);
    if (rowCount() == 0) {
        children.resize(columnCount() * count);
        rows = count;
    } else {
        rows += count;
        int index = childIndex(row, 0);
        if (index != -1)
            children.insert(index, columnCount() * count, 0);
    }
    if (!items.isEmpty()) {
        int index = childIndex(row, 0);
        int limit = qMin(items.count(), columnCount() * count);
        for (int i = 0; i < limit; ++i) {
            StandardItem *item = items.at(i);
            if (item) {
                if (item->d_func()->parent == 0) {
                    item->d_func()->setParentAndModel(q, model);
                } else {
                    qWarning("StandardItem::insertRows: Ignoring duplicate insertion of item %p",
                             item);
                    item = 0;
                }
            }
            children.replace(index, item);
            if (item)
                item->d_func()->lastKnownIndex = index;
            ++index;
        }
    }
    if (model)
        model->d_func()->rowsInserted(q, row, count);
    return true;
}

/*!
    \internal
*/
bool StandardItemPrivate::insertColumns(int column, int count, const QList<StandardItem*> &items)
{
    Q_Q(StandardItem);
    if ((count < 1) || (column < 0) || (column > columnCount()) || count == 0)
        return false;
    if (model)
        model->d_func()->columnsAboutToBeInserted(q, column, column + count - 1);
    if (columnCount() == 0) {
        children.resize(rowCount() * count);
        columns = count;
    } else {
        columns += count;
        int index = childIndex(0, column);
        for (int row = 0; row < rowCount(); ++row) {
            children.insert(index, count, 0);
            index += columnCount();
        }
    }
    if (!items.isEmpty()) {
        int limit = qMin(items.count(), rowCount() * count);
        for (int i = 0; i < limit; ++i) {
            StandardItem *item = items.at(i);
            if (item) {
                if (item->d_func()->parent == 0) {
                    item->d_func()->setParentAndModel(q, model);
                } else {
                    qWarning("StandardItem::insertColumns: Ignoring duplicate insertion of item %p",
                             item);
                    item = 0;
                }
            }
            int r = i / count;
            int c = column + (i % count);
            int index = childIndex(r, c);
            children.replace(index, item);
            if (item)
                item->d_func()->lastKnownIndex = index;
        }
    }
    if (model)
        model->d_func()->columnsInserted(q, column, count);
    return true;
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::itemChanged(StandardItem *item, const QVector<int> &roles)
{
    Q_Q(StandardItemModelBase);
    Q_ASSERT(item);
    if (item->d_func()->parent == 0) {
        // Header item
        int idx = columnHeaderItems.indexOf(item);
        if (idx != -1) {
            emit q->headerDataChanged(Qt::Horizontal, idx, idx);
        } else {
            idx = rowHeaderItems.indexOf(item);
            if (idx != -1)
                emit q->headerDataChanged(Qt::Vertical, idx, idx);
        }
    } else {
        // Normal item
        const QModelIndex index = q->indexFromItem(item);
        emit q->dataChanged(index, index, roles);
    }
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::rowsAboutToBeInserted(StandardItem *parent,
                                                      int start, int end)
{
    Q_Q(StandardItemModelBase);
    QModelIndex index = q->indexFromItem(parent);
    q->beginInsertRows(index, start, end);
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::columnsAboutToBeInserted(StandardItem *parent,
                                                         int start, int end)
{
    Q_Q(StandardItemModelBase);
    QModelIndex index = q->indexFromItem(parent);
    q->beginInsertColumns(index, start, end);
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::rowsAboutToBeRemoved(StandardItem *parent,
                                                     int start, int end)
{
    Q_Q(StandardItemModelBase);
    QModelIndex index = q->indexFromItem(parent);
    q->beginRemoveRows(index, start, end);
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::columnsAboutToBeRemoved(StandardItem *parent,
                                                        int start, int end)
{
    Q_Q(StandardItemModelBase);
    QModelIndex index = q->indexFromItem(parent);
    q->beginRemoveColumns(index, start, end);
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::rowsInserted(StandardItem *parent,
                                             int row, int count)
{
    Q_Q(StandardItemModelBase);
    if (parent == root.data())
        rowHeaderItems.insert(row, count, 0);
    q->endInsertRows();
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::columnsInserted(StandardItem *parent,
                                                int column, int count)
{
    Q_Q(StandardItemModelBase);
    if (parent == root.data())
        columnHeaderItems.insert(column, count, 0);
    q->endInsertColumns();
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::rowsRemoved(StandardItem *parent,
                                            int row, int count)
{
    Q_Q(StandardItemModelBase);
    if (parent == root.data()) {
        for (int i = row; i < row + count; ++i) {
            StandardItem *oldItem = rowHeaderItems.at(i);
            if (oldItem)
                oldItem->d_func()->setModel(0);
            delete oldItem;
        }
        rowHeaderItems.remove(row, count);
    }
    q->endRemoveRows();
}

/*!
  \internal
*/
void StandardItemModelBasePrivate::columnsRemoved(StandardItem *parent,
                                               int column, int count)
{
    Q_Q(StandardItemModelBase);
    if (parent == root.data()) {
        for (int i = column; i < column + count; ++i) {
            StandardItem *oldItem = columnHeaderItems.at(i);
            if (oldItem)
                oldItem->d_func()->setModel(0);
            delete oldItem;
        }
        columnHeaderItems.remove(column, count);
    }
    q->endRemoveColumns();
}

/*!
    \class StandardItem
    \brief The StandardItem class provides an item for use with the
    StandardItemModelBase class.
    \since 4.2
    \ingroup model-view
    \inmodule QtGui

    Items usually contain text, icons, or checkboxes.

    Each item can have its own background brush which is set with the
    setBackground() function. The current background brush can be found with
    background().  The text label for each item can be rendered with its own
    font and brush. These are specified with the setFont() and setForeground()
    functions, and read with font() and foreground().

    By default, items are enabled, editable, selectable, checkable, and can be
    used both as the source of a drag and drop operation and as a drop target.
    Each item's flags can be changed by calling setFlags(). Checkable items
    can be checked and unchecked with the setCheckState() function. The
    corresponding checkState() function indicates whether the item is
    currently checked.

    You can store application-specific data in an item by calling setData().

    Each item can have a two-dimensional table of child items. This makes it
    possible to build hierarchies of items. The typical hierarchy is the tree,
    in which case the child table is a table with a single column (a list).

    The dimensions of the child table can be set with setRowCount() and
    setColumnCount(). Items can be positioned in the child table with
    setChild(). Get a pointer to a child item with child(). New rows and
    columns of children can also be inserted with insertRow() and
    insertColumn(), or appended with appendRow() and appendColumn(). When
    using the append and insert functions, the dimensions of the child table
    will grow as needed.

    An existing row of children can be removed with removeRow() or takeRow();
    correspondingly, a column can be removed with removeColumn() or
    takeColumn().

    An item's children can be sorted by calling sortChildren().

    \section1 Subclassing

    When subclassing StandardItem to provide custom items, it is possible to
    define new types for them so that they can be distinguished from the base
    class. The type() function should be reimplemented to return a new type
    value equal to or greater than \l UserType.

    Reimplement data() and setData() if you want to perform custom handling of
    data queries and/or control how an item's data is represented.

    Reimplement clone() if you want StandardItemModelBase to be able to create
    instances of your custom item class on demand (see
    StandardItemModelBase::setItemPrototype()).

    Reimplement read() and write() if you want to control how items are
    represented in their serialized form.

    Reimplement \l{operator<()} if you want to control the semantics of item
    comparison. \l{operator<()} determines the sorted order when sorting items
    with sortChildren() or with StandardItemModelBase::sort().

    \sa StandardItemModelBase, {Item View Convenience Classes}, {Model/View Programming}
*/

/*!
    \enum StandardItem::ItemType

    This enum describes the types that are used to describe standard items.

    \value Type     The default type for standard items.
    \value UserType The minimum value for custom types. Values below UserType are
                    reserved by Qt.

    You can define new user types in StandardItem subclasses to ensure that
    custom items are treated specially; for example, when items are sorted.

    \sa type()
*/

/*!
    Constructs an item.
*/
StandardItem::StandardItem()
    : StandardItem(*new StandardItemPrivate)
{
}

/*!
    Constructs an item with the given \a text.
*/
StandardItem::StandardItem(const QString &text)
    : StandardItem(*new StandardItemPrivate)
{
    setText(text);
}

/*!
    Constructs an item with the given \a icon and \a text.
*/
StandardItem::StandardItem(const QVariant &icon, const QString &text)
    : StandardItem(text)
{
    setIcon(icon);
}

/*!
   Constructs an item with \a rows rows and \a columns columns of child items.
*/
StandardItem::StandardItem(int rows, int columns)
    : StandardItem(*new StandardItemPrivate)
{
    setRowCount(rows);
    setColumnCount(columns);
}

/*!
  \internal
*/
StandardItem::StandardItem(StandardItemPrivate &dd)
    : d_ptr(&dd)
{
    Q_D(StandardItem);
    d->q_ptr = this;
}

/*!
  Constructs a copy of \a other. Note that model() is
  not copied.

  This function is useful when reimplementing clone().
*/
StandardItem::StandardItem(const StandardItem &other)
    : d_ptr(new StandardItemPrivate)
{
    Q_D(StandardItem);
    d->q_ptr = this;
    operator=(other);
}

/*!
  Assigns \a other's data and flags to this item. Note that
  type() and model() are not copied.

  This function is useful when reimplementing clone().
*/
StandardItem &StandardItem::operator=(const StandardItem &other)
{
    Q_D(StandardItem);
    d->values = other.d_func()->values;
    return *this;
}

/*!
  Destructs the item.
  This causes the item's children to be destructed as well.
*/
StandardItem::~StandardItem()
{
    Q_D(StandardItem);
    for (StandardItem *child : qAsConst(d->children)) {
        if (child)
            child->d_func()->setModel(0);
        delete child;
    }
    d->children.clear();
    if (d->parent && d->model)
        d->parent->d_func()->childDeleted(this);
}

/*!
  Returns the item's parent item, or \nullptr if the item has no parent.
  \note For toplevel items parent() returns \nullptr. To receive toplevel
  item's parent use StandardItemModelBase::invisibleRootItem() instead.

  \sa child(), StandardItemModelBase::invisibleRootItem()
*/
StandardItem *StandardItem::parent() const
{
    Q_D(const StandardItem);
    if (!d->model || (d->model->d_func()->root.data() != d->parent))
        return d->parent;
    return 0;
}

/*!
    Sets the item's data for the given \a role to the specified \a value.

    If you subclass StandardItem and reimplement this function, your
    reimplementation should call emitDataChanged() if you do not call
    the base implementation of setData(). This will ensure that e.g.
    views using the model are notified of the changes.

    \note The default implementation treats Qt::EditRole and Qt::DisplayRole
    as referring to the same data.

    \sa Qt::ItemDataRole, data(), setFlags()
*/
void StandardItem::setData(const QVariant &value, int role)
{
    Q_D(StandardItem);
    role = (role == Qt::EditRole) ? Qt::DisplayRole : role;
    const QVector<int> roles((role == Qt::DisplayRole) ?
                                QVector<int>({Qt::DisplayRole, Qt::EditRole}) :
                                QVector<int>({role}));
    QVector<StandardItemData>::iterator it;
    for (it = d->values.begin(); it != d->values.end(); ++it) {
        if ((*it).role == role) {
            if (value.isValid()) {
                if ((*it).value.VARIANT_TYPE_ID() == value.VARIANT_TYPE_ID() && (*it).value == value)
                    return;
                (*it).value = value;
            } else {
                d->values.erase(it);
            }
            if (d->model)
                d->model->d_func()->itemChanged(this, roles);
            return;
        }
    }
    d->values.append(StandardItemData(role, value));
    if (d->model)
        d->model->d_func()->itemChanged(this, roles);
}

/*!
    \since 5.12
    Removes all the data from all roles previously set.
    \sa data(), setData()
*/
void StandardItem::clearData()
{
    Q_D(StandardItem);
    if (d->values.isEmpty())
        return;
    d->values.clear();
    if (d->model)
        d->model->d_func()->itemChanged(this, QVector<int>{});
}

/*!
    Returns the item's data for the given \a role, or an invalid
    QVariant if there is no data for the role.

    \note The default implementation treats Qt::EditRole and Qt::DisplayRole
    as referring to the same data.
*/
QVariant StandardItem::data(int role) const
{
    Q_D(const StandardItem);
    role = (role == Qt::EditRole) ? Qt::DisplayRole : role;
    QVector<StandardItemData>::const_iterator it;
    for (it = d->values.begin(); it != d->values.end(); ++it) {
        if ((*it).role == role)
            return (*it).value;
    }
    return QVariant();
}

/*!
  \since 4.4

  Causes the model associated with this item to emit a
  \l{QAbstractItemModel::dataChanged()}{dataChanged}() signal for this
  item.

  You normally only need to call this function if you have subclassed
  StandardItem and reimplemented data() and/or setData().

  \sa setData()
*/
void StandardItem::emitDataChanged()
{
    Q_D(StandardItem);
    if (d->model)
        d->model->d_func()->itemChanged(this);
}

/*!
  Sets the item flags for the item to \a flags.

  The item flags determine how the user can interact with the item.
  This is often used to disable an item.

  \sa flags(), setData()
*/
void StandardItem::setFlags(Qt::ItemFlags flags)
{
    setData((int)flags, Qt::UserRole - 1);
}

/*!
  Returns the item flags for the item.

  The item flags determine how the user can interact with the item.

  By default, items are enabled, editable, selectable, checkable, and can be
  used both as the source of a drag and drop operation and as a drop target.

  \sa setFlags()
*/
Qt::ItemFlags StandardItem::flags() const
{
    QVariant v = data(Qt::UserRole - 1);
    if (!v.isValid())
        return (Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable
                |Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled);
    return Qt::ItemFlags(v.toInt());
}

/*!
    \fn QString StandardItem::text() const

    Returns the item's text. This is the text that's presented to the user
    in a view.

    \sa setText()
*/

/*!
    \fn void StandardItem::setText(const QString &text)

    Sets the item's text to the \a text specified.

    \sa text(), setFont(), setForeground()
*/

/*!
    \fn QIcon StandardItem::icon() const

    Returns the item's icon.

    \sa setIcon(), {QAbstractItemView::iconSize}{iconSize}
*/

/*!
    \fn void StandardItem::setIcon(const QIcon &icon)

    Sets the item's icon to the \a icon specified.
*/

/*!
    \fn QString StandardItem::statusTip() const

    Returns the item's status tip.

    \sa setStatusTip(), toolTip(), whatsThis()
*/

/*!
    \fn void StandardItem::setStatusTip(const QString &statusTip)

    Sets the item's status tip to the string specified by \a statusTip.

    \sa statusTip(), setToolTip(), setWhatsThis()
*/

/*!
    \fn QString StandardItem::toolTip() const

    Returns the item's tooltip.

    \sa setToolTip(), statusTip(), whatsThis()
*/

/*!
    \fn void StandardItem::setToolTip(const QString &toolTip)

    Sets the item's tooltip to the string specified by \a toolTip.

    \sa toolTip(), setStatusTip(), setWhatsThis()
*/

/*!
    \fn QString StandardItem::whatsThis() const

    Returns the item's "What's This?" help.

    \sa setWhatsThis(), toolTip(), statusTip()
*/

/*!
    \fn void StandardItem::setWhatsThis(const QString &whatsThis)

    Sets the item's "What's This?" help to the string specified by \a whatsThis.

    \sa whatsThis(), setStatusTip(), setToolTip()
*/

/*!
    \fn QFont StandardItem::font() const

    Returns the font used to render the item's text.

    \sa setFont()
*/

/*!
    \fn void StandardItem::setFont(const QFont &font)

    Sets the font used to display the item's text to the given \a font.

    \sa font(), setText(), setForeground()
*/

/*!
    \fn QBrush StandardItem::background() const

    Returns the brush used to render the item's background.

    \sa foreground(), setBackground()
*/

/*!
    \fn void StandardItem::setBackground(const QBrush &brush)

    Sets the item's background brush to the specified \a brush.

    \sa background(), setForeground()
*/

/*!
    \fn QBrush StandardItem::foreground() const

    Returns the brush used to render the item's foreground (e.g. text).

    \sa setForeground(), background()
*/

/*!
    \fn void StandardItem::setForeground(const QBrush &brush)

    Sets the brush used to display the item's foreground (e.g. text) to the
    given \a brush.

    \sa foreground(), setBackground(), setFont()
*/

/*!
    \fn int StandardItem::textAlignment() const

    Returns the text alignment for the item's text.
*/

/*!
    \fn void StandardItem::setTextAlignment(Qt::Alignment alignment)

    Sets the text alignment for the item's text to the \a alignment
    specified.

    \sa textAlignment()
*/

/*!
    \fn QSize StandardItem::sizeHint() const

    Returns the size hint set for the item, or an invalid QSize if no
    size hint has been set.

    If no size hint has been set, the item delegate will compute the
    size hint based on the item data.

    \sa setSizeHint()
*/

/*!
    \fn void StandardItem::setSizeHint(const QSize &size)

    Sets the size hint for the item to be \a size.
    If no size hint is set, the item delegate will compute the
    size hint based on the item data.

    \sa sizeHint()
*/

/*!
    \fn Qt::CheckState StandardItem::checkState() const

    Returns the checked state of the item.

    \sa setCheckState(), isCheckable()
*/

/*!
    \fn void StandardItem::setCheckState(Qt::CheckState state)

    Sets the check state of the item to be \a state.

    \sa checkState(), setCheckable()
*/

/*!
    \fn QString StandardItem::accessibleText() const

    Returns the item's accessible text.

    The accessible text is used by assistive technologies (i.e. for users who
    cannot use conventional means of interaction).

    \sa setAccessibleText(), accessibleDescription()
*/

/*!
    \fn void StandardItem::setAccessibleText(const QString &accessibleText)

    Sets the item's accessible text to the string specified by \a accessibleText.

    The accessible text is used by assistive technologies (i.e. for users who
    cannot use conventional means of interaction).

    \sa accessibleText(), setAccessibleDescription()
*/

/*!
    \fn QString StandardItem::accessibleDescription() const

    Returns the item's accessible description.

    The accessible description is used by assistive technologies (i.e. for
    users who cannot use conventional means of interaction).

    \sa setAccessibleDescription(), accessibleText()
*/

/*!
    \fn void StandardItem::setAccessibleDescription(const QString &accessibleDescription)

    Sets the item's accessible description to the string specified by \a
    accessibleDescription.

    The accessible description is used by assistive technologies (i.e. for
    users who cannot use conventional means of interaction).

    \sa accessibleDescription(), setAccessibleText()
*/

/*!
  Sets whether the item is enabled. If \a enabled is true, the item is enabled,
  meaning that the user can interact with the item; if \a enabled is false, the
  user cannot interact with the item.

  This flag takes precedence over the other item flags; e.g. if an item is not
  enabled, it cannot be selected by the user, even if the Qt::ItemIsSelectable
  flag has been set.

  \sa isEnabled(), Qt::ItemIsEnabled, setFlags()
*/
void StandardItem::setEnabled(bool enabled)
{
    Q_D(StandardItem);
    d->changeFlags(enabled, Qt::ItemIsEnabled);
}

/*!
  \fn bool StandardItem::isEnabled() const

  Returns whether the item is enabled.

  When an item is enabled, the user can interact with it. The possible
  types of interaction are specified by the other item flags, such as
  isEditable() and isSelectable().

  The default value is true.

  \sa setEnabled(), flags()
*/

/*!
  Sets whether the item is editable. If \a editable is true, the item can be
  edited by the user; otherwise, the user cannot edit the item.

  How the user can edit items in a view is determined by the view's edit
  triggers; see QAbstractItemView::editTriggers.

  \sa isEditable(), setFlags()
*/
void StandardItem::setEditable(bool editable)
{
    Q_D(StandardItem);
    d->changeFlags(editable, Qt::ItemIsEditable);
}

/*!
  \fn bool StandardItem::isEditable() const

  Returns whether the item can be edited by the user.

  When an item is editable (and enabled), the user can edit the item by
  invoking one of the view's edit triggers; see
  QAbstractItemView::editTriggers.

  The default value is true.

  \sa setEditable(), flags()
*/

/*!
  Sets whether the item is selectable. If \a selectable is true, the item
  can be selected by the user; otherwise, the user cannot select the item.

  You can control the selection behavior and mode by manipulating their
  view properties; see QAbstractItemView::selectionMode and
  QAbstractItemView::selectionBehavior.

  \sa isSelectable(), setFlags()
*/
void StandardItem::setSelectable(bool selectable)
{
    Q_D(StandardItem);
    d->changeFlags(selectable, Qt::ItemIsSelectable);
}

/*!
  \fn bool StandardItem::isSelectable() const

  Returns whether the item is selectable by the user.

  The default value is true.

  \sa setSelectable(), flags()
*/

/*!
  Sets whether the item is user-checkable. If \a checkable is true, the
  item can be checked by the user; otherwise, the user cannot check
  the item.

  The item delegate will render a checkable item with a check box next to the
  item's text.

  \sa isCheckable(), setCheckState(), setUserTristate(), setAutoTristate()
*/
void StandardItem::setCheckable(bool checkable)
{
    Q_D(StandardItem);
    if (checkable && !isCheckable()) {
        // make sure there's data for the checkstate role
        if (!data(Qt::CheckStateRole).isValid())
            setData(Qt::Unchecked, Qt::CheckStateRole);
    }
    d->changeFlags(checkable, Qt::ItemIsUserCheckable);
}

/*!
  \fn bool StandardItem::isCheckable() const

  Returns whether the item is user-checkable.

  The default value is false.

  \sa setCheckable(), checkState(), isUserTristate(), isAutoTristate()
*/

/*!
  \fn void StandardItem::setTristate(bool tristate)
  \obsolete

  Use StandardItem::setAutoTristate(bool tristate) instead.
  For a tristate checkbox that the user can change between all three
  states, use StandardItem::setUserTristate(bool tristate) instead.
*/

/*!
  \fn void StandardItem::isTristate() const
  \obsolete

  Use StandardItem::isAutoTristate() instead.
  For a tristate checkbox that the user can change between all three
  states, use StandardItem::isUserTristate() instead.
*/

/*!
  Determines that the item is tristate and controlled by QTreeWidget if \a tristate
  is \c true.
  This enables automatic management of the state of parent items in QTreeWidget
  (checked if all children are checked, unchecked if all children are unchecked,
  or partially checked if only some children are checked).

  \since 5.6
  \sa isAutoTristate(), setCheckable(), setCheckState()
*/
void StandardItem::setAutoTristate(bool tristate)
{
    Q_D(StandardItem);
    d->changeFlags(tristate, Qt::ItemIsAutoTristate);
}

/*!
  \fn bool StandardItem::isAutoTristate() const

  Returns whether the item is tristate and is controlled by QTreeWidget.

  The default value is false.

  \since 5.6
  \sa setAutoTristate(), isCheckable(), checkState()
*/

/*!
  Sets whether the item is tristate and controlled by the user.
  If \a tristate is true, the user can cycle through three separate states;
  otherwise, the item is checkable with two states.
  (Note that this also requires that the item is checkable; see isCheckable().)

  \since 5.6
  \sa isUserTristate(), setCheckable(), setCheckState()
*/
void StandardItem::setUserTristate(bool tristate)
{
    Q_D(StandardItem);
    d->changeFlags(tristate, Qt::ItemIsUserTristate);
}

/*!
  \fn bool StandardItem::isUserTristate() const
  \since 5.6

  Returns whether the item is tristate; that is, if it's checkable with three
  separate states and the user can cycle through all three states.

  The default value is false.

  \sa setUserTristate(), isCheckable(), checkState()
*/

#if QT_DEPRECATED_SINCE(5, 6)
void StandardItem::setTristate(bool tristate)
{
    setAutoTristate(tristate);
}
#endif

/*!
  Sets whether the item is drag enabled. If \a dragEnabled is true, the item
  can be dragged by the user; otherwise, the user cannot drag the item.

  Note that you also need to ensure that item dragging is enabled in the view;
  see QAbstractItemView::dragEnabled.

  \sa isDragEnabled(), setDropEnabled(), setFlags()
*/
void StandardItem::setDragEnabled(bool dragEnabled)
{
    Q_D(StandardItem);
    d->changeFlags(dragEnabled, Qt::ItemIsDragEnabled);
}

/*!
  \fn bool StandardItem::isDragEnabled() const

  Returns whether the item is drag enabled. An item that is drag enabled can
  be dragged by the user.

  The default value is true.

  Note that item dragging must be enabled in the view for dragging to work;
  see QAbstractItemView::dragEnabled.

  \sa setDragEnabled(), isDropEnabled(), flags()
*/

/*!
  Sets whether the item is drop enabled. If \a dropEnabled is true, the item
  can be used as a drop target; otherwise, it cannot.

  Note that you also need to ensure that drops are enabled in the view; see
  QWidget::acceptDrops(); and that the model supports the desired drop actions;
  see QAbstractItemModel::supportedDropActions().

  \sa isDropEnabled(), setDragEnabled(), setFlags()
*/
void StandardItem::setDropEnabled(bool dropEnabled)
{
    Q_D(StandardItem);
    d->changeFlags(dropEnabled, Qt::ItemIsDropEnabled);
}

/*!
  \fn bool StandardItem::isDropEnabled() const

  Returns whether the item is drop enabled. When an item is drop enabled, it
  can be used as a drop target.

  The default value is true.

  \sa setDropEnabled(), isDragEnabled(), flags()
*/


/*!
  Returns the row where the item is located in its parent's child table, or
  -1 if the item has no parent.

  \sa column(), parent()
*/
int StandardItem::row() const
{
    Q_D(const StandardItem);
    QPair<int, int> pos = d->position();
    return pos.first;
}

/*!
  Returns the column where the item is located in its parent's child table,
  or -1 if the item has no parent.

  \sa row(), parent()
*/
int StandardItem::column() const
{
    Q_D(const StandardItem);
    QPair<int, int> pos = d->position();
    return pos.second;
}

/*!
  Returns the QModelIndex associated with this item.

  When you need to invoke item functionality in a QModelIndex-based API (e.g.
  QAbstractItemView), you can call this function to obtain an index that
  corresponds to the item's location in the model.

  If the item is not associated with a model, an invalid QModelIndex is
  returned.

  \sa model(), StandardItemModelBase::itemFromIndex()
*/
QModelIndex StandardItem::index() const
{
    Q_D(const StandardItem);
    return d->model ? d->model->indexFromItem(this) : QModelIndex();
}

/*!
  Returns the StandardItemModelBase that this item belongs to.

  If the item is not a child of another item that belongs to the model, this
  function returns \nullptr.

  \sa index()
*/
StandardItemModelBase *StandardItem::model() const
{
    Q_D(const StandardItem);
    return d->model;
}

/*!
    Sets the number of child item rows to \a rows. If this is less than
    rowCount(), the data in the unwanted rows is discarded.

    \sa rowCount(), setColumnCount()
*/
void StandardItem::setRowCount(int rows)
{
    int rc = rowCount();
    if (rc == rows)
        return;
    if (rc < rows)
        insertRows(qMax(rc, 0), rows - rc);
    else
        removeRows(qMax(rows, 0), rc - rows);
}

/*!
    Returns the number of child item rows that the item has.

    \sa setRowCount(), columnCount()
*/
int StandardItem::rowCount() const
{
    Q_D(const StandardItem);
    return d->rowCount();
}

/*!
    Sets the number of child item columns to \a columns. If this is less than
    columnCount(), the data in the unwanted columns is discarded.

    \sa columnCount(), setRowCount()
*/
void StandardItem::setColumnCount(int columns)
{
    int cc = columnCount();
    if (cc == columns)
        return;
    if (cc < columns)
        insertColumns(qMax(cc, 0), columns - cc);
    else
        removeColumns(qMax(columns, 0), cc - columns);
}

/*!
    Returns the number of child item columns that the item has.

    \sa setColumnCount(), rowCount()
*/
int StandardItem::columnCount() const
{
    Q_D(const StandardItem);
    return d->columnCount();
}

/*!
    Inserts a row at \a row containing \a items. If necessary, the column
    count is increased to the size of \a items.

    \sa insertRows(), insertColumn()
*/
void StandardItem::insertRow(int row, const QList<StandardItem*> &items)
{
    Q_D(StandardItem);
    if (row < 0)
        return;
    if (columnCount() < items.count())
        setColumnCount(items.count());
    d->insertRows(row, 1, items);
}

/*!
    Inserts \a items at \a row. The column count won't be changed.

    \sa insertRow(), insertColumn()
*/
void StandardItem::insertRows(int row, const QList<StandardItem*> &items)
{
    Q_D(StandardItem);
    if (row < 0)
        return;
    d->insertRows(row, items);
}

/*!
    Inserts a column at \a column containing \a items. If necessary,
    the row count is increased to the size of \a items.

    \sa insertColumns(), insertRow()
*/
void StandardItem::insertColumn(int column, const QList<StandardItem*> &items)
{
    Q_D(StandardItem);
    if (column < 0)
        return;
    if (rowCount() < items.count())
        setRowCount(items.count());
    d->insertColumns(column, 1, items);
}

/*!
    Inserts \a count rows of child items at row \a row.

    \sa insertRow(), insertColumns()
*/
void StandardItem::insertRows(int row, int count)
{
    Q_D(StandardItem);
    if (rowCount() < row) {
        count += row - rowCount();
        row = rowCount();
    }
    d->insertRows(row, count, QList<StandardItem*>());
}

/*!
    Inserts \a count columns of child items at column \a column.

    \sa insertColumn(), insertRows()
*/
void StandardItem::insertColumns(int column, int count)
{
    Q_D(StandardItem);
    if (columnCount() < column) {
        count += column - columnCount();
        column = columnCount();
    }
    d->insertColumns(column, count, QList<StandardItem*>());
}

/*!
    \fn void StandardItem::appendRow(const QList<StandardItem*> &items)

    Appends a row containing \a items. If necessary, the column count is
    increased to the size of \a items.

    \sa insertRow()
*/

/*!
    \fn void StandardItem::appendRows(const QList<StandardItem*> &items)

    Appends rows containing \a items.  The column count will not change.

    \sa insertRow()
*/

/*!
    \fn void StandardItem::appendColumn(const QList<StandardItem*> &items)

    Appends a column containing \a items. If necessary, the row count is
    increased to the size of \a items.

    \sa insertColumn()
*/

/*!
    \fn bool StandardItemModelBase::insertRow(int row, const QModelIndex &parent)

    Inserts a single row before the given \a row in the child items of the
    \a parent specified. Returns \c true if the row is inserted; otherwise
    returns \c false.

    \sa insertRows(), insertColumn(), removeRow()
*/

/*!
    \fn bool StandardItemModelBase::insertColumn(int column, const QModelIndex &parent)

    Inserts a single column before the given \a column in the child items of
    the \a parent specified. Returns \c true if the column is inserted; otherwise
    returns \c false.

    \sa insertColumns(), insertRow(), removeColumn()
*/

/*!
    \fn StandardItem::insertRow(int row, StandardItem *item)
    \overload

    Inserts a row at \a row containing \a item.

    When building a list or a tree that has only one column, this function
    provides a convenient way to insert a single new item.
*/

/*!
    \fn StandardItem::appendRow(StandardItem *item)
    \overload

    Appends a row containing \a item.

    When building a list or a tree that has only one column, this function
    provides a convenient way to append a single new item.
*/

/*!
    Removes the given \a row. The items that were in the row are deleted.

    \sa takeRow(), removeRows(), removeColumn()
*/
void StandardItem::removeRow(int row)
{
    removeRows(row, 1);
}

/*!
    Removes the given \a column. The items that were in the
    column are deleted.

    \sa takeColumn(), removeColumns(), removeRow()
*/
void StandardItem::removeColumn(int column)
{
    removeColumns(column, 1);
}

/*!
    Removes \a count rows at row \a row. The items that were in those rows are
    deleted.

    \sa removeRow(), removeColumn()
*/
void StandardItem::removeRows(int row, int count)
{
    Q_D(StandardItem);
    if ((count < 1) || (row < 0) || ((row + count) > rowCount()))
        return;
    if (d->model)
        d->model->d_func()->rowsAboutToBeRemoved(this, row, row + count - 1);
    int i = d->childIndex(row, 0);
    int n = count * d->columnCount();
    for (int j = i; j < n+i; ++j) {
        StandardItem *oldItem = d->children.at(j);
        if (oldItem)
            oldItem->d_func()->setModel(0);
        delete oldItem;
    }
    d->children.remove(qMax(i, 0), n);
    d->rows -= count;
    if (d->model)
        d->model->d_func()->rowsRemoved(this, row, count);
}

/*!
    Removes \a count columns at column \a column. The items that were in those
    columns are deleted.

    \sa removeColumn(), removeRows()
*/
void StandardItem::removeColumns(int column, int count)
{
    Q_D(StandardItem);
    if ((count < 1) || (column < 0) || ((column + count) > columnCount()))
        return;
    if (d->model)
        d->model->d_func()->columnsAboutToBeRemoved(this, column, column + count - 1);
    for (int row = d->rowCount() - 1; row >= 0; --row) {
        int i = d->childIndex(row, column);
        for (int j=i; j<i+count; ++j) {
            StandardItem *oldItem = d->children.at(j);
            if (oldItem)
                oldItem->d_func()->setModel(0);
            delete oldItem;
        }
        d->children.remove(i, count);
    }
    d->columns -= count;
    if (d->model)
        d->model->d_func()->columnsRemoved(this, column, count);
}

/*!
    Returns \c true if this item has any children; otherwise returns \c false.

    \sa rowCount(), columnCount(), child()
*/
bool StandardItem::hasChildren() const
{
    return (rowCount() > 0) && (columnCount() > 0);
}

/*!
    Sets the child item at (\a row, \a column) to \a item. This item (the parent
    item) takes ownership of \a item. If necessary, the row count and column
    count are increased to fit the item.

    \note Passing a null pointer as \a item removes the item.

    \sa child()
*/
void StandardItem::setChild(int row, int column, StandardItem *item)
{
    Q_D(StandardItem);
    d->setChild(row, column, item, true);
}

/*!
    \fn StandardItem::setChild(int row, StandardItem *item)
    \overload

    Sets the child at \a row to \a item.
*/

/*!
    Returns the child item at (\a row, \a column) if one has been set; otherwise
    returns \nullptr.

    \sa setChild(), takeChild(), parent()
*/
StandardItem *StandardItem::child(int row, int column) const
{
    Q_D(const StandardItem);
    int index = d->childIndex(row, column);
    if (index == -1)
        return 0;
    return d->children.at(index);
}

/*!
    Removes the child item at (\a row, \a column) without deleting it, and returns
    a pointer to the item. If there was no child at the given location, then
    this function returns \nullptr.

    Note that this function, unlike takeRow() and takeColumn(), does not affect
    the dimensions of the child table.

    \sa child(), takeRow(), takeColumn()
*/
StandardItem *StandardItem::takeChild(int row, int column)
{
    Q_D(StandardItem);
    StandardItem *item = 0;
    int index = d->childIndex(row, column);
    if (index != -1) {
        item = d->children.at(index);
        if (item)
            item->d_func()->setParentAndModel(0, 0);
        d->children.replace(index, 0);
    }
    return item;
}

/*!
    Removes \a row without deleting the row items, and returns a list of
    pointers to the removed items. For items in the row that have not been
    set, the corresponding pointers in the list will be 0.

    \sa removeRow(), insertRow(), takeColumn()
*/
QList<StandardItem*> StandardItem::takeRow(int row)
{
    Q_D(StandardItem);
    QList<StandardItem*> items;
    if ((row < 0) || (row >= rowCount()))
        return items;
    if (d->model)
        d->model->d_func()->rowsAboutToBeRemoved(this, row, row);

    int index = d->childIndex(row, 0);  // Will return -1 if there are no columns
    if (index != -1) {
        int col_count = d->columnCount();
        items.reserve(col_count);
        for (int column = 0; column < col_count; ++column) {
            StandardItem *ch = d->children.at(index + column);
            if (ch)
                ch->d_func()->setParentAndModel(0, 0);
            items.append(ch);
        }
        d->children.remove(index, col_count);
    }
    d->rows--;
    if (d->model)
        d->model->d_func()->rowsRemoved(this, row, 1);
    return items;
}

/*!
    Removes \a column without deleting the column items, and returns a list of
    pointers to the removed items. For items in the column that have not been
    set, the corresponding pointers in the list will be 0.

    \sa removeColumn(), insertColumn(), takeRow()
*/
QList<StandardItem*> StandardItem::takeColumn(int column)
{
    Q_D(StandardItem);
    QList<StandardItem*> items;
    if ((column < 0) || (column >= columnCount()))
        return items;
    if (d->model)
        d->model->d_func()->columnsAboutToBeRemoved(this, column, column);

    const int rowCount = d->rowCount();
    items.reserve(rowCount);
    for (int row = rowCount - 1; row >= 0; --row) {
        int index = d->childIndex(row, column);
        StandardItem *ch = d->children.at(index);
        if (ch)
            ch->d_func()->setParentAndModel(0, 0);
        d->children.remove(index);
        items.prepend(ch);
    }
    d->columns--;
    if (d->model)
        d->model->d_func()->columnsRemoved(this, column, 1);
    return items;
}

/*!
    Returns \c true if this item is less than \a other; otherwise returns \c false.

    The default implementation uses the data for the item's sort role (see
    StandardItemModelBase::sortRole) to perform the comparison if the item
    belongs to a model; otherwise, the data for the item's Qt::DisplayRole
    (text()) is used to perform the comparison.

    sortChildren() and StandardItemModelBase::sort() use this function when
    sorting items. If you want custom sorting, you can subclass StandardItem
    and reimplement this function.
*/
bool StandardItem::operator<(const StandardItem &other) const
{
    const int role = model() ? model()->sortRole() : Qt::DisplayRole;
    const Qt::CaseSensitivity cs = model() ? model()->sortCaseSensitivity() : Qt::CaseInsensitive;
    const QVariant l = data(role), r = other.data(role);
    return QAbstractItemModelPrivate::isVariantLessThan(l, r, cs);
}

/*!
    Sorts the children of the item using the given \a order, by the values in
    the given \a column.

    \note This function is recursive, therefore it sorts the children of the
    item, its grandchildren, etc.

    \sa {operator<()}
*/
void StandardItem::sortChildren(int column, Qt::SortOrder order)
{
    Q_D(StandardItem);
    if ((column < 0) || (rowCount() == 0))
        return;

    QList<QPersistentModelIndex> parents;
    if (d->model) {
        parents << index();
        emit d->model->layoutAboutToBeChanged(parents, QAbstractItemModel::VerticalSortHint);
    }
    d->sortChildren(column, order);
    if (d->model)
        emit d->model->layoutChanged(parents, QAbstractItemModel::VerticalSortHint);
}

/*!
    Returns a copy of this item. The item's children are not copied.

    When subclassing StandardItem, you can reimplement this function
    to provide StandardItemModelBase with a factory that it can use to
    create new items on demand.

    \sa StandardItemModelBase::setItemPrototype(), operator=()
*/
StandardItem *StandardItem::clone() const
{
    return new StandardItem(*this);
}

/*!
    Returns the type of this item. The type is used to distinguish custom
    items from the base class. When subclassing StandardItem, you should
    reimplement this function and return a new value greater than or equal
    to \l UserType.

    \sa StandardItem::Type
*/
int StandardItem::type() const
{
    return Type;
}

/*!
    Reads the item from stream \a in. Only the data and flags of the item are
    read, not the child items.

    \sa write()
*/
void StandardItem::read(QDataStream &in)
{
    Q_D(StandardItem);
    in >> d->values;
    qint32 flags;
    in >> flags;
    setFlags(Qt::ItemFlags(flags));
}

/*!
    Writes the item to stream \a out. Only the data and flags of the item
    are written, not the child items.

    \sa read()
*/
void StandardItem::write(QDataStream &out) const
{
    Q_D(const StandardItem);
    out << d->values;
    out << flags();
}

/*!
    \relates StandardItem
    \since 4.2

    Reads a StandardItem from stream \a in into \a item.

    This operator uses StandardItem::read().

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator>>(QDataStream &in, StandardItem &item)
{
    item.read(in);
    return in;
}

/*!
    \relates StandardItem
    \since 4.2

    Writes the StandardItem \a item to stream \a out.

    This operator uses StandardItem::write().

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator<<(QDataStream &out, const StandardItem &item)
{
    item.write(out);
    return out;
}

/*!
    \class StandardItemModelBase
    \brief The StandardItemModelBase class provides a generic model for storing custom data.
    \ingroup model-view
    \inmodule QtGui

    StandardItemModelBase can be used as a repository for standard Qt
    data types. It is one of the \l {Model/View Classes} and is part
    of Qt's \l {Model/View Programming}{model/view} framework.

    StandardItemModelBase provides a classic item-based approach to working with
    the model.  The items in a StandardItemModelBase are provided by
    StandardItem.

    StandardItemModelBase implements the QAbstractItemModel interface, which
    means that the model can be used to provide data in any view that supports
    that interface (such as QListView, QTableView and QTreeView, and your own
    custom views). For performance and flexibility, you may want to subclass
    QAbstractItemModel to provide support for different kinds of data
    repositories. For example, the QDirModel provides a model interface to the
    underlying file system.

    When you want a list or tree, you typically create an empty
    StandardItemModelBase and use appendRow() to add items to the model, and
    item() to access an item.  If your model represents a table, you typically
    pass the dimensions of the table to the StandardItemModelBase constructor and
    use setItem() to position items into the table. You can also use setRowCount()
    and setColumnCount() to alter the dimensions of the model. To insert items,
    use insertRow() or insertColumn(), and to remove items, use removeRow() or
    removeColumn().

    You can set the header labels of your model with setHorizontalHeaderLabels()
    and setVerticalHeaderLabels().

    You can search for items in the model with findItems(), and sort the model by
    calling sort().

    Call clear() to remove all items from the model.

    An example usage of StandardItemModelBase to create a table:

    \snippet code/src_gui_itemviews_StandardItemmodel.cpp 0

    An example usage of StandardItemModelBase to create a tree:

    \snippet code/src_gui_itemviews_StandardItemmodel.cpp 1

    After setting the model on a view, you typically want to react to user
    actions, such as an item being clicked. Since a QAbstractItemView provides
    QModelIndex-based signals and functions, you need a way to obtain the
    StandardItem that corresponds to a given QModelIndex, and vice
    versa. itemFromIndex() and indexFromItem() provide this mapping. Typical
    usage of itemFromIndex() includes obtaining the item at the current index
    in a view, and obtaining the item that corresponds to an index carried by
    a QAbstractItemView signal, such as QAbstractItemView::clicked(). First
    you connect the view's signal to a slot in your class:

    \snippet code/src_gui_itemviews_StandardItemmodel.cpp 2

    When you receive the signal, you call itemFromIndex() on the given model
    index to get a pointer to the item:

    \snippet code/src_gui_itemviews_StandardItemmodel.cpp 3

    Conversely, you must obtain the QModelIndex of an item when you want to
    invoke a model/view function that takes an index as argument. You can
    obtain the index either by using the model's indexFromItem() function, or,
    equivalently, by calling StandardItem::index():

    \snippet code/src_gui_itemviews_StandardItemmodel.cpp 4

    You are, of course, not required to use the item-based approach; you could
    instead rely entirely on the QAbstractItemModel interface when working with
    the model, or use a combination of the two as appropriate.

    \sa StandardItem, {Model/View Programming}, QAbstractItemModel,
    {itemviews/simpletreemodel}{Simple Tree Model example},
    {Item View Convenience Classes}
*/

/*!
    \fn void StandardItemModelBase::itemChanged(StandardItem *item)
    \since 4.2

    This signal is emitted whenever the data of \a item has changed.
*/

/*!
    Constructs a new item model with the given \a parent.
*/
StandardItemModelBase::StandardItemModelBase(QObject *parent)
    : QAbstractItemModel(*new StandardItemModelBasePrivate, parent)
{
    Q_D(StandardItemModelBase);
    d->init();
    d->root->d_func()->setModel(this);
}

/*!
    Constructs a new item model that initially has \a rows rows and \a columns
    columns, and that has the given \a parent.
*/
StandardItemModelBase::StandardItemModelBase(int rows, int columns, QObject *parent)
    : QAbstractItemModel(*new StandardItemModelBasePrivate, parent)
{
    Q_D(StandardItemModelBase);
    d->init();
    d->root->insertColumns(0, columns);
    d->columnHeaderItems.insert(0, columns, 0);
    d->root->insertRows(0, rows);
    d->rowHeaderItems.insert(0, rows, 0);
    d->root->d_func()->setModel(this);
}

/*!
  \internal
*/
StandardItemModelBase::StandardItemModelBase(StandardItemModelBasePrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(StandardItemModelBase);
    d->init();
}

/*!
    Destructs the model. The model destroys all its items.
*/
StandardItemModelBase::~StandardItemModelBase()
{
    Q_D(StandardItemModelBase);
    delete d->itemPrototype;
    qDeleteAll(d->columnHeaderItems);
    qDeleteAll(d->rowHeaderItems);
    d->root.reset();
}

/*!
    Sets the item role names to \a roleNames.
*/
void StandardItemModelBase::setItemRoleNames(const QHash<int,QByteArray> &roleNames)
{
    Q_D(StandardItemModelBase);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    d->roleNames = roleNames;
#endif
}

/*!
    Removes all items (including header items) from the model and sets the
    number of rows and columns to zero.

    \sa removeColumns(), removeRows()
*/
void StandardItemModelBase::clear()
{
    Q_D(StandardItemModelBase);
    beginResetModel();
    d->root.reset(new StandardItem);
    d->root->setFlags(Qt::ItemIsDropEnabled);
    d->root->d_func()->setModel(this);
    qDeleteAll(d->columnHeaderItems);
    d->columnHeaderItems.clear();
    qDeleteAll(d->rowHeaderItems);
    d->rowHeaderItems.clear();
    endResetModel();
}

/*!
    \since 4.2

    Returns a pointer to the StandardItem associated with the given \a index.

    Calling this function is typically the initial step when processing
    QModelIndex-based signals from a view, such as
    QAbstractItemView::activated(). In your slot, you call itemFromIndex(),
    with the QModelIndex carried by the signal as argument, to obtain a
    pointer to the corresponding StandardItem.

    Note that this function will lazily create an item for the index (using
    itemPrototype()), and set it in the parent item's child table, if no item
    already exists at that index.

    If \a index is an invalid index, this function returns \nullptr.

    \sa indexFromItem()
*/
StandardItem *StandardItemModelBase::itemFromIndex(const QModelIndex &index) const
{
    Q_D(const StandardItemModelBase);
    if ((index.row() < 0) || (index.column() < 0) || (index.model() != this))
        return 0;
    StandardItem *parent = static_cast<StandardItem*>(index.internalPointer());
    if (parent == 0)
        return 0;
    StandardItem *item = parent->child(index.row(), index.column());
    // lazy part
    if (item == 0) {
        item = d->createItem();
        parent->d_func()->setChild(index.row(), index.column(), item);
    }
    return item;
}

/*!
    \since 4.2

    Returns the QModelIndex associated with the given \a item.

    Use this function when you want to perform an operation that requires the
    QModelIndex of the item, such as
    QAbstractItemView::scrollTo(). StandardItem::index() is provided as
    convenience; it is equivalent to calling this function.

    \sa itemFromIndex(), StandardItem::index()
*/
QModelIndex StandardItemModelBase::indexFromItem(const StandardItem *item) const
{
    if (item && item->d_func()->parent) {
        QPair<int, int> pos = item->d_func()->position();
        return createIndex(pos.first, pos.second, item->d_func()->parent);
    }
    return QModelIndex();
}

/*!
    \since 4.2

    Sets the number of rows in this model to \a rows. If
    this is less than rowCount(), the data in the unwanted rows
    is discarded.

    \sa setColumnCount()
*/
void StandardItemModelBase::setRowCount(int rows)
{
    Q_D(StandardItemModelBase);
    d->root->setRowCount(rows);
}

/*!
    \since 4.2

    Sets the number of columns in this model to \a columns. If
    this is less than columnCount(), the data in the unwanted columns
    is discarded.

    \sa setRowCount()
*/
void StandardItemModelBase::setColumnCount(int columns)
{
    Q_D(StandardItemModelBase);
    d->root->setColumnCount(columns);
}

/*!
    \since 4.2

    Sets the item for the given \a row and \a column to \a item. The model
    takes ownership of the item. If necessary, the row count and column count
    are increased to fit the item. The previous item at the given location (if
    there was one) is deleted.

    \sa item()
*/
void StandardItemModelBase::setItem(int row, int column, StandardItem *item)
{
    Q_D(StandardItemModelBase);
    d->root->d_func()->setChild(row, column, item, true);
}

/*!
  \fn StandardItemModelBase::setItem(int row, StandardItem *item)
  \overload
*/

/*!
    \since 4.2

    Returns the item for the given \a row and \a column if one has been set;
    otherwise returns \nullptr.

    \sa setItem(), takeItem(), itemFromIndex()
*/
StandardItem *StandardItemModelBase::item(int row, int column) const
{
    Q_D(const StandardItemModelBase);
    return d->root->child(row, column);
}

/*!
    \since 4.2

    Returns the model's invisible root item.

    The invisible root item provides access to the model's top-level items
    through the StandardItem API, making it possible to write functions that
    can treat top-level items and their children in a uniform way; for
    example, recursive functions involving a tree model.

    \note Calling \l{QAbstractItemModel::index()}{index()} on the StandardItem object
    retrieved from this function is not valid.
*/
StandardItem *StandardItemModelBase::invisibleRootItem() const
{
    Q_D(const StandardItemModelBase);
    return d->root.data();
}

/*!
    \since 4.2

    Sets the horizontal header item for \a column to \a item.  The model takes
    ownership of the item. If necessary, the column count is increased to fit
    the item. The previous header item (if there was one) is deleted.

    \sa horizontalHeaderItem(), setHorizontalHeaderLabels(),
    setVerticalHeaderItem()
*/
void StandardItemModelBase::setHorizontalHeaderItem(int column, StandardItem *item)
{
    Q_D(StandardItemModelBase);
    if (column < 0)
        return;
    if (columnCount() <= column)
        setColumnCount(column + 1);

    StandardItem *oldItem = d->columnHeaderItems.at(column);
    if (item == oldItem)
        return;

    if (item) {
        if (item->model() == 0) {
            item->d_func()->setModel(this);
        } else {
            qWarning("StandardItem::setHorizontalHeaderItem: Ignoring duplicate insertion of item %p",
                     item);
            return;
        }
    }

    if (oldItem)
        oldItem->d_func()->setModel(0);
    delete oldItem;

    d->columnHeaderItems.replace(column, item);
    emit headerDataChanged(Qt::Horizontal, column, column);
}

/*!
    \since 4.2

    Returns the horizontal header item for \a column if one has been set;
    otherwise returns \nullptr.

    \sa setHorizontalHeaderItem(), verticalHeaderItem()
*/
StandardItem *StandardItemModelBase::horizontalHeaderItem(int column) const
{
    Q_D(const StandardItemModelBase);
    if ((column < 0) || (column >= columnCount()))
        return 0;
    return d->columnHeaderItems.at(column);
}

/*!
    \since 4.2

    Sets the vertical header item for \a row to \a item.  The model takes
    ownership of the item. If necessary, the row count is increased to fit the
    item. The previous header item (if there was one) is deleted.

    \sa verticalHeaderItem(), setVerticalHeaderLabels(),
    setHorizontalHeaderItem()
*/
void StandardItemModelBase::setVerticalHeaderItem(int row, StandardItem *item)
{
    Q_D(StandardItemModelBase);
    if (row < 0)
        return;
    if (rowCount() <= row)
        setRowCount(row + 1);

    StandardItem *oldItem = d->rowHeaderItems.at(row);
    if (item == oldItem)
        return;

    if (item) {
        if (item->model() == 0) {
            item->d_func()->setModel(this);
        } else {
            qWarning("StandardItem::setVerticalHeaderItem: Ignoring duplicate insertion of item %p",
                     item);
            return;
        }
    }

    if (oldItem)
        oldItem->d_func()->setModel(0);
    delete oldItem;

    d->rowHeaderItems.replace(row, item);
    emit headerDataChanged(Qt::Vertical, row, row);
}

/*!
    \since 4.2

    Returns the vertical header item for row \a row if one has been set;
    otherwise returns \nullptr.

    \sa setVerticalHeaderItem(), horizontalHeaderItem()
*/
StandardItem *StandardItemModelBase::verticalHeaderItem(int row) const
{
    Q_D(const StandardItemModelBase);
    if ((row < 0) || (row >= rowCount()))
        return 0;
    return d->rowHeaderItems.at(row);
}

/*!
    \since 4.2

    Sets the horizontal header labels using \a labels. If necessary, the
    column count is increased to the size of \a labels.

    \sa setHorizontalHeaderItem()
*/
void StandardItemModelBase::setHorizontalHeaderLabels(const QStringList &labels)
{
    Q_D(StandardItemModelBase);
    if (columnCount() < labels.count())
        setColumnCount(labels.count());
    for (int i = 0; i < labels.count(); ++i) {
        StandardItem *item = horizontalHeaderItem(i);
        if (!item) {
            item = d->createItem();
            setHorizontalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

/*!
    \since 4.2

    Sets the vertical header labels using \a labels. If necessary, the row
    count is increased to the size of \a labels.

    \sa setVerticalHeaderItem()
*/
void StandardItemModelBase::setVerticalHeaderLabels(const QStringList &labels)
{
    Q_D(StandardItemModelBase);
    if (rowCount() < labels.count())
        setRowCount(labels.count());
    for (int i = 0; i < labels.count(); ++i) {
        StandardItem *item = verticalHeaderItem(i);
        if (!item) {
            item = d->createItem();
            setVerticalHeaderItem(i, item);
        }
        item->setText(labels.at(i));
    }
}

/*!
    \since 4.2

    Sets the item prototype for the model to the specified \a item. The model
    takes ownership of the prototype.

    The item prototype acts as a StandardItem factory, by relying on the
    StandardItem::clone() function.  To provide your own prototype, subclass
    StandardItem, reimplement StandardItem::clone() and set the prototype to
    be an instance of your custom class. Whenever StandardItemModelBase needs to
    create an item on demand (for instance, when a view or item delegate calls
    setData())), the new items will be instances of your custom class.

    \sa itemPrototype(), StandardItem::clone()
*/
void StandardItemModelBase::setItemPrototype(const StandardItem *item)
{
    Q_D(StandardItemModelBase);
    if (d->itemPrototype != item) {
        delete d->itemPrototype;
        d->itemPrototype = item;
    }
}

/*!
    \since 4.2

    Returns the item prototype used by the model. The model uses the item
    prototype as an item factory when it needs to construct new items on
    demand (for instance, when a view or item delegate calls setData()).

    \sa setItemPrototype()
*/
const StandardItem *StandardItemModelBase::itemPrototype() const
{
    Q_D(const StandardItemModelBase);
    return d->itemPrototype;
}

/*!
    \since 4.2

    Returns a list of items that match the given \a text, using the given \a
    flags, in the given \a column.
*/
QList<StandardItem*> StandardItemModelBase::findItems(const QString &text,
                                                    Qt::MatchFlags flags, int column) const
{
    QModelIndexList indexes = match(index(0, column, QModelIndex()),
                                    Qt::DisplayRole, text, -1, flags);
    QList<StandardItem*> items;
    const int numIndexes = indexes.size();
    items.reserve(numIndexes);
    for (int i = 0; i < numIndexes; ++i)
        items.append(itemFromIndex(indexes.at(i)));
    return items;
}

/*!
    \since 4.2

    Appends a row containing \a items. If necessary, the column count is
    increased to the size of \a items.

    \sa insertRow(), appendColumn()
*/
void StandardItemModelBase::appendRow(const QList<StandardItem*> &items)
{
    invisibleRootItem()->appendRow(items);
}

/*!
    \since 4.2

    Appends a column containing \a items. If necessary, the row count is
    increased to the size of \a items.

    \sa insertColumn(), appendRow()
*/
void StandardItemModelBase::appendColumn(const QList<StandardItem*> &items)
{
    invisibleRootItem()->appendColumn(items);
}

/*!
    \since 4.2
    \fn StandardItemModelBase::appendRow(StandardItem *item)
    \overload

    When building a list or a tree that has only one column, this function
    provides a convenient way to append a single new \a item.
*/

/*!
    \since 4.2

    Inserts a row at \a row containing \a items. If necessary, the column
    count is increased to the size of \a items.

    \sa takeRow(), appendRow(), insertColumn()
*/
void StandardItemModelBase::insertRow(int row, const QList<StandardItem*> &items)
{
    invisibleRootItem()->insertRow(row, items);
}

/*!
    \since 4.2

    \fn void StandardItemModelBase::insertRow(int row, StandardItem *item)
    \overload

    Inserts a row at \a row containing \a item.

    When building a list or a tree that has only one column, this function
    provides a convenient way to append a single new item.
*/

/*!
    \since 4.2

    Inserts a column at \a column containing \a items. If necessary, the row
    count is increased to the size of \a items.

    \sa takeColumn(), appendColumn(), insertRow()
*/
void StandardItemModelBase::insertColumn(int column, const QList<StandardItem*> &items)
{
    invisibleRootItem()->insertColumn(column, items);
}

/*!
    \since 4.2

    Removes the item at (\a row, \a column) without deleting it. The model
    releases ownership of the item.

    \sa item(), takeRow(), takeColumn()
*/
StandardItem *StandardItemModelBase::takeItem(int row, int column)
{
    Q_D(StandardItemModelBase);
    return d->root->takeChild(row, column);
}

/*!
    \since 4.2

    Removes the given \a row without deleting the row items, and returns a
    list of pointers to the removed items. The model releases ownership of the
    items. For items in the row that have not been set, the corresponding
    pointers in the list will be 0.

    \sa takeColumn()
*/
QList<StandardItem*> StandardItemModelBase::takeRow(int row)
{
    Q_D(StandardItemModelBase);
    return d->root->takeRow(row);
}

/*!
    \since 4.2

    Removes the given \a column without deleting the column items, and returns
    a list of pointers to the removed items. The model releases ownership of
    the items. For items in the column that have not been set, the
    corresponding pointers in the list will be 0.

    \sa takeRow()
*/
QList<StandardItem*> StandardItemModelBase::takeColumn(int column)
{
    Q_D(StandardItemModelBase);
    return d->root->takeColumn(column);
}

/*!
    \since 4.2

    Removes the horizontal header item at \a column from the header without
    deleting it, and returns a pointer to the item. The model releases
    ownership of the item.

    \sa horizontalHeaderItem(), takeVerticalHeaderItem()
*/
StandardItem *StandardItemModelBase::takeHorizontalHeaderItem(int column)
{
    Q_D(StandardItemModelBase);
    if ((column < 0) || (column >= columnCount()))
        return 0;
    StandardItem *headerItem = d->columnHeaderItems.at(column);
    if (headerItem) {
        headerItem->d_func()->setParentAndModel(0, 0);
        d->columnHeaderItems.replace(column, 0);
    }
    return headerItem;
}

/*!
    \since 4.2

    Removes the vertical header item at \a row from the header without
    deleting it, and returns a pointer to the item. The model releases
    ownership of the item.

    \sa verticalHeaderItem(), takeHorizontalHeaderItem()
*/
StandardItem *StandardItemModelBase::takeVerticalHeaderItem(int row)
{
    Q_D(StandardItemModelBase);
    if ((row < 0) || (row >= rowCount()))
        return 0;
    StandardItem *headerItem = d->rowHeaderItems.at(row);
    if (headerItem) {
        headerItem->d_func()->setParentAndModel(0, 0);
        d->rowHeaderItems.replace(row, 0);
    }
    return headerItem;
}

/*!
    \since 4.2
    \property StandardItemModelBase::sortRole
    \brief the item role that is used to query the model's data when sorting items

    The default value is Qt::DisplayRole.

    \sa sort(), StandardItem::sortChildren()
*/
int StandardItemModelBase::sortRole() const
{
    Q_D(const StandardItemModelBase);
    return d->sortRole;
}

void StandardItemModelBase::setSortRole(int role)
{
    Q_D(StandardItemModelBase);
    d->sortRole = role;
}

Qt::CaseSensitivity StandardItemModelBase::sortCaseSensitivity() const
{
  Q_D(const StandardItemModelBase);
  return d->sortCaseSensitivity;
}

void StandardItemModelBase::setSortCaseSensitivity(Qt::CaseSensitivity cs)
{
  Q_D(StandardItemModelBase);
  d->sortCaseSensitivity = cs;
}

/*!
  \reimp
*/
int StandardItemModelBase::columnCount(const QModelIndex &parent) const
{
    Q_D(const StandardItemModelBase);
    StandardItem *item = d->itemFromIndex(parent);
    return item ? item->columnCount() : 0;
}

/*!
  \reimp
*/
QVariant StandardItemModelBase::data(const QModelIndex &index, int role) const
{
    Q_D(const StandardItemModelBase);
    StandardItem *item = d->itemFromIndex(index);
    return item ? item->data(role) : QVariant();
}

/*!
  \reimp
*/
Qt::ItemFlags StandardItemModelBase::flags(const QModelIndex &index) const
{
    Q_D(const StandardItemModelBase);
    if (!d->indexValid(index))
        return d->root->flags();
    StandardItem *item = d->itemFromIndex(index);
    if (item)
        return item->flags();
    return Qt::ItemIsSelectable
        |Qt::ItemIsEnabled
        |Qt::ItemIsEditable
        |Qt::ItemIsDragEnabled
        |Qt::ItemIsDropEnabled;
}

/*!
  \reimp
*/
bool StandardItemModelBase::hasChildren(const QModelIndex &parent) const
{
    Q_D(const StandardItemModelBase);
    StandardItem *item = d->itemFromIndex(parent);
    return item ? item->hasChildren() : false;
}

/*!
  \reimp
*/
QModelIndex StandardItemModelBase::sibling(int row, int column, const QModelIndex &idx) const
{
    return QAbstractItemModel::sibling(row, column, idx);
}

/*!
  \reimp
*/
QVariant StandardItemModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const StandardItemModelBase);
    if ((section < 0)
        || ((orientation == Qt::Horizontal) && (section >= columnCount()))
        || ((orientation == Qt::Vertical) && (section >= rowCount()))) {
        return QVariant();
    }
    StandardItem *headerItem = 0;
    if (orientation == Qt::Horizontal)
        headerItem = d->columnHeaderItems.at(section);
    else if (orientation == Qt::Vertical)
        headerItem = d->rowHeaderItems.at(section);
    return headerItem ? headerItem->data(role)
        : QAbstractItemModel::headerData(section, orientation, role);
}

/*!
    \reimp

    StandardItemModelBase supports both copy and move.
*/
Qt::DropActions StandardItemModelBase::supportedDropActions () const
{
    return Qt::CopyAction | Qt::MoveAction;
}

/*!
  \reimp
*/
QModelIndex StandardItemModelBase::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const StandardItemModelBase);
    StandardItem *parentItem = d->itemFromIndex(parent);
    if ((parentItem == 0)
        || (row < 0)
        || (column < 0)
        || (row >= parentItem->rowCount())
        || (column >= parentItem->columnCount())) {
        return QModelIndex();
    }
    return createIndex(row, column, parentItem);
}

/*!
  \reimp
*/
bool StandardItemModelBase::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(StandardItemModelBase);
    StandardItem *item = parent.isValid() ? itemFromIndex(parent) : d->root.data();
    if (item == 0)
        return false;
    return item->d_func()->insertColumns(column, count, QList<StandardItem*>());
}

/*!
  \reimp
*/
bool StandardItemModelBase::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_D(StandardItemModelBase);
    StandardItem *item = parent.isValid() ? itemFromIndex(parent) : d->root.data();
    if (item == 0)
        return false;
    return item->d_func()->insertRows(row, count, QList<StandardItem*>());
}

/*!
  \reimp
*/
QMap<int, QVariant> StandardItemModelBase::itemData(const QModelIndex &index) const
{
    Q_D(const StandardItemModelBase);
    const StandardItem *const item = d->itemFromIndex(index);
    if (!item || item == d->root.data())
        return QMap<int, QVariant>();
    return item->d_func()->itemData();
}

/*!
  \reimp
*/
QModelIndex StandardItemModelBase::parent(const QModelIndex &child) const
{
    Q_D(const StandardItemModelBase);
    if (!d->indexValid(child))
        return QModelIndex();
    StandardItem *parentItem = static_cast<StandardItem*>(child.internalPointer());
    return indexFromItem(parentItem);
}

/*!
  \reimp
*/
bool StandardItemModelBase::removeColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(StandardItemModelBase);
    StandardItem *item = d->itemFromIndex(parent);
    if ((item == 0) || (count < 1) || (column < 0) || ((column + count) > item->columnCount()))
        return false;
    item->removeColumns(column, count);
    return true;
}

/*!
  \reimp
*/
bool StandardItemModelBase::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_D(StandardItemModelBase);
    StandardItem *item = d->itemFromIndex(parent);
    if ((item == 0) || (count < 1) || (row < 0) || ((row + count) > item->rowCount()))
        return false;
    item->removeRows(row, count);
    return true;
}

/*!
  \reimp
*/
int StandardItemModelBase::rowCount(const QModelIndex &parent) const
{
    Q_D(const StandardItemModelBase);
    StandardItem *item = d->itemFromIndex(parent);
    return item ? item->rowCount() : 0;
}

/*!
  \reimp
*/
bool StandardItemModelBase::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    StandardItem *item = itemFromIndex(index);
    if (item == 0)
        return false;
    item->setData(value, role);
    return true;
}

/*!
  \since 5.12
  Removes the data stored in all the roles for the given \a index.
  Returns \c true if \a index is valid and data was cleared, \c false
  otherwise.

  \sa setData(), data()
*/
bool StandardItemModelBase::clearItemData(const QModelIndex &index)
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid))
        return false;
    Q_D(StandardItemModelBase);
    StandardItem *item = d->itemFromIndex(index);
    if (!item)
        return false;
    item->clearData();
    return true;
}

/*!
  \reimp
*/
bool StandardItemModelBase::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    Q_D(StandardItemModelBase);
    if ((section < 0)
        || ((orientation == Qt::Horizontal) && (section >= columnCount()))
        || ((orientation == Qt::Vertical) && (section >= rowCount()))) {
        return false;
    }
    StandardItem *headerItem = 0;
    if (orientation == Qt::Horizontal) {
        headerItem = d->columnHeaderItems.at(section);
        if (headerItem == 0) {
            headerItem = d->createItem();
            headerItem->d_func()->setModel(this);
            d->columnHeaderItems.replace(section, headerItem);
        }
    } else if (orientation == Qt::Vertical) {
        headerItem = d->rowHeaderItems.at(section);
        if (headerItem == 0) {
            headerItem = d->createItem();
            headerItem->d_func()->setModel(this);
            d->rowHeaderItems.replace(section, headerItem);
        }
    }
    if (headerItem) {
        headerItem->setData(value, role);
        return true;
    }
    return false;
}

/*!
  \reimp
*/
bool StandardItemModelBase::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    StandardItem *item = itemFromIndex(index);
    if (item == 0)
        return false;
    item->d_func()->setItemData(roles);
    return true;
}

/*!
  \reimp
*/
void StandardItemModelBase::sort(int column, Qt::SortOrder order)
{
    Q_D(StandardItemModelBase);
    d->root->sortChildren(column, order);
}

/*!
  \reimp
*/
QStringList StandardItemModelBase::mimeTypes() const
{
    return QAbstractItemModel::mimeTypes() << StandardItemModelBaseDataListMimeType();
}

/*!
  \reimp
*/
QMimeData *StandardItemModelBase::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = QAbstractItemModel::mimeData(indexes);
    if(!data)
        return 0;

    const QString format = StandardItemModelBaseDataListMimeType();
    if (!mimeTypes().contains(format))
        return data;
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);

    QSet<StandardItem*> itemsSet;
    QStack<StandardItem*> stack;
    itemsSet.reserve(indexes.count());
    stack.reserve(indexes.count());
    for (int i = 0; i < indexes.count(); ++i) {
        if (StandardItem *item = itemFromIndex(indexes.at(i))) {
            itemsSet << item;
            stack.push(item);
        } else {
            qWarning("StandardItemModelBase::mimeData: No item associated with invalid index");
            return 0;
        }
    }

    //remove duplicates childrens
    {
        QSet<StandardItem *> seen;
        while (!stack.isEmpty()) {
            StandardItem *itm = stack.pop();
            if (seen.contains(itm))
                continue;
            seen.insert(itm);

            const QVector<StandardItem*> &childList = itm->d_func()->children;
            for (int i = 0; i < childList.count(); ++i) {
                StandardItem *chi = childList.at(i);
                if (chi) {
                    itemsSet.erase(itemsSet.constFind(chi));
                    stack.push(chi);
                }
            }
        }
    }

    stack.reserve(itemsSet.count());
    for (StandardItem *item : qAsConst(itemsSet))
        stack.push(item);

    //stream everything recursively
    while (!stack.isEmpty()) {
        StandardItem *item = stack.pop();
        if (itemsSet.contains(item)) //if the item is selection 'top-level', stream its position
            stream << item->row() << item->column();

        stream << *item << item->columnCount() << item->d_ptr->children.count();
        stack += item->d_ptr->children;
    }

    data->setData(format, encoded);
    return data;
}


/* \internal
    Used by StandardItemModelBase::dropMimeData
    stream out an item and his children
 */
void StandardItemModelBasePrivate::decodeDataRecursive(QDataStream &stream, StandardItem *item)
{
    int colCount, childCount;
    stream >> *item;
    stream >> colCount >> childCount;
    item->setColumnCount(colCount);

    int childPos = childCount;

    while(childPos > 0) {
        childPos--;
        StandardItem *child = createItem();
        decodeDataRecursive(stream, child);
        item->setChild( childPos / colCount, childPos % colCount, child);
    }
}


/*!
  \reimp
*/
bool StandardItemModelBase::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent)
{
    Q_D(StandardItemModelBase);
    // check if the action is supported
    if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction))
        return false;
    // check if the format is supported
    const QString format = StandardItemModelBaseDataListMimeType();
    if (!data->hasFormat(format))
        return QAbstractItemModel::dropMimeData(data, action, row, column, parent);

    if (row > rowCount(parent))
        row = rowCount(parent);
    if (row == -1)
        row = rowCount(parent);
    if (column == -1)
        column = 0;

    // decode and insert
    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);


    //code based on QAbstractItemModel::decodeData
    // adapted to work with StandardItem
    int top = INT_MAX;
    int left = INT_MAX;
    int bottom = 0;
    int right = 0;
    QVector<int> rows, columns;
    QVector<StandardItem *> items;

    while (!stream.atEnd()) {
        int r, c;
        StandardItem *item = d->createItem();
        stream >> r >> c;
        d->decodeDataRecursive(stream, item);

        rows.append(r);
        columns.append(c);
        items.append(item);
        top = qMin(r, top);
        left = qMin(c, left);
        bottom = qMax(r, bottom);
        right = qMax(c, right);
    }

    // insert the dragged items into the table, use a bit array to avoid overwriting items,
    // since items from different tables can have the same row and column
    int dragRowCount = 0;
    int dragColumnCount = right - left + 1;

    // Compute the number of continuous rows upon insertion and modify the rows to match
    QVector<int> rowsToInsert(bottom + 1);
    for (int i = 0; i < rows.count(); ++i)
        rowsToInsert[rows.at(i)] = 1;
    for (int i = 0; i < rowsToInsert.count(); ++i) {
        if (rowsToInsert.at(i) == 1){
            rowsToInsert[i] = dragRowCount;
            ++dragRowCount;
        }
    }
    for (int i = 0; i < rows.count(); ++i)
        rows[i] = top + rowsToInsert.at(rows.at(i));

    QBitArray isWrittenTo(dragRowCount * dragColumnCount);

    // make space in the table for the dropped data
    int colCount = columnCount(parent);
    if (colCount < dragColumnCount + column) {
        insertColumns(colCount, dragColumnCount + column - colCount, parent);
        colCount = columnCount(parent);
    }
    insertRows(row, dragRowCount, parent);

    row = qMax(0, row);
    column = qMax(0, column);

    StandardItem *parentItem = itemFromIndex (parent);
    if (!parentItem)
        parentItem = invisibleRootItem();

    QVector<QPersistentModelIndex> newIndexes(items.size());
    // set the data in the table
    for (int j = 0; j < items.size(); ++j) {
        int relativeRow = rows.at(j) - top;
        int relativeColumn = columns.at(j) - left;
        int destinationRow = relativeRow + row;
        int destinationColumn = relativeColumn + column;
        int flat = (relativeRow * dragColumnCount) + relativeColumn;
        // if the item was already written to, or we just can't fit it in the table, create a new row
        if (destinationColumn >= colCount || isWrittenTo.testBit(flat)) {
            destinationColumn = qBound(column, destinationColumn, colCount - 1);
            destinationRow = row + dragRowCount;
            insertRows(row + dragRowCount, 1, parent);
            flat = (dragRowCount * dragColumnCount) + relativeColumn;
            isWrittenTo.resize(++dragRowCount * dragColumnCount);
        }
        if (!isWrittenTo.testBit(flat)) {
            newIndexes[j] = index(destinationRow, destinationColumn, parentItem->index());
            isWrittenTo.setBit(flat);
        }
    }

    for(int k = 0; k < newIndexes.size(); k++) {
        if (newIndexes.at(k).isValid()) {
            parentItem->setChild(newIndexes.at(k).row(), newIndexes.at(k).column(), items.at(k));
        } else {
            delete items.at(k);
        }
    }

    return true;
}

QT_END_NAMESPACE

#include "moc_standard_item_model_base.cpp"
