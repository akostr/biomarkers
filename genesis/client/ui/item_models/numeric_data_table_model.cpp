#include "numeric_data_table_model.h"

#include "logic/markup/markup_data_model.h"
#include "logic/markup/genesis_markup_enums.h"
#include "logic/markup/chromatogram_data_model.h"

#include <QFont>

using namespace GenesisMarkup;

namespace Models
{
  NumericDataTableModel::NumericDataTableModel(QObject* parent)
    : QAbstractTableModel(parent)
  {
  }

  NumericDataTableModel::~NumericDataTableModel()
  {
  }

  int NumericDataTableModel::rowCount(const QModelIndex&) const
  {
    if (!MarkupModel)
      return 0;
    return MarkupModel->getChromatogrammsCount();
  }

  int NumericDataTableModel::columnCount(const QModelIndex&) const
  {
    return static_cast<int>(HorizontalHeaders.size());
  }

  QVariant NumericDataTableModel::data(const QModelIndex& index, int role) const
  {
    //qDebug() << index.row() << index.column() << (Qt::ItemDataRole)role;
    if ((role != Qt::DisplayRole
         && role != Qt::BackgroundRole
         && role != Qt::FontRole
         && role != ChromaIdRole)
        || Names.empty())
      return QVariant();

    const auto row = index.row();
    if (row >= rowCount())
      return QVariant();

    //row data
    if(role == ChromaIdRole && row < Ids.size())
      return Ids[row];


    //first column data
    auto column = index.column();
    if(column == 0)
    {
      if(role == Qt::BackgroundRole)
        return QVariant();
      if(role == Qt::DisplayRole && row < Names.size())
        return Names[row];
      return QVariant();
    }

    //cell data
    AbstractEntityDataModel::ConstDataPtr cellData;
    {
      if (row >= Names.size())
        return QVariant();
      auto rowIt = Rows.find(Names[row]);
      if(rowIt == Rows.end())
        return QVariant();
      if (column >= HorizontalHeaders.size())
        return QVariant();
      auto colIt = rowIt->find(HorizontalHeaders[column]);
      if(colIt == rowIt->end())
        return QVariant();
      cellData = colIt.value();
    }

    if(role == Qt::BackgroundRole)
    {
      if(cellData->getData(PeakType) == PTMarker)
        return QVariant();
      else
        return cellData->getData(PeakColor);
    }
    if(role == Qt::FontRole)
    {
      if ((cellData->getData(PeakType) != PTMarker))
        return QVariant();
      QFont font;
      font.setBold(true);
      return font;
    }
    if(role == Qt::DisplayRole)
      return cellData->getData(Parameter);

    //unknown role
    return QVariant();
  }

  QVariant NumericDataTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const
  {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
      return QVariant();

    if (section > static_cast<int>(HorizontalHeaders.size()))
      return QVariant();

    return HorizontalHeaders[section];
  }

  void NumericDataTableModel::SetModel(MarkupModelPtr model)
  {
    beginResetModel();
    ClearHorizontalHeaders();

    Rows.clear();
    Names.clear();
    Ids.clear();

    if (!model)
    {
      MarkupModel = nullptr;
      endResetModel();
      return;
    }
    endResetModel();

    const auto master = model->getMaster();
    if (!master)
      return;

    const auto ents = master->getEntities(MarkupEntityTypes::TypePeak);

    QList<AbstractEntityDataModel::ConstDataPtr> titles;
    titles.reserve(ents.count());

    std::copy_if(ents.begin(), ents.end(), std::back_inserter(titles),
      [](AbstractEntityDataModel::ConstDataPtr item)
      {
        if (!item && !item->hasData(PeakType))
          return false;
        return (item->getData(PeakType) == PTMarker)
          || (item->getData(PeakType) == PTInterMarker);
      });

    std::sort(titles.begin(), titles.end(),
      [](AbstractEntityDataModel::ConstDataPtr lt, AbstractEntityDataModel::ConstDataPtr rt)
      {
        return lt->getData(PeakRetentionTime).toDouble() < rt->getData(PeakRetentionTime).toDouble();
      });

    std::transform(titles.begin(), titles.end(), std::back_inserter(HorizontalHeaders),
      [](AbstractEntityDataModel::ConstDataPtr item) { return item->getData(PeakTitle).toString(); });

    Append(master->id(), ents);

    const auto list = model->getChromatogrammsIdList();
    for (const auto& item : list)
    {
      const auto chroma = model->getChromatogramm(item);
      if (chroma->isMaster())
        continue;
      Append(chroma->id(), chroma->getEntities(MarkupEntityTypes::TypePeak));
    }
    beginResetModel();
    if(MarkupModel != model)
    {
      if(MarkupModel)
      {
        disconnect(MarkupModel.get(), nullptr, this, nullptr);
      }
      MarkupModel = model;
      if(MarkupModel)
      {
        connect(MarkupModel.get(), &MarkupDataModel::chromatogramModelReset, this, &NumericDataTableModel::UpdateData);
        connect(MarkupModel.get(), &MarkupDataModel::chromatogramEntityResetted, this, &NumericDataTableModel::UpdateData);
        connect(MarkupModel.get(), &MarkupDataModel::chromatogramEntityAboutToRemove, this, &NumericDataTableModel::UpdateData);
        connect(MarkupModel.get(), &MarkupDataModel::chromatogramEntityAdded, this, &NumericDataTableModel::UpdateData);
        connect(MarkupModel.get(), &MarkupDataModel::chromatogramEntityChanged, this, &NumericDataTableModel::UpdateData);
        connect(MarkupModel.get(), &MarkupDataModel::chromatogrammRemoved, this, &NumericDataTableModel::UpdateData);
        connect(MarkupModel.get(), &MarkupDataModel::chromatogrammSwapped, this, &NumericDataTableModel::UpdateData);
        connect(MarkupModel.get(), &MarkupDataModel::chromatogrammsCleared, this, &NumericDataTableModel::UpdateData);
        connect(MarkupModel.get(), &MarkupDataModel::dataChanged, this, &NumericDataTableModel::UpdateData);
      }
    }
    endResetModel();
  }

  void NumericDataTableModel::SetPassportData(const QList<AbstractEntityDataModel::ConstDataPtr>& data)
  {
    beginResetModel();
    PassportData = data;
    beginResetModel();
    UpdateData();
  }

  void NumericDataTableModel::UpdateData()
  {
    if (!MarkupModel)
      return;

    const auto master = MarkupModel->getMaster();
    if (!master)
      return;


    beginResetModel();

    ClearHorizontalHeaders();

    Rows.clear();
    Names.clear();

    const auto ents = master->getEntities(MarkupEntityTypes::TypePeak);

    QList<AbstractEntityDataModel::ConstDataPtr> titles;
    titles.reserve(ents.count());

    std::copy_if(ents.begin(), ents.end(), std::back_inserter(titles),
      [](AbstractEntityDataModel::ConstDataPtr item)
      {
        if (!item && !item->hasData(PeakType))
          return false;
        return (item->getData(PeakType) == PTMarker)
          || (item->getData(PeakType) == PTInterMarker);
      });

    std::sort(titles.begin(), titles.end(),
      [](AbstractEntityDataModel::ConstDataPtr lt, AbstractEntityDataModel::ConstDataPtr rt)
      {
        return lt->getData(PeakRetentionTime).toDouble() < rt->getData(PeakRetentionTime).toDouble();
      });

    std::transform(titles.begin(), titles.end(), std::back_inserter(HorizontalHeaders),
      [](AbstractEntityDataModel::ConstDataPtr item) { return item->getData(PeakTitle).toString(); });

    Append(master->id(), ents);

    const auto list = MarkupModel->getChromatogrammsIdList();
    for (const auto& item : list)
    {
      const auto chroma = MarkupModel->getChromatogramm(item);
      if (chroma->isMaster())
        continue;
      Append(chroma->id(), chroma->getEntities(MarkupEntityTypes::TypePeak));
    }
    endResetModel();
  }

  void NumericDataTableModel::SetCurrentParameter(int param)
  {
    Parameter = param;
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
  }

  void NumericDataTableModel::Append(int id, const QList<AbstractEntityDataModel::ConstDataPtr>& values)
  {
    const auto it = std::find_if(PassportData.begin(), PassportData.end(),
      [id](AbstractEntityDataModel::ConstDataPtr item)
      {
        return item->getData(PassportFileId).toInt() == id;
      });
    if (it == PassportData.end())
      return;

    QMap<QString, AbstractEntityDataModel::ConstDataPtr> newData;
    for (const auto& item : values)
    {
      if (!item && !item->hasData(PeakType))
        continue;
      if ((item->getData(PeakType) == PTMarker)
        || (item->getData(PeakType) == PTInterMarker))
      {
        newData.insert(item->getData(PeakTitle).toString(), item);
      }
    }
    const auto name = (*it)->getData(PassportFilename).toString() + "("
                      + QString::number((*it)->getData(PassportFileId).toInt())
                      + ")";
    Names.append(name);
    Ids.append(id);
    Rows.insert(name, newData);
  }

  void NumericDataTableModel::Clear()
  {
    Rows.clear();
    HorizontalHeaders.clear();
    beginResetModel();
    MarkupModel = nullptr;
    endResetModel();
  }

  void NumericDataTableModel::ClearHorizontalHeaders()
  {
    HorizontalHeaders.clear();
    HorizontalHeaders.emplace_back(tr("Name"));
  }
}
