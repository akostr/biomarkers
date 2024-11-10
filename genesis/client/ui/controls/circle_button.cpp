#include "circle_button.h"
#include "ui/genesis_window.h"
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QTime>
#include <QGraphicsDropShadowEffect>

CircleButton::CircleButton(QWidget *parent)
  : QWidget{parent},
  mCollapseAnimation(nullptr),
  mHighlightAnimation(nullptr),
  mPressAnimation(nullptr),
  mShadow(nullptr),
  mHighlighted(false),
  mInHighlightTransition(false),
  mIsCollapsed(false),
  mIsOnCollapseTransition(false)
{
  setAttribute(Qt::WA_TranslucentBackground);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto ap = AnimationsParameters();
  {
    auto c = ap.initialBackgroundColor.toHsv();
    ap.highlightedBackgroundColor = QColor::fromHsvF(c.hueF(),
                                                   c.saturationF(),
                                                   c.valueF() * 0.95);
  }
  initializeAnimations();
  setupAnimations(ap);
  recalcMask();
  setPixmap(QPixmap(":/resource/controls/collpse_lt.png"));
  mShadow = new QGraphicsDropShadowEffect(this);
  mShadow->setOffset(0,5);
  mShadow->setBlurRadius(20);
  setGraphicsEffect(mShadow);
  connect(GenesisWindow::Get(), &GenesisWindow::windowMouseMove, this, &CircleButton::onWindowMouseMove);
}

void CircleButton::mousePressEvent(QMouseEvent *)
{
  mPressAnimation->setDirection(QAnimationGroup::Forward);
  mPressAnimation->start();
}

void CircleButton::mouseReleaseEvent(QMouseEvent *event)
{
  {//click detection strictly inside button circle
    QRectF r(rect());
    QPointF d = QPointF(event->pos()) - r.center();
    //assuming that one call to sqrt will be more expensive than this
    qreal distSq = pow(d.x(), 2) + pow(d.y(), 2);
    qreal rSq = pow(mBorderPenWidth, 2) * 0.25 + pow(r.width(), 2) * 0.25;
    if(distSq < rSq)
    {
      emit clicked();
      if(mIsCheckable)
      {
        setIsChecked(!IsChecked());
      }
    }
  }

  mPressAnimation->setDirection(QAnimationGroup::Backward);
  mPressAnimation->start();
}

void CircleButton::setupAnimations(const AnimationsParameters& parameters)
{
  Q_ASSERT(!mAnimations.isEmpty());
  if(mAnimations.isEmpty())
    return;

  auto radiusAnimation = (QPropertyAnimation*)mAnimations["radius"];
  auto scaleBorderAnimation = (QPropertyAnimation*)mAnimations["borderScale"];
  auto borderColorAnimation = (QPropertyAnimation*)mAnimations["borderColor"];
  auto backgroundColorAnimation = (QPropertyAnimation*)mAnimations["backgroundColor"];
  auto pixmapShiftAnimation = (QPropertyAnimation*)mAnimations["pixmapShift"];

  const auto& p = parameters;
  //collapse animation
  radiusAnimation->setStartValue(p.expandedRadius);
  radiusAnimation->setEndValue(p.collapsedRadius);
  radiusAnimation->setDuration(p.collapseAnimationDurationMs);
  scaleBorderAnimation->setStartValue(p.expandedBorderWidth);
  scaleBorderAnimation->setEndValue(p.collapsedBorderWidth);
  scaleBorderAnimation->setDuration(p.collapseAnimationDurationMs);
  mCollapseAnimation->stop();
  mCollapseAnimation->setDirection(QAbstractAnimation::Forward);
  setRadius(p.expandedRadius);
  mCollapseDetectionRadius = mRadius;
  setBorderPenWidth(p.expandedBorderWidth);
  //highlight animation
  borderColorAnimation->setStartValue(p.initialBorderColor);
  borderColorAnimation->setEndValue(p.highlightedBorderColor);
  borderColorAnimation->setDuration(p.highlightAnimationDurationMs);
  backgroundColorAnimation->setStartValue(p.initialBackgroundColor);
  backgroundColorAnimation->setEndValue(p.highlightedBackgroundColor);
  backgroundColorAnimation->setDuration(p.highlightAnimationDurationMs);
  mHighlightAnimation->stop();
  mHighlightAnimation->setDirection(QAbstractAnimation::Forward);
  setBorderPenColor(p.initialBorderColor);
  setBackgroundColor(p.initialBackgroundColor);
  //click animation
  pixmapShiftAnimation->setStartValue(p.initialPixmapOffset);
  pixmapShiftAnimation->setEndValue(p.clickedPixmapOffset);
  pixmapShiftAnimation->setDuration(p.clickAnimationDurationMs);
  mPressAnimation->stop();
  mPressAnimation->setDirection(QAbstractAnimation::Forward);
  setPixmapShift(p.initialPixmapOffset);
}

void CircleButton::setCenterOn(QPoint pos)
{
  mCenter = pos;
  auto geom = geometry();
  geom.moveCenter(mCenter.toPoint());
  setGeometry(geom);
}

qreal CircleButton::radius() const
{
  return mRadius;
}

void CircleButton::setRadius(qreal newRadius)
{
  if (qFuzzyCompare(mRadius, newRadius))
    return;
  mRadius = newRadius;
  recalcMask();
  auto r = geometry();
  r.setSize(getActualSize());
  r.moveCenter(mCenter.toPoint());
  setGeometry(r);
  update();
  emit radiusChanged();
}

void CircleButton::setDetectionRadius(double radius)
{
  mCollapseDetectionRadius = radius;
}

bool CircleButton::IsCheckable() const
{
  return mIsCheckable;
}

void CircleButton::setCheckable(bool newIsCheckable)
{
  if (mIsCheckable == newIsCheckable)
    return;
  mIsCheckable = newIsCheckable;

  emit IsCheckableChanged();
}

bool CircleButton::IsChecked() const
{
  return mIsChecked;
}

void CircleButton::setIsChecked(bool newIsChecked)
{
  if(mIsChecked != newIsChecked)
  {
    mIsChecked = newIsChecked;
    setPixmap(mIsChecked ? QPixmap(":/resource/controls/collpse_rt.png") : QPixmap(":/resource/controls/collpse_lt.png"));
    emit toggled(mIsChecked);
    update();
  }
}

bool CircleButton::IsOnCollapseTransition() const
{
  return mIsOnCollapseTransition;
}

QPointF CircleButton::pixmapShift() const
{
  return mPixmapShift;
}

void CircleButton::setPixmapShift(QPointF newPixmapShift)
{
  if (mPixmapShift == newPixmapShift)
    return;
  mPixmapShift = newPixmapShift;
  update();
  emit pixmapShiftChanged();
}

int CircleButton::collapseDurationMs() const
{
  return mCollapseDurationMs;
}

void CircleButton::setCollapseDurationMs(int newCollapseDurationMs)
{
  if (mCollapseDurationMs == newCollapseDurationMs)
    return;
  mCollapseDurationMs = newCollapseDurationMs;
  mAnimations["radius"]->setDuration(mCollapseDurationMs);
  mAnimations["borderScale"]->setDuration(mCollapseDurationMs);

  emit collapseDurationChanged();
}

void CircleButton::enterEvent(QEnterEvent *)
{
  mHighlightAnimation->setDirection(QAnimationGroup::Forward);
  mHighlightAnimation->start();
}

void CircleButton::leaveEvent(QEvent *)
{
  mHighlightAnimation->setDirection(QAnimationGroup::Backward);
  mHighlightAnimation->start();
}

void CircleButton::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  auto r = QRectF(rect());
  //  {//debug purpose
  //    p.fillRect(r, QColor(0,0,0,0));
  //    p.setPen(Qt::red);
  //    p.setBrush(QBrush(Qt::black, Qt::SolidPattern));
  //    p.drawRect(r);
  //    QLine lh(QPoint(r.left(), r.center().y()), QPoint(r.right(), r.center().y()));
  //    QLine lv(QPoint(r.center().x(), r.top()), QPoint(r.center().x(), r.bottom()));
  //    p.drawLine(lh);
  //    p.drawLine(lv);
  //  }

  p.setBrush(QBrush(mBackgroundColor));
  if(mBorderPenWidth != 0)
    p.setPen(QPen(mBorderPenColor, mBorderPenWidth));
  else
    p.setPen(Qt::NoPen);
  QVector2D c(r.center());
  int offset = 1;
  qreal halfBorderW = (mBorderPenWidth / 2.0);
  QRectF circleRect(QPointF(offset, offset), QSize((mRadius - halfBorderW - offset) * 2, (mRadius - halfBorderW - offset) * 2));
  p.drawEllipse(circleRect);
  if(!mPixmap.isNull())
  {
    QRectF pixmapDstRect(mPixmap.rect());
    pixmapDstRect.moveCenter(circleRect.center());
    pixmapDstRect.translate(mPixmapShift);
    p.drawPixmap(pixmapDstRect, mPixmap, mPixmap.rect());
  }
  //  {//debug purpose
  //    p.setPen(Qt::red);
  //    p.setBrush(Qt::NoBrush);
  //    QLine lh(QPoint(r.left(), circleRect.center().y()), QPoint(r.right(), circleRect.center().y()));
  //    QLine lv(QPoint(circleRect.center().x(), r.top()), QPoint(circleRect.center().x(), r.bottom()));
  //    p.drawLine(lh);
  //    p.drawLine(lv);
  //    p.drawEllipse(circleRect.center(), mRadius, mRadius);
  //  }
  //  {//debug purpose
  //    p.setPen(Qt::red);
  //    p.setBrush(Qt::NoBrush);
  //    p.drawRect(this->rect());
  //  }

}

void CircleButton::recalcMask()
{
  QBitmap mask(QSize(mRadius * 2, mRadius * 2));
  mask.fill(Qt::color0);
  if(!qFuzzyCompare(mRadius, 0))
  {
    QPainter p(&mask);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::color1, mBorderPenWidth));
    p.setBrush(Qt::color1);
    p.drawEllipse(mask.rect());
  }
  setMask(mask);
}

void CircleButton::initializeAnimations()
{
  if(!mCollapseAnimation)
  {
    mCollapseAnimation = new QParallelAnimationGroup(this);
    connect(mCollapseAnimation, &QParallelAnimationGroup::finished, this, [this]()
            {
              if(mCollapseAnimation->direction() == QAnimationGroup::Forward)
              {
                setVisible(false);
                mIsCollapsed = true;
              }
            });
    connect(mCollapseAnimation, &QParallelAnimationGroup::stateChanged, this,
            [this](QAbstractAnimation::State newState, QAbstractAnimation::State)
            {
              if(newState == QAbstractAnimation::Running &&
                  mCollapseAnimation->direction() == QAbstractAnimation::Backward)
              {
                setVisible(true);
                mIsCollapsed = false;
              }
              if(newState == QAbstractAnimation::Running)
                mIsOnCollapseTransition = true;
              else
                mIsOnCollapseTransition = false;
            });
  }
  if(!mHighlightAnimation)
  {
    mHighlightAnimation = new QParallelAnimationGroup(this);
    connect(mHighlightAnimation, &QParallelAnimationGroup::finished, this, [this]()
            {
              if(mHighlightAnimation->direction() == QAnimationGroup::Forward)
                mHighlighted = true;
              else
                mHighlighted = false;
            });
    connect(mHighlightAnimation, &QParallelAnimationGroup::stateChanged, this,
            [this](QAbstractAnimation::State newState, QAbstractAnimation::State)
            {
              if(newState == QAbstractAnimation::Running)
                mInHighlightTransition = true;
              else
                mInHighlightTransition = false;
            });
  }
  if(!mPressAnimation)
  {
    mPressAnimation = new QParallelAnimationGroup(this);
  }

  if(!mAnimations.contains("radius"))
  {
    auto radiusAnimation = new QPropertyAnimation(mCollapseAnimation);
    radiusAnimation->setPropertyName("radius");
    radiusAnimation->setTargetObject(this);
    radiusAnimation->setEasingCurve(QEasingCurve::InCubic);
    mAnimations["radius"] = radiusAnimation;
    connect(radiusAnimation, &QPropertyAnimation::directionChanged, radiusAnimation,
            [radiusAnimation](QAbstractAnimation::Direction dir)
            {
              radiusAnimation->setEasingCurve(dir == QAbstractAnimation::Forward
                                                ? QEasingCurve::InCubic
                                                : QEasingCurve::InElastic);
            });
  }
  if(!mAnimations.contains("borderScale"))
  {
    auto scaleBorderAnimation = new QPropertyAnimation(mCollapseAnimation);
    scaleBorderAnimation->setPropertyName("borderPenWidth");
    scaleBorderAnimation->setTargetObject(this);
    scaleBorderAnimation->setEasingCurve(QEasingCurve::InCubic);
    mAnimations["borderScale"] = scaleBorderAnimation;
    connect(scaleBorderAnimation, &QPropertyAnimation::directionChanged, scaleBorderAnimation,
            [scaleBorderAnimation](QAbstractAnimation::Direction dir)
            {
              scaleBorderAnimation->setEasingCurve(dir == QAbstractAnimation::Forward
                                                     ? QEasingCurve::InCubic
                                                     : QEasingCurve::InElastic);
            });
  }
  if(!mAnimations.contains("borderColor"))
  {
    auto borderColorAnimation = new QPropertyAnimation(mHighlightAnimation);
    borderColorAnimation->setPropertyName("borderPenColor");
    borderColorAnimation->setTargetObject(this);
    mAnimations["borderColor"] = borderColorAnimation;
  }
  if(!mAnimations.contains("backgroundColor"))
  {
    auto backgroundColorAnimation = new QPropertyAnimation(mHighlightAnimation);
    backgroundColorAnimation->setPropertyName("backgroundColor");
    backgroundColorAnimation->setTargetObject(this);
    mAnimations["backgroundColor"] = backgroundColorAnimation;
  }
  if(!mAnimations.contains("pixmapShift"))
  {
    auto pixmapShiftAnimation = new QPropertyAnimation(mPressAnimation);
    pixmapShiftAnimation->setPropertyName("pixmapShift");
    pixmapShiftAnimation->setTargetObject(this);
    mAnimations["pixmapShift"] = pixmapShiftAnimation;
  }
}

void CircleButton::onWindowMouseMove(QPointF pos)
{
  QWidget* parentWgt(nullptr);
  if(parent()->isWidgetType())
    parentWgt = (QWidget*)parent();
  if(!parentWgt || parentWgt->isVisible())
  {
    auto mappedPos = mapFrom(GenesisWindow::Get(), pos);
    auto manhDistToCollapseBtn = (mappedPos - rect().center()).manhattanLength();
    if(isCollapsed() && manhDistToCollapseBtn < mCollapseDetectionRadius)
      expand();
    else if(!isCollapsed() && manhDistToCollapseBtn >= mCollapseDetectionRadius)
      collapse();
  }
}

QSize CircleButton::getActualSize() const
{
  int r = (mRadius) * 2;
  return QSize(r, r);
}

void CircleButton::collapse()
{
  mCollapseAnimation->setDirection(QAnimationGroup::Forward);
  if(!mIsOnCollapseTransition)
    mCollapseAnimation->start();
}

void CircleButton::expand()
{
  mCollapseAnimation->setDirection(QAnimationGroup::Backward);
  if(!mIsOnCollapseTransition)
    mCollapseAnimation->start();
}

bool CircleButton::isCollapsed()
{
  return mIsCollapsed;
}

void CircleButton::toggleCollapsing()
{
  if(!mIsOnCollapseTransition)
  {
    if(mIsCollapsed)
      expand();
    else
      collapse();
  }
  else
  {
    mCollapseAnimation->setDirection(mCollapseAnimation->direction() == QAbstractAnimation::Forward
                                       ? QAbstractAnimation::Backward
                                       : QAbstractAnimation::Forward);
  }
}

QPixmap CircleButton::pixmap() const
{
  return mPixmap;
}

void CircleButton::setPixmap(const QPixmap &newPixmap)
{
  mPixmap = newPixmap;
  update();
  emit pixmapChanged();
}

QColor CircleButton::backgroundColor() const
{
  return mBackgroundColor;
}

void CircleButton::setBackgroundColor(const QColor &newBackgroundColor)
{
  if (mBackgroundColor == newBackgroundColor)
    return;
  mBackgroundColor = newBackgroundColor;
  update();
  emit backgroundColorChanged();
}

QColor CircleButton::borderPenColor() const
{
  return mBorderPenColor;
}

void CircleButton::setBorderPenColor(const QColor &newBorderPenColor)
{
  if (mBorderPenColor == newBorderPenColor)
    return;
  mBorderPenColor = newBorderPenColor;
  update();
  emit borderPenColorChanged();
}

qreal CircleButton::borderPenWidth() const
{
  return mBorderPenWidth;
}

void CircleButton::setBorderPenWidth(qreal newBorderPenWidth)
{
  if (qFuzzyCompare(mBorderPenWidth, newBorderPenWidth))
    return;
  mBorderPenWidth = newBorderPenWidth;
  update();
  emit borderPenWidthChanged();
}

QSize CircleButton::sizeHint() const
{
  return getActualSize();
}

QSize CircleButton::minimumSizeHint() const
{
  return sizeHint();
}
