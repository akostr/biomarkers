#include "tree_model.h"
#include "tree_model_presentation.h"
#include "tree_model_item.h"

#include <genesis_style/style.h>

#include <QModelIndex>

#include <QIcon>
#include <QFont>
#include <QLocale>

namespace Details
{
  QList<QPointer<TreeModel>> TreeModel_Instances__;
}

////////////////////////////////////////////////////
//// Tree model
TreeModel::TreeModel(QObject* parent)
  : QAbstractItemModel(parent)
  , Checkable(false)
  , DoubleFormat('f')
  , DoubleFormatPrecision(2)
  , ModelIsInResetState(false)
{
  Initialize();

  Details::TreeModel_Instances__.push_back(this);
}

TreeModel::~TreeModel()
{
  Details::TreeModel_Instances__.removeOne(this);

  delete Root;
}

QList<QPointer<TreeModel>> TreeModel::GetInstances(const QString& metaClassName)
{
  QList<QPointer<TreeModel>> instances;
  for (auto& i : Details::TreeModel_Instances__)
  {
    if (QString(i->metaObject()->className()) == metaClassName)
    {
      instances << i;
    }
  }
  return instances;
}

void TreeModel::ResetInstances(const QString& metaClassName)
{
  QList<QPointer<TreeModel>> instances = GetInstances(metaClassName);
  for (auto& i : instances)
  {
    i->Reset();
  }
}

void TreeModel::Initialize()
{
  connect(this, &TreeModel::modelAboutToBeReset, this, [this](){ModelIsInResetState = true;});
  connect(this, &TreeModel::modelReset, this, [this](){ModelIsInResetState = false;});

  //// Initialize root
  Root = new TreeModelItem(nullptr, 0);
  Root->SetModel(this);

  //// Initialize columns
  Columns[0].Ids[Qt::EditRole]            = "title";
  Columns[0].HeaderData[Qt::DisplayRole]  = tr("Title");
}

bool TreeModel::IsInResetState() const
{
  return ModelIsInResetState;
}

void TreeModel::Reset()
{
  //// Clear
  Root->Clear();
  if(Root->IsEmpty() && !Root->IsFetched())
    fetchMore(QModelIndex());
}

void TreeModel::Clear()
{
  //// Clear
  Root->Clear();
}

void TreeModel::Save(QJsonObject& json) const
{
  Root->Save(json);
}

void TreeModel::Load(const QJsonObject& json)
{
  Root->Load(json);
}

bool TreeModel::GetCheckable() const
{
  return Checkable;
}

void TreeModel::SetCheckable(bool checkable)
{
  Checkable = checkable;
}

void TreeModel::SetSpan(int row, int column, int rowSpan, int columnSpan)
{
  Spans[{row, column}] = { rowSpan, columnSpan };
}

void TreeModel::ClearSpans()
{
  Spans.clear();
}

const QMap<QPair<int, int>, QPair<int, int>>& TreeModel::GetSpans() const
{
  return Spans;
}

ColumnInfo TreeModel::GetColumnInfo(int index) const
{
  return Columns.value(index);
}

//// Get root
QPointer<TreeModelItem> TreeModel::GetRoot()
{
  return Root;
}

//// Get item from index
TreeModelItem* TreeModel::GetItem(const QModelIndex& index) const
{
  return static_cast<TreeModelItem*>(index.internalPointer());
}

//// Get actions for item
QList<ActionInfo> TreeModel::GetItemActions(const QModelIndex& /*index*/)
{
  return ItemActions;
}

//// Presentation models
QPointer<TreeModelPresentation> TreeModel::GetPresentationModel()
{
  if (!PresentationModel)
    PresentationModel = new TreeModelPresentation(this);

  return PresentationModel;
}

QPointer<TreeModelPresentation> TreeModel::GetSingleColumnProxyModel(int column)
{
  if (!SingleColumnProxyModels[column])
  {
    SingleColumnProxyModels[column] = new TreeModelPresentationSingleColumn(this, column);
  }
  return SingleColumnProxyModels[column];
}

//// Export
void TreeModel::ExportCSV(QByteArray& buffer, int dataRole, int headerDataRole) const
{
  ExportCSV(this, buffer, dataRole, headerDataRole);
}

void TreeModel::ExportCSV(const QAbstractItemModel* model, QByteArray& buffer, int dataRole, int headerDataRole)
{
  int rc = model->rowCount();
  int cc = model->columnCount();

  QString exported;

  //// Header
  for (int c = 0; c < cc; c++)
  {
    auto h = model->headerData(c, Qt::Horizontal, headerDataRole);
    exported += "\t";
    exported += h.toString();
  }
  exported += "\n";

  //// Content
  for (int r = 0; r < rc; r++)
  {
    QString e;
    auto h = model->headerData(r, Qt::Vertical, headerDataRole);
    exported += h.toString();

    for (int c = 0; c < cc; c++)
    {
      auto d = model->data(model->index(r, c), dataRole);
      e += "\t";
      e += d.toString();
    }
    e += "\n";
    exported += e;
  }
  buffer = exported.toUtf8();
}

//// Implemeted
QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  QPointer<TreeModelItem> parentItem;

  if (!parent.isValid())
    parentItem = Root;
  else
    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());
  if (!parentItem)
    return QModelIndex();

  QPointer<TreeModelItem> childItem = parentItem->GetChild(row);
  if (childItem)
    return createIndex(row, column, childItem);

  return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
    return QModelIndex();

  QPointer<TreeModelItem> childItem = static_cast<TreeModelItem*>(index.internalPointer());
  if (childItem.isNull())
    return QModelIndex();

  QPointer<TreeModelItem> parentItem = childItem->GetParent();
  if (parentItem.isNull() || parentItem == Root)
    return QModelIndex();

  return createIndex(parentItem->GetRow(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
  QPointer<TreeModelItem> parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = Root;
  else
    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

  if (!parentItem.isNull())
    return parentItem->GetRowCount();
  return 0;
}

int TreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return Columns.size() + 1;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  //// Static demibold font
  QString fontFace = Style::GetSASSValue("fontFace");
  int fontSize     = Style::GetSASSValue("fontSizeScalable").toInt();
  static QFont DemiBoldFont(fontFace, fontSize, QFont::DemiBold);

  QPointer<TreeModelItem> item = static_cast<TreeModelItem*>(index.internalPointer());
  if (!item.isNull())
  {
    switch (role)
    {
      case Qt::CheckStateRole:
        {
          if (flags(index).testFlag(Qt::ItemIsUserCheckable) && index.column() == 0)
            return item->GetCheckState();
        }
        break;
      default:
        {
          if (Columns.contains(index.column()))
          {
            QString key = Columns[index.column()].Ids.value((Qt::ItemDataRole)role);
            if (!key.isEmpty())
            {
              if (item->Data.contains(key))
              {
                //// Read from item
                QVariant itemData = item->Data.value(key);
                QVariant result   = itemData;

                //// Some types need transformation
                //// ...

                return result;
              }
            }
          }
          else if (index.column() == Columns.size())
          {
            switch (role)
            {
            case Qt::DisplayRole:
            case Qt::EditRole:
              {
                static const QChar dotsChars[] = { QChar((wchar_t)8226), QChar((wchar_t)8226), QChar((wchar_t)8226), QChar((wchar_t)0) };
                static const QString dots(dotsChars);
                return dots;
              }
              break;
            default:
              break;
            }
          }
        }
        break;
    }
  }

  return QVariant();
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
  if (role == Qt::CheckStateRole)
  {
    QPointer<TreeModelItem> item = static_cast<TreeModelItem*>(index.internalPointer());
    if (!item.isNull())
    {
      item->SetCheckState(data.value<Qt::CheckState>());
      return true;
    }
  }
  else
  {
    QPointer<TreeModelItem> item = static_cast<TreeModelItem*>(index.internalPointer());
    if (!item.isNull())
    {
      if (Columns.contains(index.column()))
      {
        QString key = Columns[index.column()].Ids.value((Qt::ItemDataRole)role);
        if (!key.isEmpty())
        {
          item->Data[key] = data;
          return true;
        }
      }
    }
  }
  return false;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal)
  {
    if (Columns.contains(section))
    {
      return Columns[section].HeaderData.value((Qt::ItemDataRole)role);
    }
  }
  return QVariant();
}

bool TreeModel::hasChildren(const QModelIndex& parent) const
{
  QPointer<TreeModelItem> parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = Root;
  else
    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

  return !parentItem->IsEmpty() || parentItem->CanFetchMore();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags f = QAbstractItemModel::flags(index);
  if (Checkable)
    f |= Qt::ItemIsUserCheckable;
  if (index.column() == columnCount() - 1)
    f |= Qt::ItemIsEditable;
  return f;
}

QSize TreeModel::span(const QModelIndex& index) const
{
  if (index.isValid() && !index.parent().isValid())
  {
    QPair<int, int> k = { index.row(), index.column() };
    if (Spans.contains(k))
    {
      QPair<int, int> v;
      return QSize(v.first, v.second);
    }
  }
  return QAbstractItemModel::span(index);
}

void TreeModel::changePersistentIndex(const QModelIndex &from, const QModelIndex &to)
{
  return QAbstractItemModel::changePersistentIndex(from, to);
}

void TreeModel::TriggerAction(const QString& actionId, TreeModelItem* item)
{
  emit ActionTriggered(actionId, item);
}

////////////////////////////////////////////////////
//// Tree model static
TreeModelStatic::TreeModelStatic(QObject* parent)
  : TreeModel(parent)
{
}

TreeModelStatic::~TreeModelStatic()
{
}

//// Not a fetching model
bool TreeModelStatic::canFetchMore(const QModelIndex& /*parent*/) const
{
  return false;
}

void TreeModelStatic::fetchMore(const QModelIndex& /*parent*/)
{
}

QMap<int, ColumnInfo> TreeModel::getColumns()
{
  return Columns;
}
