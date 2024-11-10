#include "range_slider.h"

#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QStyleOptionSlider>
#include <QApplication>
#include <QStylePainter>
#include <QStyle>
#include <QToolTip>

/////////////////////////////////////////////////////////////
//// Obscured impl
class RangeSliderPrivate
{
  Q_DECLARE_PUBLIC(RangeSlider);

protected:
  RangeSlider* const q_ptr;

public:
  enum Handle 
  {
    NoHandle = 0x0,
    MinimumHandle = 0x1,
    MaximumHandle = 0x2
  };

  Q_DECLARE_FLAGS(Handles, Handle);

  explicit RangeSliderPrivate(RangeSlider& object);
  void init();

  Handle handleAtPos(const QPoint& pos, QRect& handleRect) const;

  int pixelPosToRangeValue(int pos) const;
  int pixelPosFromRangeValue(int val) const;

  void drawMinimumSlider(QStylePainter* painter) const;
  void drawMaximumSlider(QStylePainter* painter) const;
    
  int MaximumValue;
  int MinimumValue;

  int MaximumPosition;
  int MinimumPosition;

  QStyle::SubControl MinimumSliderSelected;
  QStyle::SubControl MaximumSliderSelected;

  int SubclassClickOffset;
    
  int SubclassPosition;
  
  float SubclassWidth;

  RangeSliderPrivate::Handles SelectedHandles;

  bool SymmetricMoves;

  QString HandleToolTip;

private:
  Q_DISABLE_COPY(RangeSliderPrivate);
};

// 
RangeSliderPrivate::RangeSliderPrivate(RangeSlider& object)
  : q_ptr(&object)
{
  MinimumValue          = 0;
  MaximumValue          = 100;
  MinimumPosition       = 0;
  MaximumPosition       = 100;
  MinimumSliderSelected = QStyle::SC_None;
  MaximumSliderSelected = QStyle::SC_None;
  SubclassClickOffset   = 0;
  SubclassPosition      = 0;
  SubclassWidth         = 0.0;
  SelectedHandles       = NoHandle;
  SymmetricMoves        = false;
}

// 
void RangeSliderPrivate::init()
{
  Q_Q(RangeSlider);
  MinimumValue = q->minimum();
  MaximumValue = q->maximum();
  MinimumPosition = q->minimum();
  MaximumPosition = q->maximum();
  q->connect(q, SIGNAL(rangeChanged(int,int)), q, SLOT(OnRangeChanged(int,int)));
}

// 
RangeSliderPrivate::Handle RangeSliderPrivate::handleAtPos(const QPoint& pos, QRect& handleRect) const
{
  Q_Q(const RangeSlider);

  QStyleOptionSlider option;
  q->initStyleOption(&option);

  // Test the MinimumHandle
  option.sliderPosition = MinimumPosition;
  option.sliderValue    = MinimumValue;

  QStyle::SubControl minimumControl = q->style()->hitTestComplexControl(QStyle::CC_Slider, &option, pos, q);
  QRect minimumHandleRect = q->style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, q);

  // Test if the pos is under the Maximum handle 
  option.sliderPosition = MaximumPosition;
  option.sliderValue    = MaximumValue;

  QStyle::SubControl maximumControl = q->style()->hitTestComplexControl(QStyle::CC_Slider, &option, pos, q);
  QRect maximumHandleRect = q->style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, q);

  // The pos is above both handles, select the closest handle
  if (minimumControl == QStyle::SC_SliderHandle &&
      maximumControl == QStyle::SC_SliderHandle)
  {
    int minDist = 0;
    int maxDist = 0;
    if (q->orientation() == Qt::Horizontal)
      {
        minDist = pos.x() - minimumHandleRect.left();
        maxDist = maximumHandleRect.right() - pos.x(); 
      }
    else // if (q->orientation() == Qt::Vertical)
      {
        minDist = minimumHandleRect.bottom() - pos.y();
        maxDist = pos.y() - maximumHandleRect.top(); 
      }
    Q_ASSERT(minDist >= 0 && maxDist >= 0);
    minimumControl = minDist < maxDist ? minimumControl : QStyle::SC_None;
  }

  if (minimumControl == QStyle::SC_SliderHandle)
  {
    handleRect = minimumHandleRect;
    return MinimumHandle;
  }
  if (maximumControl == QStyle::SC_SliderHandle)
  {
    handleRect = maximumHandleRect;
    return MaximumHandle;
  }
  handleRect = minimumHandleRect.united(maximumHandleRect);
  return NoHandle;
}

// Map pixel to value 
int RangeSliderPrivate::pixelPosToRangeValue(int pos) const
{
  Q_Q(const RangeSlider);
  QStyleOptionSlider option;
  q->initStyleOption(&option);

  QRect gr = q->style()->subControlRect(QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderGroove, 
                                            q);
  QRect sr = q->style()->subControlRect(QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderHandle, 
                                            q);
  int sliderMin, sliderMax, sliderLength;
  if (option.orientation == Qt::Horizontal) 
  {
    sliderLength = sr.width();
    sliderMin = gr.x();
    sliderMax = gr.right() - sliderLength + 1;
  }
  else
  {
    sliderLength = sr.height();
    sliderMin = gr.y();
    sliderMax = gr.bottom() - sliderLength + 1;
  }

  return QStyle::sliderValueFromPosition(q->minimum(), 
                                         q->maximum(), 
                                         pos - sliderMin,
                                         sliderMax - sliderMin, 
                                         option.upsideDown);
}

// Map value to pixel
int RangeSliderPrivate::pixelPosFromRangeValue(int val) const
{
  Q_Q(const RangeSlider);

  QStyleOptionSlider option;
  q->initStyleOption(&option);

  QRect gr = q->style()->subControlRect(QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderGroove, 
                                            q);
  QRect sr = q->style()->subControlRect(QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderHandle, 
                                            q);
  int sliderMin, sliderMax, sliderLength;
  if (option.orientation == Qt::Horizontal) 
  {
    sliderLength = sr.width();
    sliderMin = gr.x();
    sliderMax = gr.right() - sliderLength + 1;
  }
  else
  {
    sliderLength = sr.height();
    sliderMin = gr.y();
    sliderMax = gr.bottom() - sliderLength + 1;
  }

  return QStyle::sliderPositionFromValue(q->minimum(), 
                                          q->maximum(), 
                                          val,
                                          sliderMax - sliderMin, 
                                          option.upsideDown) + sliderMin;
}

// Draw at the bottom end of the range
void RangeSliderPrivate::drawMinimumSlider(QStylePainter* painter) const
{
  Q_Q(const RangeSlider);

  QStyleOptionSlider option;
  q->initMinimumSliderStyleOption(&option);

  option.subControls = QStyle::SC_SliderHandle;
  option.sliderValue = MinimumValue;
  option.sliderPosition = MinimumPosition;
  if (q->isMinimumSliderDown())
  {
    option.activeSubControls = QStyle::SC_SliderHandle;
    option.state |= QStyle::State_Sunken;
  }
  painter->drawComplexControl(QStyle::CC_Slider, option);
}

// Draw at the top end of the range
void RangeSliderPrivate::drawMaximumSlider(QStylePainter* painter) const
{
  Q_Q(const RangeSlider);

  QStyleOptionSlider option;
  q->initMaximumSliderStyleOption(&option);

  option.subControls = QStyle::SC_SliderHandle;
  option.sliderValue = MaximumValue;
  option.sliderPosition = MaximumPosition;
  if (q->isMaximumSliderDown())
  {
    option.activeSubControls = QStyle::SC_SliderHandle;
    option.state |= QStyle::State_Sunken;
  }
  painter->drawComplexControl(QStyle::CC_Slider, option);
}

/////////////////////////////////////////////////////////////////
// Public impl
RangeSlider::RangeSlider(QWidget* _parent)
  : QSlider(_parent)
  , d_ptr(new RangeSliderPrivate(*this))
{
  Q_D(RangeSlider);

  d->init();
}

// Ctor w/ orientation
RangeSlider::RangeSlider(Qt::Orientation o, QWidget* parentObject)
  : QSlider(o, parentObject)
  , d_ptr(new RangeSliderPrivate(*this))
{
  Q_D(RangeSlider);

  d->init();
}

// Protected ctor
RangeSlider::RangeSlider(RangeSliderPrivate* impl, QWidget* _parent)
  : QSlider(_parent)
  , d_ptr(impl)
{
  Q_D(RangeSlider);

  d->init();
}

// Protected ctor w/ orientation
RangeSlider::RangeSlider(RangeSliderPrivate* impl, Qt::Orientation o, QWidget* parentObject)
  : QSlider(o, parentObject)
  , d_ptr(impl)
{
  Q_D(RangeSlider);

  d->init();
}

// Dtor
RangeSlider::~RangeSlider()
{
}

///////////////////////////////////////////////////////////////////////////////////////
// Properties
int RangeSlider::minimumValue() const
{
  Q_D(const RangeSlider);

  return d->MinimumValue;
}

void RangeSlider::setMinimumValue(int min)
{
  Q_D(RangeSlider);

  setValues(min, qMax(d->MaximumValue,min));
}

int RangeSlider::maximumValue() const
{
  Q_D(const RangeSlider);

  return d->MaximumValue;
}

void RangeSlider::setMaximumValue(int max)
{
  Q_D(RangeSlider);

  setValues(qMin(d->MinimumValue, max), max);
}

void RangeSlider::setValues(int l, int u)
{
  Q_D(RangeSlider);

  const int minValue = qBound(minimum(), qMin(l,u), maximum());
  const int maxValue = qBound(minimum(), qMax(l,u), maximum());
  bool emitMinValChanged = (minValue != d->MinimumValue);
  bool emitMaxValChanged = (maxValue != d->MaximumValue);
  
  d->MinimumValue = minValue;
  d->MaximumValue = maxValue;
  
  bool emitMinPosChanged = (minValue != d->MinimumPosition);
  bool emitMaxPosChanged = (maxValue != d->MaximumPosition);
  d->MinimumPosition = minValue;
  d->MaximumPosition = maxValue;
  
  if (isSliderDown())
  {
    if (emitMinPosChanged || emitMaxPosChanged)
    {
      emit positionsChanged(d->MinimumPosition, d->MaximumPosition);
    }
    if (emitMinPosChanged)
    {
      emit minimumPositionChanged(d->MinimumPosition);
    }
    if (emitMaxPosChanged)
    {
      emit maximumPositionChanged(d->MaximumPosition);
    }
  }
  if (emitMinValChanged || emitMaxValChanged)
  {
    emit valuesChanged(d->MinimumValue, d->MaximumValue);
  }
  if (emitMinValChanged)
  {
    emit minimumValueChanged(d->MinimumValue);
  }
  if (emitMaxValChanged)
  {
    emit maximumValueChanged(d->MaximumValue);
  }
  if (emitMinPosChanged || emitMaxPosChanged || 
      emitMinValChanged || emitMaxValChanged)
  {
    update();
  }
}

int RangeSlider::minimumPosition() const
{
  Q_D(const RangeSlider);

  return d->MinimumPosition;
}

int RangeSlider::maximumPosition() const
{
  Q_D(const RangeSlider);

  return d->MaximumPosition;
}

void RangeSlider::setMinimumPosition(int l)
{
  Q_D(const RangeSlider);

  setPositions(l, qMax(l, d->MaximumPosition));
}

void RangeSlider::setMaximumPosition(int u)
{
  Q_D(const RangeSlider);

  setPositions(qMin(d->MinimumPosition, u), u);
}

void RangeSlider::setPositions(int min, int max)
{
  Q_D(RangeSlider);

  const int minPosition = qBound(minimum(), qMin(min, max), maximum());
  const int maxPosition = qBound(minimum(), qMax(min, max), maximum());

  bool emitMinPosChanged = (minPosition != d->MinimumPosition);
  bool emitMaxPosChanged = (maxPosition != d->MaximumPosition);
  
  if (!emitMinPosChanged && !emitMaxPosChanged)
  {
    return;
  }

  d->MinimumPosition = minPosition;
  d->MaximumPosition = maxPosition;

  if (!hasTracking())
  {
    update();
  }

  if (isSliderDown())
  {
    if (emitMinPosChanged)
    {
      emit minimumPositionChanged(d->MinimumPosition);
    }
    if (emitMaxPosChanged)
    {
      emit maximumPositionChanged(d->MaximumPosition);
    }
    if (emitMinPosChanged || emitMaxPosChanged)
    {
      emit positionsChanged(d->MinimumPosition, d->MaximumPosition);
    }
  }
  if (hasTracking())
  {
    triggerAction(SliderMove);
    setValues(d->MinimumPosition, d->MaximumPosition);
  }
}

void RangeSlider::setSymmetricMoves(bool symmetry)
{
  Q_D(RangeSlider);

  d->SymmetricMoves = symmetry;
}

bool RangeSlider::symmetricMoves() const
{
  Q_D(const RangeSlider);

  return d->SymmetricMoves;
}

// Slot
void RangeSlider::OnRangeChanged(int _minimum, int _maximum)
{
  Q_UNUSED(_minimum);
  Q_UNUSED(_maximum);
  Q_D(RangeSlider);
  setValues(d->MinimumValue, d->MaximumValue);
}

///////////////////////////////////////////////////////////////////////////
// Events
void RangeSlider::paintEvent(QPaintEvent*)
{
  Q_D(RangeSlider);

  QStyleOptionSlider option;
  initStyleOption(&option);

  QStylePainter painter(this);
  option.subControls = QStyle::SC_SliderGroove;
  option.sliderValue = minimum() - maximum();
  option.sliderPosition = minimum() - maximum();
  painter.drawComplexControl(QStyle::CC_Slider, option);

  option.sliderPosition = d->MinimumPosition;
  const QRect lr = style()->subControlRect(QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderHandle, 
                                            this);
  option.sliderPosition = d->MaximumPosition;

  const QRect ur = style()->subControlRect(QStyle::CC_Slider, 
                                            &option, 
                                            QStyle::SC_SliderHandle, 
                                            this);

  QRect sr = style()->subControlRect(QStyle::CC_Slider, 
                                      &option, 
                                      QStyle::SC_SliderGroove, 
                                      this);
  QRect rangeBox;
  if (option.orientation == Qt::Horizontal)
  {
    rangeBox = QRect(QPoint(qMin(lr.center().x(), ur.center().x()), sr.center().y() - 2),
                     QPoint(qMax(lr.center().x(), ur.center().x()), sr.center().y() + 1));
  }
  else
  {
    rangeBox = QRect(QPoint(sr.center().x() - 2, qMin(lr.center().y(), ur.center().y())),
                     QPoint(sr.center().x() + 1, qMax(lr.center().y(), ur.center().y())));
  }

  QRect groove = style()->subControlRect(QStyle::CC_Slider, 
                                                &option, 
                                                QStyle::SC_SliderGroove, 
                                                this);
  groove.adjust(0, 0, -1, 0);

  QColor highlight = palette().color(QPalette::Normal, QPalette::Highlight);
  QLinearGradient gradient;
  if (option.orientation == Qt::Horizontal)
  {
    gradient = QLinearGradient(groove.center().x(), groove.top(),
                               groove.center().x(), groove.bottom());
  }
  else
  {
    gradient = QLinearGradient(groove.left(), groove.center().y(),
                               groove.right(), groove.center().y());
  }


  gradient.setColorAt(0, highlight.darker(120));
  gradient.setColorAt(1, highlight.lighter(160));

  painter.setPen(QPen(highlight.darker(150), 0));
  painter.setBrush(gradient);
  painter.drawRect(rangeBox.intersected(groove));

  if (isMinimumSliderDown())
  {
    painter.setClipRect(ur);
    d->drawMaximumSlider(&painter);
    painter.setClipRect(lr);
    d->drawMinimumSlider(&painter);
  }
  else
  {
    painter.setClipRect(lr);
    d->drawMinimumSlider(&painter);
    painter.setClipRect(ur);
    d->drawMaximumSlider(&painter);
  }
}

// Mouse press
void RangeSlider::mousePressEvent(QMouseEvent* mouseEvent)
{
  Q_D(RangeSlider);

  if (minimum() == maximum() || (mouseEvent->buttons() ^ mouseEvent->button()))
  {
    mouseEvent->ignore();
    return;
  }
  int mepos = orientation() == Qt::Horizontal ? mouseEvent->pos().x() : mouseEvent->pos().y();

  QStyleOptionSlider option;
  initStyleOption(&option);

  QRect handleRect;
  RangeSliderPrivate::Handle handle_ = d->handleAtPos(mouseEvent->pos(), handleRect);

  if (handle_ != RangeSliderPrivate::NoHandle)
  {
    d->SubclassPosition = (handle_ == RangeSliderPrivate::MinimumHandle) ? d->MinimumPosition : d->MaximumPosition;

    // save the position of the mouse inside the handle 
    d->SubclassClickOffset = mepos - (orientation() == Qt::Horizontal ? handleRect.left() : handleRect.top());

    setSliderDown(true);

    if (d->SelectedHandles != handle_)
    {
      d->SelectedHandles = handle_;
      update(handleRect);
    }

    // Accepted
    mouseEvent->accept();
    return;
  }

  // no handles 
  QStyle::SubControl control = style()->hitTestComplexControl(QStyle::CC_Slider, &option, mouseEvent->pos(), this);
  int minCenter = (orientation() == Qt::Horizontal ? handleRect.left() : handleRect.top());
  int maxCenter = (orientation() == Qt::Horizontal ? handleRect.right() : handleRect.bottom());
  if (control == QStyle::SC_SliderGroove && mepos > minCenter && mepos < maxCenter)
  {
    // warning lost of precision it might be fatal
    d->SubclassPosition = (d->MinimumPosition + d->MaximumPosition) / 2.;
    d->SubclassClickOffset = mepos - d->pixelPosFromRangeValue(d->SubclassPosition);
    d->SubclassWidth = (d->MaximumPosition - d->MinimumPosition) / 2.;
    
    qMax(d->SubclassPosition - d->MinimumPosition, d->MaximumPosition - d->SubclassPosition);
    setSliderDown(true);

    if (!isMinimumSliderDown() || !isMaximumSliderDown())
    {
      d->SelectedHandles = QFlags<RangeSliderPrivate::Handle>(RangeSliderPrivate::MinimumHandle) 
                         | QFlags<RangeSliderPrivate::Handle>(RangeSliderPrivate::MaximumHandle);
      QRect sr = style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderGroove, this);
      update(handleRect.united(sr));
    }
    mouseEvent->accept();
    return;
  }
  mouseEvent->ignore();
}

// Mouse move
void RangeSlider::mouseMoveEvent(QMouseEvent* mouseEvent)
{
  Q_D(RangeSlider);

  if (!d->SelectedHandles)
  {
    mouseEvent->ignore();
    return;
  }
  int mepos = orientation() == Qt::Horizontal ? mouseEvent->pos().x() : mouseEvent->pos().y();

  QStyleOptionSlider option;
  initStyleOption(&option);

  const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &option, this);
  int newPosition = d->pixelPosToRangeValue(mepos - d->SubclassClickOffset);

  if (m >= 0)
  {
    const QRect r = rect().adjusted(-m, -m, m, m);
    if (!r.contains(mouseEvent->pos()))
    {
      newPosition = d->SubclassPosition;
    }
  }

  // lower/left slider is down
  if (isMinimumSliderDown() && !isMaximumSliderDown())
  {
    double newMinPos = qMin(newPosition,d->MaximumPosition);
    setPositions(newMinPos, d->MaximumPosition +
      (d->SymmetricMoves ? d->MinimumPosition - newMinPos : 0));
  }
  // upper/right slider is down
  else if (isMaximumSliderDown() && !isMinimumSliderDown())
  {
    double newMaxPos = qMax(d->MinimumPosition, newPosition);
    setPositions(d->MinimumPosition -
      (d->SymmetricMoves ? newMaxPos - d->MaximumPosition: 0),
      newMaxPos);
  }
  // Both handles are down 
  else if (isMinimumSliderDown() && isMaximumSliderDown())
  {
    setPositions(newPosition - static_cast<int>(d->SubclassWidth),
                       newPosition + static_cast<int>(d->SubclassWidth + .5));
  }
  mouseEvent->accept();
}

//  Mouse release
void RangeSlider::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
  Q_D(RangeSlider);

  QSlider::mouseReleaseEvent(mouseEvent);

  setSliderDown(false);
  d->SelectedHandles = RangeSliderPrivate::NoHandle;

  update();
}

//////////////////////////////////////////////////////////////////////
// States
bool RangeSlider::isMinimumSliderDown() const
{
  Q_D(const RangeSlider);

  return d->SelectedHandles & RangeSliderPrivate::MinimumHandle;
}

bool RangeSlider::isMaximumSliderDown() const
{
  Q_D(const RangeSlider);

  return d->SelectedHandles & RangeSliderPrivate::MaximumHandle;
}

void RangeSlider::initMinimumSliderStyleOption(QStyleOptionSlider* option) const
{
  initStyleOption(option);
}

void RangeSlider::initMaximumSliderStyleOption(QStyleOptionSlider* option) const
{
  initStyleOption(option);
}

QString RangeSlider::handleToolTip() const
{
  Q_D(const RangeSlider);

  return d->HandleToolTip;
}

void RangeSlider::setHandleToolTip(const QString& _toolTip)
{
  Q_D(RangeSlider);

  d->HandleToolTip = _toolTip;
}

bool RangeSlider::event(QEvent* _event)
{
  Q_D(RangeSlider);

  switch(_event->type())
  {
  case QEvent::ToolTip:
    {
      QHelpEvent* helpEvent = static_cast<QHelpEvent*>(_event);
      QStyleOptionSlider opt;

      // MinimumHandle
      opt.sliderPosition = d->MinimumPosition;
      opt.sliderValue = d->MinimumValue;
      initStyleOption(&opt);
      QStyle::SubControl hoveredControl = style()->hitTestComplexControl(QStyle::CC_Slider, &opt, helpEvent->pos(), this);
      if (!d->HandleToolTip.isEmpty() &&
          hoveredControl == QStyle::SC_SliderHandle)
      {
        QToolTip::showText(helpEvent->globalPos(), d->HandleToolTip.arg(minimumValue()));
        _event->accept();
        return true;
      }

      // MaximumHandle
      opt.sliderPosition = d->MaximumPosition;
      opt.sliderValue = d->MaximumValue;
      initStyleOption(&opt);
      hoveredControl = style()->hitTestComplexControl(QStyle::CC_Slider, &opt, helpEvent->pos(), this);
      if (!d->HandleToolTip.isEmpty() && hoveredControl == QStyle::SC_SliderHandle)
      {
        QToolTip::showText(helpEvent->globalPos(), d->HandleToolTip.arg(maximumValue()));
        _event->accept();
        return true;
      }
    }
  default:
    break;
  }
  return QSlider::event(_event);
}
