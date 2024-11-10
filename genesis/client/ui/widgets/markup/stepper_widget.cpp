#include "stepper_widget.h"
#include "qevent.h"
#include "logic/markup/json_serializable.cpp"
#include "qpainter.h"
#include <genesis_colors.h>
#include <genesis_style/style.h>

namespace GenesisMarkup
{
StepperWidget::StepperWidget(QWidget *parent)
  : QWidget{parent},
    mFont(Style::GetSASSValue("fontFaceNormal"),
          -1,
          Style::GetSASSValue("fontWeight").toInt()),
    mFilledBrush            (Colors::Markup::stepperFilledBrush),
    mCompletedBrush         (Colors::Markup::stepperCompletedBrush),
    mEmptyBrush             (Colors::Markup::stepperEmptyBrush),
    mHighlightedFilledBrush (Colors::Markup::stepperHighlightedFilledBrush),
    mHighlightedEmptyBrush  (Colors::Markup::stepperHighlightedEmptyBrush),
    mEmptyTextPen           (Colors::Markup::stepperEmptyTextPen),
    mFilledTextPen          (Colors::Markup::stepperFilledTextPen),
    mLabelFilledTextPen     (Colors::Markup::stepperLabelFilledTextPen),
    mLabelEmptyTextPen      (Colors::Markup::stepperLabelEmptyTextPen),
    mBorderPen(Qt::NoPen),
    mCircleRadius(10),
    mMargin(10),
    mLineWidth(2),
    mCurrentStep(-1),
    mHighlightedPosition(-1),
    mTextMaxHeight(0),
    mTextMinWidth(0)
{
  mFont.setPixelSize(Style::GetSASSValue("fontSizeSecondaryTextScalableFont").toInt());
  setSteps({});
  setMouseTracking(true);
  //Prevent devide by 0 at StepperWidget::itemAt(const QPoint &pos)
  mStepWidth = 1;
}

void StepperWidget::setSteps(const StepsList &steps)
{
  mSteps = steps;
  if(mSteps.isEmpty())
  {
    recalcStaticText();
    update();
    return;
  }
  if(mSteps.size() != 0)
    mStepWidth = geometry().width() / mSteps.size();
  else
    mStepWidth = geometry().width();
  mStepWidth = (width() - mMargin * 2) / mSteps.size();
  mCenters.clear();
  QPointF centerLinePos(mMargin + mCircleRadius, mMargin + mCircleRadius);
  for(int i = 0; i < mSteps.size(); i++)
  {
    mCenters << centerLinePos;
    centerLinePos.setX(centerLinePos.x() + mStepWidth);
  }
  recalcStaticText();
  update();
}

void StepperWidget::setFont(const QFont &font)
{
  mFont = font;
  update();
}

void StepperWidget::setFilledBrush(const QBrush &brush)
{
  mFilledBrush = brush;
  update();
}

void StepperWidget::setEmptyBrush(const QBrush &brush)
{
  mEmptyBrush = brush;
  update();
}

void StepperWidget::setHighlightedFilledBrush(const QBrush &brush)
{
  mHighlightedFilledBrush = brush;
  update();
}

void StepperWidget::setHighlightedEmptyBrush(const QBrush &brush)
{
  mHighlightedEmptyBrush = brush;
  update();
}

void StepperWidget::setEmptyTextPen(const QPen &pen)
{
  mEmptyTextPen = pen;
  update();
}

void StepperWidget::setFilledTextPen(const QPen &pen)
{
  mFilledTextPen = pen;
  update();
}

void StepperWidget::setLabelFilledTextPen(const QPen &pen)
{
  mLabelFilledTextPen = pen;
  update();
}

void StepperWidget::setLabelEmptyTextPen(const QPen &pen)
{
  mLabelEmptyTextPen = pen;
  update();
}

void StepperWidget::setBorderPen(const QPen &pen)
{
  mBorderPen = pen;
  update();
}

void StepperWidget::setCircleRadius(int pixelRadius)
{
  mCircleRadius = pixelRadius;
  update();
}

void StepperWidget::setMargin(int margin)
{
  mMargin = margin;
  update();
}

void StepperWidget::stepForward()
{
  if(mCurrentStep + 1 <= mSteps.size() - 1)
  {
    mCurrentStep++;
    emit stepChanged(mCurrentStep < 0 ? Step0DataLoad : std::get<2>(mSteps[mCurrentStep]));
    update();
  }
}

void StepperWidget::setStep(int stepIndex)
{
  if(mCurrentStep == stepIndex)
    return;
  mCurrentStep = stepIndex;
  emit stepChanged(mCurrentStep < 0 ? Step0DataLoad : std::get<2>(mSteps[mCurrentStep]));
  update();
}

QSize StepperWidget::sizeHint() const
{
  double width = std::max<int>(mMargin * 2 + mTextMinWidth, mMargin * 2 + mCircleRadius * 2 * mSteps.size());
  double height = mMargin * 2 + mCircleRadius * 2 + mTextMaxHeight;
  return QSize(width + 0.5, height + 0.5);
}

QSize StepperWidget::minimumSizeHint() const
{
  double width = mMargin * 2 + mCircleRadius * 2 * mSteps.size();
  double height = mMargin * 2 + mCircleRadius * 2 + mTextMaxHeight;
  return QSize(width + 0.5, height + 0.5);
}

void StepperWidget::mouseReleaseEvent(QMouseEvent *event)
{
  int found = -1;
  for(int i = 0; i < mSteps.size(); i++)
  {
    if((event->pos() - mCenters[i]).manhattanLength() < mCircleRadius * 2)
    {
      found = i;
      break;
    }
  }
  if(found != -1)
    setStep(found);
}

void StepperWidget::mouseMoveEvent(QMouseEvent *event)
{
  int found = -1;
  for(int i = 0; i < mSteps.size(); i++)
  {
    if((event->pos() - mCenters[i]).manhattanLength() < mCircleRadius * 2)
    {
      found = i;
      break;
    }
  }
  if(mHighlightedPosition != found)
  {
    mHighlightedPosition = found;
    update();
  }
}

QGradient StepperWidget::resolveLineGradient(const QRect &line, const QColor &leftColor, const QColor &rightColor)
{
    QLinearGradient gradientLeftRight= QLinearGradient(line.left(), line.bottom(), line.right(), line.top());
    gradientLeftRight.setSpread(QGradient::RepeatSpread);
    gradientLeftRight.setColorAt(0.0, leftColor);
    gradientLeftRight.setColorAt(1.0, rightColor);
    return gradientLeftRight;
}

QString StepperWidget::itemAt(const QPoint &pos)
{
  if(pos.y() > mCircleRadius * 2)
  {
    int x = pos.x() / (int) mStepWidth;
    mToolTipIndex = x;
    return mTooltips.value(x);
  }
  return QString();
}

void StepperWidget::paintEvent(QPaintEvent *event)
{
  if(mSteps.empty())
    return;
  QPainter painter(this);
  QRectF rect = event->rect();
  painter.setPen(mBorderPen);
  rect.setWidth(rect.width() - mMargin * 2);
  rect.setHeight(rect.height() - mMargin * 2);
  double stepWidth = rect.width() / mSteps.size();
  painter.translate(QPoint(mMargin, mMargin));
  painter.save();

  //graphics
  for(int i = 0; i < mSteps.size(); i++)
  {
    painter.save();
    painter.setBrush(resolveCircleBrushColor(i));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawEllipse(QRect(0,0,mCircleRadius * 2, mCircleRadius * 2));
    painter.restore();
    auto& number = mCircleNumbers[i];
    painter.save();
    painter.setPen(resolveCircleTextPenColor(i));
    painter.drawStaticText(QPoint(mCircleRadius - (int)((double)number.size().width() / 2.0 + 0.5),
                                  mCircleRadius - (int)((double)number.size().height() / 2.0 + 0.5)),
                           number);
    painter.restore();
    painter.save();
    QRect lineRect(QPoint(mCircleRadius * 2, mCircleRadius - mLineWidth / 2), QSize(stepWidth, mLineWidth));
    if(i < mSteps.size() - 1)
      painter.setBrush(resolveLineGradient(lineRect, resolveCircleBrushColor(i), resolveCircleBrushColor(i + 1)));
      painter.drawRect(lineRect);
    painter.restore();
    painter.translate(QPoint(stepWidth, 0));
  }
  painter.restore();
  painter.translate(QPoint(0, mCircleRadius * 2));
  painter.save();
  painter.setPen(mLabelFilledTextPen);
  //labels
  for(int i = 0; i < mLabels.size(); i++)
  {
    auto& label = mLabels[i];
    painter.drawStaticText(QPoint(0,0), label);
    painter.translate(QPoint(stepWidth, 0));
    if(i == mCurrentStep)
      painter.setPen(mLabelEmptyTextPen);
  }
  painter.restore();
}

void StepperWidget::resizeEvent(QResizeEvent *event)
{
  if(mSteps.isEmpty())
  {
    QWidget::resizeEvent(event);
    return;
  }
  mStepWidth = (event->size().width() - mMargin * 2) / mSteps.size();
  mCenters.clear();
  QPointF centerLinePos(mMargin + mCircleRadius, mMargin + mCircleRadius);
  for(int i = 0; i < mSteps.size(); i++)
  {
    mCenters << centerLinePos;
    centerLinePos.setX(centerLinePos.x() + mStepWidth);
  }
  recalcStaticText();
}

bool StepperWidget::event(QEvent *event)
{
  if (event->type() == QEvent::ToolTip)
  {
      QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

      QString tooltip = itemAt(helpEvent->pos());
      if (!tooltip.isEmpty())
      {
        mCurrentToolTip = tooltip;
        QToolTip::showText(helpEvent->globalPos(), tooltip);
      }
      else
      {
          QToolTip::hideText();
          event->ignore();
      }

      return true;
  }

  if(event->type() == QEvent::MouseMove)
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    QString tooltip = itemAt(mouseEvent->pos());
    if(tooltip != mCurrentToolTip)
    {
      QToolTip::hideText();
    }
  }

  return QWidget::event(event);
}

void StepperWidget::setLineWidth(int newLineWidth)
{
  mLineWidth = newLineWidth;
  update();
}

void StepperWidget::recalcStaticText()
{
  mLabels.clear();
  mTextMaxHeight = 0;
  mTextMinWidth = 0;
  for(int i = 0; i < mSteps.size(); i++)
  {
    auto& label = mLabels.emplaceBack(std::get<1>(mSteps[i]));
//    label.setTextFormat(Qt::RichText);
    QTextOption op = label.textOption();
    op.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    label.setTextOption(op);
    label.setTextWidth(mStepWidth);
    label.prepare(QTransform(), mFont);
    if(label.size().height() > mTextMaxHeight)
      mTextMaxHeight = label.size().height();
    mTextMinWidth += label.size().width();
  }
//  qDebug() << "height: " << mTextMaxHeight << "; width: " << mTextMinWidth;
  mCircleNumbers.clear();
  mTooltips.clear();
  for(int i = 0; i < mSteps.size(); i++)
  {
    auto number = mCircleNumbers.emplaceBack(QString::number(std::get<0>(mSteps[i])));
    number.setTextWidth(mCircleRadius * 2);
    number.prepare(QTransform(), mFont);

    auto& tooltip = mTooltips.emplaceBack(std::get<3>(mSteps[i]));
  }
  updateGeometry();
}

QColor StepperWidget::resolveCircleBrushColor(int ind)
{
  if(mCurrentStep < 0)
  {//no step selected
    if(mHighlightedPosition < 0)//no step highlited
      return mEmptyBrush.color();
    else if(mHighlightedPosition >= ind)//some step on right highlighted
      return mHighlightedEmptyBrush.color();
    else//some step on left highlighted
      return mEmptyBrush.color();
  }
  else//step selected
  {
    if(mHighlightedPosition < 0)//no step highlighted
    {
      if(mCurrentStep == ind)//current step selected
        return mFilledBrush.color();
      else if(mCurrentStep > ind)//some step on right selected
        return mCompletedBrush.color();
      else //some step on left selected
        return mEmptyBrush.color();
    }
    else if(mCurrentStep < mHighlightedPosition)//highlighted step on right of selected
    {
      if(mCurrentStep == ind)//current step selected
      {
        return mFilledBrush.color();
      }
      else if(mCurrentStep > ind)//some step on right selected
      {
        return mCompletedBrush.color();
      }
      else //(mCurrentStep < ind) some step on left selected
      {
        if(mHighlightedPosition < ind)//some step on left highlighted
          return mEmptyBrush.color();
        else//some step on right highlighted
          return mHighlightedEmptyBrush.color();
      }
    }
    else//highlighted step on left of selected
    {
      if(mCurrentStep == ind)//current step selected
      {
        if(mHighlightedPosition < ind)//some step on left highlighted
          return mHighlightedFilledBrush.color();
        else //(mHighlightedPosition >= ind) some step on right highlighted
          return mFilledBrush.color();
      }
      else if(mCurrentStep > ind)//some step on right selected
      {
        if(mHighlightedPosition < ind)//some step on left highlighted
          return mHighlightedFilledBrush.color();
        else //(mHighlightedPosition >= ind) some step on right highlighted
          return mCompletedBrush.color();
      }
      else //(mCurrentStep < ind) some step on left selected
      {
        return mEmptyBrush.color();
      }
    }
  }
}
/*
//not needed for gradient lines
//different from Circle Color Resolving by comparison symbols (< || <=)
QColor StepperWidget::resolveLineBrushColor(int ind)
{
  if(mCurrentStep < 0)
  {//no step selected
    if(mHighlightedPosition < 0)//no step highlited
      return mEmptyBrush.color();
    else if(mHighlightedPosition > ind)//some step on right highlighted
      return mHighlightedEmptyBrush.color();
    else//some step on left highlighted
      return mEmptyBrush.color();
  }
  else//step selected
  {
    if(mHighlightedPosition < 0)//no step highlighted
    {
      if(mCurrentStep > ind)//some step on right selected
        return mFilledBrush.color();
      else //some step on left selected
        return mEmptyBrush.color();
    }
    else if(mCurrentStep <= mHighlightedPosition)//highlighted step on right of selected
    {
      if(mCurrentStep > ind)//some step on right selected
      {
        return mFilledBrush.color();
      }
      else //(mCurrentStep <= ind) some step on left selected
      {
        if(mHighlightedPosition <= ind)//some step on left highlighted
          return mEmptyBrush.color();
        else//some step on right highlighted
          return mHighlightedEmptyBrush.color();
      }
    }
    else//highlighted step on left of selected
    {
      if(mCurrentStep > ind)//some step on right selected
      {
        if(mHighlightedPosition <= ind)//some step on left highlighted
          return mHighlightedFilledBrush.color();
        else //(mHighlightedPosition > ind) some step on right highlighted
          return mFilledBrush.color();
      }
      else //(mCurrentStep <= ind) some step on left selected
      {
        return mEmptyBrush.color();
      }
    }
  }
}
*/
QColor StepperWidget::resolveCircleTextPenColor(int ind)
{
  if(mCurrentStep < 0)
    return mEmptyTextPen.color();
  else if(mCurrentStep >= ind)
    return mFilledTextPen.color();
  else //if(mCurrentStep < ind)
    return mEmptyTextPen.color();
}
}//namespace GenesisMarkup
