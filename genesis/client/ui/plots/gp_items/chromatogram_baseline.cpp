#include "chromatogram_baseline.h"
#include <ui/plots/chromatogram_plot.h>
#include <logic/markup/commands.h>
#include <logic/markup/baseline_data_model.h>

namespace GenesisMarkup
{
ChromatogramBaseline::ChromatogramBaseline(ChromatogramPlot *Plot, QUuid uid)
  : GPInteractiveCurve(Plot->xAxis, Plot->yAxis),
    mUid(uid),
    mParentPlot(Plot)
{
  QColor defaultBaselineColor = {235, 51, 51, 255};
  QColor selectedScatterBaselineColor = GraphicsPlotExtendedStyle::GetDefaultColor(2);
  setSelectable(GP::stSingleData);
  setPen(defaultBaselineColor);

  auto baseScatter = scatterStyle();
  baseScatter.setShape(GPScatterStyle::ssCircle);
  baseScatter.setSize(4);
  baseScatter.setPen(defaultBaselineColor);
  baseScatter.setBrush(defaultBaselineColor);
  setScatterStyle(baseScatter);

  auto dec = selectionDecorator();
  baseScatter.setPen(selectedScatterBaselineColor);
  baseScatter.setBrush(selectedScatterBaselineColor);
  dec->setScatterStyle(baseScatter, GPScatterStyle::spAll);
}

ChromatogramBaseline::~ChromatogramBaseline()
{

}

//функция поворота линии
private:
    QPointF mCenter;  // Центр линии (точка поворота)
    bool mIsRotating = false;  // Флаг, указывающий, что линия в процессе поворота
    double mInitialAngle = 0.0;  // Начальный угол при нажатии

void ChromatogramBaseline::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        mCenter = calculateLineCenter();  // Вычисляем центр линии
        QPointF mousePos = event->scenePos();
        mInitialAngle = std::atan2(mousePos.y() - mCenter.y(), mousePos.x() - mCenter.x());  // Начальный угол
        mIsRotating = true;  // Устанавливаем флаг на начало поворота
    }
    QGraphicsItem::mousePressEvent(event);  // Стандартное поведение для нажатия
}

void ChromatogramBaseline::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (mIsRotating) {
        QPointF mousePos = event->scenePos();
        double currentAngle = std::atan2(mousePos.y() - mCenter.y(), mousePos.x() - mCenter.x());  // Текущий угол
        double deltaAngle = currentAngle - mInitialAngle;  // Разница углов
        RotateLine(deltaAngle);  // Поворот линии
    }
    QGraphicsItem::mouseMoveEvent(event);  // Стандартное поведение для перемещения
}

void ChromatogramBaseline::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (mIsRotating) {
        mIsRotating = false;  // Завершаем поворот
    }
    QGraphicsItem::mouseReleaseEvent(event);  // Стандартное поведение для отпускания кнопки
}

void ChromatogramBaseline::RotateLine(double deltaAngle) {
    QTransform transform;
    transform.translate(mCenter.x(), mCenter.y());  // Сдвигаем в центр
    transform.rotate(qRadiansToDegrees(deltaAngle));  // Поворачиваем на вычисленный угол
    transform.translate(-mCenter.x(), -mCenter.y());  // Возвращаем обратно

    // Применяем трансформацию ко всем точкам линии
    for (int i = 0; i < mBaseLineModel->size(); ++i) {
        double x = mBaseLineModel->at(i).first;
        double y = mBaseLineModel->at(i).second;
        QPointF rotatedPoint = transform.map(QPointF(x, y));  // Применяем трансформацию
        mBaseLineModel->replace(i, rotatedPoint.x(), rotatedPoint.y());  // Обновляем точку
    }
}

QPointF ChromatogramBaseline::calculateLineCenter() {
    double centerX = 0.0;
    double centerY = 0.0;

    // Вычисляем центр линии как среднее значение всех точек
    for (int i = 0; i < mBaseLineModel->size(); ++i) {
        centerX += mBaseLineModel->at(i).first;  // X координата точки
        centerY += mBaseLineModel->at(i).second; // Y координата точки
    }

    centerX /= mBaseLineModel->size();  // Находим среднее значение для X
    centerY /= mBaseLineModel->size();  // Находим среднее значение для Y

    return QPointF(centerX, centerY);  // Возвращаем центр линии как точку
}

void ChromatogramBaseline::setBaseLineModel(BaseLineDataModelPtr blineModel)
{
  if(mBaseLineModel)
    disconnect(mBaseLineModel.get(), nullptr, this, nullptr);
  mBaseLineModel = blineModel;
  if(mBaseLineModel)
  {
    connect(mBaseLineModel.get(), &BaseLineDataModel::onChanged, this, [this](const QVector<double> &keys, const QVector<double> &values)
    {
      setData(keys, values);
      if(mParentPlot)
        mParentPlot->replotOnVisibleViewPort(GraphicsPlot::rpQueuedReplot);
      emit moved();
    });
    auto lineData = mBaseLineModel->getBaseLine();
    setData(lineData.first, lineData.second);
    if(mParentPlot)
      mParentPlot->replotOnVisibleViewPort(GraphicsPlot::rpQueuedReplot);
  }
  else
  {
    setData({},{});
  }
}

void ChromatogramBaseline::addPointToBaseLine(double key, double value)
{
  auto bline = mBaseLineModel->getBaseLine();
  auto keyIter = std::lower_bound(bline.first.begin(), bline.first.end(), key);
  auto index = std::distance(bline.first.begin(), keyIter);
  auto valueIter = bline.second.begin() + index;
  bline.first.insert(keyIter, key);
  bline.second.insert(valueIter, value);
  auto cmd = new ChangeBaseLineCommand(mBaseLineModel,
                                       mBaseLineModel->getBaseLine(),
                                       bline);
  emit newCommand(cmd);
}

void ChromatogramBaseline::removePointFromBaseLine(double key)
{
  auto bline = mBaseLineModel->getBaseLine();
  auto keyIter = std::lower_bound(bline.first.begin(), bline.first.end(), key);
  if(*keyIter != key)
    return;
  auto index = std::distance(bline.first.begin(), keyIter);
  if(index == 0 || index >= bline.first.size() -1)//not first and not last points
    return;
  auto valueIter = bline.second.begin() + index;
  bline.first.erase(keyIter);
  bline.second.erase(valueIter);
  auto cmd = new ChangeBaseLineCommand(mBaseLineModel,
                                       mBaseLineModel->getBaseLine(),
                                       bline);
  emit newCommand(cmd);
}
void ChromatogramBaseline::removePointsFromBaseLine(QList<double> keys)
{
  auto bline = mBaseLineModel->getBaseLine();
  for(auto& key : keys)
  {
    auto keyIter = std::lower_bound(bline.first.begin(), bline.first.end(), key);
    if(*keyIter != key)
      return;
    auto index = std::distance(bline.first.begin(), keyIter);
    auto valueIter = bline.second.begin() + index;
    bline.first.erase(keyIter);
    bline.second.erase(valueIter);
  }
  auto cmd = new ChangeBaseLineCommand(mBaseLineModel,
                                       mBaseLineModel->getBaseLine(),
                                       bline);
  emit newCommand(cmd);
}

void ChromatogramBaseline::removeSelectedPointsFromBaseLine()
{
  auto bline = mBaseLineModel->getBaseLine();
  auto dataRanges = mSelection.dataRanges();
  bool hasChanges = false;
  for(auto it = dataRanges.rbegin(); it != dataRanges.rend(); it++)
  {
    int begin = it->begin();
    if(it->begin() == 0)//not first point
      begin++;
    int end = it->end();
    if(end >= bline.first.size()-1)//not last point
      end = bline.first.size()-2;
    if(begin > end || begin < 0 || end >= bline.first.size())
      continue;
    int count = end - begin + 1;
    bline.first.remove(begin, count);
    bline.second.remove(begin, count);
    if(!hasChanges && count > 0)
      hasChanges = true;
  }
  if(hasChanges)
  {
    mSelection.clear();
    auto cmd = new ChangeBaseLineCommand(mBaseLineModel,
                                         mBaseLineModel->getBaseLine(),
                                         bline);
    emit newCommand(cmd);
  }
}

QPair<double, GPCurveDataContainer::const_iterator> ChromatogramBaseline::closestPoint(QPoint pixelPos)
{
  auto distSqr = [this](GPCurveDataContainer::const_iterator it, const QPoint pt)->double
  {
    return GPVector2D(coordsToPixels(it->key, it->value) - pt).lengthSquared();
  };
  GPCurveDataContainer::const_iterator begin = mDataContainer->constBegin();
  GPCurveDataContainer::const_iterator end = mDataContainer->constEnd();
  double minDistSqr = distSqr(begin, pixelPos);
  GPCurveDataContainer::const_iterator closestData = begin;
  for (GPCurveDataContainer::const_iterator it = begin; it != end; ++it)
  {
    const double currentDistSqr = distSqr(it, pixelPos);
    if (currentDistSqr < minDistSqr)
    {
      minDistSqr = currentDistSqr;
      closestData = it;
    }
  }
  return {minDistSqr, closestData};
}

Qt::CursorShape ChromatogramBaseline::HitTest(QMouseEvent *event, unsigned *part)
{
  if (part)
    *part = HitPartNone;
  GenesisMarkup::ChromatogrammModes mMode = (GenesisMarkup::ChromatogrammModes)mParentPlot->model()->getChromatogramValue(ChromatogrammMode).toInt();
  if(mMode != GenesisMarkup::CMMarkup)
  {
    return Qt::ArrowCursor;
  }

  if (!mInteractive)
    return Qt::ArrowCursor;

  auto data = GetDataContainer();
  GPCurveDataContainer::const_iterator closestDataPoint = data->constEnd();
  pointDistance(event->pos(), closestDataPoint);
  if (closestDataPoint != data->constEnd())
  {
    double pointDistanceSqr = GPVector2D(coordsToPixels(closestDataPoint->key, closestDataPoint->value) - event->pos()).lengthSquared();
    if (pointDistanceSqr < mParentPlot->selectionTolerance() * 0.99 * mParentPlot->selectionTolerance() * 0.99)
    {
      if (part)
        *part = HitPartDataPoint;
      return Qt::SizeAllCursor;
    }
  }
  return Qt::ArrowCursor;
}

void ChromatogramBaseline::DragUpdateData(QMouseEvent *event)
{
  if(auto plot = qobject_cast<ChromatogramPlot*>(parentPlot()))
  {
    bool baseLineInteractionsAllowed = false;
    auto interactions = plot->stepInteractions();
    if(plot->isMaster())
      baseLineInteractionsAllowed = interactions.masterInteractions.testFlag(SIMBaseLinePointInteractions);
    else
      baseLineInteractionsAllowed = interactions.slaveInteractions.testFlag(SISBaseLinePointInteractions);

    if(baseLineInteractionsAllowed)
    {
      auto scopeGuard = QScopeGuard([&] { emit moved(); });

      if (DraggingDataPoint != -1 && DraggingData.size() == 1)
      {
        if (auto it = mDataContainer->findBegin(DraggingDataPoint, false);
            it != mDataContainer->end())
        {
          const auto dragStart = pixelsToCoords(DraggingStartPos);
          const auto drag = pixelsToCoords(event->pos());

          bool moveX = false;
          bool moveY = false;
          int baseInd = 0;
          if (const double dx = drag.x() - dragStart.x();
              fabs(dx) > std::numeric_limits<double>::epsilon()
              && (it->t != mDataContainer->first()->t && it->t != mDataContainer->last()->t))
          {
            baseInd = std::distance(mDataContainer->begin(), it);
            it->key = qBound((it - 1)->key, drag.x(), (it + 1)->key);
            moveX = true;
          }
          if (const double dy = drag.y() - dragStart.y();
              fabs(dy) > std::numeric_limits<double>::epsilon())
          {
            baseInd = std::distance(mDataContainer->begin(), it);
            it->value = drag.y();
            moveY = true;
          }
          auto selectedRanges = mSelection.dataRanges();
          for(auto rangeIt = selectedRanges.begin(); rangeIt != selectedRanges.end(); rangeIt++)
          {
            int leftBoundInd = rangeIt->begin() - 1;
            int rightBoundInd = rangeIt->end();
            if(leftBoundInd < 0)
              leftBoundInd = 0;
            if(rightBoundInd >= mDataContainer->size())
              rightBoundInd = mDataContainer->size()-1;
            double leftBound = mDataContainer->at(leftBoundInd)->key;
            double rightBound = mDataContainer->at(rightBoundInd)->key;
            for(int i = rangeIt->begin(); i < rangeIt->end(); i++)
            {
              if(i < 0 || i >= mDataContainer->size() || i == baseInd)
                continue;
              if(i == 0 || i == mDataContainer->size()-1)
              {
                moveX = false;
              }
              auto startKey = mBaseLineBeforeDragging.first[i];
              auto startValue = mBaseLineBeforeDragging.second[i];
              auto newKey = startKey;
              auto newValue = startValue;
              if(moveX)
              {
                newKey += drag.x() - dragStart.x();
                newKey = qBound(leftBound, newKey, rightBound);
              }
              if(moveY)
                newValue += drag.y() - dragStart.y();
              auto pointIt = mDataContainer->atNonConst(i);
              if(pointIt != mDataContainer->end())
              {
                pointIt->key = newKey;
                pointIt->value = newValue;
              }
            }
          }
        }
        mParentPlot->GraphicsPlot::replot(GraphicsPlot::RefreshPriority::rpQueuedRefresh);
      }
    }
  }
}

void ChromatogramBaseline::DragAccept(QMouseEvent *e)
{
  mBaseLineAfterDragging = {{},{}};
  for(auto iter = data()->constBegin(); iter != data()->constEnd(); iter++)
  {
    mBaseLineAfterDragging.first << iter->mainKey();
    mBaseLineAfterDragging.second << iter->mainValue();
  }
  GPInteractiveCurve::DragAccept(e);

  auto rollback = [this]()
  {
    setData(mBaseLineBeforeDragging.first, mBaseLineBeforeDragging.second);
    if(auto plot = qobject_cast<ChromatogramPlot*>(parentPlot()))
      plot->replotOnVisibleViewPort(GraphicsPlot::rpQueuedReplot);
  };

  if(!mBaseLineModel && mUid.isNull())
  {
    rollback();
    return;
  }
  ChangeBaseLineCommand* cmd = new ChangeBaseLineCommand(mBaseLineModel, mBaseLineBeforeDragging, mBaseLineAfterDragging);
  emit newCommand(cmd);
}


void ChromatogramBaseline::DragStart(QMouseEvent *e, unsigned part)
{
  mBaseLineBeforeDragging = {{},{}};
  for(auto iter = data()->constBegin(); iter != data()->constEnd(); iter++)
  {
    mBaseLineBeforeDragging.first << iter->mainKey();
    mBaseLineBeforeDragging.second << iter->mainValue();
  }
  GPInteractiveCurve::DragStart(e, part);
}

void ChromatogramBaseline::DragStart(QMouseEvent *event, GPCurveData *dataPoint)
{
  mBaseLineBeforeDragging = {{},{}};
  for(auto iter = data()->constBegin(); iter != data()->constEnd(); iter++)
  {
    mBaseLineBeforeDragging.first << iter->mainKey();
    mBaseLineBeforeDragging.second << iter->mainValue();
  }

  GPInteractiveCurve::DragStart(event, dataPoint);
}

}//namespace GenesisMarkup
