#pragma once

#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QObject>
#include <QUuid>
#include <QString>
#include <QMap>

#include <functional>

class TreeModel;

//////////////////////////////////////////////////////////
//// Project tree item
class TreeModelItem : public QObject
{
  Q_OBJECT

public:
  explicit TreeModelItem(TreeModelItem* parent, int row);
  ~TreeModelItem();

  //// Unique property name
  const char* GetUniquePropertyName() const;

  //// Serializable
  void Save(QJsonObject& json) const;
  void Load(const QJsonObject& json, bool emitResetModel = true);

  void Save(QVariantMap& data) const;
  void Load(const QVariantMap& data, bool emitResetModel = true);

  //// Data
  QVariantMap GetData() const;
  void        SetData(const QVariantMap& data);

  QVariant    GetData(const QString& key) const;
  void        SetData(const QString& key, const QVariant& data);

  //// Check states
  virtual void            SetCheckState(Qt::CheckState check);
  virtual Qt::CheckState  GetCheckState() const;
  virtual void            UpdateCheckState();

  QSet<QString>   GetCheckedPaths(Qt::CheckState check) const;
  void            SetCheckedPaths(Qt::CheckState check, const QSet<QString>& paths);

  //// Structure
  TreeModelItem* GetParent() const;
  TreeModelItem* GetPrev() const;
  TreeModelItem* GetNext() const;
  int            GetRow() const;

  bool IsEmpty() const;
  
  bool IsFetched() const;
  void SetFetched(bool fetched);
  
  bool CanFetchMore() const;

  QModelIndex GetIndex(int column = 0) const;

  QList<TreeModelItem*> GetChildren() const;
  QList<TreeModelItem*> GetChildrenRecursive() const;

  TreeModel* GetModel() const;
  void       SetModel(TreeModel* model);

  int            GetRowCount() const;
  TreeModelItem* GetChild(int row) const;
  TreeModelItem* GetChild(const QString& id) const;
  TreeModelItem* GetChild(const char* propertyName, const QUuid& id) const;

  //// Clear
  void Clear(bool emitResetModel = true);

  //// Sort 
  void Sort(bool recursive = true);

  //// Remove child
  void RemoveChild(int row);
  void RemoveChild(TreeModelItem* child);
  void RemoveChild(const char* propertyName, const QUuid& id);
  void RemoveChildren(const char* propertyName, const QList<QUuid>& ids);
  void RemoveChildrenRecursively(std::function<bool(TreeModelItem* child)> predicate, bool emitResetModel = true);
  TreeModelItem* MoveChildTo(int childRow, TreeModelItem *newParent, bool emitReset = true);
//  void AppendChild(TreeModelItem* newChild, int row);

  void CleanupChildrenExcept(const char* propertyName, const QList<QUuid>& newIds);
  void CleanupChildrenExcept(const char* propertyName, const QList<QVariant>& data);

  //// Get slash separated path
  QString GetPath() const;
  QString GetNodeId() const;

  //// Properties
  bool IsRoot() const;

  //// Find
  TreeModelItem* FindRecursive(TreeModelItem* item);

  //// Search by properties
  TreeModelItem* FindChild(const char* propertyName, const QVariant& propertyValue) const;
  TreeModelItem* FindRecursive(const char* propertyName, const QVariant& propertyValue) const;
  TreeModelItem* FindChild(const QString& path) const;
  TreeModelItem* FindChildAbs(const QString& path) const;
  QList<QUuid>   FindExistingChildIds(const char* propertyName);

  TreeModelItem* FindParent(const char* propertyName, const QVariant& propertyValue) const;

public slots:
  //// Notify Changed
  void NotifyChanged();

protected:
  QVariantMap     Data;
  TreeModel*      Model;
  int             Row;
  int             RowCount;
  Qt::CheckState  Check;

  friend class TreeModel;
};
