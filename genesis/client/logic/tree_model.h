#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QPointer>
#include <QIcon>
#include <QMap>

//// Fwds
class TreeModelItem;

class TreeModelPresentation;
class TreeModelPresentationSingleColumn;

////////////////////////////////////////////////////
//// Tree model column info
struct ColumnInfo
{
  enum FilterType
  {
    FilterTypeNone,           //// No filtering
    
    FilterTypeTextValuesList, //// Filter editor shall show all available text values
    FilterTypeTextSubstring,  //// Filter editor shall set substring
    FilterTypeDateRange,      //// Filter editor shall set date range
    FilterTypeIntegerRange,   //// Filter editor shall set integer range
    
    FilterTypeLast
  };
  
  //// Ctor
  ColumnInfo()
    : Filter(FilterTypeNone)
  {
  }
  
  //// QVariantMap key to access TreeModelItem::Data
  QMap<Qt::ItemDataRole, QString>         Ids;
  
  //// Type hint for specific data role
  QMap<Qt::ItemDataRole, QMetaType::Type> Types;

  //// Header data
  QMap<Qt::ItemDataRole, QVariant>        HeaderData;
  
  //// Filter
  FilterType Filter;
};

struct ActionInfo
{
  //// Action id
  QString Id;

  //// Icon path
  QString Icon;

  //// Text
  QString Text;
};

////////////////////////////////////////////////////
//// Tree model
class TreeModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit TreeModel(QObject* parent);
  virtual ~TreeModel();

  /// Statics
  static QList<QPointer<TreeModel>>  GetInstances(const QString& metaClassName);
  static void                        ResetInstances(const QString& metaClassName);

  virtual const char* GetUniquePropertyName() const { return "id"; }

  virtual void Reset();
  virtual void Clear();

  virtual void Save(QJsonObject& json) const;
  virtual void Load(const QJsonObject& json);

  //// Features
  bool GetCheckable() const;
  void SetCheckable(bool checkable);

  void SetSpan(int row, int column, int rowSpan, int columnSpan);
  void ClearSpans();
  const QMap<QPair<int, int>, QPair<int, int>>& GetSpans() const;
  
  ColumnInfo GetColumnInfo(int index) const;

  //// Get root
  QPointer<TreeModelItem> GetRoot();

  //// Get item from index
  TreeModelItem* GetItem(const QModelIndex& index) const;

  //// Get actions for item
  virtual QList<ActionInfo> GetItemActions(const QModelIndex& index);

  //// Presentation models
  QPointer<TreeModelPresentation> GetPresentationModel();
  QPointer<TreeModelPresentation> GetSingleColumnProxyModel(int column);
  
  //// Export
  void ExportCSV(QByteArray& buffer, int dataRole = Qt::EditRole, int headerDataRole = Qt::DisplayRole) const;
  static void ExportCSV(const QAbstractItemModel* model, QByteArray& buffer, int dataRole = Qt::EditRole, int headerDataRole = Qt::DisplayRole);

  //// Implemeted
  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex& child) const override;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual bool setData(const QModelIndex& index, const QVariant& data, int role) override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

  virtual QSize span(const QModelIndex& index) const override;

  virtual void changePersistentIndex(const QModelIndex &from, const QModelIndex &to);

  //// Notifications
  void TriggerAction(const QString& actionId, TreeModelItem* item);

  bool IsInResetState() const;

  QMap<int, ColumnInfo> getColumns();

signals:
  void ActionTriggered(const QString& actionId, TreeModelItem* item);

protected:
  void Initialize();

protected:
  //// Root
  QPointer<TreeModelItem>       Root;
  
  //// Icon cache
  mutable QMap<QString, QIcon*> IconCache;

  //// Checkable mode
  bool                          Checkable;

  //// Double format
  char                          DoubleFormat;
  int                           DoubleFormatPrecision;

  //// Columns by integer index
  QMap<int, ColumnInfo>         Columns;

  //// Item actions
  QList<ActionInfo>             ItemActions;

  //// Spans
  QMap<QPair<int, int>, QPair<int, int>> Spans;

  //// Presentation models
  QPointer<TreeModelPresentation>             PresentationModel;
  QMap<int, QPointer<TreeModelPresentation>>  SingleColumnProxyModels;

  bool ModelIsInResetState;

  friend class TreeModelItem;
};

////////////////////////////////////////////////////
//// Tree model static
class TreeModelStatic : public TreeModel
{
  Q_OBJECT
public:
  TreeModelStatic(QObject* parent);
  ~TreeModelStatic();

  //// Not a fetching model
  virtual bool canFetchMore(const QModelIndex& parent) const override;
  virtual void fetchMore(const QModelIndex& parent) override;
};
