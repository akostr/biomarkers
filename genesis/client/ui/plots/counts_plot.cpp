#include "counts_plot.h"

#include <cassert>
#include <algorithm>

#include "../dialogs/web_dialog_pca_items_settings.h"
#include <ui/plots/gp_items/sample_info_plate.h>
#include <ui/plots/gp_items/gpshape_with_label_item.h>
#include <genesis_style/style.h>
namespace
{
  const QSizeF defaultItemsSize = GPShapeItem::ToDefaultSize(GPShapeItem::VeryBig);
  const std::string_view SampleId = "sample_id";
}

CountsPlot::CountsPlot(QWidget* parentWidget)
  : AnalysisPlot(parentWidget)
  , ColorSortParam(Field)
  , ShapeSortParam(Layer)
  , labelsHidden(false)
  , parametersHidden(false)
  , hasExcludedItems(false)
{
  addLayer("base");
  addLayer("shape", layer("base"));
  addLayer("top", layer("shape"));
  setInteraction(GP::Interaction::iSelectItems, true);
  setMultiSelectModifier(Qt::ShiftModifier);
}

void CountsPlot::setPoints(const QList<T_Title>& titles,
  const Component& firstComp,
  const Component& secondComp)
{
  assert(titles.size() == firstComp.T.size());
  assert(titles.size() == secondComp.T.size());

  const auto shapePixelSize = defaultItemsSize; //temporary hardcoded size

  for (int i = 0; i < firstComp.T.size(); i++)
  {
    QFont font(Style::GetSASSValue("fontFaceNormal"), -1, Style::GetSASSValue("fontWeight").toInt());
    font.setPixelSize(Style::GetSASSValue("fontSizeSecondaryTextScalableFont").toInt());
    auto shape = new GPShapeWithLabelItem(this, { firstComp.T[i], secondComp.T[i] }, GPShape::ShapeType(0), shapePixelSize, "Test label text", font);
    shape->setProperty(SampleId.data(), titles[i].sampleId);
    shape->setLayer("shape");
    shape->setSelectable(true);
    shape->setMargins(5, 5, 5, 5);
    {
      shape->setShapePen(Qt::NoPen);
      shape->setShapeSelectedPen(Qt::NoPen);
      auto brush = QBrush(GPShapeItem::ToColor(GPShapeItem::DefaultColor(0)));
      shape->setColor(GPShapeItem::DefaultColor(0));
      shape->setShapeSelectedBrush(brush);
    }

    {
      shape->setPlatePen(Qt::NoPen);
      shape->setPlateSelectedPen(Qt::NoPen);
      shape->setPlateBrush(Qt::NoBrush);
      shape->setPlateSelectedBrush(QColor(86, 185, 242, 51)); //color from figma
    }

    if (titles[i].filetitle.isEmpty())
    {
      shape->setLabelText(titles[i].filename.section('/', -1));
      if (shape->getLabelText().contains('\\'))
        shape->setLabelText(shape->getLabelText().section('\\', -1));
    }
    else
      shape->setLabelText(titles[i].filetitle);

    auto plate = new SampleInfoPlate(this);
    plate->setSelectable(false);
    plate->topLeft->setParentAnchor(shape->right);
    plate->showParameters(false);

    QPointF identation = QPointF(5, shape->boundingRect().height() / 2 + 2);
    plate->topLeft->setCoords(identation);//identation from point

    plate->showParameters(true);
    plate->SetParameter(Field, titles[i].field);
    plate->SetParameter(Well, titles[i].well);
    plate->SetParameter(Layer, titles[i].layer);
    plate->SetParameter(Date, titles[i].date);
    T_Points.emplace(shape, plate);
    if (titles[i].excluded)
      excludeItem(shape);
  }

  if (ShapeStyle.empty() && ColorStyle.empty())
  {
    SetupDefaultStyle();
  }

  UpdateItemsSizes();
  UpdateItemsColorsBy(ColorSortParam, false);
  UpdateItemsShapesBy(ShapeSortParam);
}

void CountsPlot::clearData()
{
  for (auto& [key, item] : T_Points)
  {
    // Call delete items
    removeItem(item);
    removeItem(key);
  }
  T_Points.clear();
  resetExcludedItems();
}

void CountsPlot::clearStyles()
{
  ShapeStyle.clear();
  ColorStyle.clear();
  CustomItemStyle.clear();
}

void CountsPlot::handleHighlighting(QPointF viewportPos)
{
  auto canEditInactivity = [this](GPAbstractItem* item) -> bool
    {
      const auto it = std::find_if(ExcludedItems.begin(), ExcludedItems.end(),
        [&](const QPair<GPShapeWithLabelItem*, SampleInfoPlate*>& pair)
        {
          return pair.first == item || pair.second == item;
        });
      return it == ExcludedItems.end();
    };
  QList<GPShapeWithLabelItem*> keys;
  for (auto& [key, item] : T_Points)
  {
    auto shapeBoundingRect = key->boundingRect();
    //add margins by 10 pix to each side
    shapeBoundingRect.translate(-10, -10);
    shapeBoundingRect.setWidth(shapeBoundingRect.height() + 10);
    shapeBoundingRect.setHeight(shapeBoundingRect.width());

    if (shapeBoundingRect.contains(viewportPos))
    {
      keys << key;
    }

    if (key->isInactive() && canEditInactivity(key))
    {
      item->SetIsInactive(false);
      key->setInactive(false);
    }
    item->setLayer("base");
  }

  if (keys.isEmpty())
    return;

  auto currentKey = *std::min_element(keys.begin(), keys.end(),
    [](GPShapeWithLabelItem* lt, GPShapeWithLabelItem* rt)
    {
      auto ltShapeBoundingRect = lt->boundingRect();
      auto rtShapeBoundingRect = rt->boundingRect();
      return ltShapeBoundingRect.center().manhattanLength() < rtShapeBoundingRect.center().manhattanLength();
    });

  T_Points[currentKey]->setLayer("top"); //can't be nullptr cause it happens only when items is empty, but if that happens, we will return earlier.

  auto currentCombinedRect = T_Points[currentKey]->BoundingRect().united(currentKey->boundingRect());
  for (auto& [key, item] : T_Points)
  {
    if (key == currentKey)
      continue;
    if (canEditInactivity(key) && (item->BoundingRect().intersects(currentCombinedRect) || key->boundingRect().intersects(currentCombinedRect)))
    {
      item->SetIsInactive(true);
      key->setInactive(true);
    }
  }
}

void CountsPlot::UpdateItemsColorsBy(ParameterType param, bool updateLegend)
{
  for (auto& [key, item] : T_Points)
  {
    auto sampleId = key->property(SampleId.data()).toInt();
    if (CustomItemStyle.contains(sampleId))
    {
      key->setColor(CustomItemStyle[sampleId].color);
      //      key->setShapeBrush(GPShapeItem::ToColor(CustomItemStyle[fileId].color));
    }
  }
  ColorSortParam = param;
  if (ColorStyle.contains(param))
  {
    for (auto& [key, item] : T_Points)
    {
      auto sampleId = key->property(SampleId.data()).toInt();
      if (!CustomItemStyle.contains(sampleId))
      {
        auto val = T_Points[key]->GetParameter(param);
        key->setColor(ColorStyle[param][val]);
      }
    }
  }
  if (updateLegend)
    emit legendChanged();
}

void CountsPlot::UpdateItemsShapesBy(ParameterType param, bool updateLegend)
{
  for (auto& [key, item] : T_Points)
  {
    auto sampleId = key->property(SampleId.data()).toInt();
    if (CustomItemStyle.contains(sampleId))
    {
      key->SetShape(CustomItemStyle[sampleId].shape);
    }
  }
  ShapeSortParam = param;
  if (ShapeStyle.contains(param))
  {
    for (auto& [key, item] : T_Points)
    {
      auto sampleId = key->property(SampleId.data()).toInt();
      if (!CustomItemStyle.contains(sampleId))
      {
        auto val = item->GetParameter(param);
        key->SetShape(ShapeStyle[param][val]);
      }
    }
  }
  if (updateLegend)
    emit legendChanged();
}

void CountsPlot::UpdateItemsSizes()
{
  for (auto& [key, item] : T_Points)
  {
    auto sampleId = key->property(SampleId.data()).toInt();
    if (CustomItemStyle.contains(sampleId))
    {
      key->SetSize(CustomItemStyle[sampleId].size);
    }
    else
    {
      key->SetSize(defaultItemsSize);
    }
  }
}

void CountsPlot::UpdateStyle()
{
  UpdateItemsSizes();
  UpdateItemsColorsBy(ColorSortParam, false);
  UpdateItemsShapesBy(ShapeSortParam);
}

int CountsPlot::GetPointsCount()
{
  return T_Points.size();
}

ColorLegendType CountsPlot::getColorLegend() const
{
  if (!ColorStyle.contains(ColorSortParam))
    return ColorLegendType();
  return ColorStyle[ColorSortParam];
}

ShapeLegendType CountsPlot::getShapeLegend() const
{
  if (!ShapeStyle.contains(ShapeSortParam))
    return ShapeLegendType();
  return ShapeStyle[ShapeSortParam];
}

ColorStyleType CountsPlot::getColorStyle() const
{
  return ColorStyle;
}

ShapeStyleType CountsPlot::getShapeStyle() const
{
  return ShapeStyle;
}

void CountsPlot::setColorStyle(const ColorStyleType& style)
{
  ColorStyle = style;
}

void CountsPlot::setShapeStyle(const ShapeStyleType& style)
{
  ShapeStyle = style;
}

ParameterType CountsPlot::GetColorSortParam() const
{
  return ColorSortParam;
}

ParameterType CountsPlot::GetShapeSortParam() const
{
  return ShapeSortParam;
}

void CountsPlot::SetupStyle(const QList<T_Title>& titles)
{
  ColorStyle.clear();
  ShapeStyle.clear();
  CustomItemStyle.clear();
  for (int i = 0; i < ParameterType::LastType; i++)
  {
    auto param = (ParameterType)i;
    for (auto& pt : titles)
    {
      auto val = pt.getParamValue(param);
      if (!ColorStyle[param].contains(val))
      {
        ColorStyle[param].insert(val, GPShapeItem::GetCycledColorIndex(ColorStyle[param].count()));
      }
      if (!ShapeStyle[param].contains(val))
      {
        ShapeStyle[param].insert(val, GPShapeItem::GetCycledShape(ShapeStyle[param].count()));
      }
    }
  }
  {
    auto param = ParameterType::LastType;
    auto val = "";
    if (!ColorStyle[param].contains(val))
    {
      ColorStyle[param].insert(val, GPShapeItem::DefaultColor(0));
    }
    if (!ShapeStyle[param].contains(val))
    {
      ShapeStyle[param].insert(val, GPShape::Square);
    }
  }
}

void CountsPlot::ShowNames(bool isShow)
{
  labelsHidden = !isShow;

  for (auto& [key, item] : T_Points)
    key->setLabelHidden(labelsHidden);

  queuedReplot();
}

void CountsPlot::ShowPassport(bool isShow)
{
  parametersHidden = !isShow;
  for (auto& [key, item] : T_Points)
    item->showParameters(!parametersHidden);
  queuedReplot();
}

void CountsPlot::SetupDefaultStyle()
{
  ColorStyle.clear();
  ShapeStyle.clear();
  for (int i = 0; i < static_cast<int>(ParameterType::LastType); i++)
  {
    const auto param = static_cast<ParameterType>(i);
    for (auto& [key, pt] : T_Points)
    {
      auto val = pt->GetParameter(param);
      if (!ColorStyle[param].contains(val))
      {
        ColorStyle[param].insert(val, GPShapeItem::GetCycledColorIndex(ColorStyle[param].count()));
      }
      if (!ShapeStyle[param].contains(val))
      {
        ShapeStyle[param].insert(val, GPShapeItem::GetCycledShape(ShapeStyle[param].count()));
      }
    }
  }
  {
    auto param = ParameterType::LastType;
    for (auto& [key, pt] : T_Points)
    {
      auto val = pt->GetParameter(param);
      if (!ColorStyle[param].contains(val))
      {
        ColorStyle[param].insert(val, GPShapeItem::DefaultColor(0));
      }
      if (!ShapeStyle[param].contains(val))
      {
        ShapeStyle[param].insert(val, GPShape::Square);
      }
    }
  }
  UpdateItemsSizes();
  UpdateItemsColorsBy(ColorSortParam, false);
  UpdateItemsShapesBy(ShapeSortParam);
}


void CountsPlot::customizeSampleRequest(GPShapeWithLabelItem* item)
{
  auto dial = new WebDialogPCAItemsSettings(this, { item }, this);
  dial->Open();
}
void CountsPlot::customizeSelectedSamplesRequest(QList<GPAbstractItem*> items)
{
  QList<GPShapeWithLabelItem*> Items;
  for (auto& item : items)
  {
    if (auto i = dynamic_cast<GPShapeWithLabelItem*>(item))
      Items.append(i);
  }
  auto dial = new WebDialogPCAItemsSettings(this, Items, this);
  dial->Open();
}

void CountsPlot::resetCustomStyles()
{
  CustomItemStyle.clear();
  UpdateItemsSizes();
  UpdateItemsColorsBy(ColorSortParam, false);
  UpdateItemsShapesBy(ShapeSortParam);
}

void CountsPlot::excludeItem(GPShapeWithLabelItem* item)
{
  auto plate = T_Points[item];
  ExcludedItems.emplace(item, plate);
  plate->SetIsInactive(true);
  item->setInactive(true);
}

void CountsPlot::excludeItems(QList<GPShapeWithLabelItem*> items)
{
  for (auto& item : items)
    excludeItem(item);

  updateExcludedState();
}

void CountsPlot::resetExcludedItems()
{
  for (auto& pair : ExcludedItems)
  {
    if (hasItem(pair.first))
      pair.first->setInactive(false);
    if (hasItem(pair.second))
      pair.second->SetIsInactive(false);
  }
  ExcludedItems.clear();
  updateExcludedState();
}

void CountsPlot::resetExcludedItems(QList<GPShapeWithLabelItem*> items)
{
  for (auto& pair : ExcludedItems)
  {
    if (items.contains(pair.first))
    {
      pair.first->setInactive(false);
      pair.second->SetIsInactive(false);
    }
  }
  for (auto item : items)
    ExcludedItems.erase(item);
  updateExcludedState();
}

void CountsPlot::updateExcludedState()
{
  bool HasExcludedCheck = !ExcludedItems.empty();
  hasExcludedItems = HasExcludedCheck;
  emit excludedStateChanged(hasExcludedItems);
}

const CustomItemStyleMap& CountsPlot::getCustomItemStyle() const
{
  return CustomItemStyle;
}

GPShapeWithLabelItem* CountsPlot::findItemByFileId(int id)
{
  for (auto& [key, item] : T_Points)
  {
    if (key->property(SampleId.data()).toInt() == id)
      return key;
  }
  return nullptr;
}

const std::map<GPShapeWithLabelItem*, SampleInfoPlate*>& CountsPlot::GetExcludedItems() const
{
  return ExcludedItems;
}

void CountsPlot::SetCustomItemStyle(const CustomItemStyleMap& newCustomItemStyle)
{
  CustomItemStyle.insert(newCustomItemStyle);
}

void CountsPlot::ResetItemStyleToDefault(const QList<GPShapeWithLabelItem*>& items, bool update)
{
  for (auto& item : items)
  {
    CustomItemStyle.remove(item->property(SampleId.data()).toInt());
  }
  if(update)
  {
    UpdateStyle();
  }
}

void CountsPlot::FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void()> >& actions)
{
  auto selected = selectedItems();
  auto item = qobject_cast<GPShapeWithLabelItem*>(itemAt(event->pos(), true));
  if (!item && selected.empty())
  {
    //default plot context menu
    menu.setHidden(true);
    auto nameAct = menu.addAction(tr("Count plot"));
    nameAct->setEnabled(false);

  }
  else
  {
    menu.setHidden(false);
    if ((item && !selected.contains(item)) ||
      (!item && selected.size() == 1) ||
      (selected.contains(item) && selected.size() == 1)
      )
    {
      if (!item && selected.size() == 1)
        item = qobject_cast<GPShapeWithLabelItem*>(selected.first());
      //item menu
      auto nameAct = menu.addAction(item->getLabelText().toUpper());
      nameAct->setEnabled(false);
      menu.addSeparator();

      QAction* customizeSampleView = menu.addAction(tr("Customize sample view"));
      actions[customizeSampleView] = std::bind(&CountsPlot::customizeSampleRequest, this, item);
      customizeSampleView->setEnabled(true);

      if (ExcludedItems.empty())
      {
        QAction* pickForModel = menu.addAction(tr("Pick for new model"));
        actions[pickForModel] = [&, item]()
          {
            QList<GPShapeWithLabelItem*> excluded;
            std::vector<int> sampleIds;
            sampleIds.reserve(T_Points.size());
            for (const auto& [point, info] : T_Points)
            {
              if (item != point)
              {
                excluded.append(point);
                sampleIds.push_back(point->property(SampleId.data()).toInt());
              }
            }
            excludeItems(excluded);
            emit setSelectedSamplesForExcludingFromModel(sampleIds);
          };
      }
      bool bItemIsNonExcluded = true;
      for (auto& pair : ExcludedItems)
      {
        if (pair.first == item)
        {
          bItemIsNonExcluded = false;
          break;
        }
      }
      if (bItemIsNonExcluded)
      {
        QAction* excludeFromModel = menu.addAction(tr("Pick for excluding from model"));
        actions[excludeFromModel] = [&, item]()
          {
            excludeItems({ item });
            emit setSelectedSamplesForExcludingFromModel({ item->property(SampleId.data()).toInt() });
          };
        excludeFromModel->setEnabled(true);
      }
      else
      {
        QAction* reincludeToModel = menu.addAction(tr("Reset excluding from model"));
        actions[reincludeToModel] = [&, item]()
          {
            resetExcludedItems({ item });
            emit excludedItemsReset({ item->property(SampleId.data()).toInt() });
          };
        reincludeToModel->setEnabled(true);
      }

    }
    else
    {
      //whole selected items context menu

      auto nameAct = menu.addAction(tr("PICKED %n SAMPLES", "", selected.size()));
      nameAct->setEnabled(false);
      menu.addSeparator();

      QAction* customizeSamplesView = menu.addAction(tr("Customize samples view"));
      actions[customizeSamplesView] = std::bind(&CountsPlot::customizeSelectedSamplesRequest, this, selected);
      customizeSamplesView->setEnabled(true);

      QAction* showInfo = menu.addAction(tr("View samples information"));
      actions[showInfo] = std::bind(&CountsPlot::showSelectedSamplesInfo, this, selected);
      showInfo->setEnabled(true);
      if (ExcludedItems.empty())
      {
        QAction* pickForModel = menu.addAction(tr("Pick for new model"));
        actions[pickForModel] = [&, selected]()
          {
            QList<GPShapeWithLabelItem*> excluded;
            std::vector<int> fileIds;
            fileIds.reserve(T_Points.size());
            for (const auto& [point, info] : T_Points)
            {
              if (!selected.contains(point))
              {
                excluded.append(point);
                fileIds.push_back(point->property(SampleId.data()).toInt());
              }
            }
            excludeItems(excluded);
            emit setSelectedSamplesForExcludingFromModel(fileIds);
          };
        pickForModel->setEnabled(true);
      }
      bool bHasNonExcludedItem;
      {
        int selectedShapes = 0;
        for (auto& item : selected)
        {
          if (qobject_cast<GPShapeWithLabelItem*>(item))
            selectedShapes++;
        }
        for (auto& pair : ExcludedItems)
        {
          if (selected.contains(pair.first))
            selectedShapes--;
        }
        //if selected items contains any shape, that not in excluded list, than selectedShapes will > 0
        bHasNonExcludedItem = selectedShapes > 0;
      }
      if (bHasNonExcludedItem)
      {
        QAction* excludeFromModel = menu.addAction(tr("Pick for excluding from model"));
        actions[excludeFromModel] = [&, selected]()
          {
            QList< GPShapeWithLabelItem*> labelItems;
            labelItems.reserve(selected.size());
            std::transform(selected.begin(), selected.end(), std::back_inserter(labelItems),
              [&](GPAbstractItem* item) { return qobject_cast<GPShapeWithLabelItem*>(item); });
            excludeItems(labelItems);
            std::vector<int> fileIds;
            fileIds.reserve(selected.size());
            std::transform(labelItems.begin(), labelItems.end(), std::back_inserter(fileIds),
              [&](GPShapeWithLabelItem* item) { return item->property(SampleId.data()).toInt(); });
            emit setSelectedSamplesForExcludingFromModel(fileIds);
          };
        excludeFromModel->setEnabled(true);
      }

      bool bHasExcludedItems = false;
      {
        for (auto& pair : ExcludedItems)
          if (selected.contains(pair.first) || selected.contains(pair.second))
          {
            bHasExcludedItems = true;
            break;
          }
      }
      if (bHasExcludedItems)
      {
        QAction* reincludeToModel = menu.addAction(tr("Reset excluding from model"));
        actions[reincludeToModel] = [&, selected]()
          {
            QList<GPShapeWithLabelItem*> labelItems;
            labelItems.reserve(selected.size());
            std::transform(selected.begin(), selected.end(), std::back_inserter(labelItems),
              [&](GPAbstractItem* item) { return qobject_cast<GPShapeWithLabelItem*>(item); });

            resetExcludedItems(labelItems);
            std::vector<int> ids;
            ids.reserve(labelItems.size());
            std::transform(labelItems.begin(), labelItems.end(), std::back_inserter(ids),
              [&](const GPShapeWithLabelItem* item) { return item->property(SampleId.data()).toInt(); });
            emit excludedItemsReset(ids);
          };
        reincludeToModel->setEnabled(true);
      }
    }
  }
}

void CountsPlot::mousePressEvent(QMouseEvent* e)
{
  AnalysisPlot::mousePressEvent(e);
  if (e->button() == Qt::LeftButton)
    leftMouseButtonPressPosition = e->pos();
}

void CountsPlot::mouseReleaseEvent(QMouseEvent* e)
{
  AnalysisPlot::mouseReleaseEvent(e);
  if (labelsHidden || parametersHidden)
  {
    if (e->button() == Qt::LeftButton)
    {
      if (labelsHidden)
        ShowNames(false);
      if (parametersHidden)
        ShowPassport(false);

      auto item = itemAt(leftMouseButtonPressPosition, true);
      if (item && item == itemAt(e->pos(), true))
      {
        auto shape = dynamic_cast<GPShapeWithLabelItem*>(item);
        if (shape)
        {
          if (labelsHidden)
            shape->setLabelHidden(false);
          if (parametersHidden)
            T_Points[shape]->showParameters(true);
        }
      }
    }
  }
}

bool CountsPlot::getHasExcludedItems() const
{
  return hasExcludedItems;
}
