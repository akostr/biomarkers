#include "legend_editor.h"
#include "ui_legend_editor.h"

#include "logic/models/analysis_entity_model.h"
#include "legend_parameter_table_model.h"
#include "ui/controls/spinbox_item_delegate.h"
#include "ui/controls/combobox_item_delegate.h"
#include "logic/notification.h"
#include "ui/item_models/shape_item_model.h"
#include "ui/item_models/color_item_model.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class RowFilterProxy : public QSortFilterProxyModel
{
public:
  RowFilterProxy(QObject *parent = nullptr): QSortFilterProxyModel(parent){};
  void setRejectedRows(int row){mRejectedRow = row; invalidate();};

private:
  int mRejectedRow;
protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
  {
    return mRejectedRow != source_row;
  }
};

using BB = QDialogButtonBox::StandardButton;
using namespace AnalysisEntity;

namespace Dialogs
{
LegendEditor::LegendEditor(QPointer<AnalysisEntityModel> model, QWidget* parent,
                           std::optional<TPassportFilter> colorFilter,
                           std::optional<TPassportFilter> shapeFilter)
 : Dialog(parent, BB::Ok | BB::Cancel),
  ui(new Ui::LegendEditor),
  mContent(new QWidget(nullptr)),
  mInitialColorFilter(colorFilter),
  mInitialShapeFilter(shapeFilter)
{
  setupUi();
  connectSignals();
  setModel(model);
}

LegendEditor::~LegendEditor()
{
  delete ui;
}

std::optional<TPassportFilter> LegendEditor::colorFilter()
{
  return makeFilter("grouped by", ui->colorComboBox->currentData().toString());
}

std::optional<TPassportFilter> LegendEditor::shapeFilter()
{
   return makeFilter("grouped by", ui->shapeComboBox->currentData().toString());
}

QList<TLegendGroup> LegendEditor::getGroups()
{
  if (ui->colorComboBox->currentIndex() == -1 &&
    ui->shapeComboBox->currentIndex() == -1)
    return {};
  return mTableModel->getLegendData();
}

void LegendEditor::setModel(QPointer<AnalysisEntityModel> model)
{
  mModel = model;
  auto ph = model->modelData(ModelRolePassportHeaders).toStringList();
  auto pk = model->modelData(ModelRolePassportKeys).toStringList();
  pk.prepend("no filter");
  ph.prepend(tr("Not selected"));
  mParamsModel->clear();
  QList<QStandardItem*> items;
  for(int i = 0; i < std::min(ph.size(), pk.size()); i++)
  {
    auto item = new QStandardItem(ph[i]);
    item->setData(pk[i], Qt::UserRole);
    items << item;
  }
  mParamsModel->invisibleRootItem()->insertRows(0, items);
  if (mInitialColorFilter.has_value() && !mInitialColorFilter->isEmpty())
  {
    int ind = ui->colorComboBox->findData(mInitialColorFilter->begin().key());
    ui->colorComboBox->setCurrentIndex(ind);
    removeItemFromShape();
  }
  else
  {
    ui->colorComboBox->setCurrentIndex(0);
  }
  if (mInitialShapeFilter.has_value() && !mInitialShapeFilter->isEmpty())
  {
    int ind = ui->colorComboBox->findData(mInitialShapeFilter->begin().key());
    ui->shapeComboBox->setCurrentIndex(mShapesProxy->mapFromSource(mParamsModel->index(ind, 0)).row());
    removeItemFromColor();
  }
  else
  {
    ui->shapeComboBox->setCurrentIndex(0);
  }
}

void LegendEditor::setupUi()
{
  ui = new Ui::LegendEditor();
  ui->setupUi(mContent);
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Parameters start group");
  s.buttonsNames = {{BB::Ok, tr("Submit")}};
  applySettings(s);

  mParamsModel = new QStandardItemModel(0, 1, this);
  mColorsProxy = new RowFilterProxy(ui->colorComboBox);
  mColorsProxy->setSourceModel(mParamsModel);
  mShapesProxy = new RowFilterProxy(ui->shapeComboBox);
  mShapesProxy->setSourceModel(mParamsModel);
  ui->colorComboBox->setModel(mColorsProxy);
  ui->shapeComboBox->setModel(mShapesProxy);
  getContent()->layout()->addWidget(mContent);
  Size = QSizeF(0.7, 0.3);
  if (auto hh = ui->tableView->horizontalHeader())
  {
    hh->setSectionResizeMode(QHeaderView::Stretch);
    hh->setVisible(false);
  }
  if (auto vh = ui->tableView->verticalHeader())
  {
    vh->setSectionResizeMode(QHeaderView::ResizeToContents);
    vh->setVisible(false);
  }

  mTableModel = new LegendParameterTableModel();
  ui->tableView->setModel(mTableModel);

  const auto colorItemDelegate = new ComboboxItemDelegate();
  colorItemDelegate->setModel(new ColorItemModel());
  ui->tableView->setItemDelegateForColumn(1, colorItemDelegate);

  const auto shapeItemDelegate = new ComboboxItemDelegate();
  shapeItemDelegate->setModel(new ShapeItemModel());
  ui->tableView->setItemDelegateForColumn(2, shapeItemDelegate);
  ui->tableView->setItemDelegateForColumn(3, new SpinboxItemDelegate());
  ui->tableView->setStyleSheet("QTableView::item{ padding: 2xp }");
  ui->tableView->viewport()->installEventFilter(this);
}

void LegendEditor::connectSignals()
{
  connect(ui->shapeComboBox, &QComboBox::currentIndexChanged, this, &LegendEditor::updateLegendItems);
  connect(ui->shapeComboBox, &QComboBox::currentIndexChanged, this, &LegendEditor::removeItemFromColor);
  connect(ui->colorComboBox, &QComboBox::currentIndexChanged, this, &LegendEditor::updateLegendItems);
  connect(ui->colorComboBox, &QComboBox::currentIndexChanged, this, &LegendEditor::removeItemFromShape);
  connect(ui->tableView, &QTableView::doubleClicked, this,
    [&](const QModelIndex& index)
    {
      mTableModel->hideIndexDecorations(index);
    });
}

void LegendEditor::updateLegendItems()
{
  const auto colorParam = ui->colorComboBox->currentData().toString();
  const auto shapeParam = ui->shapeComboBox->currentData().toString();
  const auto colorInd = ui->colorComboBox->currentIndex();
  const auto shapeInd = ui->shapeComboBox->currentIndex();
  auto samples = mModel->getEntities(TypeSample);

  QList<TLegendGroup> group;
  QScopeGuard gurad([&]() { mTableModel->setLegendData(group); });

  if ((colorInd == 0) && (shapeInd == 0))
  {
    for (int index = 0; index < samples.size(); index++)
    {
      const auto passport = samples[index]->getData(RolePassport).value<TPassport>();
      TLegendGroup legendItem;
      legendItem.title = samples[index]->getData(RoleTitle).toString();
      legendItem.tooltip = AnalysisEntityModel::passportToSampleTitle(passport);
      legendItem.color = GPShapeItem::GetCycledColor(index);
      legendItem.shape.path = GPShape::GetPath(GPShapeItem::GetCycledShape(static_cast<GPShape::ShapeType>(index)));
      legendItem.shape.pixelSize = 8;
      legendItem.uid = samples[index]->getUid();
      group.emplace_back(legendItem);
    }
  }
  else
  {
    QSet<QString> uniqueColorParameter;
    QSet<QString> uniqueShapeParameter;
    const auto ent = mModel->getEntities(AnalysisEntity::EntityTypes::TypeSample);
    for (auto& item : ent)
    {
      auto passport = item->getData(AnalysisEntity::RolePassport).value<TPassport>();
      if(colorInd > 0)
        uniqueColorParameter.insert(passport.value(ui->colorComboBox->currentData().toString()).toString());
      if(shapeInd > 0)
        uniqueShapeParameter.insert(passport.value(ui->shapeComboBox->currentData().toString()).toString());
    }
    QList<TPassportFilter> filters;
    if (uniqueColorParameter.isEmpty())
    {
      for (auto& s : uniqueShapeParameter)
        filters.append(makeFilter(s, shapeParam));
    }
    else if (uniqueShapeParameter.isEmpty())
    {
      for (auto& s : uniqueColorParameter)
        filters.append(makeFilter(s, colorParam));
    }
    else
    {
      for (auto& colorVal : uniqueColorParameter)
      {
        for (auto& shapeVal : uniqueShapeParameter)
        {
          filters.append(makeFilter(shapeVal, shapeParam).concatMissed(makeFilter(colorVal, colorParam)));
        }
      }
    }

    filters.removeIf([&](const TPassportFilter& filter) ->bool
      {
        const auto findIt = std::find_if(samples.begin(), samples.end(),
          [&](AbstractEntityDataModel::ConstDataPtr sample)
          {
            return filter.match(sample->getData(RolePassport).value<TPassport>());
          });
        return  findIt == samples.end();
      });
    for (int index = 0; index < filters.size(); index++)
    {
      const auto title = AnalysisEntityModel::genericGroupName(colorFilter().value_or(TPassportFilter()),
        shapeFilter().value_or(TPassportFilter()), filters[index]);
      TLegendGroup legendItem;
      legendItem.title = title;
      legendItem.tooltip = title;
      legendItem.filter = filters[index];
      legendItem.color = GPShapeItem::GetCycledColor(index);
      legendItem.shape.path = GPShape::GetPath(GPShapeItem::GetCycledShape(static_cast<GPShape::ShapeType>(index)));
      legendItem.shape.pixelSize = 8;
      group.emplace_back(legendItem);
    }
  }
  Size.setHeight(mTableModel->rowCount() > 10 ? .8 : 0.3);
  UpdateGeometry();
}

void LegendEditor::updateLegendItemsFromModel()
{
  if (!mModel)
    return;
  const auto groupsUids = mModel->getEntitiesUidsSet(
    [](AbstractEntityDataModel::ConstDataPtr data) ->bool
    {
      return data->getType() == TypeGroup
        && !data->getData(RoleGroupUserCreated).toBool();
    });
  QMap<TEntityUid, TEntityUid> mGroupSampleMap;
  if (mInitialShapeFilter->isEmpty() && mInitialColorFilter->isEmpty())
  {
    for (auto& sample : mModel->getEntities(TypeSample))
    {
      auto uidsSet = sample->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
      uidsSet.intersect(groupsUids);
      if (!uidsSet.isEmpty())
        mGroupSampleMap.insert(*uidsSet.begin(), sample->getUid());
    }
  }
  QList<TLegendGroup> legendGroups;
  for (auto& groupUid : groupsUids)
  {
    const auto group = mModel->getEntity(groupUid);
    const auto title = group->getData(RoleTitle).toString();
    const auto tooltip = group->getData(RoleGroupTooltip).toString();
    TLegendGroup legendGroup;
    legendGroup.title = title;
    legendGroup.tooltip = tooltip;
    legendGroup.filter = group->getData(RolePassportFilter).value<TPassportFilter>();
    legendGroup.color = group->getData(RoleColor).value<QColor>();
    legendGroup.shape.path = group->getData(RoleShape).value<TShape>();
    legendGroup.shape.pixelSize = group->getData(RoleShapePixelSize).toInt();
    if (!mGroupSampleMap.isEmpty())
    {
      legendGroup.uid = mGroupSampleMap.value(groupUid, QUuid());
    }
    legendGroups.emplace_back(legendGroup);
  }
  mTableModel->setLegendData(legendGroups);
  Size.setHeight(mTableModel->rowCount() > 10 ? .8 : 0.3);
  UpdateGeometry();
}

void LegendEditor::removeItemFromColor()
{
  const auto currentRow = ui->shapeComboBox->currentIndex();
  if(currentRow <= 0)
  {
    mColorsProxy->setRejectedRows(-1);
    return;
  }
  auto index = mShapesProxy->index(currentRow, 0);
  auto sourceRow = mShapesProxy->mapToSource(index).row();
  mColorsProxy->setRejectedRows(sourceRow);
}

void LegendEditor::removeItemFromShape()
{
  const auto currentRow = ui->colorComboBox->currentIndex();
  if(currentRow <= 0)
  {
    mShapesProxy->setRejectedRows(-1);
    return;
  }
  auto index = mColorsProxy->index(currentRow, 0);
  auto sourceRow = mColorsProxy->mapToSource(index).row();
  mShapesProxy->setRejectedRows(sourceRow);
}

bool LegendEditor::eventFilter(QObject* object, QEvent* event)
{
  if (event->isInputEvent())
  {
    if (const auto inputEvent = static_cast<QInputEvent*>(event))
    {
      if (inputEvent->deviceType() == QInputDevice::DeviceType::Mouse)
      {
        if (const auto mouseEvent = static_cast<QMouseEvent*>(event))
        {
          if (mouseEvent->buttons().testFlag(Qt::RightButton)
            || mouseEvent->flags().testFlag(Qt::MouseEventFlag::MouseEventCreatedDoubleClick))
            return true;
        }
      }
    }
  }
  return false;
}

TPassportFilter LegendEditor::makeFilter(const QString &val, const QString& key)
{
  TPassportFilter res;
  if(mModel->modelData(ModelRolePassportKeys).toStringList().contains(key))
    res[key] = val;
  return res;
}
}//namespace Dialogs
