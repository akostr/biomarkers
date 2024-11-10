#include "covats_index_model_table.h"

#include "logic/markup/genesis_markup_enums.h"
#include "logic/markup/commands.h"
#include "logic/markup/markup_data_model.h"
#include <ui/genesis_window.h>
#include <ui/dialogs/templates/dialog.h>
#include <genesis_style/style.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

using namespace GenesisMarkup;
using namespace Core;

namespace Models
{
  CovatsIndexModelTable::CovatsIndexModelTable(QObject* parent)
    : QAbstractTableModel(parent)
    , HorizontalHeaders
    {
      tr("Title"),
      tr("Retention time"),
      tr("Index"),
      tr("Window, %"),
    }
  {
  }

  int CovatsIndexModelTable::rowCount(const QModelIndex& parent) const
  {
    return static_cast<int>(TableData.size());
  }

  int CovatsIndexModelTable::columnCount(const QModelIndex& parent) const
  {
    return static_cast<int>(HorizontalHeaders.size());
  }

  QVariant CovatsIndexModelTable::data(const QModelIndex& index, int role) const
  {
    if (role != Qt::DisplayRole || TableData.empty())
      return QVariant();

    auto row = index.row();
    if (row > rowCount())
      return QVariant();

    auto entity = TableData[row];
    auto uid = entity->getUid();

    switch (index.column())
    {
    case 0:
      return entity->getData(PeakTitle).toString();
    case 1:
      return entity->getData(ChromatogrammEntityDataRoles::PeakRetentionTime);
    case 2:
      return entity->getData(ChromatogrammEntityDataRoles::PeakCovatsIndex);
    case 3:
      return entity->getData(ChromatogrammEntityDataRoles::PeakMarkerWindow);
    default:
      return QVariant();
    }
  }

  bool CovatsIndexModelTable::setData(const QModelIndex& index, const QVariant& value, int role)
  {
    if (role != Qt::EditRole || TableData.empty())
      return false;

    const auto row = index.row();
    if (row > rowCount())
      return false;

    auto entity = TableData[row];
    auto uid = entity->getUid();

    switch (index.column())
    {
    case 2:
    {
      auto cmd = new ChangeEntityDataCommand(mChromatogramModel, uid, ChromatogrammEntityDataRoles::PeakCovatsIndex, value);
      emit newCommand(cmd);
      break;
    }
    case 3:
    {
      auto cmd = new ChangeEntityDataCommand(mChromatogramModel, uid, ChromatogrammEntityDataRoles::PeakMarkerWindow, value);
      emit newCommand(cmd);
      break;
    }
    default:
      break;
    }
    return false;
  }

  QVariant CovatsIndexModelTable::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
      return QVariant();

    if (section > static_cast<int>(HorizontalHeaders.size()))
      return QVariant();

    return HorizontalHeaders[section];
  }

  Qt::ItemFlags CovatsIndexModelTable::flags(const QModelIndex& index) const
  {
    const auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() > 1)
      return flags | Qt::ItemIsEditable;

    return flags;
  }

  void CovatsIndexModelTable::SetModel(MarkupModelPtr model)
  {
    if (mChromatogramModel)
      disconnect(mChromatogramModel.get(), nullptr, this, nullptr);
    if (mMarkupModel)
      disconnect(mMarkupModel.get(), nullptr, this, nullptr);
    mMarkupModel = model;
    mChromatogramModel = nullptr;
    onModelAboutToReset();
    if (mMarkupModel)
    {
      connect(mMarkupModel.get(), &GenesisMarkup::MarkupDataModel::dataChanged, this, &CovatsIndexModelTable::onMarkupDataChanged);
      mChromatogramModel = mMarkupModel->getMaster();
      if (mChromatogramModel)
      {
        connectModelSignals(mChromatogramModel);
        onModelReset();
      }
    }
  }

  void CovatsIndexModelTable::onEntityChanged(EntityType type, TEntityUid eId, DataRoleType role, const QVariant& value)
  {
    auto emitDataChanged = [this, &eId](int columnNumber)
    {
      for (int row = 0; row < TableData.size(); row++)
      {
        if (TableData[row]->getUid() == eId)
        {
          auto ind = index(row, columnNumber);
          emit dataChanged(ind, ind, { Qt::DisplayRole });
          return;
        }
      }
    };

    if (type == GenesisMarkup::TypePeak)
    {
      switch (role)
      {
      case GenesisMarkup::PeakType:
        if (value.toInt() == GenesisMarkup::PTMarker)
          AddData(eId);
        else
          RemoveData(eId);
        break;
      case PeakTitle:
        emitDataChanged(0);
        break;
      case PeakRetentionTime:
        emitDataChanged(1);
        break;
      case PeakCovatsIndex:
        emitDataChanged(2);
        break;
      case PeakMarkerWindow:
        emitDataChanged(3);
        break;
      default:
        break;
      }
    }
  }

  void CovatsIndexModelTable::onEntityResetted(EntityType type, TEntityUid eId, AbstractEntityDataModel::ConstDataPtr peakData)
  {
    for (int row = 0; row < TableData.size(); row++)
    {
      if (TableData[row]->getUid() == eId)
      {
        auto tl = index(row, 0);
        auto br = index(row, 3);
        emit dataChanged(tl, br, { Qt::DisplayRole });
        return;
      }
    }
  }

  void CovatsIndexModelTable::onEntityAdded(EntityType type, TEntityUid eId, AbstractEntityDataModel::ConstDataPtr peakData)
  {
    if (type == TypePeak)
      if (mChromatogramModel->getEntity(eId)->getData(PeakType).toInt() == PTMarker)
        AddData(eId);
  }

  void CovatsIndexModelTable::onEntityAboutToRemove(EntityType type, TEntityUid eId)
  {
    if (type == TypePeak)
      if (mChromatogramModel->getEntity(eId)->getData(PeakType).toInt() == PTMarker)
        RemoveData(eId);
  }

  void CovatsIndexModelTable::onMarkupDataChanged(uint role, const QVariant& data)
  {
    if (role == MarkupMasterIdRole)
    {
      onModelAboutToReset();

      auto markupModel = qobject_cast<GenesisMarkup::MarkupDataModel*>(sender());
      Q_ASSERT(markupModel);
      if (!markupModel)
        return;
      mChromatogramModel = markupModel->getMaster();
      onModelReset();
    }
  }

  void CovatsIndexModelTable::onModelAboutToReset()
  {
    Clear();
  }

  void CovatsIndexModelTable::onModelReset()
  {
    if (mChromatogramModel)
    {
      for (auto& peakId : mChromatogramModel->getUIdListOfEntities(GenesisMarkup::TypePeak))
      {
        auto peak = mChromatogramModel->getEntity(peakId);
        if (peak->hasDataAndItsValid(GenesisMarkup::PeakType) &&
          peak->getData(GenesisMarkup::PeakType).toInt() == GenesisMarkup::PTMarker)
          AddData(peakId);
      }
    }
  }

  void CovatsIndexModelTable::AddData(const TEntityUid uid)
  {
    const auto it = std::find_if(TableData.begin(), TableData.end(),
      [&uid](AbstractEntityDataModel::ConstDataPtr item)
      {
        return item->getUid() == uid;
      });

    if (it != TableData.end())
      return;

    auto entity = mChromatogramModel->getEntity(uid);
    if (entity->getData(ChromatogrammEntityDataRoles::PeakMarkerWindow) == 0.0)
    {
      auto cmd = new ChangeEntityDataCommand(mChromatogramModel, uid, ChromatogrammEntityDataRoles::PeakMarkerWindow, 2);
      emit newCommand(cmd);
    }
    beginInsertRows(index(TableData.size(), 0), TableData.size(), TableData.size());
    TableData.push_back(entity);
    endInsertRows();
  }

  void CovatsIndexModelTable::RemoveData(const TEntityUid uid)
  {
    auto iter = TableData.begin();
    while (iter != TableData.end())
    {
      if ((*iter)->getUid() == uid)
      {
        auto row = std::distance(TableData.begin(), iter);
        beginRemoveRows(index(TableData.size(), 0), row, row);
        iter = TableData.erase(iter);
        endRemoveRows();
      }
      else
      {
        iter++;
      }
    }
  }

  void CovatsIndexModelTable::Clear()
  {
    beginResetModel();
    TableData.clear();
    endResetModel();
  }

  void CovatsIndexModelTable::CalculateIndices()
  {
    auto body = new QWidget();
    auto bodyLayout = new QVBoxLayout(body);
    body->setContentsMargins(0,0,0,0);
    bodyLayout->setContentsMargins(0,0,0,0);
    auto label = new QLabel(tr("Введите значение индекса для первого маркера. Система рассчитает индексы для следующих маркеров с шагом +100."));
    label->setWordWrap(true);
    bodyLayout->addWidget(label);
    auto formLayout = new QHBoxLayout();
    bodyLayout->addLayout(formLayout);
    auto formLabel = new QLabel(tr("Индекс для М1"));
    formLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
    formLayout->addWidget(formLabel);
    auto spinBox = new QSpinBox();
    formLayout->addWidget(spinBox);
    spinBox->setMinimum(100);
    spinBox->setMaximum(100000);
    spinBox->setSingleStep(100);

    Dialogs::Templates::Dialog::Settings s;
    s.dialogHeader = tr("Установка начального индекса");
    s.buttonsNames[QDialogButtonBox::Ok] = tr("Рассчитать индексы");
    s.buttonsProperties[QDialogButtonBox::Ok] = {{"blue", true}};

    auto dial = new Dialogs::Templates::Dialog(GenesisWindow::Get(), s, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, body);
    connect(dial, &WebDialog::Accepted, this, [this, spinBox]()
    {
      if (rowCount() == 0)
        return;
      const static auto step = 100.0;
      QList<int> rowsOrder;
      rowsOrder.fill(0, rowCount());
      for (int i = 0; i < rowsOrder.size(); ++i)
        rowsOrder[i] = i;

      std::sort(rowsOrder.begin(), rowsOrder.end(), [this](const int& a, const int& b)->bool
        {
          return data(index(a, 1), Qt::DisplayRole).toDouble() < data(index(b, 1), Qt::DisplayRole).toDouble();
        });

      auto initialIndex = spinBox->value();
      //qDebug() << initialIndex;
      //      if (initialIndex < step)
      //        initialIndex = step;

      for (auto& row : rowsOrder)
      {
        setData(index(row, 2), initialIndex);
        initialIndex += step;
      }
    });
    dial->Open();
  }

  void CovatsIndexModelTable::connectModelSignals(GenesisMarkup::ChromatogrammModelPtr model)
  {
    if (!model)
      return;

    connect(model.get(), &GenesisMarkup::ChromatogramDataModel::modelAboutToReset,
      this, &CovatsIndexModelTable::onModelAboutToReset);
    connect(model.get(), &GenesisMarkup::ChromatogramDataModel::modelReset,
      this, &CovatsIndexModelTable::onModelReset);
    connect(model.get(), &GenesisMarkup::ChromatogramDataModel::entityChanged,
      this, &CovatsIndexModelTable::onEntityChanged);
    connect(model.get(), &GenesisMarkup::ChromatogramDataModel::entityResetted,
      this, &CovatsIndexModelTable::onEntityResetted);
    connect(model.get(), &GenesisMarkup::ChromatogramDataModel::entityAdded,
      this, &CovatsIndexModelTable::onEntityAdded);
    connect(model.get(), &GenesisMarkup::ChromatogramDataModel::entityAboutToRemove,
      this, &CovatsIndexModelTable::onEntityAboutToRemove);
  }
}
