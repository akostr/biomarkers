#pragma once

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qdatastream.h>

QT_BEGIN_NAMESPACE

template <class T> class QList;

class StandardItemModelBase;

class StandardItemPrivate;
class StandardItem
{
public:
    StandardItem();
    explicit StandardItem(const QString &text);
    StandardItem(const QVariant &icon, const QString &text);
    explicit StandardItem(int rows, int columns = 1);
    virtual ~StandardItem();

    virtual QVariant data(int role = Qt::UserRole + 1) const;
    virtual void setData(const QVariant &value, int role = Qt::UserRole + 1);
    void clearData();

    inline QString text() const {
        return qvariant_cast<QString>(data(Qt::DisplayRole));
    }
    inline void setText(const QString &text);

    inline QVariant icon() const {
        return data(Qt::DecorationRole);
    }
    inline void setIcon(const QVariant &icon);

    inline QString toolTip() const {
        return qvariant_cast<QString>(data(Qt::ToolTipRole));
    }
    inline void setToolTip(const QString &toolTip);

    inline QString statusTip() const {
        return qvariant_cast<QString>(data(Qt::StatusTipRole));
    }
    inline void setStatusTip(const QString &statusTip);

    inline QString whatsThis() const {
        return qvariant_cast<QString>(data(Qt::WhatsThisRole));
    }
    inline void setWhatsThis(const QString &whatsThis);

    inline QVariant sizeHint() const {
        return data(Qt::SizeHintRole);
    }
    inline void setSizeHint(const QVariant &sizeHint);

    inline QVariant font() const {
        return data(Qt::FontRole);
    }
    inline void setFont(const QVariant &font);

    inline Qt::Alignment textAlignment() const {
        return Qt::Alignment(qvariant_cast<int>(data(Qt::TextAlignmentRole)));
    }
    inline void setTextAlignment(Qt::Alignment textAlignment);

    inline QVariant background() const {
        return data(Qt::BackgroundRole);
    }
    inline void setBackground(const QVariant &brush);

    inline QVariant foreground() const {
        return data(Qt::ForegroundRole);
    }
    inline void setForeground(const QVariant &brush);

    inline Qt::CheckState checkState() const {
        return Qt::CheckState(qvariant_cast<int>(data(Qt::CheckStateRole)));
    }
    inline void setCheckState(Qt::CheckState checkState);

    inline QString accessibleText() const {
        return qvariant_cast<QString>(data(Qt::AccessibleTextRole));
    }
    inline void setAccessibleText(const QString &accessibleText);

    inline QString accessibleDescription() const {
        return qvariant_cast<QString>(data(Qt::AccessibleDescriptionRole));
    }
    inline void setAccessibleDescription(const QString &accessibleDescription);

    Qt::ItemFlags flags() const;
    void setFlags(Qt::ItemFlags flags);

    inline bool isEnabled() const {
        return (flags() & Qt::ItemIsEnabled) != 0;
    }
    void setEnabled(bool enabled);

    inline bool isEditable() const {
        return (flags() & Qt::ItemIsEditable) != 0;
    }
    void setEditable(bool editable);

    inline bool isSelectable() const {
        return (flags() & Qt::ItemIsSelectable) != 0;
    }
    void setSelectable(bool selectable);

    inline bool isCheckable() const {
        return (flags() & Qt::ItemIsUserCheckable) != 0;
    }
    void setCheckable(bool checkable);

    inline bool isAutoTristate() const {
        return (flags() & Qt::ItemIsAutoTristate) != 0;
    }
    void setAutoTristate(bool tristate);

    inline bool isUserTristate() const {
        return (flags() & Qt::ItemIsUserTristate) != 0;
    }
    void setUserTristate(bool tristate);

#if QT_DEPRECATED_SINCE(5, 6)
    QT_DEPRECATED bool isTristate() const { return isAutoTristate(); }
    QT_DEPRECATED void setTristate(bool tristate);
#endif

    inline bool isDragEnabled() const {
        return (flags() & Qt::ItemIsDragEnabled) != 0;
    }
    void setDragEnabled(bool dragEnabled);

    inline bool isDropEnabled() const {
        return (flags() & Qt::ItemIsDropEnabled) != 0;
    }
    void setDropEnabled(bool dropEnabled);

    StandardItem *parent() const;
    int row() const;
    int column() const;
    QModelIndex index() const;
    StandardItemModelBase *model() const;

    int rowCount() const;
    void setRowCount(int rows);
    int columnCount() const;
    void setColumnCount(int columns);

    bool hasChildren() const;
    StandardItem *child(int row, int column = 0) const;
    void setChild(int row, int column, StandardItem *item);
    inline void setChild(int row, StandardItem *item);

    void insertRow(int row, const QList<StandardItem*> &items);
    void insertColumn(int column, const QList<StandardItem*> &items);
    void insertRows(int row, const QList<StandardItem*> &items);
    void insertRows(int row, int count);
    void insertColumns(int column, int count);

    void removeRow(int row);
    void removeColumn(int column);
    void removeRows(int row, int count);
    void removeColumns(int column, int count);

    inline void appendRow(const QList<StandardItem*> &items);
    inline void appendRows(const QList<StandardItem*> &items);
    inline void appendColumn(const QList<StandardItem*> &items);
    inline void insertRow(int row, StandardItem *item);
    inline void appendRow(StandardItem *item);

    StandardItem *takeChild(int row, int column = 0);
    QList<StandardItem*> takeRow(int row);
    QList<StandardItem*> takeColumn(int column);

    void sortChildren(int column, Qt::SortOrder order = Qt::AscendingOrder);

    virtual StandardItem *clone() const;

    enum ItemType { Type = 0, UserType = 1000 };
    virtual int type() const;

    virtual void read(QDataStream &in);
    virtual void write(QDataStream &out) const;
    virtual bool operator<(const StandardItem &other) const;

protected:
    StandardItem(const StandardItem &other);
    StandardItem(StandardItemPrivate &dd);
    StandardItem &operator=(const StandardItem &other);
    QScopedPointer<StandardItemPrivate> d_ptr;

    void emitDataChanged();

private:
    Q_DECLARE_PRIVATE(StandardItem)
    friend class StandardItemModelBasePrivate;
    friend class StandardItemModelBase;
};

inline void StandardItem::setText(const QString &atext)
{ setData(atext, Qt::DisplayRole); }

inline void StandardItem::setIcon(const QVariant &aicon)
{ setData(aicon, Qt::DecorationRole); }

inline void StandardItem::setToolTip(const QString &atoolTip)
{ setData(atoolTip, Qt::ToolTipRole); }

inline void StandardItem::setStatusTip(const QString &astatusTip)
{ setData(astatusTip, Qt::StatusTipRole); }

inline void StandardItem::setWhatsThis(const QString &awhatsThis)
{ setData(awhatsThis, Qt::WhatsThisRole); }

inline void StandardItem::setSizeHint(const QVariant &asizeHint)
{ setData(asizeHint, Qt::SizeHintRole); }

inline void StandardItem::setFont(const QVariant &afont)
{ setData(afont, Qt::FontRole); }

inline void StandardItem::setTextAlignment(Qt::Alignment atextAlignment)
{ setData(int(atextAlignment), Qt::TextAlignmentRole); }

inline void StandardItem::setBackground(const QVariant &abrush)
{ setData(abrush, Qt::BackgroundRole); }

inline void StandardItem::setForeground(const QVariant &abrush)
{ setData(abrush, Qt::ForegroundRole); }

inline void StandardItem::setCheckState(Qt::CheckState acheckState)
{ setData(acheckState, Qt::CheckStateRole); }

inline void StandardItem::setAccessibleText(const QString &aaccessibleText)
{ setData(aaccessibleText, Qt::AccessibleTextRole); }

inline void StandardItem::setAccessibleDescription(const QString &aaccessibleDescription)
{ setData(aaccessibleDescription, Qt::AccessibleDescriptionRole); }

inline void StandardItem::setChild(int arow, StandardItem *aitem)
{ setChild(arow, 0, aitem); }

inline void StandardItem::appendRow(const QList<StandardItem*> &aitems)
{ insertRow(rowCount(), aitems); }

inline void StandardItem::appendRows(const QList<StandardItem*> &aitems)
{ insertRows(rowCount(), aitems); }

inline void StandardItem::appendColumn(const QList<StandardItem*> &aitems)
{ insertColumn(columnCount(), aitems); }

inline void StandardItem::insertRow(int arow, StandardItem *aitem)
{ insertRow(arow, QList<StandardItem*>() << aitem); }

inline void StandardItem::appendRow(StandardItem *aitem)
{ insertRow(rowCount(), aitem); }

class StandardItemModelBasePrivate;

class StandardItemModelBase : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(int sortRole READ sortRole WRITE setSortRole)

public:
    explicit StandardItemModelBase(QObject *parent = nullptr);
    StandardItemModelBase(int rows, int columns, QObject *parent = nullptr);
    ~StandardItemModelBase();

    void setItemRoleNames(const QHash<int,QByteArray> &roleNames);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    // Qt 6: Remove
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    // Qt 6: add override keyword
    bool clearItemData(const QModelIndex &index) /*override*/;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role = Qt::EditRole) override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;

    QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;

    void clear();

    using QObject::parent;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    StandardItem *itemFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromItem(const StandardItem *item) const;

    StandardItem *item(int row, int column = 0) const;
    void setItem(int row, int column, StandardItem *item);
    inline void setItem(int row, StandardItem *item);
    StandardItem *invisibleRootItem() const;

    StandardItem *horizontalHeaderItem(int column) const;
    void setHorizontalHeaderItem(int column, StandardItem *item);
    StandardItem *verticalHeaderItem(int row) const;
    void setVerticalHeaderItem(int row, StandardItem *item);

    void setHorizontalHeaderLabels(const QStringList &labels);
    void setVerticalHeaderLabels(const QStringList &labels);

    void setRowCount(int rows);
    void setColumnCount(int columns);

    void appendRow(const QList<StandardItem*> &items);
    void appendColumn(const QList<StandardItem*> &items);
    inline void appendRow(StandardItem *item);

    void insertRow(int row, const QList<StandardItem*> &items);
    void insertColumn(int column, const QList<StandardItem*> &items);
    inline void insertRow(int row, StandardItem *item);

    inline bool insertRow(int row, const QModelIndex &parent = QModelIndex());
    inline bool insertColumn(int column, const QModelIndex &parent = QModelIndex());

    StandardItem *takeItem(int row, int column = 0);
    QList<StandardItem*> takeRow(int row);
    QList<StandardItem*> takeColumn(int column);

    StandardItem *takeHorizontalHeaderItem(int column);
    StandardItem *takeVerticalHeaderItem(int row);

    const StandardItem *itemPrototype() const;
    void setItemPrototype(const StandardItem *item);

    QList<StandardItem*> findItems(const QString &text,
                                    Qt::MatchFlags flags = Qt::MatchExactly,
                                    int column = 0) const;

    int sortRole() const;
    void setSortRole(int role);

    Qt::CaseSensitivity sortCaseSensitivity() const;
    void setSortCaseSensitivity(Qt::CaseSensitivity cs);

    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData (const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

Q_SIGNALS:
    // ### Qt 6: add changed roles
    void itemChanged(StandardItem *item);

protected:
    StandardItemModelBase(StandardItemModelBasePrivate &dd, QObject *parent = nullptr);

private:
    friend class StandardItemPrivate;
    friend class StandardItem;
    Q_DISABLE_COPY(StandardItemModelBase)
    Q_DECLARE_PRIVATE(StandardItemModelBase)

    Q_PRIVATE_SLOT(d_func(), void _q_emitItemChanged(const QModelIndex &topLeft,
                                                     const QModelIndex &bottomRight))
};

inline void StandardItemModelBase::setItem(int arow, StandardItem *aitem)
{ setItem(arow, 0, aitem); }

inline void StandardItemModelBase::appendRow(StandardItem *aitem)
{ appendRow(QList<StandardItem*>() << aitem); }

inline void StandardItemModelBase::insertRow(int arow, StandardItem *aitem)
{ insertRow(arow, QList<StandardItem*>() << aitem); }

inline bool StandardItemModelBase::insertRow(int arow, const QModelIndex &aparent)
{ return QAbstractItemModel::insertRow(arow, aparent); }
inline bool StandardItemModelBase::insertColumn(int acolumn, const QModelIndex &aparent)
{ return QAbstractItemModel::insertColumn(acolumn, aparent); }

QDataStream &operator>>(QDataStream &in, StandardItem &item);
QDataStream &operator<<(QDataStream &out, const StandardItem &item);

QT_END_NAMESPACE

