#include "plot_widget.h"

#include <logic/models/pls_fit_response_model.h>
#include <genesis_style/style.h>

namespace Widgets
{
  PlotWidget::PlotWidget(QWidget* parent)
    : QWidget(parent)
  {
    SetupUi();
  }

  void PlotWidget::SetModel(IdentityModelPtr model)
  {
    if (!ObjectModel)
    {
      ObjectModel = QSharedPointer<ObjectWithModel>::create(this);
    }
    ObjectModel->SetModel(model);
  }

  std::optional<Component> PlotWidget::FindComponentByIndex(int index) const
  {
    const auto model = ObjectModel->GetModel();
    assert(model);
    const auto& plsComponent = FindPlsComponentByIndex(index);
    const auto varianceX = model->Get<DoubleVector>(PlsFitResponseModel::VarianceX);
    assert(varianceX.size() > index - 1);
    if (plsComponent.has_value())
    {
      return Component
      {
        varianceX[index - 1],
        { plsComponent->P.begin(), plsComponent->P.end() },
        { plsComponent->T.begin(), plsComponent->T.end() }
      };
    }
    else
      return std::nullopt;
  }

  std::optional<PlsComponent> PlotWidget::FindPlsComponentByIndex(int index) const
  {
    const auto model = ObjectModel->GetModel();

    index = index > 0 ? index : 1;
    const auto& plsComponents = model->Get<PlsComponents>(PlsFitResponseModel::PlsComponents);

    const auto plsComponent = std::find_if(plsComponents.begin(), plsComponents.end(),
      [index](const PlsComponent& comp)
      {
        return comp.Number == static_cast<size_t>(index);
      });

    if (plsComponent == plsComponents.end())
      return std::nullopt;

    return *plsComponent;
  }

  GPRange PlotWidget::CreateRangeFromVector(const QVector<double>& values, double eps)
  {
    if (values.isEmpty())
      return {};

    return
    {
      *std::min_element(values.begin(), values.end()) - eps,
      *std::max_element(values.begin(), values.end()) + eps
    };
  }

  QString PlotWidget::GenerateAxisLabel(int number, double explPCsVariance) const
  {
    QLocale locale(QLocale::Russian);
    const auto model = ObjectModel->GetModel();
    assert(model);
    number = number > 0 ? number : 1;
    const auto varianceY = model->Get<DoubleVector>(PlsFitResponseModel::VarianceYGrad);
    return tr("PC%1\ (%2\% X, %3\% Y)")
      .arg(number)
      .arg(locale.toString(explPCsVariance, 'f', 2))
      .arg(locale.toString(varianceY[number - 1], 'f', 2));
  }

  GPShapeWithLabelItem* PlotWidget::CreateCircle(GraphicsPlot* plot,
    const QPointF& point, const QString& title,
    GPShapeItem::DefaultColor color, QSize size)
  {
    const auto circleShape = new GPShapeWithLabelItem(
      plot, point, GPShape::Circle, size, title);
    circleShape->setMargins(5, 5, 5, 5);
    circleShape->setColor(color);
    circleShape->setShapePen(Qt::NoPen);
    circleShape->setSelectable(false);
    circleShape->setShapeSelectedPen(Qt::NoPen);
    auto brush = QBrush(GPShapeItem::ToColor(GPShapeItem::DefaultColor(0)));
    circleShape->setShapeSelectedBrush(brush);
    circleShape->setPlatePen(Qt::NoPen);
    circleShape->setPlateSelectedPen(Qt::NoPen);
    circleShape->setPlateBrush(Qt::NoBrush);
    circleShape->setPlateSelectedBrush(QColor(86, 185, 242, 51)); //color from figma

    return circleShape;
  }

  GPItemStraightLine* PlotWidget::CreateLine(GraphicsPlot* plot,
    int width, const QPointF& p1, const QPointF& p2, GPShapeItem::DefaultColor color)
  {
    const auto line = new GPItemStraightLine(plot);
    line->point1->setCoords(p1);
    line->point2->setCoords(p2);
    QPen pen;
    pen.setColor(GPShapeItem::ToColor(color));
    pen.setWidth(width);
    line->setPen(pen);
    return line;
  }

  void PlotWidget::AppendCurve(GraphicsPlot *plot,
                               const QVector<double>& xCoords, const QVector<double>& yCoords,
                               GPShapeItem::DefaultColor color, int width, GPCurve::LineStyle style, const QString& curveName)
  {
    AppendCurve(plot, xCoords, yCoords, GPShapeItem::ToColor(color), width, style, curveName);
  }

  void PlotWidget::AppendCurve(GraphicsPlot* plot,
    const QVector<double>& xCoords,
    const QVector<double>& yCoords,
    QColor color, int width, GPCurve::LineStyle style,
    const QString& curveName)
  {
    const auto curve = plot->addCurve(plot->xAxis, plot->yAxis);
    auto pen = *curve->getPen();
    pen.setWidth(width);
    pen.setColor(color);
    curve->setPen(pen);
    curve->setLineStyle(style);
    curve->addData(xCoords, yCoords);
    if (!curveName.isEmpty())
      curve->setName(curveName);
  }

  void PlotWidget::SetupUi()
  {
    ObjectModel = QSharedPointer<ObjectWithModel>::create(this);

    setStyleSheet(Style::Genesis::GetUiStyle());
    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
  }

  // bool PlotWidget::eventFilter(QObject* watched, QEvent* event)
  // {
  //   if (auto plot = qobject_cast<GraphicsPlot*>(watched))
  //   {
  //     if (event->type() == QEvent::Wheel)
  //     {
  //       auto e = (QWheelEvent*)(event);
  //       if (!e->modifiers().testFlag(Qt::ControlModifier))
  //       {
  //         e->ignore();
  //         auto p = parent();
  //         while (p)
  //         {
  //           if (auto sa = qobject_cast<QScrollArea*>(p))
  //           {
  //             auto ne = new QWheelEvent(e->position(), e->globalPosition(), e->pixelDelta(), e->angleDelta(), e->buttons(), e->modifiers(), e->phase(), e->inverted(), e->source(), e->pointingDevice());
  //             QCoreApplication::sendEvent(sa->verticalScrollBar(), ne);
  //             break;
  //           }
  //           p = p->parent();
  //         }
  //         return true;
  //       }
  //     }
  //   }
  //   return false;
  // }

  // void PlotWidget::childEvent(QChildEvent* event)
  // {
  //   if (event->type() == QChildEvent::ChildAdded)
  //   {
  //     //because in this moment the child is not fully constructed
  //     //and we can't cast it to desired GraphicsPlot* now, we
  //     //should do it sometimes later
  //     if (newChildren.isEmpty())
  //     {
  //       //until the event loop turns, this will not be called
  //       QTimer::singleShot(0, this, [this]()
  //         {
  //           for (auto child : newChildren)
  //           {
  //             if (auto plot = qobject_cast<GraphicsPlot*>(child))
  //             {
  //               plot->installEventFilter(this);
  //             }
  //           }
  //           newChildren.clear();
  //         });
  //     }
  //     newChildren << event->child();
  //   }
  // }
}
