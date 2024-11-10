#pragma once

#include <QtCore/private/qabstractitemmodel_p.h>

#include <QtCore/qlist.h>
#include <QtCore/qpair.h>
#include <QtCore/qstack.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class StandardItemData
{
public:
    inline StandardItemData() : role(-1) {}
    inline StandardItemData(int r, const QVariant &v) : role(r), value(v) {}
    inline StandardItemData(const std::pair<const int&, const QVariant&> &p) : role(p.first), value(p.second) {}
    int role;
    QVariant value;
    inline bool operator==(const StandardItemData &other) const { return role == other.role && value == other.value; }
};
Q_DECLARE_TYPEINFO(StandardItemData, Q_MOVABLE_TYPE);

inline QDataStream &operator>>(QDataStream &in, StandardItemData &data)
{
    in >> data.role;
    in >> data.value;
    return in;
}

inline QDataStream &operator<<(QDataStream &out, const StandardItemData &data)
{
    out << data.role;
    out << data.value;
    return out;
}

inline QDebug &operator<<(QDebug &debug, const StandardItemData &data)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << data.role
                    << " "
                    << data.value;
    return debug.space();
}

class StandardItemPrivate
{
    Q_DECLARE_PUBLIC(StandardItem)
public:
    inline StandardItemPrivate()
        : model(0),
          parent(0),
          rows(0),
          columns(0),
          q_ptr(0),
          lastKnownIndex(-1)
        { }

    inline int childIndex(int row, int column) const {
        if ((row < 0) || (column < 0)
            || (row >= rowCount()) || (column >= columnCount())) {
            return -1;
        }
        return (row * columnCount()) + column;
    }
    inline int childIndex(const StandardItem *child) const {
        const int lastChild = children.size() - 1;
        int &childsLastIndexInParent = child->d_func()->lastKnownIndex;
        if (childsLastIndexInParent != -1 && childsLastIndexInParent <= lastChild) {
            if (children.at(childsLastIndexInParent) == child)
                return childsLastIndexInParent;
        } else {
            childsLastIndexInParent = lastChild / 2;
        }

        // assuming the item is in the vicinity of the previous index, iterate forwards and
        // backwards through the children
        int backwardIter = childsLastIndexInParent - 1;
        int forwardIter = childsLastIndexInParent;
        Q_FOREVER {
            if (forwardIter <= lastChild) {
                if (children.at(forwardIter) == child) {
                    childsLastIndexInParent = forwardIter;
                    break;
                }
                ++forwardIter;
            } else if (backwardIter < 0) {
                childsLastIndexInParent = -1;
                break;
            }
            if (backwardIter >= 0) {
                if (children.at(backwardIter) == child) {
                    childsLastIndexInParent = backwardIter;
                    break;
                }
                --backwardIter;
            }
        }
        return childsLastIndexInParent;
    }
    QPair<int, int> position() const;
    void setChild(int row, int column, StandardItem *item,
                  bool emitChanged = false);
    inline int rowCount() const {
        return rows;
    }
    inline int columnCount() const {
        return columns;
    }
    void childDeleted(StandardItem *child);

    void setModel(StandardItemModelBase *mod);

    inline void setParentAndModel(
        StandardItem *par,
        StandardItemModelBase *mod) {
        setModel(mod);
        parent = par;
    }

    void changeFlags(bool enable, Qt::ItemFlags f);
    void setItemData(const QMap<int, QVariant> &roles);
    const QMap<int, QVariant> itemData() const;

    bool insertRows(int row, int count, const QList<StandardItem*> &items);
    bool insertRows(int row, const QList<StandardItem*> &items);
    bool insertColumns(int column, int count, const QList<StandardItem*> &items);

    void sortChildren(int column, Qt::SortOrder order);

    StandardItemModelBase *model;
    StandardItem *parent;
    QVector<StandardItemData> values;
    QVector<StandardItem*> children;
    int rows;
    int columns;

    StandardItem *q_ptr;

    mutable int lastKnownIndex; // this is a cached value
};

class StandardItemModelBasePrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(StandardItemModelBase)

public:
    StandardItemModelBasePrivate();
    virtual ~StandardItemModelBasePrivate();

    void init();

    inline StandardItem *createItem() const {
        return itemPrototype ? itemPrototype->clone() : new StandardItem;
    }

    inline StandardItem *itemFromIndex(const QModelIndex &index) const {
        Q_Q(const StandardItemModelBase);
        if (!index.isValid())
            return root.data();
        if (index.model() != q)
            return 0;
        StandardItem *parent = static_cast<StandardItem*>(index.internalPointer());
        if (parent == 0)
            return 0;
        return parent->child(index.row(), index.column());
    }

    void sort(StandardItem *parent, int column, Qt::SortOrder order);
    void itemChanged(StandardItem *item, const QVector<int>& roles = QVector<int>());
    void rowsAboutToBeInserted(StandardItem *parent, int start, int end);
    void columnsAboutToBeInserted(StandardItem *parent, int start, int end);
    void rowsAboutToBeRemoved(StandardItem *parent, int start, int end);
    void columnsAboutToBeRemoved(StandardItem *parent, int start, int end);
    void rowsInserted(StandardItem *parent, int row, int count);
    void columnsInserted(StandardItem *parent, int column, int count);
    void rowsRemoved(StandardItem *parent, int row, int count);
    void columnsRemoved(StandardItem *parent, int column, int count);

    void _q_emitItemChanged(const QModelIndex &topLeft,
                            const QModelIndex &bottomRight);

    void decodeDataRecursive(QDataStream &stream, StandardItem *item);

    QVector<StandardItem*> columnHeaderItems;
    QVector<StandardItem*> rowHeaderItems;
    QScopedPointer<StandardItem> root;
    const StandardItem *itemPrototype;
    int sortRole;
    Qt::CaseSensitivity sortCaseSensitivity;
};

QT_END_NAMESPACE
