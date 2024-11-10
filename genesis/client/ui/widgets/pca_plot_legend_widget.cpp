#include "pca_plot_legend_widget.h"
#include "logic/models/analysis_entity_model.h"
#include <ui/widgets/legend/legend_shape_item.h>
#include "ui/flow_layout.h"

using namespace AnalysisEntity;

PcaPlotLegendWidget::PcaPlotLegendWidget(QWidget* parent)
  : PlotLegendWidgetRefactor(parent)
{}

void PcaPlotLegendWidget::setModel(QPointer<AnalysisEntityModel> model)
{
  if (model == mModel)
    return;

  if (mModel)
  {
    mModelConnections.clear();
    disconnect(mModel, nullptr, this, nullptr);
  }
  mModel = model;
  if (mModel)
  {
    connect(mModel, &AnalysisEntityModel::modelReset, this, &PcaPlotLegendWidget::onModelReset);
  }
  onModelReset();
}

void PcaPlotLegendWidget::setHottelingMode(bool inHottelingMode)
{
  if (inHottelingMode != mInHottelingMode)
  {
    mInHottelingMode = inHottelingMode;
    if (mInHottelingMode)
      mNonParticipantsGroupUid = mModel->getEntities([](AbstractEntityDataModel::ConstDataPtr ent)->bool
        {
          return ent->getType() == TypeHottelingGroup && !ent->getData(RoleGroupUserCreated).toBool();
        }).first()->getUid();
    else
      mNonParticipantsGroupUid = QUuid();
    onModelReset();
  }
}

void PcaPlotLegendWidget::onEntityChanged(EntityType type, TEntityUid eId, DataRoleType role, const QVariant& value)
{
  if ((!mInHottelingMode && type == TypeGroup)
    || (mInHottelingMode && type == TypeHottelingGroup))
  {
    switch (role)
    {
    case RoleShape:
      setShape(eId, value.value<TShape>());
      break;
    case RoleColor:
      setColor(eId, value.value<TColor>());
      break;
    case RoleTitle:
    {
      QString tooltip;
      // qDebug() << "rename group by group" << eId << formTitle(eId);
      setTitle(eId, formTitle(eId, &tooltip));
      setToolTip(eId, tooltip);
      break;
    }
    default:
      break;
    }
  }
  else if (mInHottelingMode && type == TypeSample && role == RoleGroupsUidSet)
  {
    // qDebug() << "recalc groups by entities";
    for (auto& ent : mModel->getEntities(TypeHottelingGroup))
    {
      QString tooltip;
      auto uid = ent->getUid();
      // qDebug() << "rename group" << uid << formTitle(uid);
      setTitle(uid, formTitle(uid, &tooltip));
      setToolTip(uid, tooltip);
    }
  }
}

void PcaPlotLegendWidget::onEntityResetted(EntityType type, TEntityUid eId, AbstractEntityDataModel::ConstDataPtr data)
{
  if ((!mInHottelingMode && type == TypeGroup)
    || (mInHottelingMode && type == TypeHottelingGroup))
  {
    auto widgets = mItemWidgets.value(eId, {});
    for (auto& w : widgets)
    {
      w->setShape(data->getData(RoleShape).value<TShape>());
      w->setShapeBrush(data->getData(RoleColor).value<TColor>());
      QString tooltip;
      // qDebug() << "reset item" << eId << formTitle(data);
      w->setText(formTitle(data, &tooltip));
      w->setToolTip(tooltip);
    }
  }
}

void PcaPlotLegendWidget::onEntityAdded(EntityType type, TEntityUid eId, AbstractEntityDataModel::ConstDataPtr data)
{
  if ((!mInHottelingMode && type == TypeGroup)
    || (mInHottelingMode && type == TypeHottelingGroup))
  {
    LegendItem item;
    item.itemId = eId;
    item.shape = data->getData(RoleShape).value<TShape>();
    item.color = data->getData(RoleColor).value<TColor>();
    item.title = formTitle(data, &item.tooltip);
    int layoutId = 0;//hardcode
    // qDebug() << "create item" << item.itemId << item.title;
    addItem(layoutId, item, true);
  }
}

void PcaPlotLegendWidget::onEntityAboutToRemove(EntityType type, TEntityUid eId)
{
  if ((!mInHottelingMode && type == TypeGroup)
    || (mInHottelingMode && type == TypeHottelingGroup))
  {
    int layoutId = 0;//hardcode
    removeItem(layoutId, eId);
  }
}

void PcaPlotLegendWidget::onModelAboutToReset()
{
  if (mModelConnections.isEmpty())
    return;
  for (auto& conn : mModelConnections)
    disconnect(conn);
  mModelConnections.clear();
}

void PcaPlotLegendWidget::onModelReset()
{
  if (mModelConnections.isEmpty())
  {
    mModelConnections << connect(mModel, &AnalysisEntityModel::entityChanged, this, &PcaPlotLegendWidget::onEntityChanged);
    mModelConnections << connect(mModel, &AnalysisEntityModel::entityResetted, this, &PcaPlotLegendWidget::onEntityResetted);
    mModelConnections << connect(mModel, &AnalysisEntityModel::entityAdded, this, &PcaPlotLegendWidget::onEntityAdded);
    mModelConnections << connect(mModel, &AnalysisEntityModel::entityAboutToRemove, this, &PcaPlotLegendWidget::onEntityAboutToRemove);
    mModelConnections << connect(mModel, &AnalysisEntityModel::modelAboutToReset, this, &PcaPlotLegendWidget::onModelAboutToReset);
    mModelConnections << connect(mModel, &AnalysisEntityModel::groupColorChanged, this, &PcaPlotLegendWidget::scanModel);
    mModelConnections << connect(mModel, &AnalysisEntityModel::groupShapeChanged, this, &PcaPlotLegendWidget::scanModel);
  }

  scanModel();
}

void PcaPlotLegendWidget::scanModel()
{
  clear();
  if (!mModel)
    return;

  const TEntityType type = mInHottelingMode ? TypeHottelingGroup : TypeGroup;
  const auto samples = mModel->getEntities(TypeSample);
  TGroupsUidSet groupsFromSamples;
  for (const auto& sample : samples)
  {
    groupsFromSamples.unite(sample->getData(RoleGroupsUidSet).value<TGroupsUidSet>());
  }
  const auto entities = mModel->getEntities([&](AnalysisEntityModel::ConstDataPtr item) -> bool
    {
      return item->getType() == type && groupsFromSamples.contains(item->getUid());
    });

  QList<LegendItem> items;
  for (const auto& entity : entities)
  {
    LegendItem item;
    item.itemId = entity->getUid();
    item.shape = entity->getData(RoleShape).value<TShape>();
    item.color = entity->getData(RoleColor).value<TColor>();
    item.title = formTitle(entity, &item.tooltip);
    items << item;
  }

  addLayout(items);
  QMetaObject::invokeMethod(this, &PcaPlotLegendWidget::expand, Qt::QueuedConnection);
}

QString PcaPlotLegendWidget::formTitle(AbstractEntityDataModel::ConstDataPtr entity, QString* tooltip)
{
  QString title;
  if (mInHottelingMode)
  {
    auto groupTitle = entity->getData(RoleTitle).value<TGroupName>();
    if (mNonParticipantsGroupUid != entity->getUid())
      title = QString("%1(<a href=\"#\">%2</a>)").arg(groupTitle);
    else
      title = QString("%1(%2)").arg(groupTitle);
    auto samplesCount = mModel->countEntities([entity](AbstractEntityDataModel::ConstDataPtr ent)->bool
      {
        return ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>().contains(entity->getUid());
      });
    title = title.arg(samplesCount);
    if (tooltip)
    {
      if (mNonParticipantsGroupUid != entity->getUid())
        *tooltip = formToolTip(samplesCount, groupTitle);
      else
        *tooltip = QString();
    }
  }
  else
  {
    if (tooltip)
      *tooltip = QString();
    title = entity->getData(RoleTitle).value<TGroupName>();
  }
  return title;
}

QString PcaPlotLegendWidget::formTitle(TEntityUid eId, QString* tooltip)
{
  return formTitle(mModel->getEntity(eId), tooltip);
}

QString PcaPlotLegendWidget::formToolTip(int samplesCount, const QString& groupTitle)
{
  if (samplesCount < 2)
    return tr("Group \"%1\" contains %n sample(s), it's not enough for calculation", "", samplesCount).arg(groupTitle);
  else
    return tr("Group \"%1\" contains enough samples for calculation").arg(groupTitle);
}

void PcaPlotLegendWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
}
